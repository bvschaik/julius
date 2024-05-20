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

#include "tinyfiledialogs.h"

#ifndef USE_TINYFILEDIALOGS
char *tinyfd_selectFolderDialog(char const *aTitle, char const *aDefaultPath) { return 0; }
#else

#if defined(__GNUC__) || defined(__clang__)
#ifndef _GNU_SOURCE
 #define _GNU_SOURCE /* used only to resolve symbolic links. Can be commented out */
 #ifndef _POSIX_C_SOURCE
  #ifdef __FreeBSD__
    #define _POSIX_C_SOURCE 199506L /* 199506L is enough for freebsd for realpath() */
  #elif defined(__illumos__) || defined(__solaris__)
    #define _POSIX_C_SOURCE 200112L /* illumos/solaris needs 200112L for realpath() */
  #else
    #define _POSIX_C_SOURCE 2 /* to accept POSIX 2 in old ANSI C standards */
  #endif
 #endif
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
 #ifdef __BORLANDC__
  #define _getch getch
 #endif
 #ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
 #endif
 #include <windows.h>
 #include <shlobj.h>
 #define TINYFD_SLASH "\\"
#else
 #include <dirent.h> /* on old systems try <sys/dir.h> instead */
 #include <sys/utsname.h>
 #include <signal.h> /* on old systems try <sys/signal.h> instead */
 #include <unistd.h>
 #define TINYFD_SLASH "/"
#endif /* _WIN32 */

#define MAX_PATH_OR_CMD 1024 /* _MAX_PATH or MAX_PATH */

#ifdef _MSC_VER
#pragma warning(disable:4996) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#pragma warning(disable:4100) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#pragma warning(disable:4706) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#endif

static int tfd_quoteDetected(char const * aString)
{
	char const * p;

	if (!aString) return 0;

	p = aString;
	if ( strchr(p, '\''))
	{
		return 1;
	}

	if ( strchr(p, '\"'))
	{
		return 1;
	}

	if ( strchr(p, '`'))
	{
		return 1;
	}

	p = aString;
	while ((p = strchr(p, '$')))
	{
		p ++ ;
		if ( ( * p == '(' ) || ( * p == '_' ) || isalpha( * p) ) return 1 ;
	}

	return 0;
}

#ifdef _WIN32

#if !defined(WC_ERR_INVALID_CHARS)
/* undefined prior to Vista, so not yet in MINGW header file */
#define WC_ERR_INVALID_CHARS 0x00000000 /* 0x00000080 for MINGW maybe ? */
#endif

static int sizeUtf16From8(char const * aUtf8string)
{
		return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				aUtf8string, -1, NULL, 0);
}

static int sizeUtf16FromMbcs(char const * aMbcsString)
{
		return MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
				aMbcsString, -1, NULL, 0);
}

static int sizeUtf8(wchar_t const * aUtf16string)
{
		return WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
				aUtf16string, -1, NULL, 0, NULL, NULL);
}

static wchar_t* tinyfd_mbcsTo16(char const* aMbcsString)
{
	static wchar_t* lMbcsString = NULL;
	int lSize;

	free(lMbcsString);
	if (!aMbcsString) { lMbcsString = NULL; return NULL; }
	lSize = sizeUtf16FromMbcs(aMbcsString);
	if (lSize)
	{
		lMbcsString = (wchar_t*) malloc(lSize * sizeof(wchar_t));
		lSize = MultiByteToWideChar(CP_ACP, 0, aMbcsString, -1, lMbcsString, lSize);
	}
	else wcscpy(lMbcsString, L"");
	return lMbcsString;
}

static wchar_t * tinyfd_utf8to16(char const * aUtf8string)
{
		static wchar_t * lUtf16string = NULL;
		int lSize;

		free(lUtf16string);
		if (!aUtf8string) {lUtf16string = NULL; return NULL;}
		lSize = sizeUtf16From8(aUtf8string);
	if (lSize)
	{
		lUtf16string = (wchar_t*) malloc(lSize * sizeof(wchar_t));
		lSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			aUtf8string, -1, lUtf16string, lSize);
		return lUtf16string;
	}
	else
	{
		/* let's try mbcs anyway */
		lUtf16string = NULL;
		return tinyfd_mbcsTo16(aUtf8string);
	}
}

static char * tinyfd_utf16to8(wchar_t const * aUtf16string)
{
		static char * lUtf8string = NULL;
		int lSize;

		free(lUtf8string);
		if (!aUtf16string) { lUtf8string = NULL; return NULL; }
		lSize = sizeUtf8(aUtf16string);
	if (lSize)
	{
		lUtf8string = (char*) malloc(lSize);
		lSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, aUtf16string, -1, lUtf8string, lSize, NULL, NULL);
	}
	else strcpy(lUtf8string, "");
		return lUtf8string;
}

static int dirExists(char const * aDirPath)
{
#if (defined(__MINGW32_MAJOR_VERSION) && !defined(__MINGW64__) && (__MINGW32_MAJOR_VERSION <= 3)) || defined(__BORLANDC__) || defined(__WATCOMC__)
	struct _stat lInfo;
#else
	struct __stat64 lInfo;
#endif
		wchar_t * lTmpWChar;
		int lStatRet;
				size_t lDirLen;

				if (!aDirPath)
						return 0;
				lDirLen = strlen(aDirPath);
				if (!lDirLen)
						return 1;
				if ( (lDirLen == 2) && (aDirPath[1] == ':') )
						return 1;

					lTmpWChar = tinyfd_utf8to16(aDirPath);
#if (defined(__MINGW32_MAJOR_VERSION) && !defined(__MINGW64__) && (__MINGW32_MAJOR_VERSION <= 3)) || defined(__BORLANDC__) || defined(__WATCOMC__)
		lStatRet = _wstat(lTmpWChar, &lInfo);
#else
		lStatRet = _wstat64(lTmpWChar, &lInfo);
#endif
		if (lStatRet != 0)
				return 0;
		else if (lInfo.st_mode & S_IFDIR)
				return 1;
		else
					return 0;
}

static BOOL CALLBACK BrowseCallbackProcW_enum(HWND hWndChild, LPARAM lParam)
{
	wchar_t buf[255];
	(void)lParam;
	GetClassNameW(hWndChild, buf, sizeof(buf));
	if (wcscmp(buf, L"SysTreeView32") == 0)
	{
		HTREEITEM hNode = TreeView_GetSelection(hWndChild);
		TreeView_EnsureVisible(hWndChild, hNode);
		return FALSE;
	}
	return TRUE;
}

static int __stdcall BrowseCallbackProcW(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	(void)lp;
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)pData);
			break;
		case BFFM_SELCHANGED:
			EnumChildWindows(hwnd, BrowseCallbackProcW_enum, 0);
	}
	return 0;
}

static wchar_t * tinyfd_selectFolderDialogW(
		wchar_t const * aTitle, /* NULL or "" */
		wchar_t const * aDefaultPath) /* NULL or "" */
{
		static wchar_t lBuff[MAX_PATH_OR_CMD];
		wchar_t * lRetval;

		BROWSEINFOW bInfo;
		LPITEMIDLIST lpItem;
		HRESULT lHResult;

		lHResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

		bInfo.hwndOwner = GetForegroundWindow();
		bInfo.pidlRoot = NULL;
		bInfo.pszDisplayName = lBuff;
		bInfo.lpszTitle = aTitle && wcslen(aTitle) ? aTitle : NULL;
		if (lHResult == S_OK || lHResult == S_FALSE)
		{
				bInfo.ulFlags = BIF_USENEWUI;
		}
		bInfo.lpfn = BrowseCallbackProcW;
		bInfo.lParam = (LPARAM)aDefaultPath;
		bInfo.iImage = -1;

		lpItem = SHBrowseForFolderW(&bInfo);
		if (!lpItem)
				{
						lRetval = NULL;
				}
				else
		{
				SHGetPathFromIDListW(lpItem, lBuff);
								lRetval = lBuff ;
		}

		if (lHResult == S_OK || lHResult == S_FALSE)
		{
				CoUninitialize();
		}
				return lRetval;
}

static char * selectFolderDialogWinGui(
		char * aoBuff,
		char const * aTitle, /*  NULL or "" */
		char const * aDefaultPath) /* NULL or "" */
{
		wchar_t lTitle[128] = L"";
		wchar_t lDefaultPath[MAX_PATH_OR_CMD] = L"";
		wchar_t * lTmpWChar;
		char * lTmpChar;

		if (aTitle)
		{
				lTmpWChar = tinyfd_utf8to16(aTitle);
				wcscpy(lTitle, lTmpWChar);
		}
		if (aDefaultPath)
		{
				lTmpWChar = tinyfd_utf8to16(aDefaultPath);
				wcscpy(lDefaultPath, lTmpWChar);
		}

		lTmpWChar = tinyfd_selectFolderDialogW(
				lTitle,
				lDefaultPath);

		if (!lTmpWChar)
		{
				return NULL;
		}

		lTmpChar = tinyfd_utf16to8(lTmpWChar);
		strcpy(aoBuff, lTmpChar);

		return aoBuff;
}

char * tinyfd_selectFolderDialog(
		char const * aTitle , /* NULL or "" */
		char const * aDefaultPath ) /* NULL or "" */
{
		static char lBuff[MAX_PATH_OR_CMD];
		char * p = NULL;

		if (tfd_quoteDetected(aTitle)) return tinyfd_selectFolderDialog("INVALID TITLE WITH QUOTES", aDefaultPath);
		if (tfd_quoteDetected(aDefaultPath)) return tinyfd_selectFolderDialog(aTitle, "INVALID DEFAULT_PATH WITH QUOTES");

	if (!getenv("SSH_CLIENT") || getenv("DISPLAY"))
		{
				p = selectFolderDialogWinGui(lBuff, aTitle, aDefaultPath);
		}

		if ( ! p || ! strlen( p ) || ! dirExists( p ) )
		{
				return NULL ;
		}
		return p ;
}

#else /* unix */

static char gPython2Name[16];
static char gPython3Name[16];

static int tfd_isDarwin(void)
{
		static int lsIsDarwin = -1 ;
		struct utsname lUtsname ;
		if ( lsIsDarwin < 0 )
		{
				lsIsDarwin = !uname(&lUtsname) && !strcmp(lUtsname.sysname,"Darwin") ;
		}
		return lsIsDarwin ;
}

static char * getPathWithoutFinalSlash(
		char * aoDestination, /* make sure it is allocated, use _MAX_PATH */
		char const * aSource) /* aoDestination and aSource can be the same */
{
		char const * lTmp ;
		if ( aSource )
		{
				lTmp = strrchr(aSource, '/');
				if (!lTmp)
				{
						lTmp = strrchr(aSource, '\\');
				}
				if (lTmp)
				{
						strncpy(aoDestination, aSource, lTmp - aSource );
						aoDestination[lTmp - aSource] = '\0';
				}
				else
				{
						* aoDestination = '\0';
				}
		}
		else
		{
				* aoDestination = '\0';
		}
		return aoDestination;
}

static void ensureFinalSlash( char * aioString )
{
		if ( aioString && strlen( aioString ) )
		{
				char * lastcar = aioString + strlen( aioString ) - 1 ;
				if ( strncmp( lastcar , TINYFD_SLASH , 1 ) )
				{
						strcat( lastcar , TINYFD_SLASH ) ;
				}
		}
}

static int dirExists( char const * aDirPath )
{
		DIR * lDir ;
		if ( ! aDirPath || ! strlen( aDirPath ) )
				return 0 ;
		lDir = opendir( aDirPath ) ;
		if ( ! lDir )
		{
			return 0 ;
		}
		closedir( lDir ) ;
		return 1 ;
}

static int detectPresence( char const * aExecutable )
{
   char lBuff[MAX_PATH_OR_CMD] ;
   char lTestedString[MAX_PATH_OR_CMD] = "command -v " ;
   FILE * lIn ;
#ifdef _GNU_SOURCE
   char* lAllocatedCharString;
   int lSubstringUndetected;
#endif

   strcat( lTestedString , aExecutable ) ;
   strcat( lTestedString, " 2>/dev/null ");
   lIn = popen( lTestedString , "r" ) ;
   if ( ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
	&& ( ! strchr( lBuff , ':' ) ) && ( strncmp(lBuff, "no ", 3) ) )
   {   /* present */
	  pclose( lIn ) ;

#ifdef _GNU_SOURCE /*to bypass this, just comment out "#define _GNU_SOURCE" at the top of the file*/
	  if ( lBuff[strlen( lBuff ) -1] == '\n' ) lBuff[strlen( lBuff ) -1] = '\0' ;
	  lAllocatedCharString = realpath(lBuff,NULL); /*same as canonicalize_file_name*/
	  lSubstringUndetected = ! strstr(lAllocatedCharString, aExecutable);
	  free(lAllocatedCharString);
	  if (lSubstringUndetected)
	  {
		 return 0;
	  }
#endif /*_GNU_SOURCE*/

	  return 1 ;
   }
   else
   {
	  pclose( lIn ) ;
	  return 0 ;
   }
}

static char * getVersion( char const * aExecutable ) /*version must be first numeral*/
{
		static char lBuff[MAX_PATH_OR_CMD] ;
		char lTestedString[MAX_PATH_OR_CMD] ;
		FILE * lIn ;
		char * lTmp ;

	strcpy( lTestedString , aExecutable ) ;
	strcat( lTestedString , " --version" ) ;

	lIn = popen( lTestedString , "r" ) ;
		lTmp = fgets( lBuff , sizeof( lBuff ) , lIn ) ;
		pclose( lIn ) ;

		lTmp += strcspn(lTmp,"0123456789");
		 /* printf("lTmp:%s\n", lTmp); */
		return lTmp ;
}

static int * getMajorMinorPatch( char const * aExecutable )
{
		static int lArray[3] ;
		char * lTmp ;

		lTmp = (char *) getVersion(aExecutable);
		lArray[0] = atoi( strtok(lTmp," ,.-") ) ;
		/* printf("lArray0 %d\n", lArray[0]); */
		lArray[1] = atoi( strtok(0," ,.-") ) ;
		/* printf("lArray1 %d\n", lArray[1]); */
		lArray[2] = atoi( strtok(0," ,.-") ) ;
		/* printf("lArray2 %d\n", lArray[2]); */

		if ( !lArray[0] && !lArray[1] && !lArray[2] ) return NULL;
		return lArray ;
}

static int tryCommand( char const * aCommand )
{
		char lBuff[MAX_PATH_OR_CMD] ;
		FILE * lIn ;

		lIn = popen( aCommand , "r" ) ;
		if ( fgets( lBuff , sizeof( lBuff ) , lIn ) == NULL )
		{       /* present */
				pclose( lIn ) ;
				return 1 ;
		}
		else
		{
				pclose( lIn ) ;
				return 0 ;
		}

}

static int isTerminalRunning(void)
{
		static int lIsTerminalRunning = -1 ;
		if ( lIsTerminalRunning < 0 )
		{
				lIsTerminalRunning = isatty(1);
		}
		return lIsTerminalRunning;
}

static char * terminalName(void)
{
		static char lTerminalName[128] = "*" ;
		char lShellName[64] = "*" ;
		int * lArray;

		if ( lTerminalName[0] == '*' )
		{
				if ( detectPresence( "bash" ) )
				{
						strcpy(lShellName , "bash -c " ) ; /*good for basic input*/
				}
								else
								{
										strcpy(lTerminalName , "" ) ;
										return NULL ;
								}

				if ( tfd_isDarwin() )
				{
										if ( * strcpy(lTerminalName , "/opt/X11/bin/xterm" )
					  && detectPresence( lTerminalName ) )
						{
								strcat(lTerminalName , " -fa 'DejaVu Sans Mono' -fs 10 -title tinyfiledialogs -e " ) ;
								strcat(lTerminalName , lShellName ) ;
						}
						else
						{
								strcpy(lTerminalName , "" ) ;
						}
				}
				else if ( * strcpy(lTerminalName,"xterm") /*good (small without parameters)*/
						&& detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -fa 'DejaVu Sans Mono' -fs 10 -title tinyfiledialogs -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"terminator") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -x " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"lxterminal") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"konsole") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"kterm") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"tilix") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"xfce4-terminal") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -x " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"mate-terminal") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -x " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"Eterm") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"evilvte") /*good*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else if ( * strcpy(lTerminalName,"pterm") /*good (only letters)*/
						  && detectPresence(lTerminalName) )
				{
						strcat(lTerminalName , " -e " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
								else if ( * strcpy(lTerminalName,"gnome-terminal")
				&& detectPresence(lTerminalName) && (lArray = getMajorMinorPatch(lTerminalName))
								&& ((lArray[0]<3) || (lArray[0]==3 && lArray[1]<=6)) )
				{
						strcat(lTerminalName , " --disable-factory -x " ) ;
						strcat(lTerminalName , lShellName ) ;
				}
				else
				{
						strcpy(lTerminalName , "" ) ;
				}
				/* bad: koi rxterm guake tilda vala-terminal qterminal kgx
				aterm Terminal terminology sakura lilyterm weston-terminal
				roxterm termit xvt rxvt mrxvt urxvt */
		}
		if ( strlen(lTerminalName) )
		{
				return lTerminalName ;
		}
		else
		{
				return NULL ;
		}
}

static int graphicMode(void)
{
		return getenv("DISPLAY")
					|| (tfd_isDarwin() && (!getenv("SSH_TTY") || getenv("DISPLAY") ) ) ;
}

static int xdialogPresent(void)
{
	static int lXdialogPresent = -1 ;
	if ( lXdialogPresent < 0 )
	{
		lXdialogPresent = detectPresence("Xdialog") ;
	}
	return lXdialogPresent && graphicMode( ) ;
}

static int gdialogPresent(void)
{
	static int lGdialoglPresent = -1 ;
	if ( lGdialoglPresent < 0 )
	{
		lGdialoglPresent = detectPresence( "gdialog" ) ;
	}
	return lGdialoglPresent && graphicMode( ) ;
}

static int osascriptPresent(void)
{
	static int lOsascriptPresent = -1 ;
	if ( lOsascriptPresent < 0 )
	{
				lOsascriptPresent = detectPresence( "osascript" ) ;
	}
		return lOsascriptPresent && graphicMode() && !getenv("SSH_TTY") ;
}

static int tfd_qarmaPresent(void)
{
		static int lQarmaPresent = -1 ;
		if ( lQarmaPresent < 0 )
		{
				lQarmaPresent = detectPresence("qarma") ;
		}
		return lQarmaPresent && graphicMode( ) ;
}

static int tfd_matedialogPresent(void)
{
		static int lMatedialogPresent = -1 ;
		if ( lMatedialogPresent < 0 )
		{
				lMatedialogPresent = detectPresence("matedialog") ;
		}
		return lMatedialogPresent && graphicMode( ) ;
}

static int tfd_shellementaryPresent(void)
{
		static int lShellementaryPresent = -1 ;
		if ( lShellementaryPresent < 0 )
		{
				lShellementaryPresent = 0 ; /*detectPresence("shellementary"); shellementary is not ready yet */
		}
		return lShellementaryPresent && graphicMode( ) ;
}

static int tfd_xpropPresent(void)
{
	static int lXpropReady = 0 ;
	static int lXpropDetected = -1 ;
	char lBuff[MAX_PATH_OR_CMD] ;
	FILE * lIn ;

	if ( lXpropDetected < 0 )
	{
		lXpropDetected = detectPresence("xprop") ;
	}

	if ( !lXpropReady && lXpropDetected )
	{	/* xwayland Debian issue reported by Kay F. Jahnke and solved with his help */
		lIn = popen( "xprop -root 32x '	$0' _NET_ACTIVE_WINDOW" , "r" ) ;
		if ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
		{
			if ( ! strstr( lBuff , "not found" ) )
			{
				lXpropReady = 1 ;
			}
		}
		pclose( lIn ) ;
	}
	return graphicMode() ? lXpropReady : 0 ;
}

static int tfd_zenityPresent(void)
{
		static int lZenityPresent = -1 ;
		if ( lZenityPresent < 0 )
		{
				lZenityPresent = detectPresence("zenity") ;
		}
		return lZenityPresent && graphicMode( ) ;
}

static int tfd_yadPresent(void)
{
   static int lYadPresent = -1;
   if (lYadPresent < 0)
   {
	  lYadPresent = detectPresence("yad");
   }
   return lYadPresent && graphicMode();
}

static int tfd_zenity3Present(void)
{
		static int lZenity3Present = -1 ;
		char lBuff[MAX_PATH_OR_CMD] ;
		FILE * lIn ;
				int lIntTmp ;

		if ( lZenity3Present < 0 )
		{
				lZenity3Present = 0 ;
				if ( tfd_zenityPresent() )
				{
						lIn = popen( "zenity --version" , "r" ) ;
						if ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
						{
								if ( atoi(lBuff) >= 3 )
								{
									lZenity3Present = 3 ;
									lIntTmp = atoi(strtok(lBuff,".")+2 ) ;
									if ( lIntTmp >= 18 )
									{
											lZenity3Present = 5 ;
									}
									else if ( lIntTmp >= 10 )
									{
											lZenity3Present = 4 ;
									}
																}
								else if ( ( atoi(lBuff) == 2 ) && ( atoi(strtok(lBuff,".")+2 ) >= 32 ) )
								{
										lZenity3Present = 2 ;
								}
						}
						pclose( lIn ) ;
				}
		}
		return graphicMode() ? lZenity3Present : 0 ;
}

static int tfd_kdialogPresent(void)
{
    static int lKdialogPresent = -1 ;
    char lBuff[MAX_PATH_OR_CMD] ;
    FILE * lIn ;
    char * lDesktop;

    if ( lKdialogPresent < 0 )
    {
        lDesktop = getenv("XDG_SESSION_DESKTOP");
        if ( !lDesktop  || ( strcmp(lDesktop, "KDE") && strcmp(lDesktop, "lxqt") ) )
        {
            if ( tfd_zenityPresent() )
            {
                lKdialogPresent = 0 ;
                return lKdialogPresent ;
            }
        }

        lKdialogPresent = detectPresence("kdialog") ;
        if ( lKdialogPresent && !getenv("SSH_TTY") )
        {
            lIn = popen( "kdialog --attach 2>&1" , "r" ) ;
            if ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
            {
                if ( ! strstr( "Unknown" , lBuff ) )
                {
                    lKdialogPresent = 2 ;
                }
            }
            pclose( lIn ) ;

            if (lKdialogPresent == 2)
            {
                lKdialogPresent = 1 ;
                lIn = popen( "kdialog --passivepopup 2>&1" , "r" ) ;
                if ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
                {
                    if ( ! strstr( "Unknown" , lBuff ) )
                    {
                        lKdialogPresent = 2 ;
                    }
                }
                pclose( lIn ) ;
            }
        }
    }
    return graphicMode() ? lKdialogPresent : 0 ;
}

static int osx9orBetter(void)
{
		static int lOsx9orBetter = -1 ;
		char lBuff[MAX_PATH_OR_CMD] ;
		FILE * lIn ;
		int V,v;

		if ( lOsx9orBetter < 0 )
		{
				lOsx9orBetter = 0 ;
				lIn = popen( "osascript -e 'set osver to system version of (system info)'" , "r" ) ;
				V = 0 ;
				if ( ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
						&& ( 2 == sscanf(lBuff, "%d.%d", &V, &v) ) )
				{
						V = V * 100 + v;
						if ( V >= 1009 )
						{
								lOsx9orBetter = 1 ;
						}
				}
				pclose( lIn ) ;
		}
		return lOsx9orBetter ;
}

static int python3Present(void)
{
		static int lPython3Present = -1 ;

		if ( lPython3Present < 0 )
		{
				lPython3Present = 0 ;
				strcpy(gPython3Name , "python3" ) ;
				if ( detectPresence(gPython3Name) ) lPython3Present = 1;
		}
				return lPython3Present ;
}

static int python2Present(void)
{
		static int lPython2Present = -1 ;

		if ( lPython2Present < 0 )
		{
				lPython2Present = 0 ;
				strcpy(gPython2Name , "python2" ) ;
				if ( detectPresence(gPython2Name) ) lPython2Present = 1;
		}
		return lPython2Present ;
}

static int tkinter3Present(void)
{
		static int lTkinter3Present = -1 ;
		char lPythonCommand[256];
		char lPythonParams[128] =
				"-S -c \"try:\n\timport tkinter;\nexcept:\n\tprint(0);\"";

		if ( lTkinter3Present < 0 )
		{
				lTkinter3Present = 0 ;
				if ( python3Present() )
				{
						sprintf( lPythonCommand , "%s %s" , gPython3Name , lPythonParams ) ;
						lTkinter3Present = tryCommand(lPythonCommand) ;
				}
		}
				return lTkinter3Present && graphicMode() && !(tfd_isDarwin() && getenv("SSH_TTY") );
}

static int tkinter2Present(void)
{
		static int lTkinter2Present = -1 ;
		char lPythonCommand[256];
		char lPythonParams[128] =
				"-S -c \"try:\n\timport Tkinter;\nexcept:\n\tprint 0;\"";

		if ( lTkinter2Present < 0 )
		{
				lTkinter2Present = 0 ;
				if ( python2Present() )
				{
						sprintf( lPythonCommand , "%s %s" , gPython2Name , lPythonParams ) ;
						lTkinter2Present = tryCommand(lPythonCommand) ;
				}
		}
		return lTkinter2Present && graphicMode() && !(tfd_isDarwin() && getenv("SSH_TTY") );
}

char * tinyfd_selectFolderDialog(
    char const * aTitle , /* "" */
    char const * aDefaultPath ) /* "" */
{
    static char lBuff[MAX_PATH_OR_CMD] ;
	static char lLastDirectory[MAX_PATH_OR_CMD] = "$PWD";

    char lDialogString[MAX_PATH_OR_CMD] ;
    FILE * lIn ;
    int lWasGraphicDialog = 0 ;
    int lWasXterm = 0 ;
    lBuff[0]='\0';

	if (tfd_quoteDetected(aTitle)) return tinyfd_selectFolderDialog("INVALID TITLE WITH QUOTES", aDefaultPath);
	if (tfd_quoteDetected(aDefaultPath)) return tinyfd_selectFolderDialog(aTitle, "INVALID DEFAULT_PATH WITH QUOTES");

	char *realPath = NULL;

	if (aDefaultPath && strlen(aDefaultPath)) realPath = realpath(aDefaultPath, NULL);

		if ( osascriptPresent( ))
		{
				strcpy( lDialogString , "osascript ");
				if ( ! osx9orBetter() ) strcat( lDialogString , " -e 'tell application \"System Events\"' -e 'Activate'");
				strcat( lDialogString , " -e 'try' -e 'POSIX path of ( choose folder ");
				if ( aTitle && strlen(aTitle) )
				{
				strcat(lDialogString, "with prompt \"") ;
				strcat(lDialogString, aTitle) ;
				strcat(lDialogString, "\" ") ;
				}
				if ( realPath && strlen(realPath) )
				{
						strcat(lDialogString, "default location \"") ;
						strcat(lDialogString, realPath ) ;
						strcat(lDialogString , "\" " ) ;
				}
				strcat( lDialogString , ")' " ) ;
				strcat(lDialogString, "-e 'on error number -128' " ) ;
				strcat(lDialogString, "-e 'end try'") ;
				if ( ! osx9orBetter() ) strcat( lDialogString, " -e 'end tell'") ;
		}
		else if ( tfd_kdialogPresent() )
		{
				strcpy( lDialogString , "kdialog" ) ;
								if ( (tfd_kdialogPresent() == 2) && tfd_xpropPresent() )
				{
						strcat(lDialogString, " --attach=$(xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)"); /* contribution: Paul Rouget */
				}
				strcat( lDialogString , " --getexistingdirectory " ) ;

				if ( realPath && strlen(realPath) )
				{
						if ( realPath[0] != '/' )
						{
                            strcat(lDialogString, lLastDirectory) ;
                            strcat(lDialogString , "/" ) ;
						}
						strcat(lDialogString, "\"") ;
						strcat(lDialogString, realPath ) ;
						strcat(lDialogString , "\"" ) ;
				}
				else
				{
                        strcat(lDialogString, lLastDirectory) ;
                        strcat(lDialogString , "/" ) ;
				}

				if ( aTitle && strlen(aTitle) )
				{
						strcat(lDialogString, " --title \"") ;
						strcat(lDialogString, aTitle) ;
						strcat(lDialogString, "\"") ;
				}
		}
		else if ( tfd_zenityPresent() || tfd_matedialogPresent() || tfd_shellementaryPresent() || tfd_qarmaPresent() )
		{
				if ( tfd_zenityPresent() )
				{
						strcpy( lDialogString , "zenity" ) ;
												if ( (tfd_zenity3Present() >= 4) && !getenv("SSH_TTY") && tfd_xpropPresent() )
						{
								strcat( lDialogString, " --attach=$(sleep .01;xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)"); /* contribution: Paul Rouget */
						}
				}
				else if ( tfd_matedialogPresent() )
				{
						strcpy( lDialogString , "matedialog" ) ;
				}
				else if ( tfd_shellementaryPresent() )
				{
						strcpy( lDialogString , "shellementary" ) ;
				}
				else
				{
						strcpy( lDialogString , "qarma" ) ;
												if ( !getenv("SSH_TTY") && tfd_xpropPresent() )
						{
								strcat(lDialogString, " --attach=$(xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)"); /* contribution: Paul Rouget */
						}
				}
				strcat( lDialogString , " --file-selection --directory" ) ;

				strcat(lDialogString, " --title=\"") ;
				if (aTitle && strlen(aTitle)) strcat(lDialogString, aTitle) ;
				strcat(lDialogString, "\"") ;

				if ( realPath && strlen(realPath) )
				{
						strcat(lDialogString, " --filename=\"") ;
						strcat(lDialogString, realPath) ;
						strcat(lDialogString, "\"") ;
				}
				strcat( lDialogString , " 2>/dev/null ");
		}
		else if (tfd_yadPresent())
		{
		   strcpy(lDialogString, "yad --file --directory");
		   if (aTitle && strlen(aTitle))
		   {
			  strcat(lDialogString, " --title=\"");
			  strcat(lDialogString, aTitle);
			  strcat(lDialogString, "\"");
		   }
		   if (realPath && strlen(realPath))
		   {
			  strcat(lDialogString, " --filename=\"");
			  strcat(lDialogString, realPath);
			  strcat(lDialogString, "\"");
		   }
		   strcat(lDialogString, " 2>/dev/null ");
	  }
	  else if ( !xdialogPresent() && tkinter3Present( ) )
				{
						strcpy( lDialogString , gPython3Name ) ;
						strcat( lDialogString ,
								" -S -c \"import tkinter;from tkinter import filedialog;root=tkinter.Tk();root.withdraw();");
						strcat( lDialogString , "res=filedialog.askdirectory(");
						if ( aTitle && strlen(aTitle) )
						{
								strcat(lDialogString, "title='") ;
								strcat(lDialogString, aTitle) ;
								strcat(lDialogString, "',") ;
						}
						if ( realPath && strlen(realPath) )
						{
								strcat(lDialogString, "initialdir='") ;
								strcat(lDialogString, realPath ) ;
								strcat(lDialogString , "'" ) ;
						}
						strcat( lDialogString, ");\nif not isinstance(res, tuple):\n\tprint(res)\n\"" ) ;
				}
				else if ( !xdialogPresent() && tkinter2Present( ) )
		{
								strcpy( lDialogString , "export PYTHONIOENCODING=utf-8;" ) ;
				strcat( lDialogString , gPython2Name ) ;
				if ( ! isTerminalRunning( ) && tfd_isDarwin( ) )
				{
				strcat( lDialogString , " -i" ) ;  /* for osx without console */
				}
		strcat( lDialogString ,
" -S -c \"import Tkinter,tkFileDialog;root=Tkinter.Tk();root.withdraw();");

		if ( tfd_isDarwin( ) )
		{
						strcat( lDialogString ,
"import os;os.system('''/usr/bin/osascript -e 'tell app \\\"Finder\\\" to set \
frontmost of process \\\"Python\\\" to true' ''');");
				}

				strcat( lDialogString , "print tkFileDialog.askdirectory(");
			if ( aTitle && strlen(aTitle) )
			{
						strcat(lDialogString, "title='") ;
						strcat(lDialogString, aTitle) ;
						strcat(lDialogString, "',") ;
			}
		if ( realPath && strlen(realPath) )
		{
								strcat(lDialogString, "initialdir='") ;
								strcat(lDialogString, realPath ) ;
								strcat(lDialogString , "'" ) ;
				}
				strcat( lDialogString , ")\"" ) ;
		}
		else if ( xdialogPresent() )
		{
				if ( xdialogPresent( ) )
				{
						lWasGraphicDialog = 1 ;
						strcpy( lDialogString , "(Xdialog " ) ;
				}
				else if ( isTerminalRunning( ) )
				{
						strcpy( lDialogString , "(dialog " ) ;
				}
				else
				{
						lWasXterm = 1 ;
						strcpy( lDialogString , terminalName() ) ;
						strcat( lDialogString , "'(" ) ;
						strcat( lDialogString , " " ) ;
				}

				if ( aTitle && strlen(aTitle) )
				{
						strcat(lDialogString, "--title \"") ;
						strcat(lDialogString, aTitle) ;
						strcat(lDialogString, "\" ") ;
				}

				if ( !xdialogPresent() && !gdialogPresent() )
				{
						strcat(lDialogString, "--backtitle \"") ;
						strcat(lDialogString,
								"tab: focus | /: populate | spacebar: fill text field | ok: TEXT FIELD ONLY") ;
						strcat(lDialogString, "\" ") ;
				}

				strcat( lDialogString , "--dselect \"" ) ;
				if ( realPath && strlen(realPath) )
				{
						strcat(lDialogString, realPath) ;
						ensureFinalSlash(lDialogString);
				}
				else
				{
						strcat(lDialogString, "./") ;
				}

				if ( lWasGraphicDialog )
				{
						strcat(lDialogString, "\" 0 60 ) 2>&1 ") ;
				}
				else
				{
						strcat(lDialogString, "\" 0 60  >/dev/tty) ") ;
						if ( lWasXterm )
						{
						  strcat( lDialogString ,
								"2>/tmp/tinyfd.txt';cat /tmp/tinyfd.txt;rm /tmp/tinyfd.txt");
						}
						else
						{
								strcat(lDialogString, "2>&1 ; clear >/dev/tty") ;
						}
				}
		}
		else
		{
				free((void *)realPath);
				return NULL ;
		}
	free((void *)realPath);
	if ( ! ( lIn = popen( lDialogString , "r" ) ) )
	{
		return NULL ;
	}
    while ( fgets( lBuff , sizeof( lBuff ) , lIn ) != NULL )
    {}
    pclose( lIn ) ;
	if ( strlen( lBuff ) && lBuff[strlen( lBuff ) -1] == '\n' )
	{
		lBuff[strlen( lBuff ) -1] = '\0' ;
	}
    /* printf( "lBuff: %s\n" , lBuff ) ; */
    if ( ! strlen( lBuff ) || ! dirExists( lBuff ) )
    {
            return NULL ;
    }

	getPathWithoutFinalSlash( lLastDirectory , lBuff ) ;

    return lBuff ;
}

#endif /* _WIN32 */

#ifdef _MSC_VER
#pragma warning(default:4996)
#pragma warning(default:4100)
#pragma warning(default:4706)
#endif

#endif // USE_TINYFILEDIALOGS
