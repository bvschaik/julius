
/* pngstruct.h - header file for PNG reference library
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

#ifndef PNGSTRUCT_H
#define PNGSTRUCT_H
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#include "zlib/zlib.h"
#ifdef const
#  undef const
#endif
#if ZLIB_VERNUM < 0x1260
#  define PNGZ_MSG_CAST(s) png_constcast(char*,s)
#  define PNGZ_INPUT_CAST(b) png_constcast(png_bytep,b)
#else
#  define PNGZ_MSG_CAST(s) (s)
#  define PNGZ_INPUT_CAST(b) (b)
#endif
#ifndef ZLIB_IO_MAX
#  define ZLIB_IO_MAX ((uInt)-1)
#endif

#ifdef PNG_WRITE_SUPPORTED
typedef struct png_compression_buffer
{
   struct png_compression_buffer *next;
   png_byte                       output[1];
} png_compression_buffer, *png_compression_bufferp;

#define PNG_COMPRESSION_BUFFER_SIZE(pp)\
   (offsetof(png_compression_buffer, output) + (pp)->zbuffer_size)
#endif

struct png_struct_def
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf jmp_buf_local;
   png_longjmp_ptr longjmp_fn;/* setjmp non-local goto function. */
   jmp_buf *jmp_buf_ptr;
   size_t jmp_buf_size;
#endif
   png_error_ptr error_fn;
#ifdef PNG_WARNINGS_SUPPORTED
   png_error_ptr warning_fn;
#endif
   png_voidp error_ptr;
   png_rw_ptr write_data_fn;
   png_rw_ptr read_data_fn;
   png_voidp io_ptr;

#ifdef PNG_READ_USER_TRANSFORM_SUPPORTED
   png_user_transform_ptr read_user_transform_fn;
#endif

#ifdef PNG_WRITE_USER_TRANSFORM_SUPPORTED
   png_user_transform_ptr write_user_transform_fn;
#endif
#ifdef PNG_USER_TRANSFORM_PTR_SUPPORTED
#if defined(PNG_READ_USER_TRANSFORM_SUPPORTED) || \
    defined(PNG_WRITE_USER_TRANSFORM_SUPPORTED)
   png_voidp user_transform_ptr;
   png_byte user_transform_depth;
   png_byte user_transform_channels;
#endif
#endif

   png_uint_32 mode;
   png_uint_32 flags;
   png_uint_32 transformations;

   png_uint_32 zowner;
   z_stream    zstream;

#ifdef PNG_WRITE_SUPPORTED
   png_compression_bufferp zbuffer_list;
   uInt                    zbuffer_size;

   int zlib_level;
   int zlib_method;
   int zlib_window_bits;
   int zlib_mem_level;
   int zlib_strategy;
#endif
#ifdef PNG_WRITE_SUPPORTED
   int zlib_set_level;
   int zlib_set_method;
   int zlib_set_window_bits;
   int zlib_set_mem_level;
   int zlib_set_strategy;
#endif

   png_uint_32 width;
   png_uint_32 height;
   png_uint_32 num_rows;
   png_uint_32 usr_width;
   size_t rowbytes;
   png_uint_32 iwidth;
   png_uint_32 row_number;
   png_uint_32 chunk_name;
   png_bytep prev_row;
   png_bytep row_buf;
#ifdef PNG_WRITE_FILTER_SUPPORTED
   png_bytep try_row;
   png_bytep tst_row;
#endif
   size_t info_rowbytes;

   png_uint_32 idat_size;
   png_uint_32 crc;
   png_colorp palette;
   png_uint_16 num_palette;
#ifdef PNG_CHECK_FOR_INVALID_INDEX_SUPPORTED
   int num_palette_max;
#endif

   png_uint_16 num_trans;
   png_byte compression;
   png_byte filter;
   png_byte interlaced;
   png_byte pass;
   png_byte do_filter;
   png_byte color_type;
   png_byte bit_depth;
   png_byte usr_bit_depth;
   png_byte pixel_depth;
   png_byte channels;
#ifdef PNG_WRITE_SUPPORTED
   png_byte usr_channels;
#endif
   png_byte sig_bytes;
   png_byte maximum_pixel_depth;
   png_byte transformed_pixel_depth;
#if ZLIB_VERNUM >= 0x1240
   png_byte zstream_start;
#endif

#ifdef PNG_WRITE_FLUSH_SUPPORTED
   png_flush_ptr output_flush_fn;
   png_uint_32 flush_dist;
   png_uint_32 flush_rows;
#endif

   png_read_status_ptr read_row_fn;
   png_write_status_ptr write_row_fn;

#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)
   png_bytepp offset_table_ptr;
   png_bytep offset_table;
   png_uint_16 offset_table_number;
   png_uint_16 offset_table_count;
   png_uint_16 offset_table_count_free;
#endif

#if PNG_LIBPNG_VER < 10700
#ifdef PNG_TIME_RFC1123_SUPPORTED
   char time_buffer[29];
#endif
#endif

   png_uint_32 free_me;
   png_bytep big_row_buf;
   png_byte compression_type;
   size_t old_big_row_buf_size;
   png_bytep big_prev_row;
   void (*read_filter[PNG_FILTER_VALUE_LAST-1])(png_row_infop row_info,
      png_bytep row, png_const_bytep prev_row);

};
#endif
