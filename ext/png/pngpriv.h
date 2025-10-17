
/* pngpriv.h - private declarations for use inside libpng
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

#ifndef PNGPRIV_H
#define PNGPRIV_H
#ifndef _POSIX_SOURCE
# define _POSIX_SOURCE 1
#endif

#ifndef PNG_VERSION_INFO_ONLY
#  include <stdlib.h>
#  include <string.h>
#endif

#define PNGLIB_BUILD
#if defined(HAVE_CONFIG_H) && !defined(PNG_NO_CONFIG_H)
#  include <config.h>
#  define PNG_RESTRICT restrict
#endif
#ifndef PNGLCONF_H
#  include "pnglibconf.h"
#endif
#if defined(PNG_PREFIX) && !defined(PNGPREFIX_H)
#  include "pngprefix.h"
#endif

#ifdef PNG_USER_CONFIG
#  include "pngusr.h"
#  ifndef PNG_USER_PRIVATEBUILD
#    define PNG_USER_PRIVATEBUILD "Custom libpng build"
#  endif
#  ifndef PNG_USER_DLLFNAME_POSTFIX
#    define PNG_USER_DLLFNAME_POSTFIX "Cb"
#  endif
#endif

#ifndef PNG_ARM_NEON_OPT
#  if (defined(__ARM_NEON__) || defined(__ARM_NEON)) && \
   defined(PNG_ALIGNED_MEMORY_SUPPORTED)
#     define PNG_ARM_NEON_OPT 2
#  else
#     define PNG_ARM_NEON_OPT 0
#  endif
#endif

#if PNG_ARM_NEON_OPT > 0
#  define PNG_FILTER_OPTIMIZATIONS png_init_filter_functions_neon

#  ifndef PNG_ARM_NEON_IMPLEMENTATION
#     if defined(__ARM_NEON__) || defined(__ARM_NEON)
#        if defined(__clang__)
#        elif defined(__GNUC__)
#           if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6)
#              define PNG_ARM_NEON_IMPLEMENTATION 2
#           endif
#        endif
#     else
#        if !defined(__aarch64__)
#          define PNG_ARM_NEON_IMPLEMENTATION 2
#        endif
#     endif
#  endif

#  ifndef PNG_ARM_NEON_IMPLEMENTATION
#     define PNG_ARM_NEON_IMPLEMENTATION 1
#  endif
#endif

#ifndef PNG_MIPS_MSA_OPT
#  if defined(__mips_msa) && (__mips_isa_rev >= 5) && defined(PNG_ALIGNED_MEMORY_SUPPORTED)
#     define PNG_MIPS_MSA_OPT 2
#  else
#     define PNG_MIPS_MSA_OPT 0
#  endif
#endif

#ifndef PNG_POWERPC_VSX_OPT
#  if defined(__PPC64__) && defined(__ALTIVEC__) && defined(__VSX__)
#     define PNG_POWERPC_VSX_OPT 2
#  else
#     define PNG_POWERPC_VSX_OPT 0
#  endif
#endif

#ifndef PNG_INTEL_SSE_OPT
#   ifdef PNG_INTEL_SSE
#     if defined(__SSE4_1__) || defined(__AVX__) || defined(__SSSE3__) || \
       defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || \
       (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#         define PNG_INTEL_SSE_OPT 1
#      else
#         define PNG_INTEL_SSE_OPT 0
#      endif
#   else
#      define PNG_INTEL_SSE_OPT 0
#   endif
#endif

#if PNG_INTEL_SSE_OPT > 0
#   ifndef PNG_INTEL_SSE_IMPLEMENTATION
#      if defined(__SSE4_1__) || defined(__AVX__)
#         define PNG_INTEL_SSE_IMPLEMENTATION 3
#      elif defined(__SSSE3__)
#         define PNG_INTEL_SSE_IMPLEMENTATION 2
#      elif defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || \
       (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#         define PNG_INTEL_SSE_IMPLEMENTATION 1
#      else
#         define PNG_INTEL_SSE_IMPLEMENTATION 0
#      endif
#   endif

#   if PNG_INTEL_SSE_IMPLEMENTATION > 0
#      define PNG_FILTER_OPTIMIZATIONS png_init_filter_functions_sse2
#   endif
#else
#   define PNG_INTEL_SSE_IMPLEMENTATION 0
#endif

#if PNG_MIPS_MSA_OPT > 0
#  define PNG_FILTER_OPTIMIZATIONS png_init_filter_functions_msa
#  ifndef PNG_MIPS_MSA_IMPLEMENTATION
#     if defined(__mips_msa)
#        if defined(__clang__)
#        elif defined(__GNUC__)
#           if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
#              define PNG_MIPS_MSA_IMPLEMENTATION 2
#           endif
#        endif
#     else
#        define PNG_MIPS_MSA_IMPLEMENTATION 2
#     endif
#  endif

#  ifndef PNG_MIPS_MSA_IMPLEMENTATION
#     define PNG_MIPS_MSA_IMPLEMENTATION 1
#  endif
#endif

#if PNG_POWERPC_VSX_OPT > 0
#  define PNG_FILTER_OPTIMIZATIONS png_init_filter_functions_vsx
#  define PNG_POWERPC_VSX_IMPLEMENTATION 1
#endif


#ifndef PNG_BUILD_DLL
#  ifdef DLL_EXPORT
#     define PNG_BUILD_DLL
#  else
#     ifdef _WINDLL
#        define PNG_BUILD_DLL
#     else
#        ifdef __DLL__
#           define PNG_BUILD_DLL
#        else
#        endif
#     endif
#  endif
#endif
#ifndef PNG_IMPEXP
#  ifdef PNG_BUILD_DLL
#     define PNG_IMPEXP PNG_DLL_EXPORT
#  else
#     define PNG_IMPEXP
#  endif
#endif
#ifndef PNG_DEPRECATED
#  define PNG_DEPRECATED
#endif
#ifndef PNG_PRIVATE
#  define PNG_PRIVATE
#endif
#ifndef PNG_INTERNAL_DATA
#  define PNG_INTERNAL_DATA(type, name, array) PNG_LINKAGE_DATA type name array
#endif

#ifndef PNG_INTERNAL_FUNCTION
#  define PNG_INTERNAL_FUNCTION(type, name, args, attributes)\
      PNG_LINKAGE_FUNCTION PNG_FUNCTION(type, name, args, PNG_EMPTY attributes)
#endif

#ifndef PNG_INTERNAL_CALLBACK
#  define PNG_INTERNAL_CALLBACK(type, name, args, attributes)\
      PNG_LINKAGE_CALLBACK PNG_FUNCTION(type, (PNGCBAPI name), args,\
         PNG_EMPTY attributes)
#endif

#ifndef PNG_FP_EXPORT
#  ifndef PNG_FLOATING_POINT_SUPPORTED
#     define PNG_FP_EXPORT(ordinal, type, name, args)\
         PNG_INTERNAL_FUNCTION(type, name, args, PNG_EMPTY);
#     ifndef PNG_VERSION_INFO_ONLY
         typedef struct png_incomplete png_double;
         typedef png_double*           png_doublep;
         typedef const png_double*     png_const_doublep;
         typedef png_double**          png_doublepp;
#     endif
#  endif
#endif
#ifndef PNG_FIXED_EXPORT
#  ifndef PNG_FIXED_POINT_SUPPORTED
#     define PNG_FIXED_EXPORT(ordinal, type, name, args)\
         PNG_INTERNAL_FUNCTION(type, name, args, PNG_EMPTY);
#  endif
#endif

#include "png.h"
#ifndef PNG_DLL_EXPORT
#  define PNG_DLL_EXPORT
#endif

#ifndef PNG_RELEASE_BUILD
#  define PNG_RELEASE_BUILD (PNG_LIBPNG_BUILD_BASE_TYPE >= PNG_LIBPNG_BUILD_RC)
#endif

#if defined(MAXSEG_64K) && !defined(PNG_MAX_MALLOC_64K)
#  define PNG_MAX_MALLOC_64K
#endif

#ifndef PNG_UNUSED
#  define PNG_UNUSED(param) (void)param;
#endif
#if (PNG_ZBUF_SIZE > 65536L) && defined(PNG_MAX_MALLOC_64K)
#  undef PNG_ZBUF_SIZE
#  define PNG_ZBUF_SIZE 65536L
#endif
#ifdef PNG_WARNINGS_SUPPORTED
#  define PNG_WARNING_PARAMETERS(p) png_warning_parameters p;
#else
#  define png_warning_parameter(p,number,string) ((void)0)
#  define png_warning_parameter_unsigned(p,number,format,value) ((void)0)
#  define png_warning_parameter_signed(p,number,format,value) ((void)0)
#  define png_formatted_warning(pp,p,message) ((void)(pp))
#  define PNG_WARNING_PARAMETERS(p)
#endif
#ifndef PNG_ERROR_TEXT_SUPPORTED
#  define png_fixed_error(s1,s2) png_err(s1)
#endif
#ifdef PNG_FIXED_POINT_SUPPORTED
#  define PNGFAPI PNGAPI
#else
#  define PNGFAPI
#endif

#ifndef PNG_VERSION_INFO_ONLY
#ifdef __cplusplus
#  define png_voidcast(type, value) static_cast<type>(value)
#  define png_constcast(type, value) const_cast<type>(value)
#  define png_aligncast(type, value) \
   static_cast<type>(static_cast<void*>(value))
#  define png_aligncastconst(type, value) \
   static_cast<type>(static_cast<const void*>(value))
#else
#  define png_voidcast(type, value) (value)
#  ifdef _WIN64
#     ifdef __GNUC__
         typedef unsigned long long png_ptruint;
#     else
         typedef unsigned __int64 png_ptruint;
#     endif
#  else
      typedef unsigned long png_ptruint;
#  endif
#  define png_constcast(type, value) ((type)(png_ptruint)(const void*)(value))
#  define png_aligncast(type, value) ((void*)(value))
#  define png_aligncastconst(type, value) ((const void*)(value))
#endif

#if defined(PNG_FLOATING_POINT_SUPPORTED) ||\
    defined(PNG_FLOATING_ARITHMETIC_SUPPORTED)
#  include <float.h>

#  include <math.h>

#  if defined(_AMIGA) && defined(__SASC) && defined(_M68881)
#    include <m68881.h>
#  endif
#endif
#if defined(__TURBOC__) && defined(__MSDOS__)
#  include <mem.h>
#  include <alloc.h>
#endif

#if defined(WIN32) || defined(_Windows) || defined(_WINDOWS) || \
    defined(_WIN32) || defined(__WIN32__)
#  include <windows.h>
#endif
#endif
#ifndef PNG_ABORT
#  ifdef _WINDOWS_
#    define PNG_ABORT() ExitProcess(0)
#  else
#    define PNG_ABORT() abort()
#  endif
#endif
#define PNG_ALIGN_NONE   0
#define PNG_ALIGN_ALWAYS 1
#ifdef offsetof
#  define PNG_ALIGN_OFFSET 2
#else
#  define PNG_ALIGN_OFFSET -1
#endif
#define PNG_ALIGN_SIZE   3

#ifndef PNG_ALIGN_TYPE
#  define PNG_ALIGN_TYPE PNG_ALIGN_SIZE
#endif

#if PNG_ALIGN_TYPE == PNG_ALIGN_SIZE
#  define png_alignof(type) (sizeof (type))
#else
#  if PNG_ALIGN_TYPE == PNG_ALIGN_OFFSET
#     define png_alignof(type) offsetof(struct{char c; type t;}, t)
#  else
#     if PNG_ALIGN_TYPE == PNG_ALIGN_ALWAYS
#        define png_alignof(type) (1)
#     endif
#  endif
#endif
#ifdef png_alignof
#  define png_isaligned(ptr, type)\
   (((type)((const char*)ptr-(const char*)0) & \
   (type)(png_alignof(type)-1)) == 0)
#else
#  define png_isaligned(ptr, type) 0
#endif

#define PNG_HAVE_IDAT               0x04U
#define PNG_HAVE_IEND               0x10U
#define PNG_HAVE_CHUNK_HEADER      0x100U
#define PNG_WROTE_tIME             0x200U
#define PNG_WROTE_INFO_BEFORE_PLTE 0x400U
#define PNG_BACKGROUND_IS_GRAY     0x800U
#define PNG_HAVE_PNG_SIGNATURE    0x1000U
#define PNG_HAVE_CHUNK_AFTER_IDAT 0x2000U
#define PNG_IS_READ_STRUCT        0x8000U
#define PNG_BGR                 0x0001U
#define PNG_INTERLACE           0x0002U
#define PNG_PACK                0x0004U
#define PNG_SHIFT               0x0008U
#define PNG_SWAP_BYTES          0x0010U
#define PNG_INVERT_MONO         0x0020U
#define PNG_QUANTIZE            0x0040U
#define PNG_COMPOSE             0x0080U
#define PNG_BACKGROUND_EXPAND   0x0100U
#define PNG_EXPAND_16           0x0200U
#define PNG_16_TO_8             0x0400U
#define PNG_RGBA                0x0800U
#define PNG_EXPAND              0x1000U
#define PNG_GAMMA               0x2000U
#define PNG_GRAY_TO_RGB         0x4000U
#define PNG_FILLER              0x8000U
#define PNG_PACKSWAP           0x10000U
#define PNG_SWAP_ALPHA         0x20000U
#define PNG_STRIP_ALPHA        0x40000U
#define PNG_INVERT_ALPHA       0x80000U
#define PNG_USER_TRANSFORM    0x100000U
#define PNG_RGB_TO_GRAY_ERR   0x200000U
#define PNG_RGB_TO_GRAY_WARN  0x400000U
#define PNG_RGB_TO_GRAY       0x600000U
#define PNG_ENCODE_ALPHA      0x800000U
#define PNG_ADD_ALPHA        0x1000000U
#define PNG_EXPAND_tRNS      0x2000000U
#define PNG_SCALE_16_TO_8    0x4000000U
#define PNG_STRUCT_PNG   0x0001U
#define PNG_STRUCT_INFO  0x0002U
#define PNG_FLAG_ZLIB_CUSTOM_STRATEGY     0x0001U
#define PNG_FLAG_ZSTREAM_INITIALIZED      0x0002U
#define PNG_FLAG_ZSTREAM_ENDED            0x0008U
#define PNG_FLAG_ROW_INIT                 0x0040U
#define PNG_FLAG_FILLER_AFTER             0x0080U
#define PNG_FLAG_CRC_ANCILLARY_USE        0x0100U
#define PNG_FLAG_CRC_ANCILLARY_NOWARN     0x0200U
#define PNG_FLAG_CRC_CRITICAL_USE         0x0400U
#define PNG_FLAG_CRC_CRITICAL_IGNORE      0x0800U
#define PNG_FLAG_ASSUME_sRGB              0x1000U
#define PNG_FLAG_OPTIMIZE_ALPHA           0x2000U
#define PNG_FLAG_DETECT_UNINITIALIZED     0x4000U
#define PNG_FLAG_LIBRARY_MISMATCH        0x20000U
#define PNG_FLAG_STRIP_ERROR_NUMBERS     0x40000U
#define PNG_FLAG_STRIP_ERROR_TEXT        0x80000U
#define PNG_FLAG_BENIGN_ERRORS_WARN     0x100000U
#define PNG_FLAG_APP_WARNINGS_WARN      0x200000U
#define PNG_FLAG_APP_ERRORS_WARN        0x400000U

#define PNG_FLAG_CRC_ANCILLARY_MASK (PNG_FLAG_CRC_ANCILLARY_USE | \
                                     PNG_FLAG_CRC_ANCILLARY_NOWARN)

#define PNG_FLAG_CRC_CRITICAL_MASK  (PNG_FLAG_CRC_CRITICAL_USE | \
                                     PNG_FLAG_CRC_CRITICAL_IGNORE)

#define PNG_FLAG_CRC_MASK           (PNG_FLAG_CRC_ANCILLARY_MASK | \
                                     PNG_FLAG_CRC_CRITICAL_MASK)

#define PNG_COLOR_DIST(c1, c2) (abs((int)((c1).red) - (int)((c2).red)) + \
   abs((int)((c1).green) - (int)((c2).green)) + \
   abs((int)((c1).blue) - (int)((c2).blue)))

#define PNG_DIV65535(v24) (((v24) + 32895) >> 16)
#define PNG_DIV257(v16) PNG_DIV65535((png_uint_32)(v16) * 255)
#define PNG_ROWBYTES(pixel_bits, width) \
    ((pixel_bits) >= 8 ? \
    ((size_t)(width) * (((size_t)(pixel_bits)) >> 3)) : \
    (( ((size_t)(width) * ((size_t)(pixel_bits))) + 7) >> 3) )

#define PNG_TRAILBITS(pixel_bits, width) \
    (((pixel_bits) * ((width) % (png_uint_32)8)) % 8)

#define PNG_PADBITS(pixel_bits, width) \
    ((8 - PNG_TRAILBITS(pixel_bits, width)) % 8)
#define PNG_OUT_OF_RANGE(value, ideal, delta) \
   ( (value) < (ideal)-(delta) || (value) > (ideal)+(delta) )
#ifdef PNG_FLOATING_POINT_SUPPORTED
#define png_float(png_ptr, fixed, s) (.00001 * (fixed))

#ifdef PNG_FIXED_POINT_MACRO_SUPPORTED
#define png_fixed(png_ptr, fp, s) ((fp) <= 21474 && (fp) >= -21474 ?\
    ((png_fixed_point)(100000 * (fp))) : (png_fixed_error(png_ptr, s),0))
#endif
#endif

#define PNG_32b(b,s) ((png_uint_32)(b) << (s))
#define PNG_U32(b1,b2,b3,b4) \
   (PNG_32b(b1,24) | PNG_32b(b2,16) | PNG_32b(b3,8) | PNG_32b(b4,0))

#define png_IDAT PNG_U32( 73,  68,  65,  84)
#define png_IEND PNG_U32( 73,  69,  78,  68)
#define png_IHDR PNG_U32( 73,  72,  68,  82)
#define png_PLTE PNG_U32( 80,  76,  84,  69)
#define png_bKGD PNG_U32( 98,  75,  71,  68)
#define png_cHRM PNG_U32( 99,  72,  82,  77)
#define png_eXIf PNG_U32(101,  88,  73, 102)
#define png_fRAc PNG_U32(102,  82,  65,  99)
#define png_gAMA PNG_U32(103,  65,  77,  65)
#define png_gIFg PNG_U32(103,  73,  70, 103)
#define png_gIFt PNG_U32(103,  73,  70, 116)
#define png_gIFx PNG_U32(103,  73,  70, 120)
#define png_hIST PNG_U32(104,  73,  83,  84)
#define png_iCCP PNG_U32(105,  67,  67,  80)
#define png_iTXt PNG_U32(105,  84,  88, 116)
#define png_oFFs PNG_U32(111,  70,  70, 115)
#define png_pCAL PNG_U32(112,  67,  65,  76)
#define png_pHYs PNG_U32(112,  72,  89, 115)
#define png_sBIT PNG_U32(115,  66,  73,  84)
#define png_sCAL PNG_U32(115,  67,  65,  76)
#define png_sPLT PNG_U32(115,  80,  76,  84)
#define png_sRGB PNG_U32(115,  82,  71,  66)
#define png_sTER PNG_U32(115,  84,  69,  82)
#define png_tEXt PNG_U32(116,  69,  88, 116)
#define png_tIME PNG_U32(116,  73,  77,  69)
#define png_tRNS PNG_U32(116,  82,  78,  83)
#define png_zTXt PNG_U32(122,  84,  88, 116)
#define PNG_CHUNK_FROM_STRING(s)\
   PNG_U32(0xff & (s)[0], 0xff & (s)[1], 0xff & (s)[2], 0xff & (s)[3])
#define PNG_STRING_FROM_CHUNK(s,c)\
   (void)(((char*)(s))[0]=(char)(((c)>>24) & 0xff), \
   ((char*)(s))[1]=(char)(((c)>>16) & 0xff),\
   ((char*)(s))[2]=(char)(((c)>>8) & 0xff), \
   ((char*)(s))[3]=(char)((c & 0xff)))
#define PNG_CSTRING_FROM_CHUNK(s,c)\
   (void)(PNG_STRING_FROM_CHUNK(s,c), ((char*)(s))[4] = 0)
#define PNG_CHUNK_ANCILLARY(c)   (1 & ((c) >> 29))
#define PNG_CHUNK_CRITICAL(c)     (!PNG_CHUNK_ANCILLARY(c))
#define PNG_CHUNK_PRIVATE(c)      (1 & ((c) >> 21))
#define PNG_CHUNK_RESERVED(c)     (1 & ((c) >> 13))
#define PNG_CHUNK_SAFE_TO_COPY(c) (1 & ((c) >>  5))
#define PNG_GAMMA_MAC_OLD 151724
#define PNG_GAMMA_MAC_INVERSE 65909
#define PNG_GAMMA_sRGB_INVERSE 45455
#ifndef PNG_VERSION_INFO_ONLY

#include "pngstruct.h"
#include "pnginfo.h"
#if PNG_ZLIB_VERNUM != 0 && PNG_ZLIB_VERNUM != ZLIB_VERNUM
#  error ZLIB_VERNUM != PNG_ZLIB_VERNUM \
      "-I (include path) error: see the notes in pngpriv.h"
#endif
typedef const png_uint_16p * png_const_uint_16pp;
#if defined(PNG_SIMPLIFIED_READ_SUPPORTED) ||\
   defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)
#ifdef PNG_SIMPLIFIED_READ_SUPPORTED
PNG_INTERNAL_DATA(const png_uint_16, png_sRGB_table, [256]);
#endif

PNG_INTERNAL_DATA(const png_uint_16, png_sRGB_base, [512]);
PNG_INTERNAL_DATA(const png_byte, png_sRGB_delta, [512]);

#define PNG_sRGB_FROM_LINEAR(linear) \
  ((png_byte)(0xff & ((png_sRGB_base[(linear)>>15] \
   + ((((linear) & 0x7fff)*png_sRGB_delta[(linear)>>15])>>12)) >> 8)))
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define PNG_UNEXPECTED_ZLIB_RETURN (-7)
PNG_INTERNAL_FUNCTION(void, png_zstream_error,(png_structrp png_ptr, int ret),
   PNG_EMPTY);

#ifdef PNG_WRITE_SUPPORTED
PNG_INTERNAL_FUNCTION(void,png_free_buffer_list,(png_structrp png_ptr,
   png_compression_bufferp *list),PNG_EMPTY);
#endif

#if defined(PNG_FLOATING_POINT_SUPPORTED) && \
   !defined(PNG_FIXED_POINT_MACRO_SUPPORTED) && \
   (defined(PNG_gAMA_SUPPORTED) || defined(PNG_cHRM_SUPPORTED) || \
   defined(PNG_sCAL_SUPPORTED) || defined(PNG_READ_BACKGROUND_SUPPORTED) || \
   defined(PNG_READ_RGB_TO_GRAY_SUPPORTED)) || \
   (defined(PNG_sCAL_SUPPORTED) && \
   defined(PNG_FLOATING_ARITHMETIC_SUPPORTED))
PNG_INTERNAL_FUNCTION(png_fixed_point,png_fixed,(png_const_structrp png_ptr,
   double fp, png_const_charp text),PNG_EMPTY);
#endif
PNG_INTERNAL_FUNCTION(int,png_user_version_check,(png_structrp png_ptr,
   png_const_charp user_png_ver),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(png_voidp,png_malloc_base,(png_const_structrp png_ptr,
   png_alloc_size_t size),PNG_ALLOCATED);

#if defined(PNG_TEXT_SUPPORTED) || defined(PNG_sPLT_SUPPORTED) ||\
   defined(PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED)
PNG_INTERNAL_FUNCTION(png_voidp,png_malloc_array,(png_const_structrp png_ptr,
   int nelements, size_t element_size),PNG_ALLOCATED);
PNG_INTERNAL_FUNCTION(png_voidp,png_realloc_array,(png_const_structrp png_ptr,
   png_const_voidp array, int old_elements, int add_elements,
   size_t element_size),PNG_ALLOCATED);
#endif
PNG_INTERNAL_FUNCTION(png_structp,png_create_png_struct,
   (png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn,
    png_error_ptr warn_fn, png_voidp mem_ptr, png_malloc_ptr malloc_fn,
    png_free_ptr free_fn),PNG_ALLOCATED);
PNG_INTERNAL_FUNCTION(void,png_destroy_png_struct,(png_structrp png_ptr),
   PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_free_jmpbuf,(png_structrp png_ptr),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(voidpf,png_zalloc,(voidpf png_ptr, uInt items, uInt size),
   PNG_ALLOCATED);
PNG_INTERNAL_FUNCTION(void,png_zfree,(voidpf png_ptr, voidpf ptr),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void PNGCBAPI,png_default_read_data,(png_structp png_ptr,
    png_bytep data, size_t length),PNG_EMPTY);

#ifdef PNG_PROGRESSIVE_READ_SUPPORTED
PNG_INTERNAL_FUNCTION(void PNGCBAPI,png_push_fill_buffer,(png_structp png_ptr,
    png_bytep buffer, size_t length),PNG_EMPTY);
#endif

PNG_INTERNAL_FUNCTION(void PNGCBAPI,png_default_write_data,(png_structp png_ptr,
    png_bytep data, size_t length),PNG_EMPTY);

#ifdef PNG_WRITE_FLUSH_SUPPORTED
#  ifdef PNG_STDIO_SUPPORTED
PNG_INTERNAL_FUNCTION(void PNGCBAPI,png_default_flush,(png_structp png_ptr),
   PNG_EMPTY);
#  endif
#endif
PNG_INTERNAL_FUNCTION(void,png_reset_crc,(png_structrp png_ptr),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_write_data,(png_structrp png_ptr,
    png_const_bytep data, size_t length),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_sig,(png_structrp png_ptr,
   png_inforp info_ptr),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(png_uint_32,png_read_chunk_header,(png_structrp png_ptr),
   PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_data,(png_structrp png_ptr, png_bytep data,
    size_t length),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_crc_read,(png_structrp png_ptr, png_bytep buf,
    png_uint_32 length),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(int,png_crc_finish,(png_structrp png_ptr,
   png_uint_32 skip),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(int,png_crc_error,(png_structrp png_ptr),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_calculate_crc,(png_structrp png_ptr,
   png_const_bytep ptr, size_t length),PNG_EMPTY);

#ifdef PNG_WRITE_FLUSH_SUPPORTED
PNG_INTERNAL_FUNCTION(void,png_flush,(png_structrp png_ptr),PNG_EMPTY);
#endif
PNG_INTERNAL_FUNCTION(void,png_write_IHDR,(png_structrp png_ptr,
   png_uint_32 width, png_uint_32 height, int bit_depth, int color_type,
   int compression_method, int filter_method, int interlace_method),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_write_PLTE,(png_structrp png_ptr,
   png_const_colorp palette, png_uint_32 num_pal),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_compress_IDAT,(png_structrp png_ptr,
   png_const_bytep row_data, png_alloc_size_t row_data_length, int flush),
   PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_write_IEND,(png_structrp png_ptr),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_write_finish_row,(png_structrp png_ptr),
    PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_write_start_row,(png_structrp png_ptr),
    PNG_EMPTY);

#ifndef PNG_USE_COMPILE_TIME_MASKS
#  define PNG_USE_COMPILE_TIME_MASKS 1
#endif
PNG_INTERNAL_FUNCTION(void,png_combine_row,(png_const_structrp png_ptr,
    png_bytep row, int display),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row,(png_structrp pp, png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row, int filter),PNG_EMPTY);

#if PNG_ARM_NEON_OPT > 0
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_up_neon,(png_row_infop row_info,
    png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub3_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub4_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg3_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg4_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth3_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth4_neon,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
#endif

#if PNG_MIPS_MSA_OPT > 0
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_up_msa,(png_row_infop row_info,
    png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub3_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub4_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg3_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg4_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth3_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth4_msa,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
#endif

#if PNG_POWERPC_VSX_OPT > 0
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_up_vsx,(png_row_infop row_info,
    png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub3_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub4_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg3_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg4_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth3_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth4_vsx,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
#endif

#if PNG_INTEL_SSE_IMPLEMENTATION > 0
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub3_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_sub4_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg3_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_avg4_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth3_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_filter_row_paeth4_sse2,(png_row_infop
    row_info, png_bytep row, png_const_bytep prev_row),PNG_EMPTY);
#endif
PNG_INTERNAL_FUNCTION(void,png_write_find_filter,(png_structrp png_ptr,
    png_row_infop row_info),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_read_start_row,(png_structrp png_ptr),PNG_EMPTY);

#if ZLIB_VERNUM >= 0x1240
PNG_INTERNAL_FUNCTION(int,png_zlib_inflate,(png_structrp png_ptr, int flush),
      PNG_EMPTY);
#  define PNG_INFLATE(pp, flush) png_zlib_inflate(pp, flush)
#else
#  define PNG_INFLATE(pp, flush) inflate(&(pp)->zstream, flush)
#endif
PNG_INTERNAL_FUNCTION(void,png_handle_IHDR,(png_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 length),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_handle_PLTE,(png_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 length),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_handle_IEND,(png_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 length),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_check_chunk_name,(png_const_structrp png_ptr,
    png_uint_32 chunk_name),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_check_chunk_length,(png_const_structrp png_ptr,
    png_uint_32 chunk_length),PNG_EMPTY);

PNG_INTERNAL_FUNCTION(void,png_handle_unknown,(png_structrp png_ptr,
    png_inforp info_ptr, png_uint_32 length, int keep),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(void,png_check_IHDR,(png_const_structrp png_ptr,
    png_uint_32 width, png_uint_32 height, int bit_depth,
    int color_type, int interlace_type, int compression_type,
    int filter_type),PNG_EMPTY);
PNG_INTERNAL_FUNCTION(size_t,png_safecat,(png_charp buffer, size_t bufsize,
   size_t pos, png_const_charp string),PNG_EMPTY);

#  define png_app_warning(pp,s) png_warning(pp,s)
#  define png_app_error(pp,s) png_error(pp,s)

PNG_INTERNAL_FUNCTION(void,png_chunk_report,(png_const_structrp png_ptr,
   png_const_charp message, int error),PNG_EMPTY);
#define PNG_CHUNK_WARNING     0
#define PNG_CHUNK_WRITE_ERROR 1
#define PNG_CHUNK_ERROR       2
#  if PNG_ARM_NEON_OPT > 0
PNG_INTERNAL_FUNCTION(void, png_init_filter_functions_neon,
   (png_structp png_ptr, unsigned int bpp), PNG_EMPTY);
#endif

#if PNG_MIPS_MSA_OPT > 0
PNG_INTERNAL_FUNCTION(void, png_init_filter_functions_msa,
   (png_structp png_ptr, unsigned int bpp), PNG_EMPTY);
#endif

#  if PNG_INTEL_SSE_IMPLEMENTATION > 0
PNG_INTERNAL_FUNCTION(void, png_init_filter_functions_sse2,
   (png_structp png_ptr, unsigned int bpp), PNG_EMPTY);
#  endif

PNG_INTERNAL_FUNCTION(png_uint_32, png_check_keyword, (png_structrp png_ptr,
   png_const_charp key, png_bytep new_key), PNG_EMPTY);

#if PNG_ARM_NEON_IMPLEMENTATION == 1
PNG_INTERNAL_FUNCTION(void,
                      png_riffle_palette_neon,
                      (png_structrp),
                      PNG_EMPTY);
PNG_INTERNAL_FUNCTION(int,
                      png_do_expand_palette_rgba8_neon,
                      (png_structrp,
                       png_row_infop,
                       png_const_bytep,
                       const png_bytepp,
                       const png_bytepp),
                      PNG_EMPTY);
PNG_INTERNAL_FUNCTION(int,
                      png_do_expand_palette_rgb8_neon,
                      (png_structrp,
                       png_row_infop,
                       png_const_bytep,
                       const png_bytepp,
                       const png_bytepp),
                      PNG_EMPTY);
#endif

#include "pngdebug.h"

#ifdef __cplusplus
}
#endif

#endif
#endif
