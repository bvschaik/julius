
/* png.h - header file for PNG reference library
 *
 * libpng version 1.6.37 - April 14, 2019
 *
 * Copyright (c) 2018-2019 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license. (See LICENSE, below.)
 *
 * Authors and maintainers:
 *   libpng versions 0.71, May 1995, through 0.88, January 1996: Guy Schalnat
 *   libpng versions 0.89, June 1996, through 0.96, May 1997: Andreas Dilger
 *   libpng versions 0.97, January 1998, through 1.6.35, July 2018:
 *     Glenn Randers-Pehrson
 *   libpng versions 1.6.36, December 2018, through 1.6.37, April 2019:
 *     Cosmin Truta
 *   See also "Contributing Authors", below.
 *
 * Changed for Julius by José Cadete (crudelios) on 11 Feb 2020.
 * The library was heavily trimmed, with many files or unused code/comments
 * removed and compiler settings disabled to make it lightweight.
 * Therefore, do not use this version in your projects.
 */

 /*
  * COPYRIGHT NOTICE, DISCLAIMER, and LICENSE
  * =========================================
  *
  * PNG Reference Library License version 2
  * ---------------------------------------
  *
  *  * Copyright (c) 1995-2019 The PNG Reference Library Authors.
  *  * Copyright (c) 2018-2019 Cosmin Truta.
  *  * Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson.
  *  * Copyright (c) 1996-1997 Andreas Dilger.
  *  * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
  *
  * The software is supplied "as is", without warranty of any kind,
  * express or implied, including, without limitation, the warranties
  * of merchantability, fitness for a particular purpose, title, and
  * non-infringement.  In no event shall the Copyright owners, or
  * anyone distributing the software, be liable for any damages or
  * other liability, whether in contract, tort or otherwise, arising
  * from, out of, or in connection with the software, or the use or
  * other dealings in the software, even if advised of the possibility
  * of such damage.
  *
  * Permission is hereby granted to use, copy, modify, and distribute
  * this software, or portions hereof, for any purpose, without fee,
  * subject to the following restrictions:
  *
  *  1. The origin of this software must not be misrepresented; you
  *     must not claim that you wrote the original software.  If you
  *     use this software in a product, an acknowledgment in the product
  *     documentation would be appreciated, but is not required.
  *
  *  2. Altered source versions must be plainly marked as such, and must
  *     not be misrepresented as being the original software.
  *
  *  3. This Copyright notice may not be removed or altered from any
  *     source or altered source distribution.
  *
  *
  * PNG Reference Library License version 1 (for libpng 0.5 through 1.6.35)
  * -----------------------------------------------------------------------
  *
  * libpng versions 1.0.7, July 1, 2000, through 1.6.35, July 15, 2018 are
  * Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson, are
  * derived from libpng-1.0.6, and are distributed according to the same
  * disclaimer and license as libpng-1.0.6 with the following individuals
  * added to the list of Contributing Authors:
  *
  *     Simon-Pierre Cadieux
  *     Eric S. Raymond
  *     Mans Rullgard
  *     Cosmin Truta
  *     Gilles Vollant
  *     James Yu
  *     Mandar Sahastrabuddhe
  *     Google Inc.
  *     Vadim Barkov
  *
  * and with the following additions to the disclaimer:
  *
  *     There is no warranty against interference with your enjoyment of
  *     the library or against infringement.  There is no warranty that our
  *     efforts or the library will fulfill any of your particular purposes
  *     or needs.  This library is provided with all faults, and the entire
  *     risk of satisfactory quality, performance, accuracy, and effort is
  *     with the user.
  *
  * Some files in the "contrib" directory and some configure-generated
  * files that are distributed with libpng have other copyright owners, and
  * are released under other open source licenses.
  *
  * libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
  * Copyright (c) 1998-2000 Glenn Randers-Pehrson, are derived from
  * libpng-0.96, and are distributed according to the same disclaimer and
  * license as libpng-0.96, with the following individuals added to the
  * list of Contributing Authors:
  *
  *     Tom Lane
  *     Glenn Randers-Pehrson
  *     Willem van Schaik
  *
  * libpng versions 0.89, June 1996, through 0.96, May 1997, are
  * Copyright (c) 1996-1997 Andreas Dilger, are derived from libpng-0.88,
  * and are distributed according to the same disclaimer and license as
  * libpng-0.88, with the following individuals added to the list of
  * Contributing Authors:
  *
  *     John Bowler
  *     Kevin Bracey
  *     Sam Bushell
  *     Magnus Holmgren
  *     Greg Roelofs
  *     Tom Tanner
  *
  * Some files in the "scripts" directory have other copyright owners,
  * but are released under this license.
  *
  * libpng versions 0.5, May 1995, through 0.88, January 1996, are
  * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
  *
  * For the purposes of this copyright and license, "Contributing Authors"
  * is defined as the following set of individuals:
  *
  *     Andreas Dilger
  *     Dave Martindale
  *     Guy Eric Schalnat
  *     Paul Schmidt
  *     Tim Wegner
  *
  * The PNG Reference Library is supplied "AS IS".  The Contributing
  * Authors and Group 42, Inc. disclaim all warranties, expressed or
  * implied, including, without limitation, the warranties of
  * merchantability and of fitness for any purpose.  The Contributing
  * Authors and Group 42, Inc. assume no liability for direct, indirect,
  * incidental, special, exemplary, or consequential damages, which may
  * result from the use of the PNG Reference Library, even if advised of
  * the possibility of such damage.
  *
  * Permission is hereby granted to use, copy, modify, and distribute this
  * source code, or portions hereof, for any purpose, without fee, subject
  * to the following restrictions:
  *
  *  1. The origin of this source code must not be misrepresented.
  *
  *  2. Altered versions must be plainly marked as such and must not
  *     be misrepresented as being the original source.
  *
  *  3. This Copyright notice may not be removed or altered from any
  *     source or altered source distribution.
  *
  * The Contributing Authors and Group 42, Inc. specifically permit,
  * without fee, and encourage the use of this source code as a component
  * to supporting the PNG file format in commercial products.  If you use
  * this source code in a product, acknowledgment is not required but would
  * be appreciated.
  *
  * END OF COPYRIGHT NOTICE, DISCLAIMER, and LICENSE.
  *
  * TRADEMARK
  * =========
  *
  * The name "libpng" has not been registered by the Copyright owners
  * as a trademark in any jurisdiction.  However, because libpng has
  * been distributed and maintained world-wide, continually since 1995,
  * the Copyright owners claim "common-law trademark protection" in any
  * jurisdiction where common-law trademark is recognized.
  */

#ifndef PNG_H
#define PNG_H
#define PNG_LIBPNG_VER_STRING "1.6.37.f-julius-build"
#define PNG_HEADER_VERSION_STRING " libpng version 1.6.37.f-julius-build - February 11, 2020\n"

#define PNG_LIBPNG_VER_SONUM   16
#define PNG_LIBPNG_VER_DLLNUM  16
#define PNG_LIBPNG_VER_MAJOR   1
#define PNG_LIBPNG_VER_MINOR   6
#define PNG_LIBPNG_VER_RELEASE 37
#define PNG_LIBPNG_VER_BUILD  0
#define PNG_LIBPNG_BUILD_ALPHA    1
#define PNG_LIBPNG_BUILD_BETA     2
#define PNG_LIBPNG_BUILD_RC       3
#define PNG_LIBPNG_BUILD_STABLE   4
#define PNG_LIBPNG_BUILD_RELEASE_STATUS_MASK 7
#define PNG_LIBPNG_BUILD_PATCH    8
#define PNG_LIBPNG_BUILD_PRIVATE 16
#define PNG_LIBPNG_BUILD_SPECIAL 32

#define PNG_LIBPNG_BUILD_BASE_TYPE PNG_LIBPNG_BUILD_STABLE
#define PNG_LIBPNG_VER 10637
#ifndef PNGLCONF_H
#   include "pnglibconf.h"
#endif

#ifndef PNG_VERSION_INFO_ONLY
#  include "pngconf.h"
#endif

#ifdef PNG_USER_PRIVATEBUILD
#  define PNG_LIBPNG_BUILD_TYPE \
       (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_PRIVATE)
#else
#  ifdef PNG_LIBPNG_SPECIALBUILD
#    define PNG_LIBPNG_BUILD_TYPE \
         (PNG_LIBPNG_BUILD_BASE_TYPE | PNG_LIBPNG_BUILD_SPECIAL)
#  else
#    define PNG_LIBPNG_BUILD_TYPE (PNG_LIBPNG_BUILD_BASE_TYPE)
#  endif
#endif

#ifndef PNG_VERSION_INFO_ONLY
#ifdef __cplusplus
extern "C" {
#endif
#define png_libpng_ver png_get_header_ver(NULL)
typedef char* png_libpng_version_1_6_37;
typedef struct png_struct_def png_struct;
typedef const png_struct * png_const_structp;
typedef png_struct * png_structp;
typedef png_struct * * png_structpp;
typedef struct png_info_def png_info;
typedef png_info * png_infop;
typedef const png_info * png_const_infop;
typedef png_info * * png_infopp;
typedef png_struct * PNG_RESTRICT png_structrp;
typedef const png_struct * PNG_RESTRICT png_const_structrp;
typedef png_info * PNG_RESTRICT png_inforp;
typedef const png_info * PNG_RESTRICT png_const_inforp;
typedef struct png_color_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
} png_color;
typedef png_color * png_colorp;
typedef const png_color * png_const_colorp;
typedef png_color * * png_colorpp;

typedef struct png_color_16_struct
{
   png_byte index;
   png_uint_16 red;
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 gray;
} png_color_16;
typedef png_color_16 * png_color_16p;
typedef const png_color_16 * png_const_color_16p;
typedef png_color_16 * * png_color_16pp;

typedef struct png_color_8_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
   png_byte gray;
   png_byte alpha;
} png_color_8;
typedef png_color_8 * png_color_8p;
typedef const png_color_8 * png_const_color_8p;
typedef png_color_8 * * png_color_8pp;
typedef struct png_sPLT_entry_struct
{
   png_uint_16 red;
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 alpha;
   png_uint_16 frequency;
} png_sPLT_entry;
typedef png_sPLT_entry * png_sPLT_entryp;
typedef const png_sPLT_entry * png_const_sPLT_entryp;
typedef png_sPLT_entry * * png_sPLT_entrypp;

typedef struct png_sPLT_struct
{
   png_charp name;
   png_byte depth;
   png_sPLT_entryp entries;
   png_int_32 nentries;
} png_sPLT_t;
typedef png_sPLT_t * png_sPLT_tp;
typedef const png_sPLT_t * png_const_sPLT_tp;
typedef png_sPLT_t * * png_sPLT_tpp;
#define PNG_TEXT_COMPRESSION_NONE_WR -3
#define PNG_TEXT_COMPRESSION_zTXt_WR -2
#define PNG_TEXT_COMPRESSION_NONE    -1
#define PNG_TEXT_COMPRESSION_zTXt     0
#define PNG_ITXT_COMPRESSION_NONE     1
#define PNG_ITXT_COMPRESSION_zTXt     2
#define PNG_TEXT_COMPRESSION_LAST     3
typedef struct png_time_struct
{
   png_uint_16 year;
   png_byte month;
   png_byte day;
   png_byte hour;
   png_byte minute;
   png_byte second;
} png_time;
typedef png_time * png_timep;
typedef const png_time * png_const_timep;
typedef png_time * * png_timepp;
#define PNG_HAVE_IHDR  0x01
#define PNG_HAVE_PLTE  0x02
#define PNG_AFTER_IDAT 0x08
#define PNG_UINT_31_MAX ((png_uint_32)0x7fffffffL)
#define PNG_UINT_32_MAX ((png_uint_32)(-1))
#define PNG_SIZE_MAX ((size_t)(-1))
#define PNG_FP_1    100000
#define PNG_FP_HALF  50000
#define PNG_FP_MAX  ((png_fixed_point)0x7fffffffL)
#define PNG_FP_MIN  (-PNG_FP_MAX)
#define PNG_COLOR_MASK_PALETTE    1
#define PNG_COLOR_MASK_COLOR      2
#define PNG_COLOR_MASK_ALPHA      4
#define PNG_COLOR_TYPE_GRAY 0
#define PNG_COLOR_TYPE_PALETTE  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)
#define PNG_COLOR_TYPE_RGB        (PNG_COLOR_MASK_COLOR)
#define PNG_COLOR_TYPE_RGB_ALPHA  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA)
#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA)
#define PNG_COLOR_TYPE_RGBA  PNG_COLOR_TYPE_RGB_ALPHA
#define PNG_COLOR_TYPE_GA  PNG_COLOR_TYPE_GRAY_ALPHA
#define PNG_COMPRESSION_TYPE_BASE 0
#define PNG_COMPRESSION_TYPE_DEFAULT PNG_COMPRESSION_TYPE_BASE
#define PNG_FILTER_TYPE_BASE      0
#define PNG_INTRAPIXEL_DIFFERENCING 64
#define PNG_FILTER_TYPE_DEFAULT   PNG_FILTER_TYPE_BASE
#define PNG_INTERLACE_NONE        0
#define PNG_INTERLACE_ADAM7       1
#define PNG_INTERLACE_LAST        2
#define PNG_OFFSET_PIXEL          0
#define PNG_OFFSET_MICROMETER     1
#define PNG_OFFSET_LAST           2
#define PNG_EQUATION_LINEAR       0
#define PNG_EQUATION_BASE_E       1
#define PNG_EQUATION_ARBITRARY    2
#define PNG_EQUATION_HYPERBOLIC   3
#define PNG_EQUATION_LAST         4
#define PNG_SCALE_UNKNOWN         0
#define PNG_SCALE_METER           1
#define PNG_SCALE_RADIAN          2
#define PNG_SCALE_LAST            3
#define PNG_RESOLUTION_UNKNOWN    0
#define PNG_RESOLUTION_METER      1
#define PNG_RESOLUTION_LAST       2
#define PNG_sRGB_INTENT_PERCEPTUAL 0
#define PNG_sRGB_INTENT_RELATIVE   1
#define PNG_sRGB_INTENT_SATURATION 2
#define PNG_sRGB_INTENT_ABSOLUTE   3
#define PNG_sRGB_INTENT_LAST       4
#define PNG_KEYWORD_MAX_LENGTH     79
#define PNG_MAX_PALETTE_LENGTH    256
#define PNG_INFO_gAMA 0x0001U
#define PNG_INFO_sBIT 0x0002U
#define PNG_INFO_cHRM 0x0004U
#define PNG_INFO_PLTE 0x0008U
#define PNG_INFO_tRNS 0x0010U
#define PNG_INFO_bKGD 0x0020U
#define PNG_INFO_hIST 0x0040U
#define PNG_INFO_pHYs 0x0080U
#define PNG_INFO_oFFs 0x0100U
#define PNG_INFO_tIME 0x0200U
#define PNG_INFO_pCAL 0x0400U
#define PNG_INFO_sRGB 0x0800U
#define PNG_INFO_iCCP 0x1000U
#define PNG_INFO_sPLT 0x2000U
#define PNG_INFO_sCAL 0x4000U
#define PNG_INFO_IDAT 0x8000U
#define PNG_INFO_eXIf 0x10000U
typedef struct png_row_info_struct
{
   png_uint_32 width;
   size_t rowbytes;
   png_byte color_type;
   png_byte bit_depth;
   png_byte channels;
   png_byte pixel_depth;
} png_row_info;

typedef png_row_info * png_row_infop;
typedef png_row_info * * png_row_infopp;
typedef PNG_CALLBACK(void, *png_error_ptr, (png_structp, png_const_charp));
typedef PNG_CALLBACK(void, *png_rw_ptr, (png_structp, png_bytep, size_t));
typedef PNG_CALLBACK(void, *png_flush_ptr, (png_structp));
typedef PNG_CALLBACK(void, *png_read_status_ptr, (png_structp, png_uint_32,
    int));
typedef PNG_CALLBACK(void, *png_write_status_ptr, (png_structp, png_uint_32,
    int));

#ifdef PNG_SETJMP_SUPPORTED
PNG_FUNCTION(void, (PNGCAPI *png_longjmp_ptr), PNGARG((jmp_buf, int)), typedef);
#endif
#define PNG_TRANSFORM_IDENTITY       0x0000
#define PNG_TRANSFORM_STRIP_16       0x0001
#define PNG_TRANSFORM_STRIP_ALPHA    0x0002
#define PNG_TRANSFORM_PACKING        0x0004
#define PNG_TRANSFORM_PACKSWAP       0x0008
#define PNG_TRANSFORM_EXPAND         0x0010
#define PNG_TRANSFORM_INVERT_MONO    0x0020
#define PNG_TRANSFORM_SHIFT          0x0040
#define PNG_TRANSFORM_BGR            0x0080
#define PNG_TRANSFORM_SWAP_ALPHA     0x0100
#define PNG_TRANSFORM_SWAP_ENDIAN    0x0200
#define PNG_TRANSFORM_INVERT_ALPHA   0x0400
#define PNG_TRANSFORM_STRIP_FILLER   0x0800
#define PNG_TRANSFORM_STRIP_FILLER_BEFORE PNG_TRANSFORM_STRIP_FILLER
#define PNG_TRANSFORM_STRIP_FILLER_AFTER 0x1000
#define PNG_TRANSFORM_GRAY_TO_RGB   0x2000
#define PNG_TRANSFORM_EXPAND_16     0x4000
#if INT_MAX >= 0x8000
#define PNG_TRANSFORM_SCALE_16      0x8000
#endif
#define PNG_FLAG_MNG_EMPTY_PLTE     0x01
#define PNG_FLAG_MNG_FILTER_64      0x04
#define PNG_ALL_MNG_FEATURES        0x05
typedef PNG_CALLBACK(png_voidp, *png_malloc_ptr, (png_structp,
    png_alloc_size_t));
typedef PNG_CALLBACK(void, *png_free_ptr, (png_structp, png_voidp));
PNG_EXPORT(1, png_uint_32, png_access_version_number, (void));
PNG_EXPORT(2, void, png_set_sig_bytes, (png_structrp png_ptr, int num_bytes));
PNG_EXPORT(3, int, png_sig_cmp, (png_const_bytep sig, size_t start,
    size_t num_to_check));
#define png_check_sig(sig, n) !png_sig_cmp((sig), 0, (n))
PNG_EXPORTA(4, png_structp, png_create_read_struct,
    (png_const_charp user_png_ver, png_voidp error_ptr,
    png_error_ptr error_fn, png_error_ptr warn_fn),
    PNG_ALLOCATED);
PNG_EXPORTA(5, png_structp, png_create_write_struct,
    (png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn,
    png_error_ptr warn_fn),
    PNG_ALLOCATED);

PNG_EXPORT(6, size_t, png_get_compression_buffer_size,
    (png_const_structrp png_ptr));

PNG_EXPORT(7, void, png_set_compression_buffer_size, (png_structrp png_ptr,
    size_t size));
#ifdef PNG_SETJMP_SUPPORTED
PNG_EXPORT(8, jmp_buf*, png_set_longjmp_fn, (png_structrp png_ptr,
    png_longjmp_ptr longjmp_fn, size_t jmp_buf_size));
#  define png_jmpbuf(png_ptr) \
      (*png_set_longjmp_fn((png_ptr), longjmp, (sizeof (jmp_buf))))
#else
#  define png_jmpbuf(png_ptr) \
      (LIBPNG_WAS_COMPILED_WITH__PNG_NO_SETJMP)
#endif
PNG_EXPORTA(9, void, png_longjmp, (png_const_structrp png_ptr, int val),
    PNG_NORETURN);
PNG_EXPORT(13, void, png_write_sig, (png_structrp png_ptr));
PNG_EXPORT(14, void, png_write_chunk, (png_structrp png_ptr, png_const_bytep
    chunk_name, png_const_bytep data, size_t length));
PNG_EXPORT(15, void, png_write_chunk_start, (png_structrp png_ptr,
    png_const_bytep chunk_name, png_uint_32 length));
PNG_EXPORT(16, void, png_write_chunk_data, (png_structrp png_ptr,
    png_const_bytep data, size_t length));
PNG_EXPORT(17, void, png_write_chunk_end, (png_structrp png_ptr));
PNG_EXPORTA(18, png_infop, png_create_info_struct, (png_const_structrp png_ptr),
    PNG_ALLOCATED);
PNG_EXPORTA(19, void, png_info_init_3, (png_infopp info_ptr,
    size_t png_info_struct_size), PNG_DEPRECATED);
PNG_EXPORT(20, void, png_write_info_before_PLTE,
    (png_structrp png_ptr, png_const_inforp info_ptr));
PNG_EXPORT(21, void, png_write_info,
    (png_structrp png_ptr, png_const_inforp info_ptr));

#ifdef PNG_WRITE_FLUSH_SUPPORTED
PNG_EXPORT(51, void, png_set_flush, (png_structrp png_ptr, int nrows));
PNG_EXPORT(52, void, png_write_flush, (png_structrp png_ptr));
#endif
PNG_EXPORT(53, void, png_start_read_image, (png_structrp png_ptr));
PNG_EXPORT(54, void, png_read_update_info, (png_structrp png_ptr,
    png_inforp info_ptr));
PNG_EXPORT(58, void, png_write_row, (png_structrp png_ptr,
    png_const_bytep row));
PNG_EXPORT(59, void, png_write_rows, (png_structrp png_ptr, png_bytepp row,
    png_uint_32 num_rows));
PNG_EXPORT(60, void, png_write_image, (png_structrp png_ptr, png_bytepp image));
PNG_EXPORT(61, void, png_write_end, (png_structrp png_ptr,
    png_inforp info_ptr));
PNG_EXPORT(63, void, png_destroy_info_struct, (png_const_structrp png_ptr,
    png_infopp info_ptr_ptr));
PNG_EXPORT(64, void, png_destroy_read_struct, (png_structpp png_ptr_ptr,
    png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr));
PNG_EXPORT(65, void, png_destroy_write_struct, (png_structpp png_ptr_ptr,
    png_infopp info_ptr_ptr));
PNG_EXPORT(66, void, png_set_crc_action, (png_structrp png_ptr, int crit_action,
    int ancil_action));
#define PNG_CRC_DEFAULT       0
#define PNG_CRC_ERROR_QUIT    1
#define PNG_CRC_WARN_DISCARD  2
#define PNG_CRC_WARN_USE      3
#define PNG_CRC_QUIET_USE     4
#define PNG_CRC_NO_CHANGE     5

#ifdef PNG_WRITE_SUPPORTED
PNG_EXPORT(67, void, png_set_filter, (png_structrp png_ptr, int method,
    int filters));
#endif
#define PNG_NO_FILTERS     0x00
#define PNG_FILTER_NONE    0x08
#define PNG_FILTER_SUB     0x10
#define PNG_FILTER_UP      0x20
#define PNG_FILTER_AVG     0x40
#define PNG_FILTER_PAETH   0x80
#define PNG_FAST_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP)
#define PNG_ALL_FILTERS (PNG_FAST_FILTERS | PNG_FILTER_AVG | PNG_FILTER_PAETH)
#define PNG_FILTER_VALUE_NONE  0
#define PNG_FILTER_VALUE_SUB   1
#define PNG_FILTER_VALUE_UP    2
#define PNG_FILTER_VALUE_AVG   3
#define PNG_FILTER_VALUE_PAETH 4
#define PNG_FILTER_VALUE_LAST  5

#ifdef PNG_WRITE_SUPPORTED
#define PNG_FILTER_HEURISTIC_DEFAULT    0
#define PNG_FILTER_HEURISTIC_UNWEIGHTED 1
#define PNG_FILTER_HEURISTIC_WEIGHTED   2
#define PNG_FILTER_HEURISTIC_LAST       3
#ifdef PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED
PNG_EXPORT(69, void, png_set_compression_level, (png_structrp png_ptr,
    int level));

PNG_EXPORT(70, void, png_set_compression_mem_level, (png_structrp png_ptr,
    int mem_level));

PNG_EXPORT(71, void, png_set_compression_strategy, (png_structrp png_ptr,
    int strategy));
PNG_EXPORT(72, void, png_set_compression_window_bits, (png_structrp png_ptr,
    int window_bits));

PNG_EXPORT(73, void, png_set_compression_method, (png_structrp png_ptr,
    int method));
#endif
#endif

#ifdef PNG_STDIO_SUPPORTED
PNG_EXPORT(74, void, png_init_io, (png_structrp png_ptr, png_FILE_p fp));
#endif

PNG_EXPORT(75, void, png_set_error_fn, (png_structrp png_ptr,
    png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warning_fn));
PNG_EXPORT(76, png_voidp, png_get_error_ptr, (png_const_structrp png_ptr));
PNG_EXPORT(77, void, png_set_write_fn, (png_structrp png_ptr, png_voidp io_ptr,
    png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));
PNG_EXPORT(78, void, png_set_read_fn, (png_structrp png_ptr, png_voidp io_ptr,
    png_rw_ptr read_data_fn));
PNG_EXPORT(79, png_voidp, png_get_io_ptr, (png_const_structrp png_ptr));

PNG_EXPORT(80, void, png_set_read_status_fn, (png_structrp png_ptr,
    png_read_status_ptr read_row_fn));

PNG_EXPORT(81, void, png_set_write_status_fn, (png_structrp png_ptr,
    png_write_status_ptr write_row_fn));

PNG_EXPORTA(94, png_voidp, png_malloc, (png_const_structrp png_ptr,
    png_alloc_size_t size), PNG_ALLOCATED);
PNG_EXPORTA(95, png_voidp, png_calloc, (png_const_structrp png_ptr,
    png_alloc_size_t size), PNG_ALLOCATED);
PNG_EXPORTA(96, png_voidp, png_malloc_warn, (png_const_structrp png_ptr,
    png_alloc_size_t size), PNG_ALLOCATED);
PNG_EXPORT(97, void, png_free, (png_const_structrp png_ptr, png_voidp ptr));
PNG_EXPORT(98, void, png_free_data, (png_const_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 free_me, int num));
PNG_EXPORT(99, void, png_data_freer, (png_const_structrp png_ptr,
    png_inforp info_ptr, int freer, png_uint_32 mask));
#define PNG_DESTROY_WILL_FREE_DATA 1
#define PNG_SET_WILL_FREE_DATA 1
#define PNG_USER_WILL_FREE_DATA 2
#define PNG_FREE_HIST 0x0008U
#define PNG_FREE_ICCP 0x0010U
#define PNG_FREE_SPLT 0x0020U
#define PNG_FREE_ROWS 0x0040U
#define PNG_FREE_PCAL 0x0080U
#define PNG_FREE_SCAL 0x0100U
#ifdef PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
#  define PNG_FREE_UNKN 0x0200U
#endif
#define PNG_FREE_PLTE 0x1000U
#define PNG_FREE_TRNS 0x2000U
#define PNG_FREE_TEXT 0x4000U
#define PNG_FREE_EXIF 0x8000U
#define PNG_FREE_ALL  0xffffU
#define PNG_FREE_MUL  0x4220U
PNG_EXPORTA(104, void, png_err, (png_const_structrp png_ptr), PNG_NORETURN);
#  define png_error(s1,s2) png_err(s1)
#  define png_chunk_error(s1,s2) png_err(s1)

#  define png_warning(s1,s2) ((void)(s1))
#  define png_chunk_warning(s1,s2) ((void)(s1))

#  ifdef PNG_ALLOW_BENIGN_ERRORS
#    define png_benign_error png_warning
#    define png_chunk_benign_error png_chunk_warning
#  else
#    define png_benign_error png_error
#    define png_chunk_benign_error png_chunk_error
#  endif
PNG_EXPORT(110, png_uint_32, png_get_valid, (png_const_structrp png_ptr,
    png_const_inforp info_ptr, png_uint_32 flag));
PNG_EXPORT(111, size_t, png_get_rowbytes, (png_const_structrp png_ptr,
    png_const_inforp info_ptr));
PNG_EXPORT(114, png_byte, png_get_channels, (png_const_structrp png_ptr,
    png_const_inforp info_ptr));

PNG_EXPORT(143, png_uint_32, png_get_IHDR, (png_const_structrp png_ptr,
    png_const_inforp info_ptr, png_uint_32 *width, png_uint_32 *height,
    int *bit_depth, int *color_type, int *interlace_method,
    int *compression_method, int *filter_method));

PNG_EXPORT(144, void, png_set_IHDR, (png_const_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
    int color_type, int interlace_method, int compression_method,
    int filter_method));

PNG_EXPORT(151, png_uint_32, png_get_PLTE, (png_const_structrp png_ptr,
   png_inforp info_ptr, png_colorp *palette, int *num_palette));

PNG_EXPORT(152, void, png_set_PLTE, (png_structrp png_ptr,
    png_inforp info_ptr, png_const_colorp palette, int num_palette));
PNG_EXPORT(177, void, png_set_invalid, (png_const_structrp png_ptr,
    png_inforp info_ptr, int mask));

PNG_EXPORT(180, png_const_charp, png_get_copyright,
    (png_const_structrp png_ptr));
PNG_EXPORT(181, png_const_charp, png_get_header_ver,
    (png_const_structrp png_ptr));
PNG_EXPORT(182, png_const_charp, png_get_header_version,
    (png_const_structrp png_ptr));
PNG_EXPORT(183, png_const_charp, png_get_libpng_ver,
    (png_const_structrp png_ptr));
#define PNG_HANDLE_CHUNK_AS_DEFAULT   0
#define PNG_HANDLE_CHUNK_NEVER        1
#define PNG_HANDLE_CHUNK_IF_SAFE      2
#define PNG_HANDLE_CHUNK_ALWAYS       3
#define PNG_HANDLE_CHUNK_LAST         4
#define PNG_INTERLACE_ADAM7_PASSES 7
#define PNG_PASS_START_ROW(pass) (((1&~(pass))<<(3-((pass)>>1)))&7)
#define PNG_PASS_START_COL(pass) (((1& (pass))<<(3-(((pass)+1)>>1)))&7)
#define PNG_PASS_ROW_OFFSET(pass) ((pass)>2?(8>>(((pass)-1)>>1)):8)
#define PNG_PASS_COL_OFFSET(pass) (1<<((7-(pass))>>1))
#define PNG_PASS_ROW_SHIFT(pass) ((pass)>2?(8-(pass))>>1:3)
#define PNG_PASS_COL_SHIFT(pass) ((pass)>1?(7-(pass))>>1:3)
#define PNG_PASS_ROWS(height, pass) (((height)+(((1<<PNG_PASS_ROW_SHIFT(pass))\
   -1)-PNG_PASS_START_ROW(pass)))>>PNG_PASS_ROW_SHIFT(pass))
#define PNG_PASS_COLS(width, pass) (((width)+(((1<<PNG_PASS_COL_SHIFT(pass))\
   -1)-PNG_PASS_START_COL(pass)))>>PNG_PASS_COL_SHIFT(pass))
#define PNG_ROW_FROM_PASS_ROW(y_in, pass) \
   (((y_in)<<PNG_PASS_ROW_SHIFT(pass))+PNG_PASS_START_ROW(pass))
#define PNG_COL_FROM_PASS_COL(x_in, pass) \
   (((x_in)<<PNG_PASS_COL_SHIFT(pass))+PNG_PASS_START_COL(pass))
#define PNG_PASS_MASK(pass,off) ( \
   ((0x110145AF>>(((7-(off))-(pass))<<2)) & 0xF) | \
   ((0x01145AF0>>(((7-(off))-(pass))<<2)) & 0xF0))

#define PNG_ROW_IN_INTERLACE_PASS(y, pass) \
   ((PNG_PASS_MASK(pass,0) >> ((y)&7)) & 1)
#define PNG_COL_IN_INTERLACE_PASS(x, pass) \
   ((PNG_PASS_MASK(pass,1) >> ((x)&7)) & 1)

#  define png_composite(composite, fg, alpha, bg)                      \
   (composite) =                                                       \
       (png_byte)(0xff & (((png_uint_16)(fg) * (png_uint_16)(alpha) +  \
       (png_uint_16)(bg) * (png_uint_16)(255 - (png_uint_16)(alpha)) + \
       127) / 255))

#  define png_composite_16(composite, fg, alpha, bg)                       \
   (composite) =                                                           \
       (png_uint_16)(0xffff & (((png_uint_32)(fg) * (png_uint_32)(alpha) + \
       (png_uint_32)(bg)*(png_uint_32)(65535 - (png_uint_32)(alpha)) +     \
       32767) / 65535))

PNG_EXPORT(204, png_uint_32, png_get_uint_31, (png_const_structrp png_ptr,
    png_const_bytep buf));
#ifdef PNG_WRITE_INT_FUNCTIONS_SUPPORTED
PNG_EXPORT(205, void, png_save_uint_32, (png_bytep buf, png_uint_32 i));
#endif
#ifdef PNG_SAVE_INT_32_SUPPORTED
PNG_EXPORT(206, void, png_save_int_32, (png_bytep buf, png_int_32 i));
#endif
#ifdef PNG_WRITE_INT_FUNCTIONS_SUPPORTED
PNG_EXPORT(207, void, png_save_uint_16, (png_bytep buf, unsigned int i));
#endif

#ifdef PNG_USE_READ_MACROS
#  define PNG_get_uint_32(buf) \
   (((png_uint_32)(*(buf)) << 24) + \
    ((png_uint_32)(*((buf) + 1)) << 16) + \
    ((png_uint_32)(*((buf) + 2)) << 8) + \
    ((png_uint_32)(*((buf) + 3))))
#  define PNG_get_uint_16(buf) \
   ((png_uint_16) \
    (((unsigned int)(*(buf)) << 8) + \
    ((unsigned int)(*((buf) + 1)))))

#  define PNG_get_int_32(buf) \
   ((png_int_32)((*(buf) & 0x80) \
    ? -((png_int_32)(((png_get_uint_32(buf)^0xffffffffU)+1U)&0x7fffffffU)) \
    : (png_int_32)png_get_uint_32(buf)))
#  ifndef PNG_PREFIX
#    define png_get_uint_32(buf) PNG_get_uint_32(buf)
#    define png_get_uint_16(buf) PNG_get_uint_16(buf)
#    define png_get_int_32(buf)  PNG_get_int_32(buf)
#  endif
#else
#  ifdef PNG_PREFIX
#    define PNG_get_uint_32 (png_get_uint_32)
#    define PNG_get_uint_16 (png_get_uint_16)
#    define PNG_get_int_32  (png_get_int_32)
#  endif
#endif
#ifdef PNG_EXPORT_LAST_ORDINAL
  PNG_EXPORT_LAST_ORDINAL(249);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif
