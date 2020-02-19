
/* pngset.c - storage of image information into info struct
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "pngpriv.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)

void PNGAPI
png_set_IHDR(png_const_structrp png_ptr, png_inforp info_ptr,
    png_uint_32 width, png_uint_32 height, int bit_depth,
    int color_type, int interlace_type, int compression_type,
    int filter_type)
{
   png_debug1(1, "in %s storage function", "IHDR");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   info_ptr->width = width;
   info_ptr->height = height;
   info_ptr->bit_depth = (png_byte)bit_depth;
   info_ptr->color_type = (png_byte)color_type;
   info_ptr->compression_type = (png_byte)compression_type;
   info_ptr->filter_type = (png_byte)filter_type;
   info_ptr->interlace_type = (png_byte)interlace_type;

   png_check_IHDR (png_ptr, info_ptr->width, info_ptr->height,
       info_ptr->bit_depth, info_ptr->color_type, info_ptr->interlace_type,
       info_ptr->compression_type, info_ptr->filter_type);

   if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      info_ptr->channels = 1;

   else if ((info_ptr->color_type & PNG_COLOR_MASK_COLOR) != 0)
      info_ptr->channels = 3;

   else
      info_ptr->channels = 1;

   if ((info_ptr->color_type & PNG_COLOR_MASK_ALPHA) != 0)
      info_ptr->channels++;

   info_ptr->pixel_depth = (png_byte)(info_ptr->channels * info_ptr->bit_depth);

   info_ptr->rowbytes = PNG_ROWBYTES(info_ptr->pixel_depth, width);
}

void PNGAPI
png_set_PLTE(png_structrp png_ptr, png_inforp info_ptr,
    png_const_colorp palette, int num_palette)
{

   png_uint_32 max_palette_length;

   png_debug1(1, "in %s storage function", "PLTE");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   max_palette_length = (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ?
      (1 << info_ptr->bit_depth) : PNG_MAX_PALETTE_LENGTH;

   if (num_palette < 0 || num_palette > (int) max_palette_length)
   {
      if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
         png_error(png_ptr, "Invalid palette length");

      else
      {
         png_warning(png_ptr, "Invalid palette length");

         return;
      }
   }

   if ((num_palette > 0 && palette == NULL) ||
      (num_palette == 0
#        ifdef PNG_MNG_FEATURES_SUPPORTED
            && (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0
#        endif
      ))
   {
      png_error(png_ptr, "Invalid palette");
   }
   png_free_data(png_ptr, info_ptr, PNG_FREE_PLTE, 0);
   png_ptr->palette = png_voidcast(png_colorp, png_calloc(png_ptr,
       PNG_MAX_PALETTE_LENGTH * (sizeof (png_color))));

   if (num_palette > 0)
      memcpy(png_ptr->palette, palette, (unsigned int)num_palette *
          (sizeof (png_color)));
   info_ptr->palette = png_ptr->palette;
   info_ptr->num_palette = png_ptr->num_palette = (png_uint_16)num_palette;

   info_ptr->free_me |= PNG_FREE_PLTE;

   info_ptr->valid |= PNG_INFO_PLTE;
}

void PNGAPI
png_set_compression_buffer_size(png_structrp png_ptr, size_t size)
{
   if (png_ptr == NULL)
      return;

   if (size == 0 || size > PNG_UINT_31_MAX)
      png_error(png_ptr, "invalid compression buffer size");

#  ifdef PNG_WRITE_SUPPORTED
   if ((png_ptr->mode & PNG_IS_READ_STRUCT) == 0)
   {
      if (png_ptr->zowner != 0)
      {
         png_warning(png_ptr,
             "Compression buffer size cannot be changed because it is in use");

         return;
      }

#ifndef __COVERITY__
      if (size > ZLIB_IO_MAX)
      {
         png_warning(png_ptr,
             "Compression buffer size limited to system maximum");
         size = ZLIB_IO_MAX;
      }
#endif

      if (size < 6)
      {
         png_warning(png_ptr,
             "Compression buffer size cannot be reduced below 6");

         return;
      }

      if (png_ptr->zbuffer_size != size)
      {
         png_free_buffer_list(png_ptr, &png_ptr->zbuffer_list);
         png_ptr->zbuffer_size = (uInt)size;
      }
   }
#  endif
}

void PNGAPI
png_set_invalid(png_const_structrp png_ptr, png_inforp info_ptr, int mask)
{
   if (png_ptr != NULL && info_ptr != NULL)
      info_ptr->valid &= (unsigned int)(~mask);
}
#endif
