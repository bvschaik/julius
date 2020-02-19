
/* pngwrite.c - general routines to write a PNG file
 *
 * Copyright (c) 2018-2019 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "pngpriv.h"
#ifdef PNG_SIMPLIFIED_WRITE_STDIO_SUPPORTED
#  include <errno.h>
#endif

#ifdef PNG_WRITE_SUPPORTED

void PNGAPI
png_write_info_before_PLTE(png_structrp png_ptr, png_const_inforp info_ptr)
{
   png_debug(1, "in png_write_info_before_PLTE");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if ((png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE) == 0)
   {
      png_write_sig(png_ptr);
      png_write_IHDR(png_ptr, info_ptr->width, info_ptr->height,
          info_ptr->bit_depth, info_ptr->color_type, info_ptr->compression_type,
          info_ptr->filter_type,
#ifdef PNG_WRITE_INTERLACING_SUPPORTED
          info_ptr->interlace_type
#else
          0
#endif
         );

      png_ptr->mode |= PNG_WROTE_INFO_BEFORE_PLTE;
   }
}

void PNGAPI
png_write_info(png_structrp png_ptr, png_const_inforp info_ptr)
{
   png_debug(1, "in png_write_info");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   png_write_info_before_PLTE(png_ptr, info_ptr);

   if ((info_ptr->valid & PNG_INFO_PLTE) != 0)
      png_write_PLTE(png_ptr, info_ptr->palette,
          (png_uint_32)info_ptr->num_palette);

   else if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      png_error(png_ptr, "Valid palette required for paletted images");
}
void PNGAPI
png_write_end(png_structrp png_ptr, png_inforp info_ptr)
{
   png_debug(1, "in png_write_end");

   if (png_ptr == NULL)
      return;

   if ((png_ptr->mode & PNG_HAVE_IDAT) == 0)
      png_error(png_ptr, "No IDATs written into file");

   png_ptr->mode |= PNG_AFTER_IDAT;
   png_write_IEND(png_ptr);
#ifdef PNG_WRITE_FLUSH_SUPPORTED
#  ifdef PNG_WRITE_FLUSH_AFTER_IEND_SUPPORTED
   png_flush(png_ptr);
#  endif
#endif
}
PNG_FUNCTION(png_structp,PNGAPI
png_create_write_struct,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),PNG_ALLOCATED)
{
   png_structrp png_ptr = png_create_png_struct(user_png_ver, error_ptr,
       error_fn, warn_fn, NULL, NULL, NULL);

   if (png_ptr != NULL)
   {
      png_ptr->zbuffer_size = PNG_ZBUF_SIZE;
      png_ptr->zlib_strategy = PNG_Z_DEFAULT_STRATEGY;
      png_ptr->zlib_level = PNG_Z_DEFAULT_COMPRESSION;
      png_ptr->zlib_mem_level = 8;
      png_ptr->zlib_window_bits = 15;
      png_ptr->zlib_method = 8;
#if PNG_RELEASE_BUILD
      png_ptr->flags |= PNG_FLAG_APP_WARNINGS_WARN;
#endif
      png_set_write_fn(png_ptr, NULL, NULL, NULL);
   }

   return png_ptr;
}
void PNGAPI
png_write_rows(png_structrp png_ptr, png_bytepp row,
    png_uint_32 num_rows)
{
   png_uint_32 i;
   png_bytepp rp;

   png_debug(1, "in png_write_rows");

   if (png_ptr == NULL)
      return;
   for (i = 0, rp = row; i < num_rows; i++, rp++)
   {
      png_write_row(png_ptr, *rp);
   }
}
void PNGAPI
png_write_image(png_structrp png_ptr, png_bytepp image)
{
   png_uint_32 i;
   png_bytepp rp;

   if (png_ptr == NULL)
      return;

   png_debug(1, "in png_write_image");
    for (i = 0, rp = image; i < png_ptr->height; i++, rp++)
    {
        png_write_row(png_ptr, *rp);
    }
}
void PNGAPI
png_write_row(png_structrp png_ptr, png_const_bytep row)
{
   png_row_info row_info;

   if (png_ptr == NULL)
      return;

   png_debug2(1, "in png_write_row (row %u, pass %d)",
       png_ptr->row_number, png_ptr->pass);
   if (png_ptr->row_number == 0 && png_ptr->pass == 0)
   {
      if ((png_ptr->mode & PNG_WROTE_INFO_BEFORE_PLTE) == 0)
         png_error(png_ptr,
             "png_write_info was never called before png_write_row");

      png_write_start_row(png_ptr);
   }
   row_info.color_type = png_ptr->color_type;
   row_info.width = png_ptr->usr_width;
   row_info.channels = png_ptr->usr_channels;
   row_info.bit_depth = png_ptr->usr_bit_depth;
   row_info.pixel_depth = (png_byte)(row_info.bit_depth * row_info.channels);
   row_info.rowbytes = PNG_ROWBYTES(row_info.pixel_depth, row_info.width);

   png_debug1(3, "row_info->color_type = %d", row_info.color_type);
   png_debug1(3, "row_info->width = %u", row_info.width);
   png_debug1(3, "row_info->channels = %d", row_info.channels);
   png_debug1(3, "row_info->bit_depth = %d", row_info.bit_depth);
   png_debug1(3, "row_info->pixel_depth = %d", row_info.pixel_depth);
   png_debug1(3, "row_info->rowbytes = %lu", (unsigned long)row_info.rowbytes);
   memcpy(png_ptr->row_buf + 1, row, row_info.rowbytes);
   if (row_info.pixel_depth != png_ptr->pixel_depth ||
       row_info.pixel_depth != png_ptr->transformed_pixel_depth)
      png_error(png_ptr, "internal write transform logic error");
   png_write_find_filter(png_ptr, &row_info);

   if (png_ptr->write_row_fn != NULL)
      (*(png_ptr->write_row_fn))(png_ptr, png_ptr->row_number, png_ptr->pass);
}

#ifdef PNG_WRITE_FLUSH_SUPPORTED
void PNGAPI
png_set_flush(png_structrp png_ptr, int nrows)
{
   png_debug(1, "in png_set_flush");

   if (png_ptr == NULL)
      return;

   png_ptr->flush_dist = (nrows < 0 ? 0 : (png_uint_32)nrows);
}
void PNGAPI
png_write_flush(png_structrp png_ptr)
{
   png_debug(1, "in png_write_flush");

   if (png_ptr == NULL)
      return;
   if (png_ptr->row_number >= png_ptr->num_rows)
      return;

   png_compress_IDAT(png_ptr, NULL, 0, Z_SYNC_FLUSH);
   png_ptr->flush_rows = 0;
   png_flush(png_ptr);
}
#endif
static void
png_write_destroy(png_structrp png_ptr)
{
   png_debug(1, "in png_write_destroy");
   if ((png_ptr->flags & PNG_FLAG_ZSTREAM_INITIALIZED) != 0)
      deflateEnd(&png_ptr->zstream);
   png_free_buffer_list(png_ptr, &png_ptr->zbuffer_list);
   png_free(png_ptr, png_ptr->row_buf);
   png_ptr->row_buf = NULL;
}
void PNGAPI
png_destroy_write_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
{
   png_debug(1, "in png_destroy_write_struct");

   if (png_ptr_ptr != NULL)
   {
      png_structrp png_ptr = *png_ptr_ptr;

      if (png_ptr != NULL)
      {
         png_destroy_info_struct(png_ptr, info_ptr_ptr);

         *png_ptr_ptr = NULL;
         png_write_destroy(png_ptr);
         png_destroy_png_struct(png_ptr);
      }
   }
}
void PNGAPI
png_set_filter(png_structrp png_ptr, int method, int filters)
{
   png_debug(1, "in png_set_filter");

   if (png_ptr == NULL)
      return;

   if (method == PNG_FILTER_TYPE_BASE)
   {
      switch (filters & (PNG_ALL_FILTERS | 0x07))
      {
         case PNG_FILTER_VALUE_NONE:
            png_ptr->do_filter = PNG_FILTER_NONE; break;

         default:
            png_app_error(png_ptr, "Unknown row filter for method 0");
      }
   }
   else
      png_error(png_ptr, "Unknown custom filter method");
}

#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
void PNGAPI
png_set_compression_level(png_structrp png_ptr, int level)
{
   png_debug(1, "in png_set_compression_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_level = level;
}

void PNGAPI
png_set_compression_mem_level(png_structrp png_ptr, int mem_level)
{
   png_debug(1, "in png_set_compression_mem_level");

   if (png_ptr == NULL)
      return;

   png_ptr->zlib_mem_level = mem_level;
}

void PNGAPI
png_set_compression_strategy(png_structrp png_ptr, int strategy)
{
   png_debug(1, "in png_set_compression_strategy");

   if (png_ptr == NULL)
      return;
   png_ptr->flags |= PNG_FLAG_ZLIB_CUSTOM_STRATEGY;
   png_ptr->zlib_strategy = strategy;
}
void PNGAPI
png_set_compression_window_bits(png_structrp png_ptr, int window_bits)
{
   if (png_ptr == NULL)
      return;
   if (window_bits > 15)
   {
      png_warning(png_ptr, "Only compression windows <= 32k supported by PNG");
      window_bits = 15;
   }

   else if (window_bits < 8)
   {
      png_warning(png_ptr, "Only compression windows >= 256 supported by PNG");
      window_bits = 8;
   }

   png_ptr->zlib_window_bits = window_bits;
}

void PNGAPI
png_set_compression_method(png_structrp png_ptr, int method)
{
   png_debug(1, "in png_set_compression_method");

   if (png_ptr == NULL)
      return;
   if (method != 8)
      png_warning(png_ptr, "Only compression method 8 is supported by PNG");

   png_ptr->zlib_method = method;
}
#endif

void PNGAPI
png_set_write_status_fn(png_structrp png_ptr, png_write_status_ptr write_row_fn)
{
   if (png_ptr == NULL)
      return;

   png_ptr->write_row_fn = write_row_fn;
}

#endif
