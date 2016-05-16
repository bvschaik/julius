/*
	libsmacker - A C library for decoding .smk Smacker Video files
	Copyright (C) 2012-2013 Greg Kennedy

	See smacker.h for more information.

	smk_malloc.h
		"Safe" implementations of malloc and free.
*/

#ifndef SMK_MALLOC_H
#define SMK_MALLOC_H

#include <stdlib.h>
#include <stdio.h>
/* for memset */
#include <string.h>

/* "safe" null check:
	branches to an error block if pointer is null */
#define smk_null_check(p) \
{ \
	if (!p) \
	{ \
		fprintf(stderr,"libsmacker::smk_null_check(" #p "): ERROR: NULL POINTER at line %lu, file %s\n", (unsigned long)__LINE__, __FILE__); \
		goto error; \
	} \
}

/*
	Safe free: attempts to prevent double-free by setting pointer to NULL.
		Warns on attempts to free a NULL pointer.
*/
#define smk_free(p) \
{ \
	if (p) \
	{ \
		free(p); \
		p = NULL; \
	} \
/*	else \
	{ \
		fprintf(stderr,"libsmacker::smk_free(" #p ") - Warning: attempt to free NULL pointer (file: %s, line: %lu)\n", __FILE__, (unsigned long)__LINE__); \
	} */ \
}

/*
	Safe malloc: exits if malloc() returns NULL.
	Ideally, one should not exit() in a library. However, if you cannot
		malloc(), you probably have bigger problems.
*/
#define smk_malloc(p,x) \
{ \
	if (p) \
	{ \
		fprintf(stderr,"libsmacker::smk_malloc(" #p ",%lu) - Warning: freeing non-NULL pointer before malloc (file: %s, line: %lu)\n", (unsigned long) x, __FILE__, (unsigned long)__LINE__); \
		smk_free(p); \
	} \
	p = malloc(x); \
	if (!p) \
	{ \
		fprintf(stderr,"libsmacker::smk_malloc(" #p ",%lu) - ERROR: malloc() returned NULL (file: %s, line: %lu)\n", (unsigned long) x,  __FILE__, (unsigned long)__LINE__); \
		exit(EXIT_FAILURE); \
	} \
	memset(p,0,x); \
}

#endif
