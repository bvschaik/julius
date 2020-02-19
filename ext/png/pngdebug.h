
/* pngdebug.h - Debugging macros for libpng, also used in pngtest.c
 *
 * Copyright (c) 2018 Cosmin Truta
 * Copyright (c) 1998-2002,2004,2006-2013 Glenn Randers-Pehrson
 * Copyright (c) 1996-1997 Andreas Dilger
 * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */
#ifndef PNGDEBUG_H
#define PNGDEBUG_H
#  ifndef PNG_LITERAL_SHARP
#    define PNG_LITERAL_SHARP 0x23
#  endif
#  ifndef PNG_LITERAL_LEFT_SQUARE_BRACKET
#    define PNG_LITERAL_LEFT_SQUARE_BRACKET 0x5b
#  endif
#  ifndef PNG_LITERAL_RIGHT_SQUARE_BRACKET
#    define PNG_LITERAL_RIGHT_SQUARE_BRACKET 0x5d
#  endif
#  ifndef PNG_STRING_NEWLINE
#    define PNG_STRING_NEWLINE "\n"
#  endif

#ifndef png_debug
#  define png_debug(l, m) ((void)0)
#endif
#ifndef png_debug1
#  define png_debug1(l, m, p1) ((void)0)
#endif
#ifndef png_debug2
#  define png_debug2(l, m, p1, p2) ((void)0)
#endif
#endif
