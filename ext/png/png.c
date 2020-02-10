
/* png.c - location for general purpose libpng functions
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
typedef png_libpng_version_1_6_37 Your_png_h_is_not_version_1_6_37;

#ifdef __GNUC__
#if __GNUC__ == 7 && __GNUC_MINOR__ == 1
#define GCC_STRICT_OVERFLOW 1
#endif
#endif
#ifndef GCC_STRICT_OVERFLOW
#define GCC_STRICT_OVERFLOW 0
#endif

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
PNG_FUNCTION(voidpf,
png_zalloc,(voidpf png_ptr, uInt items, uInt size),PNG_ALLOCATED)
{
   png_alloc_size_t num_bytes = size;

   if (png_ptr == NULL)
      return NULL;

   if (items >= (~(png_alloc_size_t)0)/size)
   {
      png_warning (png_voidcast(png_structrp, png_ptr),
          "Potential overflow in png_zalloc()");
      return NULL;
   }

   num_bytes *= items;
   return png_malloc_warn(png_voidcast(png_structrp, png_ptr), num_bytes);
}
void
png_zfree(voidpf png_ptr, voidpf ptr)
{
   png_free(png_voidcast(png_const_structrp,png_ptr), ptr);
}
void
png_reset_crc(png_structrp png_ptr)
{
   png_ptr->crc = (png_uint_32)crc32(0, Z_NULL, 0);
}
void
png_calculate_crc(png_structrp png_ptr, png_const_bytep ptr, size_t length)
{
   int need_crc = 1;

   if (PNG_CHUNK_ANCILLARY(png_ptr->chunk_name) != 0)
   {
      if ((png_ptr->flags & PNG_FLAG_CRC_ANCILLARY_MASK) ==
          (PNG_FLAG_CRC_ANCILLARY_USE | PNG_FLAG_CRC_ANCILLARY_NOWARN))
         need_crc = 0;
   }

   else
   {
      if ((png_ptr->flags & PNG_FLAG_CRC_CRITICAL_IGNORE) != 0)
         need_crc = 0;
   }
   if (need_crc != 0 && length > 0)
   {
      uLong crc = png_ptr->crc;

      do
      {
         uInt safe_length = (uInt)length;
#ifndef __COVERITY__
         if (safe_length == 0)
            safe_length = (uInt)-1;
#endif

         crc = crc32(crc, ptr, safe_length);
         ptr += safe_length;
         length -= safe_length;
      }
      while (length > 0);
      png_ptr->crc = (png_uint_32)crc;
   }
}
int
png_user_version_check(png_structrp png_ptr, png_const_charp user_png_ver)
{

   if (user_png_ver != NULL)
   {
      int i = -1;
      int found_dots = 0;

      do
      {
         i++;
         if (user_png_ver[i] != PNG_LIBPNG_VER_STRING[i])
            png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;
         if (user_png_ver[i] == '.')
            found_dots++;
      } while (found_dots < 2 && user_png_ver[i] != 0 &&
            PNG_LIBPNG_VER_STRING[i] != 0);
   }

   else
      png_ptr->flags |= PNG_FLAG_LIBRARY_MISMATCH;

   if ((png_ptr->flags & PNG_FLAG_LIBRARY_MISMATCH) != 0)
   {
#ifdef PNG_WARNINGS_SUPPORTED
      size_t pos = 0;
      char m[128];

      pos = png_safecat(m, (sizeof m), pos,
          "Application built with libpng-");
      pos = png_safecat(m, (sizeof m), pos, user_png_ver);
      pos = png_safecat(m, (sizeof m), pos, " but running with ");
      pos = png_safecat(m, (sizeof m), pos, PNG_LIBPNG_VER_STRING);
      PNG_UNUSED(pos)

      png_warning(png_ptr, m);
#endif

#ifdef PNG_ERROR_NUMBERS_SUPPORTED
      png_ptr->flags = 0;
#endif

      return 0;
   }
   return 1;
}
PNG_FUNCTION(png_structp,
png_create_png_struct,(png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn, png_voidp mem_ptr,
    png_malloc_ptr malloc_fn, png_free_ptr free_fn),PNG_ALLOCATED)
{
   png_struct create_struct;
#  ifdef PNG_SETJMP_SUPPORTED
      jmp_buf create_jmp_buf;
#  endif
   memset(&create_struct, 0, (sizeof create_struct));
      PNG_UNUSED(mem_ptr)
      PNG_UNUSED(malloc_fn)
      PNG_UNUSED(free_fn)
   png_set_error_fn(&create_struct, error_ptr, error_fn, warn_fn);

#  ifdef PNG_SETJMP_SUPPORTED
      if (!setjmp(create_jmp_buf))
#  endif
      {
#  ifdef PNG_SETJMP_SUPPORTED
         create_struct.jmp_buf_ptr = &create_jmp_buf;
         create_struct.jmp_buf_size = 0;
         create_struct.longjmp_fn = longjmp;
#  endif
         if (png_user_version_check(&create_struct, user_png_ver) != 0)
         {
            png_structrp png_ptr = png_voidcast(png_structrp,
                png_malloc_warn(&create_struct, (sizeof *png_ptr)));

            if (png_ptr != NULL)
            {
               create_struct.zstream.zalloc = png_zalloc;
               create_struct.zstream.zfree = png_zfree;
               create_struct.zstream.opaque = png_ptr;

#              ifdef PNG_SETJMP_SUPPORTED
               create_struct.jmp_buf_ptr = NULL;
               create_struct.jmp_buf_size = 0;
               create_struct.longjmp_fn = 0;
#              endif

               *png_ptr = create_struct;
               return png_ptr;
            }
         }
      }
   return NULL;
}
PNG_FUNCTION(png_infop,PNGAPI
png_create_info_struct,(png_const_structrp png_ptr),PNG_ALLOCATED)
{
   png_inforp info_ptr;

   png_debug(1, "in png_create_info_struct");

   if (png_ptr == NULL)
      return NULL;
   info_ptr = png_voidcast(png_inforp, png_malloc_base(png_ptr,
       (sizeof *info_ptr)));

   if (info_ptr != NULL)
      memset(info_ptr, 0, (sizeof *info_ptr));

   return info_ptr;
}
void PNGAPI
png_destroy_info_struct(png_const_structrp png_ptr, png_infopp info_ptr_ptr)
{
   png_inforp info_ptr = NULL;

   png_debug(1, "in png_destroy_info_struct");

   if (png_ptr == NULL)
      return;

   if (info_ptr_ptr != NULL)
      info_ptr = *info_ptr_ptr;

   if (info_ptr != NULL)
   {
      *info_ptr_ptr = NULL;

      png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
      memset(info_ptr, 0, (sizeof *info_ptr));
      png_free(png_ptr, info_ptr);
   }
}
PNG_FUNCTION(void,PNGAPI
png_info_init_3,(png_infopp ptr_ptr, size_t png_info_struct_size),
    PNG_DEPRECATED)
{
   png_inforp info_ptr = *ptr_ptr;

   png_debug(1, "in png_info_init_3");

   if (info_ptr == NULL)
      return;

   if ((sizeof (png_info)) > png_info_struct_size)
   {
      *ptr_ptr = NULL;
      free(info_ptr);
      info_ptr = png_voidcast(png_inforp, png_malloc_base(NULL,
          (sizeof *info_ptr)));
      if (info_ptr == NULL)
         return;
      *ptr_ptr = info_ptr;
   }
   memset(info_ptr, 0, (sizeof *info_ptr));
}
void PNGAPI
png_data_freer(png_const_structrp png_ptr, png_inforp info_ptr,
    int freer, png_uint_32 mask)
{
   png_debug(1, "in png_data_freer");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   if (freer == PNG_DESTROY_WILL_FREE_DATA)
      info_ptr->free_me |= mask;

   else if (freer == PNG_USER_WILL_FREE_DATA)
      info_ptr->free_me &= ~mask;

   else
      png_error(png_ptr, "Unknown freer parameter in png_data_freer");
}

void PNGAPI
png_free_data(png_const_structrp png_ptr, png_inforp info_ptr, png_uint_32 mask,
    int num)
{
   png_debug(1, "in png_free_data");

   if (png_ptr == NULL || info_ptr == NULL)
      return;
   if (((mask & PNG_FREE_PLTE) & info_ptr->free_me) != 0)
   {
      png_free(png_ptr, info_ptr->palette);
      info_ptr->palette = NULL;
      info_ptr->valid &= ~PNG_INFO_PLTE;
      info_ptr->num_palette = 0;
   }

   if (num != -1)
      mask &= ~PNG_FREE_MUL;

   info_ptr->free_me &= ~mask;
}
#endif
png_voidp PNGAPI
png_get_io_ptr(png_const_structrp png_ptr)
{
   if (png_ptr == NULL)
      return (NULL);

   return (png_ptr->io_ptr);
}

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
#  ifdef PNG_STDIO_SUPPORTED
void PNGAPI
png_init_io(png_structrp png_ptr, png_FILE_p fp)
{
   png_debug(1, "in png_init_io");

   if (png_ptr == NULL)
      return;

   png_ptr->io_ptr = (png_voidp)fp;
}
#  endif

#  ifdef PNG_SAVE_INT_32_SUPPORTED
void PNGAPI
png_save_int_32(png_bytep buf, png_int_32 i)
{
   png_save_uint_32(buf, (png_uint_32)i);
}
#  endif

#endif

png_const_charp PNGAPI
png_get_copyright(png_const_structrp png_ptr)
{
   PNG_UNUSED(png_ptr)
#ifdef PNG_STRING_COPYRIGHT
   return PNG_STRING_COPYRIGHT
#else
   return PNG_STRING_NEWLINE \
      "libpng version 1.6.37" PNG_STRING_NEWLINE \
      "Copyright (c) 2018-2019 Cosmin Truta" PNG_STRING_NEWLINE \
      "Copyright (c) 1998-2002,2004,2006-2018 Glenn Randers-Pehrson" \
      PNG_STRING_NEWLINE \
      "Copyright (c) 1996-1997 Andreas Dilger" PNG_STRING_NEWLINE \
      "Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc." \
      PNG_STRING_NEWLINE;
#endif
}
png_const_charp PNGAPI
png_get_libpng_ver(png_const_structrp png_ptr)
{
   return png_get_header_ver(png_ptr);
}

png_const_charp PNGAPI
png_get_header_ver(png_const_structrp png_ptr)
{
   PNG_UNUSED(png_ptr)
   return PNG_LIBPNG_VER_STRING;
}

png_const_charp PNGAPI
png_get_header_version(png_const_structrp png_ptr)
{
   PNG_UNUSED(png_ptr)
#ifdef __STDC__
   return PNG_HEADER_VERSION_STRING
#  ifndef PNG_READ_SUPPORTED
      " (NO READ SUPPORT)"
#  endif
      PNG_STRING_NEWLINE;
#else
   return PNG_HEADER_VERSION_STRING;
#endif
}
png_uint_32 PNGAPI
png_access_version_number(void)
{
   return((png_uint_32)PNG_LIBPNG_VER);
}

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)
void
png_zstream_error(png_structrp png_ptr, int ret)
{
   if (png_ptr->zstream.msg == NULL) switch (ret)
   {
      default:
      case Z_OK:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("unexpected zlib return code");
         break;

      case Z_STREAM_END:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("unexpected end of LZ stream");
         break;

      case Z_NEED_DICT:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("missing LZ dictionary");
         break;

      case Z_ERRNO:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("zlib IO error");
         break;

      case Z_STREAM_ERROR:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("bad parameters to zlib");
         break;

      case Z_DATA_ERROR:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("damaged LZ stream");
         break;

      case Z_MEM_ERROR:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("insufficient memory");
         break;

      case Z_BUF_ERROR:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("truncated");
         break;

      case Z_VERSION_ERROR:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("unsupported zlib version");
         break;

      case PNG_UNEXPECTED_ZLIB_RETURN:
         png_ptr->zstream.msg = PNGZ_MSG_CAST("unexpected zlib return");
         break;
   }
}

#ifdef __GNUC__
static int
png_gt(size_t a, size_t b)
{
   return a > b;
}
#else
#   define png_gt(a,b) ((a) > (b))
#endif

void
png_check_IHDR(png_const_structrp png_ptr,
    png_uint_32 width, png_uint_32 height, int bit_depth,
    int color_type, int interlace_type, int compression_type,
    int filter_type)
{
   int error = 0;
   if (width == 0)
   {
      png_warning(png_ptr, "Image width is zero in IHDR");
      error = 1;
   }

   if (width > PNG_UINT_31_MAX)
   {
      png_warning(png_ptr, "Invalid image width in IHDR");
      error = 1;
   }

   if (png_gt(((width + 7) & (~7U)),
       ((PNG_SIZE_MAX
           - 48
           - 1)
           / 8)
           - 1))
   {
      png_warning(png_ptr, "Image width is too large for this architecture");
      error = 1;
   }

   if (width > PNG_USER_WIDTH_MAX)
   {
      png_warning(png_ptr, "Image width exceeds user limit in IHDR");
      error = 1;
   }

   if (height == 0)
   {
      png_warning(png_ptr, "Image height is zero in IHDR");
      error = 1;
   }

   if (height > PNG_UINT_31_MAX)
   {
      png_warning(png_ptr, "Invalid image height in IHDR");
      error = 1;
   }

   if (height > PNG_USER_HEIGHT_MAX)
   {
      png_warning(png_ptr, "Image height exceeds user limit in IHDR");
      error = 1;
   }
   if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 &&
       bit_depth != 8 && bit_depth != 16)
   {
      png_warning(png_ptr, "Invalid bit depth in IHDR");
      error = 1;
   }

   if (color_type < 0 || color_type == 1 ||
       color_type == 5 || color_type > 6)
   {
      png_warning(png_ptr, "Invalid color type in IHDR");
      error = 1;
   }

   if (((color_type == PNG_COLOR_TYPE_PALETTE) && bit_depth > 8) ||
       ((color_type == PNG_COLOR_TYPE_RGB ||
         color_type == PNG_COLOR_TYPE_GRAY_ALPHA ||
         color_type == PNG_COLOR_TYPE_RGB_ALPHA) && bit_depth < 8))
   {
      png_warning(png_ptr, "Invalid color type/bit depth combination in IHDR");
      error = 1;
   }

   if (interlace_type >= PNG_INTERLACE_LAST)
   {
      png_warning(png_ptr, "Unknown interlace method in IHDR");
      error = 1;
   }

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
   {
      png_warning(png_ptr, "Unknown compression method in IHDR");
      error = 1;
   }

   if (filter_type != PNG_FILTER_TYPE_BASE)
   {
      png_warning(png_ptr, "Unknown filter method in IHDR");
      error = 1;
   }

   if (error == 1)
      png_error(png_ptr, "Invalid IHDR data");
}
#endif
