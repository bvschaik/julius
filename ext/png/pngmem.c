
/* pngmem.c - stub functions for memory allocation
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2014,2016 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file provides a location for all memory allocation.  Users who
 * need special memory handling are expected to supply replacement
 * functions for png_malloc() and png_free(), and to use
 * png_create_read_struct_2() and png_create_write_struct_2() to
 * identify the replacement functions.
 */

#include "pngpriv.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
void
png_destroy_png_struct(png_structrp png_ptr)
{
   if (png_ptr != NULL)
   {
      png_struct dummy_struct = *png_ptr;
      memset(png_ptr, 0, (sizeof *png_ptr));
      png_free(&dummy_struct, png_ptr);

#     ifdef PNG_SETJMP_SUPPORTED
         png_free_jmpbuf(&dummy_struct);
#     endif
   }
}
PNG_FUNCTION(png_voidp,PNGAPI
png_calloc,(png_const_structrp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

   ret = png_malloc(png_ptr, size);

   if (ret != NULL)
      memset(ret, 0, size);

   return ret;
}
PNG_FUNCTION(png_voidp,
png_malloc_base,(png_const_structrp png_ptr, png_alloc_size_t size),
    PNG_ALLOCATED)
{
#ifndef PNG_USER_MEM_SUPPORTED
   PNG_UNUSED(png_ptr)
#endif
   if (size > 0 && size <= PNG_SIZE_MAX
#     ifdef PNG_MAX_MALLOC_64K
         && size <= 65536U
#     endif
      )
   {
#ifdef PNG_USER_MEM_SUPPORTED
      if (png_ptr != NULL && png_ptr->malloc_fn != NULL)
         return png_ptr->malloc_fn(png_constcast(png_structrp,png_ptr), size);

      else
#endif
         return malloc((size_t)size);
   }

   else
      return NULL;
}
PNG_FUNCTION(png_voidp,PNGAPI
png_malloc,(png_const_structrp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

   if (png_ptr == NULL)
      return NULL;

   ret = png_malloc_base(png_ptr, size);

   if (ret == NULL)
       png_error(png_ptr, "Out of memory");

   return ret;
}
PNG_FUNCTION(png_voidp,PNGAPI
png_malloc_warn,(png_const_structrp png_ptr, png_alloc_size_t size),
    PNG_ALLOCATED)
{
   if (png_ptr != NULL)
   {
      png_voidp ret = png_malloc_base(png_ptr, size);

      if (ret != NULL)
         return ret;

      png_warning(png_ptr, "Out of memory");
   }

   return NULL;
}
void PNGAPI
png_free(png_const_structrp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

   free(ptr);
}

#endif
