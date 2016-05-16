/*
	libsmacker - A C library for decoding .smk Smacker Video files
	Copyright (C) 2012-2013 Greg Kennedy

	See smacker.h for more information.

	smacker.c
		Main implementation file of libsmacker.
		Open, close, query, render, advance and seek an smk
*/

#include "smacker.h"

/* safe malloc and free */
#include "smk_malloc.h"

/* data structures */
#include "smk_bitstream.h"
#include "smk_hufftree.h"

/* fp error handling */
#include <errno.h>
/* memset */
#include <string.h>

/* GLOBALS */
/* tree processing order */
#define SMK_TREE_MMAP	0
#define SMK_TREE_MCLR	1
#define SMK_TREE_FULL	2
#define SMK_TREE_TYPE	3

/* SMACKER DATA STRUCTURES */
struct smk_t
{
	/* meta-info */
	/* file mode: see flags, smacker.h */
	unsigned char	mode;

	/* microsec per frame, total frames
		stored as a double to handle scaling (large positive millisec / frame values may
		overflow a ul */
	double	usf;

	unsigned long	f;
	/* does file contain a ring frame? */
	unsigned char	ring_frame;

	/* Index of current frame */
	unsigned long	cur_frame;

	/* SOURCE union.
		Where the data is going to be read from (or be stored),
		depending on the file mode. */
	union
	{
		struct
		{
			/* on-disk mode */
			FILE	*fp;
			unsigned long	*chunk_offset;
		} file;

		/* in-memory mode: unprocessed chunks */
		unsigned char	**chunk_data;
	} source;

	/* shared
		array of "chunk sizes"*/
	unsigned long	*chunk_size;

	/* Holds per-frame flags (i.e. 'keyframe') */
	unsigned char	*keyframe;
	/* Holds per-frame type mask (e.g. 'audio track 3, 2, and palette swap') */
	unsigned char	*frame_type;

	/* pointers to video and audio structures */
	/* Video data type: enable/disable decode switch,
		video info and flags,
		pointer to last-decoded-palette */
	struct smk_video_t
	{
		/* enable/disable decode switch */
		unsigned char enable;

		/* video info */
		unsigned long	w;
		unsigned long	h;
		/* Y scale mode (constants defined in smacker.h)
			0: unscaled
			1: doubled
			2: interlaced */
		unsigned char	y_scale_mode;

		/* version ('2' or '4') */
		unsigned char	v;

		/* Huffman trees */
/*		unsigned long tree_size[4]; */
		struct smk_huff_big_t	*tree[4];

		/* Palette data type: pointer to last-decoded-palette */
		unsigned char *palette;
		/* Last-unpacked frame */
		unsigned char *frame;
	} *video;

	/* audio structure */
	struct smk_audio_t
	{
		/* enable/disable switch (per track) */
		unsigned char enable;

		/* Info */
		unsigned char	channels;
		unsigned char	bitdepth;
		unsigned long	rate;

/* 		long	max_buffer; */

		/* compression type
			0: raw PCM
			1: SMK DPCM
			2: Bink (Perceptual), unsupported */
		unsigned char	compress;

		/* pointer to last-decoded-audio-buffer */
		void	*buffer;
		unsigned long	buffer_size;
	} *audio[7];
};

union smk_read_t
{
	FILE *file;
	unsigned char *ram;
};

/* An fread wrapper: consumes N bytes, or returns -1
	on failure (when size doesn't match expected) */
static char smk_read_file(void *buf, const size_t size, FILE *fp)
{
	/* don't bother checking buf or fp, fread does it for us */
	size_t bytesRead = fread(buf,1,size,fp);
	if (bytesRead != size)
	{
		fprintf(stderr,"libsmacker::smk_read_file(buf,%lu,fp) - ERROR: Short read, %lu bytes returned (errno: %d)\n",(unsigned long)size, (unsigned long)bytesRead,errno);
		perror ("\tError reported was");
		return -1;
	}
	return 0;
}

/* A memcpy wrapper: consumes N bytes, or returns -1
	on failure (when size too low) */
static char smk_read_memory(void *buf, const unsigned long size, unsigned char **p, unsigned long *p_size)
{
	if (size > *p_size)
	{
		fprintf(stderr,"libsmacker::smk_read_memory(buf,%lu,p,%lu) - ERROR: Short read\n",(unsigned long)size, (unsigned long)*p_size);
		return -1;
	}
	memcpy(buf,*p,size);
	*p += size;
	*p_size -= size;
	return 0;
}

/* Helper functions to do the reading, plus
   byteswap from LE to host order */
/* read n bytes from (source) into ret */
#define smk_read(ret,n) \
{ \
	if(m) \
	{ \
		r = (smk_read_file(ret,n,fp.file)); \
	} \
	else \
	{ \
		r = (smk_read_memory(ret,n,&fp.ram,&size)); \
	} \
	if (r < 0) \
	{ \
		fprintf(stderr,"libsmacker::smk_read(...) - Errors encountered on read, bailing out (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}

/* Calls smk_read, but returns a ul */
#define smk_read_ul(p) \
{ \
	smk_read(buf,4); \
	p = ((unsigned long) buf[3] << 24) | \
		((unsigned long) buf[2] << 16) | \
		((unsigned long) buf[1] << 8) | \
		((unsigned long) buf[0]); \
}

/* PUBLIC FUNCTIONS */
/* open an smk (from a generic Source) */
smk smk_open_generic(const unsigned char m, union smk_read_t fp, unsigned long size, const unsigned char process_mode)
{
	smk s = NULL;

	/* Temporary variables */
	long temp_l;
	unsigned long temp_u;

	/* r is used by macros above for return code */
	char r;
	unsigned char buf[4] = {'\0'};

	/* video hufftrees are stored as a large chunk (bitstream)
		these vars are used to load, then decode them */
	unsigned char *hufftree_chunk = NULL;
	unsigned long tree_size;
	/* a bitstream struct */
	struct smk_bit_t *bs = NULL;

	/* safe malloc the structure */
	smk_malloc (s,sizeof (struct smk_t));
	/* safe malloc the structure components */
	smk_malloc(s->video,sizeof(struct smk_video_t));

	/* Check for a valid signature */
	smk_read(buf,3);
	if (buf[0] != 'S' || buf[1] != 'M' || buf[2] != 'K')
	{
		fprintf(stderr,"libsmacker::smk_open_generic - ERROR: invalid SMKn signature (got: %s)\n",buf);
		goto error;
	}

	/* Read .smk file version */
	smk_read(&s->video->v,1);
	if (s->video->v != '2' && s->video->v != '4')
	{
		fprintf(stderr,"libsmacker::smk_open_generic - Warning: invalid SMK version %c (expected: 2 or 4)\n",s->video->v);
		/* take a guess */
		if (s->video->v < '4')
		{
			s->video->v = '2';
		}
		else
		{
			s->video->v = '4';
		}
		fprintf(stderr,"\tProcessing will continue as type %c\n",s->video->v);
	}

	/* width, height, total num frames */
	smk_read_ul(s->video->w);
	smk_read_ul(s->video->h);

	smk_read_ul(s->f);

	/* frames per second calculation */
	smk_read_ul(temp_u);
	temp_l = (int)temp_u;
	if (temp_l > 0)
	{
		/* millisec per frame */
		s->usf = temp_l * 1000;
	}
	else if (temp_l < 0)
	{
		/* 10 microsec per frame */
		s->usf = temp_l * -10;
	}
	else
	{
		/* defaults to 10 usf (= 100000 microseconds) */
		s->usf = 100000;
	}

	/* Video flags follow.
		Ring frame is important to libsmacker.
		Y scale / Y interlace go in the Video flags.
		The user should scale appropriately. */
	smk_read_ul(temp_u);
	if (temp_u & 0x01)
	{
		s->ring_frame = 1;
	}
	if (temp_u & 0x02)
	{
		s->video->y_scale_mode = SMK_FLAG_Y_DOUBLE;
	}
	if (temp_u & 0x04)
	{
		if (s->video->y_scale_mode == SMK_FLAG_Y_DOUBLE)
		{
			fputs("libsmacker::smk_open_generic - Warning: SMK file specifies both Y-Double AND Y-Interlace.\n",stderr);
		}
		s->video->y_scale_mode = SMK_FLAG_Y_INTERLACE;
	}

	/* Max buffer size for each audio track - we don't use
		but calling application might. */
	for (temp_l = 0; temp_l < 7; temp_l ++)
	{
/*		smk_read_ul(s->audio[temp_u]->max_buffer); */
		smk_read_ul(temp_u);
	}

	/* Read size of "hufftree chunk" - save for later. */
	smk_read_ul(tree_size);

	/* "unpacked" sizes of each huff tree - we don't use
		but calling application might. */
	for (temp_l = 0; temp_l < 4; temp_l ++)
	{
/*		smk_read_ul(s->video->tree_size[temp_u]); */
		smk_read_ul(temp_u);
	}

	/* read audio rate data */
	for (temp_l = 0; temp_l < 7; temp_l ++)
	{
		smk_read_ul(temp_u);
		if (temp_u & (1 << 30))
		{
			/* Audio track specifies "exists" flag, malloc structure and copy components. */
			smk_malloc(s->audio[temp_l],sizeof(struct smk_audio_t));
			if (temp_u & (1 << 31))
			{
				s->audio[temp_l]->compress = 1;
			}
			s->audio[temp_l]->bitdepth = ((temp_u & (1 << 29)) ? 16 : 8);
			s->audio[temp_l]->channels = ((temp_u & (1 << 28)) ? 2 : 1);
			if (temp_u & (0x03 << 26))
			{
				fprintf(stderr,"libsmacker::smk_open_generic - Warning: audio track %ld is compressed with Bink (perceptual) Audio Codec: this is currently unsupported by libsmacker\n",temp_l);
				s->audio[temp_l]->compress = 2;
			}
			/* Bits 25 & 24 are unused. */
			s->audio[temp_l]->rate = (temp_u & 0x00FFFFFF);
		}
	}

	/* Skip over Dummy field */
	smk_read_ul(temp_u);

	/* FrameSizes and Keyframe marker are stored together. */
	smk_malloc(s->keyframe,(s->f + s->ring_frame));
	smk_malloc(s->chunk_size,(s->f + s->ring_frame) * sizeof(unsigned long));

	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u ++)
	{
		smk_read_ul(s->chunk_size[temp_u]);

		/* Set Keyframe */
		if (s->chunk_size[temp_u] & 0x01)
		{
			s->keyframe[temp_u] = 1;
		}
		/* Bits 1 is used, but the purpose is unknown. */
		s->chunk_size[temp_u] &= 0xFFFFFFFC;
	}

	/* That was easy... Now read FrameTypes! */
	smk_malloc(s->frame_type,(s->f + s->ring_frame));
	for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u ++)
	{
		smk_read(&s->frame_type[temp_u],1);
	}

	/* HuffmanTrees
		We know the sizes already: read and assemble into
		something actually parse-able at run-time */
	smk_malloc(hufftree_chunk,tree_size);
	smk_read(hufftree_chunk,tree_size);

	/* set up a Bitstream */
	bs = smk_bs_init(hufftree_chunk, tree_size);
	/* create some tables */
	for (temp_u = 0; temp_u < 4; temp_u ++)
	{
		smk_huff_big_safe_build(bs,s->video->tree[temp_u]);
	}

	/* clean up */
	smk_free(bs);
	smk_free(hufftree_chunk);

	/* final processing: depending on ProcessMode, handle what to do with rest of file data */
	s->mode = process_mode;

	/* Handle the rest of the data.
		For MODE_MEMORY, read the chunks and store */
	if (s->mode == SMK_MODE_MEMORY)
	{
		smk_malloc(s->source.chunk_data,(s->f + s->ring_frame) * sizeof(unsigned char*));
		for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u ++)
		{
			smk_malloc(s->source.chunk_data[temp_u],s->chunk_size[temp_u]);
			smk_read(s->source.chunk_data[temp_u],s->chunk_size[temp_u]);
		}
	}
	else
	{
		/* MODE_STREAM: don't read anything now, just precompute offsets.
			use fseek to verify that the file is "complete" */
		smk_malloc(s->source.file.chunk_offset,(s->f + s->ring_frame) * sizeof(unsigned long));
		for (temp_u = 0; temp_u < (s->f + s->ring_frame); temp_u ++)
		{
			s->source.file.chunk_offset[temp_u] = ftell(fp.file);
			if (fseek(fp.file,s->chunk_size[temp_u],SEEK_CUR))
			{
				fprintf(stderr,"libsmacker::smk_open - ERROR: fseek to frame %lu not OK.\n",temp_u);
				perror ("\tError reported was");
				goto error;
			}
		}
	}

	return s;

error:
	smk_free(bs);
	smk_free(hufftree_chunk);
	smk_close(s);
	return NULL;
}

/* open an smk (from a memory buffer) */
smk smk_open_memory(const unsigned char* buffer, const unsigned long size)
{
	smk s = NULL;

	union smk_read_t fp;

	smk_null_check(buffer);

	/* set up the read union for Memory mode */
	fp.ram = (unsigned char *)buffer;

	if (!(s = smk_open_generic(0,fp,size,SMK_MODE_MEMORY)))
	{
		fprintf(stderr,"libsmacker::smk_open_memory(buffer,%lu) - ERROR: Fatal error in smk_open_generic, returning NULL.\n",size);
	}

error:
	return s;
}

/* open an smk (from a file) */
smk smk_open_file(const char* filename, const unsigned char mode)
{
	smk s = NULL;

	union smk_read_t fp;

	smk_null_check(filename);

	if (!(fp.file = fopen(filename,"rb")))
	{
		fprintf(stderr,"libsmacker::smk_open_file(%s,%u) - ERROR: could not open file (errno: %d)\n",filename,mode,errno);
		perror ("\tError reported was");
		goto error;
	}

	if (!(s = smk_open_generic(1,fp,0,mode)))
	{
		fprintf(stderr,"libsmacker::smk_open_file(%s,%u) - ERROR: Fatal error in smk_open_generic, returning NULL.\n",filename,mode);
		fclose(fp.file);
		goto error;
	}

	if (mode == SMK_MODE_MEMORY)
	{
		fclose(fp.file);
	}
	else
	{
		s->source.file.fp = fp.file;
	}

error:
	return s;
}

/* close out an smk file and clean up memory */
void smk_close(smk s)
{
	unsigned long u;

	smk_null_check(s);

	/* free video sub-components */
	if (s->video)
	{
		for (u = 0; u < 4; u ++)
		{
			smk_huff_free(s->video->tree[u]->t);
			smk_free(s->video->tree[u]);
		}
		smk_free(s->video->palette);
		smk_free(s->video->frame);
		smk_free(s->video);
	}

	/* free audio sub-components */
	for (u=0; u<7; u++)
	{
		if (s->audio[u])
		{
			smk_free(s->audio[u]->buffer);
			smk_free(s->audio[u]);
		}
	}

	smk_free(s->keyframe);
	smk_free(s->frame_type);

	if (s->mode == SMK_MODE_DISK)
	{
		/* disk-mode */
		if (s->source.file.fp)
		{
			fclose(s->source.file.fp);
		}
		smk_free(s->source.file.chunk_offset);
	}
	else
	{
		/* mem-mode */
		if (s->source.chunk_data != NULL)
		{
			for (u=0; u<(s->f + s->ring_frame); u++)
			{
				smk_free(s->source.chunk_data[u]);
			}
			smk_free(s->source.chunk_data);
		}
	}
	smk_free(s->chunk_size);

	smk_free(s);

error: ;
}

/* tell some info about the file */
char smk_info_all(const smk object, unsigned long *frame, unsigned long *frame_count, double *usf)
{
	/* sanity check */
	smk_null_check(object);
	if (!frame && !frame_count && !usf)
	{
		fputs("libsmacker::smk_info_all(object,frame,frame_count,usf) - ERROR: Request for info with all-NULL return references\n",stderr);
		goto error;
	}
	if (frame)
	{
		*frame = (object->cur_frame % object->f);
	}
	if (frame_count)
	{
		*frame_count = object->f;
	}
	if (usf)
	{
		*usf = object->usf;
	}
	return 0;

error:
	return -1;
}

char smk_info_video(const smk object, unsigned long *w, unsigned long *h, unsigned char *y_scale_mode)
{
	/* sanity check */
	smk_null_check(object);
	if (!w && !h && !y_scale_mode)
	{
		fputs("libsmacker::smk_info_all(object,w,h,y_scale_mode) - ERROR: Request for info with all-NULL return references\n",stderr);
		return -1;
	}
	if (w)
	{
		*w = object->video->w;
	}
	if (h)
	{
		*h = object->video->h;
	}
	if (y_scale_mode)
	{
		*y_scale_mode = object->video->y_scale_mode;
	}
	return 0;

error:
	return -1;
}

char smk_info_audio(const smk object, unsigned char *track_mask, unsigned char channels[7], unsigned char bitdepth[7], unsigned long audio_rate[7])
{
	unsigned char i;

	/* sanity check */
	smk_null_check(object);

	if (!track_mask && !channels && !bitdepth && !audio_rate)
	{
		fputs("libsmacker::smk_info_audio(object,track_mask,channels,bitdepth,audio_rate) - ERROR: Request for info with all-NULL return references\n",stderr);
		return -1;
	}
	if (track_mask)
	{
		*track_mask = ( (object->audio[0] != NULL) |
			 ( (object->audio[1] != NULL) << 1 ) |
			 ( (object->audio[2] != NULL) << 2 ) |
			 ( (object->audio[3] != NULL) << 3 ) |
			 ( (object->audio[4] != NULL) << 4 ) |
			 ( (object->audio[5] != NULL) << 5 ) |
			 ( (object->audio[6] != NULL) << 6 ) );
	}
	if (channels)
	{
		for (i = 0; i < 7; i ++)
		{
			channels[i] = ( object->audio[i] ? object->audio[i]->channels : 0);
		}
	}
	if (bitdepth)
	{
		for (i = 0; i < 7; i ++)
		{
			bitdepth[i] = ( object->audio[i] ? object->audio[i]->bitdepth : 0);
		}
	}
	if (audio_rate)
	{
		for (i = 0; i < 7; i ++)
		{
			audio_rate[i] = ( object->audio[i] ? object->audio[i]->rate : 0);
		}
	}
	return 0;

error:
	return -1;
}

/* Enable-disable switches */
char smk_enable_all(smk object, const unsigned char mask)
{
	unsigned char i;

	/* sanity check */
	smk_null_check(object);
	smk_null_check(object->video);

	object->video->enable = (mask & 0x80);

	for (i = 0; i < 7; i ++)
	{
		if (object->audio[i])
		{
			object->audio[i]->enable = (mask & (0x01 << i));
		}
	}

	return 0;

error:
	return -1;
}

char smk_enable_video(smk object, const unsigned char enable)
{
	/* sanity check */
	smk_null_check(object);
	smk_null_check(object->video);

	object->video->enable = enable;
	return 0;

error:
	return -1;
}

char smk_enable_audio(smk object, const unsigned char track, const unsigned char enable)
{
	/* sanity check */
	smk_null_check(object);
	smk_null_check(object->audio[track]);

	object->audio[track]->enable = enable;
	return 0;

error:
	return -1;
}

unsigned char * smk_get_palette(const smk object)
{
	smk_null_check(object);
	smk_null_check(object->video);

	return object->video->palette;

error:
	return NULL;
}
unsigned char * smk_get_video(const smk object)
{
	smk_null_check(object);
	smk_null_check(object->video);

	return object->video->frame;

error:
	return NULL;
}
unsigned char * smk_get_audio(const smk object, const unsigned char t)
{
	smk_null_check(object);
	smk_null_check(object->audio[t]);

	return object->audio[t]->buffer;

error:
	return NULL;
}
unsigned long smk_get_audio_size(const smk object, const unsigned char t)
{
	smk_null_check(object);
	smk_null_check(object->audio[t]);

	return object->audio[t]->buffer_size;

error:
	return 0;
}

/* Decompresses a palette-frame. */
static char smk_render_palette(struct smk_video_t *s, unsigned char *p, unsigned long size)
{
	unsigned short i,j,k;
	unsigned char *t= NULL;

	/* Smacker palette map */
	const unsigned char palmap[64] =
	{
		0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C,
		0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C,
		0x41, 0x45, 0x49, 0x4D, 0x51, 0x55, 0x59, 0x5D,
		0x61, 0x65, 0x69, 0x6D, 0x71, 0x75, 0x79, 0x7D,
		0x82, 0x86, 0x8A, 0x8E, 0x92, 0x96, 0x9A, 0x9E,
		0xA2, 0xA6, 0xAA, 0xAE, 0xB2, 0xB6, 0xBA, 0xBE,
		0xC3, 0xC7, 0xCB, 0xCF, 0xD3, 0xD7, 0xDB, 0xDF,
		0xE3, 0xE7, 0xEB, 0xEF, 0xF3, 0xF7, 0xFB, 0xFF
	};

	/* sanity check */
	smk_null_check(s);
	smk_null_check(p);

	/* Allocate a placeholder for our palette. */
	smk_malloc(t,768);

	i = 0; /* index into NEW palette */
	j = 0; /* Index into OLD palette */

	while ( (i < 768) && (size > 0) ) /* looping index into NEW palette */
	{
		if ((*p) & 0x80)
		{
			/* Copy (c + 1) color entries of the previous palette
				to the next entries of the new palette. */
			k = (((*p) & 0x7F) + 1) * 3;
			p ++; size --;

			/* check for overflow condition */
			if (i + k > 768)
			{
				fprintf(stderr,"libsmacker::palette_render(s,p,size) - ERROR: overflow, 0x80 attempt to copy %d bytes from %d\n",k,i);
				goto error;
			}

			/* if prev palette exists, copy... else memset black */
			if (s->palette)
			{
				memcpy(&t[i],&s->palette[i],k);
			}
			/* smkmalloc already set t to 0 */
			/* else
			{
				memset(&t[i],0,k);
			} */
			i += k;
		}
		else if ((*p) & 0x40)
		{
			/* Copy (c + 1) color entries of the previous palette,
				starting from entry (s)
				to the next entries of the new palette. */
			if (size < 2)
			{
				fputs("libsmacker::palette_render(s,p,size) - ERROR: 0x40 ran out of bytes for copy\n",stderr);
				goto error;
			}

			/* pick "count" items to copy */
			k = (((*p) & 0x3F) + 1) * 3;  /* count */
			p ++; size --;

			/* start offset of old palette */
			j = (*p) * 3;
			p ++; size --;

			if (j + k > 768 || i + k > 768)
			{
				fprintf(stderr,"libsmacker::palette_render(s,p,size) - ERROR: overflow, 0x40 attempt to copy %d bytes from %d to %d\n",k,j,i);
				goto error;
			}

			if (s->palette)
			{
				memcpy(&t[i],&s->palette[j],k);
			}
			/* else
			{
				memset(&t[i],0,k);
			} */
			i += k;
			j += k;
		}
		else
		{
			if (size < 3)
			{
				fprintf(stderr,"libsmacker::palette_render - ERROR: 0x3F ran out of bytes for copy, size=%lu\n",size);
				goto error;
			}
			t[i++] = palmap[(*p) & 0x3F];
			p++; size --;
			/* To be extremely correct we should blow up if (*p) exceeds 0x3F, but
				I can't be bothered, so just mask it */
			t[i++] = palmap[(*p) & 0x3F];
			p++; size --;
			t[i++] = palmap[(*p) & 0x3F];
			p++; size --;
		}
	}

	if (i < 768)
	{
		fprintf(stderr,"libsmacker::palette_render - ERROR: did not completely fill palette (idx=%u)\n",i);
		goto error;
	}

	/* free old palette frame if one exists */
	smk_free (s->palette);
	s->palette = t;

	return 0;

error:
	/* Free old palette, set to current, return -1
		The new palette probably has errors but is preferrable to a black screen */
	smk_free (s->palette);
	s->palette = t;
	return -1;
}

static char smk_render_video(struct smk_video_t *s, unsigned char *p, unsigned int size)
{
	unsigned char *t = NULL,s1,s2;
	unsigned short temp;
	unsigned long i,j,k, row, col,skip;

	/* used for video decoding */
	struct smk_bit_t *bs = NULL;

	/* results from a tree lookup */
	long unpack;

	/* unpack, broken into pieces */
	unsigned char type;
	unsigned char blocklen;
	unsigned char typedata;

	const unsigned short sizetable[64] = {
		1,	 2,	3,	4,	5,	6,	7,	8,
		9,	10,   11,   12,   13,   14,   15,   16,
		17,   18,   19,   20,   21,   22,   23,   24,
		25,   26,   27,   28,   29,   30,   31,   32,
		33,   34,   35,   36,   37,   38,   39,   40,
		41,   42,   43,   44,   45,   46,   47,   48,
		49,   50,   51,   52,   53,   54,   55,   56,
		57,   58,   59,  128,  256,  512, 1024, 2048
	};

	/* sanity check */
	smk_null_check(s);
	smk_null_check(p);

	smk_malloc(t,s->w * s->h);

	row = 0;
	col = 0;

	/* Set up a bitstream for video unpacking */
	/* We could check the return code but it will only fail if p is null and we already verified that. */
	bs = smk_bs_init (p, size);

	/* Reset the cache on all bigtrees */
	smk_huff_big_reset(s->tree[0]);
	smk_huff_big_reset(s->tree[1]);
	smk_huff_big_reset(s->tree[2]);
	smk_huff_big_reset(s->tree[3]);

	while ( row < s->h )
	{
		smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_TYPE],unpack);

		type = ((unpack & 0x0003));
		blocklen = ((unpack & 0x00FC) >> 2);
		typedata = ((unpack & 0xFF00) >> 8);

		/* support for v4 full-blocks */
		if (type == 1 && s->v == '4')
		{
			if (smk_bs_read_1(bs))
			{
				type = 4;
			}
			else if (smk_bs_read_1(bs))
			{
				type = 5;
			}
		}

		for (j = 0; (j < sizetable[blocklen]) && (row < s->h); j ++)
		{
			skip = (row * s->w) + col;
			switch(type)
			{
				case 0:
					smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_MCLR],unpack);
					s1 = (unpack & 0xFF00) >> 8;
					s2 = (unpack & 0x00FF);
					smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_MMAP],unpack);

					temp = 0x01;
					for (k = 0; k < 4; k ++)
					{
						for (i = 0; i < 4; i ++)
						{
							if (unpack & temp)
							{
								t[skip + i] = s1;
							}
							else
							{
								t[skip + i] = s2;
							}
							temp = temp << 1;
						}
						skip += s->w;
					}
					break;

				case 1: /* FULL BLOCK */
					for (k = 0; k < 4; k ++)
					{
						smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_FULL],unpack);
						t[skip + 3] = ((unpack & 0xFF00) >> 8);
						t[skip + 2] = (unpack & 0x00FF);
						smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_FULL],unpack);
						t[skip + 1] = ((unpack & 0xFF00) >> 8);
						t[skip] = (unpack & 0x00FF);
						skip += s->w;
					}
					break;
				case 2: /* VOID BLOCK */
					if (s->frame)
					{
						memcpy(&t[skip], &s->frame[skip], 4);
						skip += s->w;
						memcpy(&t[skip], &s->frame[skip], 4);
						skip += s->w;
						memcpy(&t[skip], &s->frame[skip], 4);
						skip += s->w;
						memcpy(&t[skip], &s->frame[skip], 4);
					}
					break;
				case 3: /* SOLID BLOCK */
					memset(&t[skip],typedata,4);
					skip += s->w;
					memset(&t[skip],typedata,4);
					skip += s->w;
					memset(&t[skip],typedata,4);
					skip += s->w;
					memset(&t[skip],typedata,4);
					break;
				case 4: /* V4 DOUBLE BLOCK */
					for (k = 0; k < 2; k ++)
					{
						smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_FULL],unpack);
						for (i = 0; i < 2; i ++)
						{
							memset(&t[skip + 2],(unpack & 0xFF00) >> 8,2);
							memset(&t[skip],(unpack & 0x00FF),2);
							skip += s->w;
						}
					}
					break;
				case 5: /* V4 HALF BLOCK */
					for (k = 0; k < 2; k ++)
					{
						smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_FULL],unpack);
						t[skip + 3] = ((unpack & 0xFF00) >> 8);
						t[skip + 2] = (unpack & 0x00FF);
						t[skip + s->w + 3] = ((unpack & 0xFF00) >> 8);
						t[skip + s->w + 2] = (unpack & 0x00FF);
						smk_huff_big_safe_lookup(bs,s->tree[SMK_TREE_FULL],unpack);
						t[skip + 1] = ((unpack & 0xFF00) >> 8);
						t[skip] = (unpack & 0x00FF);
						t[skip + s->w + 1] = ((unpack & 0xFF00) >> 8);
						t[skip + s->w] = (unpack & 0x00FF);
						skip += (s->w << 1);
					}
					break;
			}
			col += 4;
			if (col >= s->w)
			{
				col = 0;
				row += 4;
			}
		}
	}

	smk_free(bs);
	smk_free (s->frame);
	s->frame = t;

	return 0;

error:
	smk_free(bs);
	smk_free (s->frame);
	s->frame = t;
	return -1;
}

/* Decompress audio track i. */
static char smk_render_audio(struct smk_audio_t *s, unsigned char *p, unsigned long size)
{
	unsigned int j,k;
	unsigned char *t = NULL;
	struct smk_bit_t *bs = NULL;

	char bit;
	short unpack, unpack2;

	/* used for audio decoding */
	struct smk_huff_t *aud_tree[4] = {NULL,NULL,NULL,NULL};

	/* sanity check */
	smk_null_check(s);
	smk_null_check(p);

	if (!s->compress)
	{
		/* Raw PCM data, update buffer size and malloc */
		s->buffer_size = size;
		smk_malloc(t,s->buffer_size);

		memcpy(t,p,size);
	}
	else if (s->compress == 1)
	{
		/* SMACKER DPCM compression */
		/* need at least 4 bytes to process */
		if (size < 4)
		{
			fputs("libsmacker::smk_render_audio() - ERROR: need 4 bytes to get unpacked output buffer size.\n",stderr);
			goto error;
		}
		/* chunk is compressed (huff-compressed dpcm), retrieve unpacked buffer size */
		s->buffer_size = ((unsigned int) p[3] << 24) |
						((unsigned int) p[2] << 16) |
						((unsigned int) p[1] << 8) |
						((unsigned int) p[0]);

		p += 4;
		size -= 4;

		/* malloc a buffer to hold unpacked audio */
		smk_malloc(t,s->buffer_size);

		/* Compressed audio: must unpack here */
		/*  Set up a bitstream */
		bs = smk_bs_init (p, size);

		smk_bs_safe_read_1(bs,bit);

		if (!bit)
		{
			fputs("libsmacker::smk_render_audio - ERROR: initial get_bit returned 0\n",stderr);
			goto error;
		}

		smk_bs_safe_read_1(bs,bit);
		if (s->channels != (bit == 1 ? 2 : 1))
		{
			fputs("libsmacker::smk_render - ERROR: mono/stereo mismatch\n",stderr);
		}
		smk_bs_safe_read_1(bs,bit);
		if (s->bitdepth != (bit == 1 ? 16 : 8))
		{
			fputs("libsmacker::smk_render - ERROR: 8-/16-bit mismatch\n",stderr);
		}

		/* build the trees */
		smk_huff_safe_build(bs,aud_tree[0]);
		j = 1;
		k = 1;
		if (s->bitdepth == 16)
		{
			smk_huff_safe_build(bs,aud_tree[1]);
			k = 2;
		}
		if (s->channels == 2)
		{
			smk_huff_safe_build(bs,aud_tree[2]);
			j = 2;
			k = 2;
			if (s->bitdepth == 16)
			{
				smk_huff_safe_build(bs,aud_tree[3]);
				k = 4;
			}
		}

		/* read initial sound level */
		if (s->channels == 2)
		{
			smk_bs_safe_read_8(bs,unpack);
			if (s->bitdepth == 16)
			{
				smk_bs_safe_read_8(bs,((short *)t)[1])
				((short *)t)[1] |= (unpack << 8);
			}
			else
			{
				((unsigned char *)t)[1] = (unsigned char)unpack;
			}
		}
		smk_bs_safe_read_8(bs,unpack);
		if (s->bitdepth == 16)
		{
				smk_bs_safe_read_8(bs,((short *)t)[0])
				((short *)t)[0] |= (unpack << 8);
		}
		else
		{
			((unsigned char *)t)[0] = (unsigned char)unpack;
		}

		/* All set: let's read some DATA! */
		while (k < s->buffer_size)
		{
			if (s->bitdepth == 8)
			{
				smk_huff_safe_lookup(bs,aud_tree[0],unpack);
				((unsigned char *)t)[j] = (char)unpack + ((unsigned char *)t)[j - s->channels];
				j ++;
				k++;
			}
			else
			{
				smk_huff_safe_lookup(bs,aud_tree[0],unpack);
				smk_huff_safe_lookup(bs,aud_tree[1],unpack2);
				((short *)t)[j] = (short) ( unpack | (unpack2 << 8) ) + ((short *)t)[j - s->channels];
				j ++;
				k+=2;
			}
			if (s->channels == 2)
			{
				if (s->bitdepth == 8)
				{
					smk_huff_safe_lookup(bs,aud_tree[2],unpack);
					((unsigned char *)t)[j] = (char)unpack + ((unsigned char *)t)[j - 2];
					j ++;
					k++;
				}
				else
				{
					smk_huff_safe_lookup(bs,aud_tree[2],unpack);
					smk_huff_safe_lookup(bs,aud_tree[3],unpack2);
					((short *)t)[j] = (short) ( unpack | (unpack2 << 8) ) + ((short *)t)[j - 2];
					j ++;
					k+=2;
				}
			}
		}

		/* All done with the trees, free them. */
		for (j = 0; j < 4; j ++)
		{
			if (aud_tree[j])
			{
				smk_huff_free(aud_tree[j]);
			}
		}

		/* free bitstream */
		smk_free(bs);
	}

	smk_free(s->buffer);
	s->buffer = t;

	return 0;

error:
	/* All done with the trees, free them. */
	for (j = 0; j < 4; j ++)
	{
		if (aud_tree[j])
		{
			smk_huff_free(aud_tree[j]);
		}
	}

	smk_free(bs);
	smk_free(s->buffer);
	s->buffer = t;

	return -1;
}

/* "Renders" (unpacks) the frame at cur_frame
   Preps all the image and audio pointers */
static char smk_render(smk s)
{
	unsigned long i,size;
	unsigned char *buffer = NULL, *p,track;

	/* sanity check */
	smk_null_check(s);

	/* Retrieve current chunk_size for this frame. */
	if (!(i = s->chunk_size[s->cur_frame]))
	{
		fprintf(stderr,"libsmacker::smk_render(s) - Warning: frame %lu: chunk_size is 0.\n",s->cur_frame);
		goto error;
	}

	if (s->mode == SMK_MODE_DISK)
	{
		/* Skip to frame in file */
		if (fseek(s->source.file.fp,s->source.file.chunk_offset[s->cur_frame],SEEK_SET))
		{
			fprintf(stderr,"libsmacker::smk_render(s) - ERROR: fseek to frame %lu (offset %lu) failed.\n",s->cur_frame,s->source.file.chunk_offset[s->cur_frame]);
			perror ("\tError reported was");
			goto error;
		}

		/* In disk-streaming mode: make way for our incoming chunk buffer */
		smk_malloc(buffer, i);

		/* Read into buffer */
		if ( smk_read_file(buffer,s->chunk_size[s->cur_frame],s->source.file.fp) < 0)
		{
			fprintf(stderr,"libsmacker::smk_render(s) - ERROR: frame %lu (offset %lu): smk_read had errors.\n",s->cur_frame,s->source.file.chunk_offset[s->cur_frame]);
			goto error;
		}
	}
	else
	{
		/* Just point buffer at the right place */
		if (!s->source.chunk_data[s->cur_frame])
		{
			fprintf(stderr,"libsmacker::smk_render(s) - ERROR: frame %lu: memory chunk is a NULL pointer.\n",s->cur_frame);
			goto error;
		}
		buffer = s->source.chunk_data[s->cur_frame];
	}

	p = buffer;

	/* Palette record first */
	if (s->frame_type[s->cur_frame] & 0x01)
	{
		/* need at least 1 byte to process */
		if (!i)
		{
			fprintf(stderr,"libsmacker::smk_render(s) - ERROR: frame %lu: insufficient data for a palette rec.\n",s->cur_frame);
			goto error;
		}

		/* Byte 1 in block, times 4, tells how many
			subsequent bytes are present */
		size = 4 * (*p);

		/* If video rendering enabled, kick this off for decode. */
		if (s->video->enable)
		{
			smk_render_palette(s->video,p + 1,size - 1);
		}
		p += size;
		i -= size;
	}

	/* Unpack audio chunks */
	for (track = 0; track < 7; track ++)
	{
		if (s->frame_type[s->cur_frame] & (0x02 << track))
		{
			/* need at least 4 byte to process */
			if (i < 4)
			{
				fprintf(stderr,"libsmacker::smk_render(s) - ERROR: frame %lu: insufficient data for audio[%u] rec.\n",s->cur_frame,track);
				goto error;
			}

			/* First 4 bytes in block tell how many
				subsequent bytes are present */
			size = (((unsigned int) p[3] << 24) |
					((unsigned int) p[2] << 16) |
					((unsigned int) p[1] << 8) |
					((unsigned int) p[0]));

			/* If audio rendering enabled, kick this off for decode. */
			if (s->audio[track] != NULL && s->audio[track]->enable)
			{
				smk_render_audio(s->audio[track],p + 4,size - 4);
			}
			p += size;
			i -= size;
		}
	}

	/* Unpack video chunk */
	if (s->video->enable)
	{
		smk_render_video(s->video, p,i);
	}

	if (s->mode == SMK_MODE_DISK)
	{
		/* Remember that buffer we allocated?  Trash it */
		smk_free(buffer);
	}

	return 0;

error:
	if (s->mode == SMK_MODE_DISK)
	{
		/* Remember that buffer we allocated?  Trash it */
		smk_free(buffer);
	}

	return -1;
}

/* rewind to first frame and unpack */
char smk_first(smk s)
{
	smk_null_check(s);

	s->cur_frame = 0;
	if ( smk_render(s) < 0)
	{
		fprintf(stderr,"libsmacker::smk_first(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);
		goto error;
	}

	if (s->f == 1) return SMK_LAST;
	return SMK_MORE;

error:
	return -1;
}

/* advance to next frame */
char smk_next(smk s)
{
	smk_null_check(s);

	if (s->cur_frame + 1 < (s->f + s->ring_frame))
	{
		s->cur_frame ++;
		if ( smk_render(s) < 0)
		{
			fprintf(stderr,"libsmacker::smk_next(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);
			goto error;
		}
		if (s->cur_frame + 1 == (s->f + s->ring_frame))
		{
			return SMK_LAST;
		}
		return SMK_MORE;
	}
	else if (s->ring_frame)
	{
		s->cur_frame = 1;
		if ( smk_render(s) < 0)
		{
			fprintf(stderr,"libsmacker::smk_next(s) - Warning: frame %lu: smk_render returned errors.\n",s->cur_frame);
			goto error;
		}
		if (s->cur_frame + 1 == (s->f + s->ring_frame))
		{
			return SMK_LAST;
		}
		return SMK_MORE;
	}
	return SMK_DONE;

error:
	return -1;
}

/* seek to a keyframe in an smk */
char smk_seek_keyframe(smk s, unsigned long f)
{
	smk_null_check(s);

	/* rewind (or fast forward!) exactly to f */
	s->cur_frame = f;

	/* roll back to previous keyframe in stream, or 0 if no keyframes exist */
	while (s->cur_frame > 0 && !(s->keyframe[s->cur_frame]))
	{
		s->cur_frame --;
	}

	/* render the frame: we're ready */
	if ( smk_render(s) < 0)
	{
		fprintf(stderr,"libsmacker::smk_seek_keyframe(s,%lu) - Warning: frame %lu: smk_render returned errors.\n",f,s->cur_frame);
		goto error;
	}

	return 0;

error:
	return -1;
}
