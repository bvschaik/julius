/*
	libsmacker - A C library for decoding .smk Smacker Video files
	Copyright (C) 2012-2013 Greg Kennedy

	See smacker.h for more information.

	smk_hufftree.c
		Implementation of Smacker Huffman coding trees.
*/

#include "smk_hufftree.h"

/* malloc and friends */
#include "smk_malloc.h"

/* function to recursively delete a huffman tree */
void smk_huff_free(struct smk_huff_t *t)
{
	/* Sanity check: do not double-free */
	smk_null_check(t);

	/* If this is not a leaf node, free child trees first */
	if (t->b0)
	{
		smk_huff_free(t->b0);
		smk_huff_free(t->u.b1);
	}

	/* Safe-delete tree node. */
	smk_free(t);

error: ;
}

/* safe build with built-in error jump */
#define smk_huff_safe_build_rec(bs,p) \
{ \
	if (!(p = smk_huff_build_rec(bs))) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_safe_build_rec(" #bs "," #p ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}

/* Recursive tree-building function. */
static struct smk_huff_t *smk_huff_build_rec(struct smk_bit_t *bs)
{
	struct smk_huff_t *ret = NULL;
	char bit;

	/* sanity check */
	smk_null_check(bs);

	/* Read the bit */
	smk_bs_safe_read_1(bs,bit);

	/* Malloc a structure. */
	smk_malloc(ret,sizeof(struct smk_huff_t));

	if (bit)
	{
		/* Bit set: this forms a Branch node. */
		/* Recursively attempt to build the Left branch. */
		smk_huff_safe_build_rec(bs,ret->b0);

		/* Everything is still OK: attempt to build the Right branch. */
		smk_huff_safe_build_rec(bs,ret->u.b1);

		/* return branch pointer here */
		return ret;
	}

	/* Bit unset signifies a Leaf node. */
	/* Attempt to read value */
	smk_bs_safe_read_8(bs,ret->u.leaf.value);

	/* smk_malloc sets entries to 0 by default */
	/* ret->b0 = NULL; */
	ret->u.leaf.escapecode = 0xFF;

	return ret;

error:
	smk_free(ret);
	return NULL;
}

/*
	Entry point for huff_build.  Basically just checks the start/end tags
	and calls smk_huff_build_rec recursive function.
*/
struct smk_huff_t *smk_huff_build(struct smk_bit_t* bs)
{
	struct smk_huff_t *ret = NULL;
	char bit;

	/* sanity check */
	smk_null_check(bs);

	/* Smacker huff trees begin with a set-bit. */
	smk_bs_safe_read_1(bs,bit);

	if (!bit)
	{
		/* Got a bit, but it was not 1. In theory, there could be a smk file
			without this particular tree. */
		fputs("libsmacker::smk_huff_build(bs) - Warning: initial get_bit returned 0\n",stderr);
		goto error;
	}

	/* Begin parsing the tree data. */
	smk_huff_safe_build_rec(bs,ret);

	/* huff trees end with an unset-bit */
	smk_bs_safe_read_1(bs,bit);

	if (bit)
	{
		fputs("libsmacker::smk_huff_build(bs) - ERROR: final get_bit returned 1\n",stderr);
		goto error;
	}

	return ret;

error:
	smk_free(ret);
	return NULL;
}

/* Look up an 8-bit value from a basic huff tree.
	Return -1 on error. */
short smk_huff_lookup (struct smk_bit_t *bs, const struct smk_huff_t *t)
{
	char bit;

	/* sanity check */
	smk_null_check(bs);
	smk_null_check(t);

	if (!t->b0)
	{
		/* Reached a Leaf node.  Return its value. */
		return t->u.leaf.value;
	}

	smk_bs_safe_read_1(bs,bit);

	if (bit)
	{
		/* get_bit returned Set, follow Right branch. */
		return smk_huff_lookup(bs,t->u.b1);
	}

	/* follow Right branch */
	return smk_huff_lookup(bs,t->b0);

error:
	return -1;
}

#define smk_huff_big_safe_build_rec(bs,cache,low8,hi8,p) \
{ \
	if (!(p = smk_huff_big_build_rec(bs,cache,low8,hi8))) \
	{ \
		fprintf(stderr,"libsmacker::smk_huff_big_safe_build_rec(" #bs "," #cache "," #low8 "," #hi8 "," #p ") - ERROR (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
		goto error; \
	} \
}

/* Recursively builds a Big tree. */
static struct smk_huff_t *smk_huff_big_build_rec(struct smk_bit_t *bs, const unsigned short cache[3], const struct smk_huff_t *low8, const struct smk_huff_t *hi8)
{
	struct smk_huff_t *ret = NULL;

	char bit;
	short lowval;

	/* sanity check */
	smk_null_check(bs);
	smk_null_check(cache);
	smk_null_check(low8);
	smk_null_check(hi8);

	/* Get the first bit */
	smk_bs_safe_read_1(bs,bit);

	/* Malloc a structure. */
	smk_malloc(ret,sizeof(struct smk_huff_t));

	if (bit)
	{
		/* Recursively attempt to build the Left branch. */
		smk_huff_big_safe_build_rec(bs,cache,low8,hi8,ret->b0);

		/* Recursively attempt to build the Left branch. */
		smk_huff_big_safe_build_rec(bs,cache,low8,hi8,ret->u.b1);

		/* return branch pointer here */
		return ret;
	}

	/* Bit unset signifies a Leaf node. */
	smk_huff_safe_lookup(bs,low8,lowval);
	smk_huff_safe_lookup(bs,hi8,ret->u.leaf.value);

	/* Looks OK: we got low and hi values.  Return a new LEAF */
	/* ret->b0 = NULL; */
	ret->u.leaf.value = lowval | (ret->u.leaf.value << 8);

	/* Last: when building the tree, some Values may correspond to cache positions.
		Identify these values and set the Escape code byte accordingly. */
	if (ret->u.leaf.value == cache[0])
	{
		ret->u.leaf.escapecode = 0;
	}
	else if (ret->u.leaf.value == cache[1])
	{
		ret->u.leaf.escapecode = 1;
	}
	else if (ret->u.leaf.value == cache[2])
	{
		ret->u.leaf.escapecode = 2;
	}
	else
	{
		ret->u.leaf.escapecode = 0xFF;
	}

	return ret;

error:
	smk_free(ret);
	return NULL;
}

/* Entry point for building a big 16-bit tree. */
struct smk_huff_big_t *smk_huff_big_build(struct smk_bit_t* bs)
{
	struct smk_huff_big_t *big = NULL;

	struct smk_huff_t *low8 = NULL, *hi8 = NULL;

	short lowval;

	char bit;
	unsigned char i;

	/* sanity check */
	smk_null_check(bs);

	/* Smacker huff trees begin with a set-bit. */
	smk_bs_safe_read_1(bs,bit);

	if (!bit)
	{
		fputs("libsmacker::smk_huff_big_build(bs) - ERROR: initial get_bit returned 0\n",stderr);
		goto error;
	}

	/* build low-8-bits tree */
	smk_huff_safe_build(bs,low8);
	/* build hi-8-bits tree */
	smk_huff_safe_build(bs,hi8);

	/* Everything looks OK so far.  Time to malloc structure. */
	smk_malloc(big,sizeof(struct smk_huff_big_t));

	/* Init the escape code cache. */
	for (i = 0; i < 3; i ++)
	{
		smk_bs_safe_read_8(bs,lowval);
		smk_bs_safe_read_8(bs,big->cache[i]);
		big->cache[i] = lowval | (big->cache[i] << 8);
/* fprintf(stderr,"Escape code cache: cache[%d] = %d\n",i,big->cache[i]); */
	}

	/* Finally, call recursive function to retrieve the Bigtree. */
	smk_huff_big_safe_build_rec(bs,big->cache,low8,hi8,big->t);

	/* Done with 8-bit hufftrees, free them. */
	smk_huff_free(hi8);
	smk_huff_free(low8);

	/* Check final end tag. */
	smk_bs_safe_read_1(bs,bit);

	if (bit)
	{
		fputs("libsmacker::smk_huff_big_build(bs) - ERROR: final get_bit returned 1\n",stderr);
		goto error;
	}

	return big;

error:
	smk_free(big);
	smk_free(hi8);
	smk_free(low8);
	return NULL;
}

static int smk_huff_big_lookup_rec (struct smk_bit_t *bs, unsigned short cache[3], struct smk_huff_t *t)
{
	unsigned short val;
	char bit;

	/* sanity check */
	smk_null_check(bs);
	smk_null_check(cache);
	smk_null_check(t);

	/* Reached a Leaf node */
	if (!t->b0)
	{
		if (t->u.leaf.escapecode != 0xFF)
		{
			/* Found escape code. Retrieve value from Cache. */
			val = cache[t->u.leaf.escapecode];
		}
		else
		{
			/* Use value directly. */
			val = t->u.leaf.value;
		}

		if ( cache[0] != val)
		{
			/* Update the cache, by moving val to the front of the queue,
				if it isn't already there. */
			cache[2] = cache[1];
			cache[1] = cache[0];
			cache[0] = val;
		}

		return val;
	}

	smk_bs_safe_read_1(bs,bit);

	if (bit)
	{
		/* get_bit returned Set, follow Right branch. */
		return smk_huff_big_lookup_rec(bs,cache,t->u.b1);
	}

	return smk_huff_big_lookup_rec(bs,cache,t->b0);

error:
	return -1;
}

/* Convenience call-out for recursive bigtree lookup function */
long smk_huff_big_lookup (struct smk_bit_t *bs, struct smk_huff_big_t *big)
{
	return smk_huff_big_lookup_rec(bs,big->cache,big->t);
}

/* Resets a Big hufftree cache */
void smk_huff_big_reset (struct smk_huff_big_t *big)
{
	big->cache[0] = 0;
	big->cache[1] = 0;
	big->cache[2] = 0;
}
