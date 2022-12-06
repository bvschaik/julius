/*
Note: This file is a heavily stripped-down version of Tinyfiledialogs, customized for Julius.

If you are interested in Tinyfiledialogs, please download its original version from the links below.

Tinyfiledialogs:
Copyright (c) 2014 - 2018 Guillaume Vareille http://ysengrin.com
http://tinyfiledialogs.sourceforge.net

Thanks for contributions, bug corrections & thorough testing to:
- Don Heyse http://ldglite.sf.net for bug corrections & thorough testing!
- Paul Rouget

- License -

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software.  If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef TINYFILEDIALOGS_H
#define TINYFILEDIALOGS_H

#if defined(_WIN32) || defined(__APPLE__) || defined (__unix__)  && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#define USE_TINYFILEDIALOGS 1
#endif

#ifdef USE_TINYFILEDIALOGS

#ifdef	__cplusplus
extern "C" {
#endif

extern char const tinyfd_needs[]; /* info about requirements */
extern int tinyfd_verbose; /* 0 (default) or 1 : on unix, prints the command line calls */
extern int tinyfd_silent; /* 1 (default) or 0 : on unix,
                          hide errors and warnings from called dialog*/

int tinyfd_messageBox(
	char const * const aTitle , /* NULL or "" */
	char const * const aMessage , /* NULL or "" may contain \n \t */
	char const * const aDialogType , /* "ok" "okcancel" */
	char const * const aIconType , /* "info" "warning" "error" "question" */
	int const aDefaultButton ) ;
		/* 0 for cancel, 1 for ok */

char const * tinyfd_selectFolderDialog(
	char const * const aTitle); /* NULL or "" */
		/* returns NULL on cancel */

#ifdef	__cplusplus
}
#endif

#endif /* USE_TINYFILEDIALOGS */

#endif /* TINYFILEDIALOGS_H */
