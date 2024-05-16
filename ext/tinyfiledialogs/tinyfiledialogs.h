/*
Note: This file is a heavily stripped-down version of Tinyfiledialogs, customized for Augustus.

If you are interested in Tinyfiledialogs, please download its original version from the links below.

SPDX-License-Identifier: Zlib
Copyright (c) 2014 - 2024 Guillaume Vareille http://ysengrin.com

********* TINY FILE DIALOGS OFFICIAL WEBSITE IS ON SOURCEFORGE *********
  _________
 /         \ tinyfiledialogs.c v3.18.1 [May 2, 2024] zlib licence
 |tiny file| Unique code file created [November 9, 2014]
 | dialogs |
 \____  ___/ http://tinyfiledialogs.sourceforge.net
	  \|     git clone http://git.code.sf.net/p/tinyfiledialogs/code tinyfd
			  ____________________________________________
			 |                                            |
			 |   email: tinyfiledialogs at ysengrin.com   |
			 |____________________________________________|

If you like tinyfiledialogs, please upvote my stackoverflow answer
https://stackoverflow.com/a/47651444

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

char * tinyfd_selectFolderDialog(
	char const * aTitle, /* NULL or "" */
	char const * aDefaultPath); /* NULL or "" */
		/* returns NULL on cancel */

#endif /* TINYFILEDIALOGS_H */
