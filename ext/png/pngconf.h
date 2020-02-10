
/* pngconf.h - machine-configurable file for libpng
 *
 * libpng version 1.6.37
 *
 * Copyright (c) 2018-2019 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2016,2018 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Any machine specific code is near the front of this file, so if you
 * are configuring libpng for a machine, you may want to read the section
 * starting here down to where it starts to typedef png_color, png_text,
 * and png_info.
 */

#ifndef PNGCONF_H
#define PNGCONF_H

#ifndef PNG_BUILDING_SYMBOL_TABLE
#include <limits.h>
#include <stddef.h>

#ifdef PNG_STDIO_SUPPORTED
#  include <stdio.h>
#endif

#ifdef PNG_SETJMP_SUPPORTED
#  include <setjmp.h>
#endif

#ifdef PNG_CONVERT_tIME_SUPPORTED
#  include <time.h>
#endif

#endif
#define PNG_CONST const
#ifndef PNG_READ_INT_FUNCTIONS_SUPPORTED
#  define PNG_USE_READ_MACROS
#endif
#if !defined(PNG_NO_USE_READ_MACROS) && !defined(PNG_USE_READ_MACROS)
#  if PNG_DEFAULT_READ_MACROS
#    define PNG_USE_READ_MACROS
#  endif
#endif
#ifndef PNGARG
#  define PNGARG(arglist) arglist
#endif
#if defined(_Windows) || defined(_WINDOWS) || defined(WIN32) ||\
    defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#  if PNG_API_RULE == 2
#    define PNGCAPI __watcall
#  endif

#  if defined(__GNUC__) || (defined(_MSC_VER) && (_MSC_VER >= 800))
#    define PNGCAPI __cdecl
#    if PNG_API_RULE == 1
#      define PNGAPI __stdcall
#    endif
#  else
#    ifndef PNGCAPI
#      define PNGCAPI _cdecl
#    endif
#    if PNG_API_RULE == 1 && !defined(PNGAPI)
#      define PNGAPI _stdcall
#    endif
#  endif

#  if defined(PNGAPI) && !defined(PNG_USER_PRIVATEBUILD)
#     error "PNG_USER_PRIVATEBUILD must be defined if PNGAPI is changed"
#  endif

#  if (defined(_MSC_VER) && _MSC_VER < 800) ||\
      (defined(__BORLANDC__) && __BORLANDC__ < 0x500)
#    ifndef PNG_EXPORT_TYPE
#      define PNG_EXPORT_TYPE(type) type PNG_IMPEXP
#    endif
#    define PNG_DLL_EXPORT __export
#  else
#    define PNG_DLL_EXPORT __declspec(dllexport)
#    ifndef PNG_DLL_IMPORT
#      define PNG_DLL_IMPORT __declspec(dllimport)
#    endif
#  endif

#else
#  if (defined(__IBMC__) || defined(__IBMCPP__)) && defined(__OS2__)
#    define PNGAPI _System
#  else
#  endif
#endif
#ifndef PNGCAPI
#  define PNGCAPI
#endif
#ifndef PNGCBAPI
#  define PNGCBAPI PNGCAPI
#endif
#ifndef PNGAPI
#  define PNGAPI PNGCAPI
#endif
#ifndef PNG_IMPEXP
#  if defined(PNG_USE_DLL) && defined(PNG_DLL_IMPORT)
#    define PNG_IMPEXP PNG_DLL_IMPORT
#  endif

#  ifndef PNG_IMPEXP
#    define PNG_IMPEXP
#  endif
#endif
#ifndef PNG_FUNCTION
#  define PNG_FUNCTION(type, name, args, attributes) attributes type name args
#endif

#ifndef PNG_EXPORT_TYPE
#  define PNG_EXPORT_TYPE(type) PNG_IMPEXP type
#endif

#ifndef PNG_EXPORTA
#  define PNG_EXPORTA(ordinal, type, name, args, attributes) \
      PNG_FUNCTION(PNG_EXPORT_TYPE(type), (PNGAPI name), PNGARG(args), \
      PNG_LINKAGE_API attributes)
#endif
#define PNG_EMPTY

#define PNG_EXPORT(ordinal, type, name, args) \
   PNG_EXPORTA(ordinal, type, name, args, PNG_EMPTY)
#ifndef PNG_REMOVED
#  define PNG_REMOVED(ordinal, type, name, args, attributes)
#endif

#ifndef PNG_CALLBACK
#  define PNG_CALLBACK(type, name, args) type (PNGCBAPI name) PNGARG(args)
#endif

#ifndef PNG_NO_PEDANTIC_WARNINGS
#  ifndef PNG_PEDANTIC_WARNINGS_SUPPORTED
#    define PNG_PEDANTIC_WARNINGS_SUPPORTED
#  endif
#endif

#ifdef PNG_PEDANTIC_WARNINGS_SUPPORTED
#  if defined(__clang__) && defined(__has_attribute)
#    if !defined(PNG_USE_RESULT) && __has_attribute(__warn_unused_result__)
#      define PNG_USE_RESULT __attribute__((__warn_unused_result__))
#    endif
#    if !defined(PNG_NORETURN) && __has_attribute(__noreturn__)
#      define PNG_NORETURN __attribute__((__noreturn__))
#    endif
#    if !defined(PNG_ALLOCATED) && __has_attribute(__malloc__)
#      define PNG_ALLOCATED __attribute__((__malloc__))
#    endif
#    if !defined(PNG_DEPRECATED) && __has_attribute(__deprecated__)
#      define PNG_DEPRECATED __attribute__((__deprecated__))
#    endif
#    if !defined(PNG_PRIVATE)
#      ifdef __has_extension
#        if __has_extension(attribute_unavailable_with_message)
#          define PNG_PRIVATE __attribute__((__unavailable__(\
             "This function is not exported by libpng.")))
#        endif
#      endif
#    endif
#    ifndef PNG_RESTRICT
#      define PNG_RESTRICT __restrict
#    endif

#  elif defined(__GNUC__)
#    ifndef PNG_USE_RESULT
#      define PNG_USE_RESULT __attribute__((__warn_unused_result__))
#    endif
#    ifndef PNG_NORETURN
#      define PNG_NORETURN   __attribute__((__noreturn__))
#    endif
#    if __GNUC__ >= 3
#      ifndef PNG_ALLOCATED
#        define PNG_ALLOCATED  __attribute__((__malloc__))
#      endif
#      ifndef PNG_DEPRECATED
#        define PNG_DEPRECATED __attribute__((__deprecated__))
#      endif
#      ifndef PNG_PRIVATE
#        if 0
#          define PNG_PRIVATE \
            __attribute__((warning("This function is not exported by libpng.")))
#        else
#          define PNG_PRIVATE \
            __attribute__((__deprecated__))
#        endif
#      endif
#      if ((__GNUC__ > 3) || !defined(__GNUC_MINOR__) || (__GNUC_MINOR__ >= 1))
#        ifndef PNG_RESTRICT
#          define PNG_RESTRICT __restrict
#        endif
#      endif
#    endif

#  elif defined(_MSC_VER)  && (_MSC_VER >= 1300)
#    ifndef PNG_USE_RESULT
#      define PNG_USE_RESULT
#    endif
#    ifndef PNG_NORETURN
#      define PNG_NORETURN   __declspec(noreturn)
#    endif
#    ifndef PNG_ALLOCATED
#      if (_MSC_VER >= 1400)
#        define PNG_ALLOCATED __declspec(restrict)
#      endif
#    endif
#    ifndef PNG_DEPRECATED
#      define PNG_DEPRECATED __declspec(deprecated)
#    endif
#    ifndef PNG_PRIVATE
#      define PNG_PRIVATE __declspec(deprecated)
#    endif
#    ifndef PNG_RESTRICT
#      if (_MSC_VER >= 1400)
#        define PNG_RESTRICT __restrict
#      endif
#    endif

#  elif defined(__WATCOMC__)
#    ifndef PNG_RESTRICT
#      define PNG_RESTRICT __restrict
#    endif
#  endif
#endif

#ifndef PNG_DEPRECATED
#  define PNG_DEPRECATED
#endif
#ifndef PNG_USE_RESULT
#  define PNG_USE_RESULT
#endif
#ifndef PNG_NORETURN
#  define PNG_NORETURN
#endif
#ifndef PNG_ALLOCATED
#  define PNG_ALLOCATED
#endif
#ifndef PNG_PRIVATE
#  define PNG_PRIVATE
#endif
#ifndef PNG_RESTRICT
#  define PNG_RESTRICT
#endif

#ifndef PNG_FP_EXPORT
#  ifdef PNG_FLOATING_POINT_SUPPORTED
#     define PNG_FP_EXPORT(ordinal, type, name, args)\
         PNG_EXPORT(ordinal, type, name, args);
#  else
#     define PNG_FP_EXPORT(ordinal, type, name, args)
#  endif
#endif
#ifndef PNG_FIXED_EXPORT
#  ifdef PNG_FIXED_POINT_SUPPORTED
#     define PNG_FIXED_EXPORT(ordinal, type, name, args)\
         PNG_EXPORT(ordinal, type, name, args);
#  else
#     define PNG_FIXED_EXPORT(ordinal, type, name, args)
#  endif
#endif

#ifndef PNG_BUILDING_SYMBOL_TABLE
#if CHAR_BIT == 8 && UCHAR_MAX == 255
   typedef unsigned char png_byte;
#else
#  error "libpng requires 8-bit bytes"
#endif

#if INT_MIN == -32768 && INT_MAX == 32767
   typedef int png_int_16;
#elif SHRT_MIN == -32768 && SHRT_MAX == 32767
   typedef short png_int_16;
#else
#  error "libpng requires a signed 16-bit type"
#endif

#if UINT_MAX == 65535
   typedef unsigned int png_uint_16;
#elif USHRT_MAX == 65535
   typedef unsigned short png_uint_16;
#else
#  error "libpng requires an unsigned 16-bit type"
#endif

#if INT_MIN < -2147483646 && INT_MAX > 2147483646
   typedef int png_int_32;
#elif LONG_MIN < -2147483646 && LONG_MAX > 2147483646
   typedef long int png_int_32;
#else
#  error "libpng requires a signed 32-bit (or more) type"
#endif

#if UINT_MAX > 4294967294U
   typedef unsigned int png_uint_32;
#elif ULONG_MAX > 4294967294U
   typedef unsigned long int png_uint_32;
#else
#  error "libpng requires an unsigned 32-bit (or more) type"
#endif
typedef size_t png_size_t;
typedef ptrdiff_t png_ptrdiff_t;
#ifndef PNG_SMALL_SIZE_T
#  if (defined(__TURBOC__) && !defined(__FLAT__)) ||\
   (defined(_MSC_VER) && defined(MAXSEG_64K))
#     define PNG_SMALL_SIZE_T
#  endif
#endif
#ifdef PNG_SMALL_SIZE_T
   typedef png_uint_32 png_alloc_size_t;
#else
   typedef size_t png_alloc_size_t;
#endif
typedef png_int_32 png_fixed_point;
typedef void                  * png_voidp;
typedef const void            * png_const_voidp;
typedef png_byte              * png_bytep;
typedef const png_byte        * png_const_bytep;
typedef png_uint_32           * png_uint_32p;
typedef const png_uint_32     * png_const_uint_32p;
typedef png_int_32            * png_int_32p;
typedef const png_int_32      * png_const_int_32p;
typedef png_uint_16           * png_uint_16p;
typedef const png_uint_16     * png_const_uint_16p;
typedef png_int_16            * png_int_16p;
typedef const png_int_16      * png_const_int_16p;
typedef char                  * png_charp;
typedef const char            * png_const_charp;
typedef png_fixed_point       * png_fixed_point_p;
typedef const png_fixed_point * png_const_fixed_point_p;
typedef size_t                * png_size_tp;
typedef const size_t          * png_const_size_tp;

#ifdef PNG_STDIO_SUPPORTED
typedef FILE            * png_FILE_p;
#endif

#ifdef PNG_FLOATING_POINT_SUPPORTED
typedef double       * png_doublep;
typedef const double * png_const_doublep;
#endif
typedef png_byte        * * png_bytepp;
typedef png_uint_32     * * png_uint_32pp;
typedef png_int_32      * * png_int_32pp;
typedef png_uint_16     * * png_uint_16pp;
typedef png_int_16      * * png_int_16pp;
typedef const char      * * png_const_charpp;
typedef char            * * png_charpp;
typedef png_fixed_point * * png_fixed_point_pp;
#ifdef PNG_FLOATING_POINT_SUPPORTED
typedef double          * * png_doublepp;
#endif
typedef char            * * * png_charppp;

#endif

#endif
