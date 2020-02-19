
/* pngerror.c - stub functions for i/o and memory allocation
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2017 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * This file provides a location for all error handling.  Users who
 * need special error handling are expected to write replacement functions
 * and use png_set_error_fn() to use those functions.  See the instructions
 * at each function.
 */

#include "pngpriv.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)

static PNG_FUNCTION(void, png_default_error,PNGARG((png_const_structrp png_ptr,
    png_const_charp error_message)),PNG_NORETURN);

PNG_FUNCTION(void,PNGAPI
png_err,(png_const_structrp png_ptr),PNG_NORETURN)
{
   if (png_ptr != NULL && png_ptr->error_fn != NULL)
      (*(png_ptr->error_fn))(png_constcast(png_structrp,png_ptr), "");
   png_default_error(png_ptr, "");
}
size_t
png_safecat(png_charp buffer, size_t bufsize, size_t pos,
    png_const_charp string)
{
   if (buffer != NULL && pos < bufsize)
   {
      if (string != NULL)
         while (*string != '\0' && pos < bufsize-1)
           buffer[pos++] = *string++;

      buffer[pos] = '\0';
   }

   return pos;
}

#define PNG_MAX_ERROR_TEXT 196

void
png_chunk_report(png_const_structrp png_ptr, png_const_charp message, int error)
{
#  ifndef PNG_WARNINGS_SUPPORTED
      PNG_UNUSED(message)
#  endif


#  ifdef PNG_WRITE_SUPPORTED
      {
         if (error < PNG_CHUNK_WRITE_ERROR)
            png_app_warning(png_ptr, message);

         else
            png_app_error(png_ptr, message);
      }
#  endif
}

#ifdef PNG_SETJMP_SUPPORTED
jmp_buf* PNGAPI
png_set_longjmp_fn(png_structrp png_ptr, png_longjmp_ptr longjmp_fn,
    size_t jmp_buf_size)
{
   if (png_ptr == NULL)
      return NULL;

   if (png_ptr->jmp_buf_ptr == NULL)
   {
      png_ptr->jmp_buf_size = 0;

      if (jmp_buf_size <= (sizeof png_ptr->jmp_buf_local))
         png_ptr->jmp_buf_ptr = &png_ptr->jmp_buf_local;

      else
      {
         png_ptr->jmp_buf_ptr = png_voidcast(jmp_buf *,
             png_malloc_warn(png_ptr, jmp_buf_size));

         if (png_ptr->jmp_buf_ptr == NULL)
            return NULL;

         png_ptr->jmp_buf_size = jmp_buf_size;
      }
   }

   else
   {
      size_t size = png_ptr->jmp_buf_size;

      if (size == 0)
      {
         size = (sizeof png_ptr->jmp_buf_local);
         if (png_ptr->jmp_buf_ptr != &png_ptr->jmp_buf_local)
         {
            png_error(png_ptr, "Libpng jmp_buf still allocated");
         }
      }

      if (size != jmp_buf_size)
      {
         png_warning(png_ptr, "Application jmp_buf size changed");
         return NULL;
      }
   }
   png_ptr->longjmp_fn = longjmp_fn;
   return png_ptr->jmp_buf_ptr;
}

void
png_free_jmpbuf(png_structrp png_ptr)
{
   if (png_ptr != NULL)
   {
      jmp_buf *jb = png_ptr->jmp_buf_ptr;
      if (jb != NULL && png_ptr->jmp_buf_size > 0)
      {
         if (jb != &png_ptr->jmp_buf_local)
         {
            jmp_buf free_jmp_buf;

            if (!setjmp(free_jmp_buf))
            {
               png_ptr->jmp_buf_ptr = &free_jmp_buf;
               png_ptr->jmp_buf_size = 0;
               png_ptr->longjmp_fn = longjmp;
               png_free(png_ptr, jb);
            }
         }
      }
      png_ptr->jmp_buf_size = 0;
      png_ptr->jmp_buf_ptr = NULL;
      png_ptr->longjmp_fn = 0;
   }
}
#endif
static PNG_FUNCTION(void,
png_default_error,(png_const_structrp png_ptr, png_const_charp error_message),
    PNG_NORETURN)
{
   PNG_UNUSED(error_message)
   png_longjmp(png_ptr, 1);
}

PNG_FUNCTION(void,PNGAPI
png_longjmp,(png_const_structrp png_ptr, int val),PNG_NORETURN)
{
#ifdef PNG_SETJMP_SUPPORTED
   if (png_ptr != NULL && png_ptr->longjmp_fn != NULL &&
       png_ptr->jmp_buf_ptr != NULL)
      png_ptr->longjmp_fn(*png_ptr->jmp_buf_ptr, val);
#else
   PNG_UNUSED(png_ptr)
   PNG_UNUSED(val)
#endif
   PNG_ABORT();
}
void PNGAPI
png_set_error_fn(png_structrp png_ptr, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warning_fn)
{
   if (png_ptr == NULL)
      return;

   png_ptr->error_ptr = error_ptr;
   png_ptr->error_fn = error_fn;
#ifdef PNG_WARNINGS_SUPPORTED
   png_ptr->warning_fn = warning_fn;
#else
   PNG_UNUSED(warning_fn)
#endif
}
png_voidp PNGAPI
png_get_error_ptr(png_const_structrp png_ptr)
{
   if (png_ptr == NULL)
      return NULL;

   return ((png_voidp)png_ptr->error_ptr);
}
#endif
