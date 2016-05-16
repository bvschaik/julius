/*
	libsmacker - A C library for decoding .smk Smacker Video files
	Copyright (C) 2012-2013 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree.h
		SMK huffmann trees.  There are two types:
		- a basic 8-bit tree, and
		- a "big" 16-bit tree which includes a cache for recently
			searched values.
*/

#ifndef SMK_HUFFTREE_H
#define SMK_HUFFTREE_H

#include "smk_bitstream.h"

/*
	Tree node structure.
	If b0 is non-null, this is a branch, and b1 from the union should be used.
	If b0 is null, this is a leaf, and val / escape code from union should be used.
*/
struct smk_huff_t
{
	struct smk_huff_t *b0;
	union
	{
		struct smk_huff_t *b1;
		struct
		{
			unsigned short value;
			unsigned char escapecode;
		} leaf;
	} u;
};

/*
	"Big"tree struct: holds a huff_t structure,
	as well as a cache of three 16-bit values.
*/
struct smk_huff_big_t
{
	struct smk_huff_t *t;
	unsigned short cache[3];
};

/* function to recursively delete a huffman tree */
void smk_huff_free(struct smk_huff_t *);

/* This macro interrogates return code from smk_huff_lookup and
	jumps to error label if problems occur. */
#define smk_huff_safe_build(bs,t) \
{ \
	if (!(t = smk_huff_build(bs))) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_safe_build(" #bs "," #t ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}
/* Build a tree from a bitstream */
struct smk_huff_t *smk_huff_build(struct smk_bit_t *);

/* This macro interrogates return code from smk_huff_lookup and
	jumps to error label if problems occur. */
#define smk_huff_safe_lookup(bs,t,s) \
{ \
	if ((short)(s = smk_huff_lookup(bs,t)) < 0) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_safe_lookup(" #bs "," #t "," #s ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}
/* Look up an 8-bit value in the referenced tree by following a bitstream
	returns -1 on error */
short smk_huff_lookup (struct smk_bit_t *, const struct smk_huff_t *);

#define smk_huff_big_safe_build(bs,t) \
{ \
	if (!(t = smk_huff_big_build(bs))) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_big_safe_build(" #bs "," #t ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}
/* Build a bigtree from a bitstream */
struct smk_huff_big_t *smk_huff_big_build(struct smk_bit_t *);

#define smk_huff_big_safe_lookup(bs,t,s) \
{ \
	if ((s = smk_huff_big_lookup(bs,t)) < 0) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_big_safe_lookup(" #bs "," #t "," #s ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}
/* Look up a 16-bit value in the bigtree by following a bitstream
	returns -1 on error */
long smk_huff_big_lookup (struct smk_bit_t *, struct smk_huff_big_t *);

/* Reset the cache in a bigtree */
void smk_huff_big_reset (struct smk_huff_big_t *);

#endif
