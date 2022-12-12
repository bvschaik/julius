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

#ifndef __sun
#define _POSIX_C_SOURCE 2 /* to accept POSIX 2 in old ANSI C standards */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "tinyfiledialogs.h"

#ifdef USE_TINYFILEDIALOGS

#ifdef _WIN32
#ifdef __BORLANDC__
#define _getch getch
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#include <shlobj.h>
#include <conio.h>
#include <commdlg.h>
#else
#include <limits.h>
#include <unistd.h>
#include <dirent.h> /* on old systems try <sys/dir.h> instead */
#include <termios.h>
#include <sys/utsname.h>
#include <signal.h> /* on old systems try <sys/signal.h> instead */
#endif /* _WIN32 */

#define MAX_PATH_OR_CMD 1024 /* _MAX_PATH or MAX_PATH */
#define MAX_MULTIPLE_FILES 32

int tinyfd_verbose = 0; /* on unix: prints the command line calls */
int tinyfd_silent = 1; /* 1 (default) or 0 : on unix,
                        hide errors and warnings from called dialog*/

#ifdef _WIN32
char const tinyfd_needs[] = "\
 ___________\n\
/           \\ \n\
| tiny file |\n\
|  dialogs  |\n\
\\_____  ____/\n\
      \\|\
\ntiny file dialogs on Windows needs:\
\n   a graphic display\
\nor dialog.exe (enhanced console mode)\
\nor a console for basic input";
#else
char const tinyfd_needs[] = "\
 ___________\n\
/           \\ \n\
| tiny file |\n\
|  dialogs  |\n\
\\_____  ____/\n\
      \\|\
\ntiny file dialogs on UNIX needs:\
\n   applescript\
\nor kdialog\
\nor zenity (or matedialog or qarma)\
\nor python (2 or 3)\
\n + tkinter + python-dbus (optional)\
\nor dialog (opens console if needed)\
\nor xterm + bash\
\n   (opens console for basic input)\
\nor existing console for basic input";
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#pragma warning(disable:4100) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#pragma warning(disable:4706) /* allows usage of strncpy, strcpy, strcat, sprintf, fopen */
#endif

#define RETURN_CACHED_INT(setter) static int result = -1; if (result == -1) { result = setter; } return result;

#ifdef _WIN32

#if !defined(WC_ERR_INVALID_CHARS)
/* undefined prior to Vista, so not yet in MINGW header file */
#define WC_ERR_INVALID_CHARS 0x00000080
#endif

static wchar_t *utf8to16(char const *const utf8)
{
    int charactersNeeded = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, NULL, 0);
    wchar_t *utf16 = (wchar_t *) malloc(charactersNeeded * sizeof(wchar_t));
    int charactersWritten = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, utf16, charactersNeeded);
    if (charactersWritten == 0) {
        free(utf16);
        return NULL;
    }
    return utf16;
}

static char *utf16to8(wchar_t const *const utf16)
{
    int bytesNeeded = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, -1, NULL, 0, NULL, NULL);
    char *utf8 = (char *) malloc(bytesNeeded);
    int bytesWritten = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16, -1, utf8, bytesNeeded, NULL, NULL);
    if (bytesWritten == 0) {
        free(utf8);
        return NULL;
    }
    return utf8;
}

static int dirExists(wchar_t const *const dirPath)
{
    if (!dirPath) {
        return 0;
    }
    size_t dirLen = wcslen(dirPath);
    if (dirLen == 0) {
        return 0;
    } else if (dirLen == 2 && dirPath[1] == L':') {
        return 1;
    }

    struct _stat info;
    int statResult = _wstat(dirPath, &info);
    return statResult == 0 && info.st_mode & S_IFDIR;
}

int tinyfd_messageBox(
    char const *const aTitle, /* NULL or "" */
    char const *const aMessage, /* NULL or ""  may contain \n and \t */
    char const *const aDialogType, /* "ok" "okcancel" */
    char const *const aIconType, /* "info" "warning" "error" "question" */
    int const aDefaultButton) /* 0 for cancel, 1 for ok */
{
    wchar_t *wTitle = utf8to16(aTitle);
    wchar_t *wMessage = utf8to16(aMessage);

    UINT mbCode;
    if (aIconType && strcmp("warning", aIconType) == 0) {
        mbCode = MB_ICONWARNING;
    } else if (aIconType && strcmp("error", aIconType) == 0) {
        mbCode = MB_ICONERROR;
    } else if (aIconType && strcmp("question", aIconType) == 0) {
        mbCode = MB_ICONQUESTION;
    } else {
        mbCode = MB_ICONINFORMATION;
    }

    if (aDialogType && strcmp("okcancel", aDialogType) == 0) {
        mbCode |= MB_OKCANCEL;
        if (aDefaultButton == 0) {
            mbCode |= MB_DEFBUTTON2;
        }
    } else {
        mbCode |= MB_OK;
    }
    mbCode |= MB_TOPMOST;

    int mbResult = MessageBoxW(GetForegroundWindow(), wMessage, wTitle, mbCode);

    free(wTitle);
    free(wMessage);

    if ((aDialogType && strcmp("okcancel", aDialogType)) || (mbResult == IDOK)) {
        return 1;
    } else {
        return 0;
    }
}

static int __stdcall BrowseCallbackProcW(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM) pData);
    }
    return 0;
}

char const *tinyfd_selectFolderDialog(char const *const aTitle) /* NULL or "" */
{
    static char resultBuff[MAX_PATH_OR_CMD];
    static wchar_t wBuff[MAX_PATH_OR_CMD];

    wchar_t *wTitle = utf8to16(aTitle);

    BROWSEINFOW bInfo;
    HRESULT hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    bInfo.hwndOwner = GetForegroundWindow();
    bInfo.pidlRoot = NULL;
    bInfo.pszDisplayName = wBuff;
    bInfo.lpszTitle = wTitle && wcslen(wTitle) ? wTitle : NULL;
    if (hResult == S_OK || hResult == S_FALSE) {
        bInfo.ulFlags = BIF_USENEWUI;
    }
    bInfo.lpfn = BrowseCallbackProcW;
    bInfo.lParam = 0;
    bInfo.iImage = -1;
    bInfo.ulFlags |= BIF_RETURNONLYFSDIRS;

    LPITEMIDLIST lpItem = SHBrowseForFolderW(&bInfo);
    if (lpItem) {
        SHGetPathFromIDListW(lpItem, wBuff);
    }

    if (hResult == S_OK || hResult == S_FALSE) {
        CoUninitialize();
    }

    free(wTitle);

    if (!dirExists(wBuff)) {
        return NULL;
    }

    char *dirPath = utf16to8(wBuff);
    strcpy(resultBuff, dirPath);
    free(dirPath);

    return resultBuff;
}

#else /* unix */

static int gWarningDisplayed = 0;

static char const gTitle[] = "missing software! (we will try basic console input)";

static char gPython2Name[16];
static char gPython3Name[16];

static void replaceSubStr(
    char const *const aSource,
    char const *const aOldSubStr,
    char const *const aNewSubStr,
    char *const aoDestination)
{
    char const *pOccurence;
    char const *p;
    char const *lNewSubStr = "";
    size_t lOldSubLen = strlen(aOldSubStr);

    if (!aSource) {
        *aoDestination = '\0';
        return;
    }
    if (!aOldSubStr) {
        strcpy(aoDestination, aSource);
        return;
    }
    if (aNewSubStr) {
        lNewSubStr = aNewSubStr;
    }
    p = aSource;
    *aoDestination = '\0';
    while ((pOccurence = strstr(p, aOldSubStr)) != NULL) {
        strncat(aoDestination, p, pOccurence - p);
        strcat(aoDestination, lNewSubStr);
        p = pOccurence + lOldSubLen;
    }
    strcat(aoDestination, p);
}

static int dirExists(char const *const dirPath)
{
    if (!dirPath || !strlen(dirPath)) {
        return 0;
    }
    DIR *dir = opendir(dirPath);
    if (!dir) {
        return 0;
    }
    closedir(dir);
    return 1;
}

static int detectPresence(char const *const aExecutable)
{
    char lBuff[MAX_PATH_OR_CMD];
    char lTestedString[MAX_PATH_OR_CMD] = "which ";
    FILE *lIn;

    strcat(lTestedString, aExecutable);
    strcat(lTestedString, " 2>/dev/null ");
    lIn = popen(lTestedString, "r");
    if ((fgets(lBuff, sizeof(lBuff), lIn) != NULL)
                && (!strchr(lBuff, ':'))
                && (strncmp(lBuff, "no ", 3))) { /* present */
        pclose(lIn);
        if (tinyfd_verbose) printf("detectPresence %s %d\n", aExecutable, 1);
        return 1;
    } else {
        pclose(lIn);
        if (tinyfd_verbose) printf("detectPresence %s %d\n", aExecutable, 0);
        return 0;
    }
}


static int tryCommand(char const *const aCommand)
{
    char lBuff[MAX_PATH_OR_CMD];
    FILE *lIn;

    lIn = popen(aCommand, "r");
    if (fgets(lBuff, sizeof(lBuff), lIn) == NULL) { /* present */
        pclose(lIn);
        return 1;
    } else {
        pclose(lIn);
        return 0;
    }
}

static int copyAndDetectPresence(char *aExecutable, char const *const path)
{
    strcpy(aExecutable, path);
    return detectPresence(aExecutable);
}

static int isTerminalRunning(void)
{
    static int lIsTerminalRunning = -1;
    if (lIsTerminalRunning < 0) {
        lIsTerminalRunning = isatty(1);
        if (tinyfd_verbose) printf("isTerminalRunning %d\n", lIsTerminalRunning);
    }
    return lIsTerminalRunning;
}


static char const *terminalName(void)
{
    static char lTerminalName[128] = "*";
    char lShellName[64] = "*";

    if (lTerminalName[0] == '*') {
        if (detectPresence("bash")) {
            strcpy(lShellName, "bash -c "); /*good for basic input*/
        } else {
            strcpy(lTerminalName, "");
            return NULL;
        }

        if (copyAndDetectPresence(lTerminalName, "xterm")) { /*good (small without parameters)*/
            strcat(lTerminalName, " -fa 'DejaVu Sans Mono' -fs 10 -title tinyfiledialogs -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "terminator")) { /*good*/
            strcat(lTerminalName, " -x ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "lxterminal")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "konsole")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "kterm")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "tilix")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "xfce4-terminal")) { /*good*/
            strcat(lTerminalName, " -x ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "mate-terminal")) { /*good*/
            strcat(lTerminalName, " -x ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "Eterm")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "evilvte")) { /*good*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else if (copyAndDetectPresence(lTerminalName, "pterm")) { /*good (only letters)*/
            strcat(lTerminalName, " -e ");
            strcat(lTerminalName, lShellName);
        } else {
            strcpy(lTerminalName, "");
        }
        /* bad: koi rxterm guake tilda vala-terminal qterminal
        aterm Terminal terminology sakura lilyterm weston-terminal
        roxterm termit xvt rxvt mrxvt urxvt */
    }
    if (strlen(lTerminalName)) {
        return lTerminalName;
    } else {
        return NULL;
    }
}

static int xmessagePresent(void)
{
    RETURN_CACHED_INT(detectPresence("xmessage"));
}

static int gxmessagePresent(void)
{
    RETURN_CACHED_INT(detectPresence("gxmessage"));
}

static int gmessagePresent(void)
{
    RETURN_CACHED_INT(detectPresence("gmessage"));
}

static int xdialogPresent(void)
{
    RETURN_CACHED_INT(detectPresence("Xdialog"));
}

static int gdialogPresent(void)
{
    RETURN_CACHED_INT(detectPresence("gdialog"));
}

static int osascriptPresent(void)
{
    RETURN_CACHED_INT(detectPresence("osascript"));
}

static int qarmaPresent(void)
{
    RETURN_CACHED_INT(detectPresence("qarma"));
}

static int matedialogPresent(void)
{
    RETURN_CACHED_INT(detectPresence("matedialog"));
}

static int zenityPresent(void)
{
    RETURN_CACHED_INT(detectPresence("zenity"));
}

static int zenity3Present(void)
{
    static int lZenity3Present = -1;
    char lBuff[MAX_PATH_OR_CMD];
    FILE *lIn;
    int lIntTmp;

    if (lZenity3Present < 0) {
        lZenity3Present = 0;
        if (zenityPresent()) {
            lIn = popen("zenity --version", "r");
            if (fgets(lBuff, sizeof(lBuff), lIn) != NULL) {
                if (atoi(lBuff) >= 3) {
                    lZenity3Present = 3;
                    lIntTmp = atoi(strtok(lBuff, ".") + 2);
                    if (lIntTmp >= 18) {
                        lZenity3Present = 5;
                    } else if (lIntTmp >= 10) {
                        lZenity3Present = 4;
                    }
                } else if ((atoi(lBuff) == 2) && (atoi(strtok(lBuff, ".") + 2) >= 32)) {
                    lZenity3Present = 2;
                }
                if (tinyfd_verbose) printf("zenity %d\n", lZenity3Present);
            }
            pclose(lIn);
        }
    }
    return lZenity3Present;
}

static int kdialogPresent(void)
{
    static int lKdialogPresent = -1;
    char lBuff[MAX_PATH_OR_CMD];
    FILE *lIn;
    char *lDesktop;

    if (lKdialogPresent < 0) {
        if (zenityPresent()) {
            lDesktop = getenv("XDG_SESSION_DESKTOP");
            if (!lDesktop || (strcmp(lDesktop, "KDE") && strcmp(lDesktop, "lxqt"))) {
                lKdialogPresent = 0;
                return lKdialogPresent;
            }
        }

        lKdialogPresent = detectPresence("kdialog");
        if (lKdialogPresent && !getenv("SSH_TTY")) {
            lIn = popen("kdialog --attach 2>&1", "r");
            if (fgets(lBuff, sizeof(lBuff), lIn) != NULL) {
                if (!strstr("Unknown", lBuff)) {
                    lKdialogPresent = 2;
                    if (tinyfd_verbose) printf("kdialog-attach %d\n", lKdialogPresent);
                }
            }
            pclose(lIn);

            if (lKdialogPresent == 2) {
                lKdialogPresent = 1;
                lIn = popen("kdialog --passivepopup 2>&1", "r");
                if (fgets(lBuff, sizeof(lBuff), lIn) != NULL) {
                    if (!strstr("Unknown", lBuff)) {
                        lKdialogPresent = 2;
                        if (tinyfd_verbose) printf("kdialog-popup %d\n", lKdialogPresent);
                    }
                }
                pclose(lIn);
            }
        }
    }
    return lKdialogPresent;
}

static int python2Present(void)
{
    static int lPython2Present = -1;
    int i;

    if (lPython2Present < 0) {
        lPython2Present = 0;
        strcpy(gPython2Name, "python2");
        if (detectPresence(gPython2Name)) {
            lPython2Present = 1;
        } else {
            for (i = 9; i >= 0; i--) {
                sprintf(gPython2Name, "python2.%d", i);
                if (detectPresence(gPython2Name)) {
                    lPython2Present = 1;
                    break;
                }
            }
        }
        if (tinyfd_verbose) printf("lPython2Present %d\n", lPython2Present);
        if (tinyfd_verbose) printf("gPython2Name %s\n", gPython2Name);
    }
    return lPython2Present;
}

static int python3Present(void)
{
    static int lPython3Present = -1;
    int i;

    if (lPython3Present < 0) {
        lPython3Present = 0;
        strcpy(gPython3Name, "python3");
        if (detectPresence(gPython3Name)) {
            lPython3Present = 1;
        } else {
            for (i = 9; i >= 0; i--) {
                sprintf(gPython3Name, "python3.%d", i);
                if (detectPresence(gPython3Name)) {
                    lPython3Present = 1;
                    break;
                }
            }
        }
        if (tinyfd_verbose) printf("lPython3Present %d\n", lPython3Present);
        if (tinyfd_verbose) printf("gPython3Name %s\n", gPython3Name);
    }
    return lPython3Present;
}

static int tkinter2Present(void)
{
    static int lTkinter2Present = -1;
    char lPythonCommand[300];
    char lPythonParams[256] = "-S -c \"try:\n\timport Tkinter;\nexcept:\n\tprint 0;\"";

    if (lTkinter2Present < 0) {
        lTkinter2Present = 0;
        if (python2Present()) {
            sprintf(lPythonCommand, "%s %s", gPython2Name, lPythonParams);
            lTkinter2Present = tryCommand(lPythonCommand);
        }
        if (tinyfd_verbose) printf("lTkinter2Present %d\n", lTkinter2Present);
    }
    return lTkinter2Present;
}

static int tkinter3Present(void)
{
    static int lTkinter3Present = -1;
    char lPythonCommand[300];
    char lPythonParams[256] =
        "-S -c \"try:\n\timport tkinter;\nexcept:\n\tprint(0);\"";

    if (lTkinter3Present < 0) {
        lTkinter3Present = 0;
        if (python3Present()) {
            sprintf(lPythonCommand, "%s %s", gPython3Name, lPythonParams);
            lTkinter3Present = tryCommand(lPythonCommand);
        }
        if (tinyfd_verbose) printf("lTkinter3Present %d\n", lTkinter3Present);
    }
    return lTkinter3Present;
}


int tinyfd_messageBox(
    char const *const aTitle, /* NULL or "" */
    char const *const aMessage, /* NULL or ""  may contain \n and \t */
    char const *const aDialogType, /* "ok" "okcancel" */
    char const *const aIconType, /* "info" "warning" "error" "question" */
    int const aDefaultButton) /* 0 for cancel, 1 for ok */
{
    char lBuff[MAX_PATH_OR_CMD];
    char *lDialogString = NULL;
    char *lpDialogString;
    FILE *lIn;
    int lResult;
    char lChar;
    struct termios infoOri;
    struct termios info;
    size_t lTitleLen;
    size_t lMessageLen;

    lBuff[0] = '\0';

    lTitleLen = aTitle ? strlen(aTitle) : 0;
    lMessageLen = aMessage ? strlen(aMessage) : 0;
    lDialogString = (char *) malloc(MAX_PATH_OR_CMD + lTitleLen + lMessageLen);

    if (osascriptPresent()) {
        strcpy(lDialogString, "osascript ");
        strcat(lDialogString, " -e 'try' -e 'set {vButton} to {button returned} of ( display dialog \"");
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, aMessage);
        }
        strcat(lDialogString, "\" ");
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "with title \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\" ");
        }
        strcat(lDialogString, "with icon ");
        if (aIconType && !strcmp("error", aIconType)) {
            strcat(lDialogString, "stop ");
        } else if (aIconType && !strcmp("warning", aIconType)) {
            strcat(lDialogString, "caution ");
        } else { /* question or info */
            strcat(lDialogString, "note ");
        }
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            if (!aDefaultButton) {
                strcat(lDialogString, "default button \"Cancel\" ");
            }
        } else {
            strcat(lDialogString, "buttons {\"OK\"} ");
            strcat(lDialogString, "default button \"OK\" ");
        }
        strcat(lDialogString, ")' ");

        strcat(lDialogString,
"-e 'if vButton is \"Yes\" then' -e 'return 1'\
 -e 'else if vButton is \"OK\" then' -e 'return 1'\
 -e 'else if vButton is \"No\" then' -e 'return 2'\
 -e 'else' -e 'return 0' -e 'end if' ");

        strcat(lDialogString, "-e 'on error number -128' ");
        strcat(lDialogString, "-e '0' ");

        strcat(lDialogString, "-e 'end try'");
    } else if (kdialogPresent()) {
        strcpy(lDialogString, "kdialog");
        strcat(lDialogString, " --");
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            if (aIconType && (!strcmp("warning", aIconType) || !strcmp("error", aIconType))) {
                strcat(lDialogString, "warning");
            }
            strcat(lDialogString, "yesno");
        } else if (aIconType && !strcmp("error", aIconType)) {
            strcat(lDialogString, "error");
        } else if (aIconType && !strcmp("warning", aIconType)) {
            strcat(lDialogString, "sorry");
        } else {
            strcat(lDialogString, "msgbox");
        }
        strcat(lDialogString, " \"");
        if (aMessage) {
            strcat(lDialogString, aMessage);
        }
        strcat(lDialogString, "\"");
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, " --yes-label Ok --no-label Cancel");
        }
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, " --title \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\"");
        }

        strcat(lDialogString, ";if [ $? = 0 ];then echo 1;else echo 0;fi");
    } else if (zenityPresent() || matedialogPresent() || qarmaPresent()) {
        if (zenityPresent()) {
            strcpy(lDialogString, "szAnswer=$(zenity");
        } else if (matedialogPresent()) {
            strcpy(lDialogString, "szAnswer=$(matedialog");
        } else {
            strcpy(lDialogString, "szAnswer=$(qarma");
        }
        strcat(lDialogString, " --");

        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, "question --ok-label=Ok --cancel-label=Cancel");
        } else if (aIconType && !strcmp("error", aIconType)) {
            strcat(lDialogString, "error");
        } else if (aIconType && !strcmp("warning", aIconType)) {
            strcat(lDialogString, "warning");
        } else {
            strcat(lDialogString, "info");
        }
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, " --title=\"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\"");
        }
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, " --no-wrap --text=\"");
            strcat(lDialogString, aMessage);
            strcat(lDialogString, "\"");
        }
        if ((zenity3Present() >= 3) || (!zenityPresent() && qarmaPresent())) {
            strcat(lDialogString, " --icon-name=dialog-");
            if (aIconType && (!strcmp("question", aIconType)
                || !strcmp("error", aIconType)
                || !strcmp("warning", aIconType))) {
                strcat(lDialogString, aIconType);
            } else {
                strcat(lDialogString, "information");
            }
        }

        if (tinyfd_silent) strcat(lDialogString, " 2>/dev/null ");

        strcat(lDialogString, ");if [ $? = 0 ];then echo 1;else echo 0;fi");
    } else if (!gxmessagePresent() && !gmessagePresent() && !gdialogPresent() && !xdialogPresent() && tkinter2Present()) {
        strcpy(lDialogString, gPython2Name);
        strcat(lDialogString, " -S -c \"import Tkinter,tkMessageBox;root=Tkinter.Tk();root.withdraw();");

        strcat(lDialogString, "res=tkMessageBox.");
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, "askokcancel(");
            if (aDefaultButton) {
                strcat(lDialogString, "default=tkMessageBox.OK,");
            } else {
                strcat(lDialogString, "default=tkMessageBox.CANCEL,");
            }
        } else {
            strcat(lDialogString, "showinfo(");
        }

        strcat(lDialogString, "icon='");
        if (aIconType && (!strcmp("question", aIconType)
            || !strcmp("error", aIconType)
            || !strcmp("warning", aIconType))) {
            strcat(lDialogString, aIconType);
        } else {
            strcat(lDialogString, "info");
        }

        strcat(lDialogString, "',");
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "title='");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "',");
        }
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, "message='");
            lpDialogString = lDialogString + strlen(lDialogString);
            replaceSubStr(aMessage, "\n", "\\n", lpDialogString);
            strcat(lDialogString, "'");
        }

        strcat(lDialogString, ");\n\
if res is False :\n\tprint 0\n\
else :\n\tprint 1\n\"");
    } else if (!gxmessagePresent() && !gmessagePresent() && !gdialogPresent() && !xdialogPresent() && tkinter3Present()) {
        strcpy(lDialogString, gPython3Name);
        strcat(lDialogString,
                " -S -c \"import tkinter;from tkinter import messagebox;root=tkinter.Tk();root.withdraw();");

        strcat(lDialogString, "res=messagebox.");
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, "askokcancel(");
            if (aDefaultButton) {
                strcat(lDialogString, "default=messagebox.OK,");
            } else {
                strcat(lDialogString, "default=messagebox.CANCEL,");
            }
        } else {
            strcat(lDialogString, "showinfo(");
        }

        strcat(lDialogString, "icon='");
        if (aIconType && (!strcmp("question", aIconType)
            || !strcmp("error", aIconType)
            || !strcmp("warning", aIconType))) {
            strcat(lDialogString, aIconType);
        } else {
            strcat(lDialogString, "info");
        }

        strcat(lDialogString, "',");
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "title='");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "',");
        }
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, "message='");
            lpDialogString = lDialogString + strlen(lDialogString);
            replaceSubStr(aMessage, "\n", "\\n", lpDialogString);
            strcat(lDialogString, "'");
        }

        strcat(lDialogString, ");\n\
if res is False :\n\tprint(0)\n\
else :\n\tprint(1)\n\"");
    } else if (gxmessagePresent() || gmessagePresent() || (!gdialogPresent() && !xdialogPresent() && xmessagePresent())) {
        if (gxmessagePresent()) {
            strcpy(lDialogString, "gxmessage");
        } else if (gmessagePresent()) {
            strcpy(lDialogString, "gmessage");
        } else {
            strcpy(lDialogString, "xmessage");
        }

        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, " -buttons Ok:1,Cancel:0");
            switch (aDefaultButton) {
                case 1: strcat(lDialogString, " -default Ok"); break;
                case 0: strcat(lDialogString, " -default Cancel"); break;
            }
        } else {
            strcat(lDialogString, " -buttons Ok:1");
            strcat(lDialogString, " -default Ok");
        }

        strcat(lDialogString, " -center \"");
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, aMessage);
        }
        strcat(lDialogString, "\"");
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, " -title  \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\"");
        }
        strcat(lDialogString, " ; echo $? ");
    } else if (xdialogPresent() || gdialogPresent()) {
        if (gdialogPresent()) {
            strcpy(lDialogString, "(gdialog ");
        } else if (xdialogPresent()) {
            strcpy(lDialogString, "(Xdialog ");
        }

        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "--title \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\" ");
        }

        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            if (!aDefaultButton) {
                strcat(lDialogString, "--defaultno ");
            }
            strcat(lDialogString, "--yes-label \"Ok\" --no-label \"Cancel\" --yesno ");
        } else {
            strcat(lDialogString, "--msgbox ");

        }
        strcat(lDialogString, "\"");
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, aMessage);
        }
        strcat(lDialogString, "\" ");

        strcat(lDialogString, "10 60 ) 2>&1;if [ $? = 0 ];then echo 1;else echo 0;fi");
    } else if (!isTerminalRunning() && terminalName()) {
        strcpy(lDialogString, terminalName());
        strcat(lDialogString, "'");
        if (!gWarningDisplayed) {
            gWarningDisplayed = 1;
            strcat(lDialogString, "echo \"");
            strcat(lDialogString, gTitle);
            strcat(lDialogString, "\";");
            strcat(lDialogString, "echo \"");
            strcat(lDialogString, tinyfd_needs);
            strcat(lDialogString, "\";echo;echo;");
        }
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "echo \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\";echo;");
        }
        if (aMessage && strlen(aMessage)) {
            strcat(lDialogString, "echo \"");
            strcat(lDialogString, aMessage);
            strcat(lDialogString, "\"; ");
        }
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            strcat(lDialogString, "echo -n \"[O]kay/[C]ancel: \"; ");
            strcat(lDialogString, "stty sane -echo;");
            strcat(lDialogString, "answer=$( while ! head -c 1 | grep -i [oc];do true ;done);");
            strcat(lDialogString, "if echo \"$answer\" | grep -iq \"^o\";then\n");
            strcat(lDialogString, "\techo 1\nelse\n\techo 0\nfi");
        } else {
            strcat(lDialogString, "echo -n \"press enter to continue \"; ");
            strcat(lDialogString, "stty sane -echo;");
            strcat(lDialogString, "answer=$( while ! head -c 1;do true ;done);echo 1");
        }
        strcat(lDialogString, " >/tmp/tinyfd.txt';cat /tmp/tinyfd.txt;rm /tmp/tinyfd.txt");
    } else {
        if (!gWarningDisplayed) {
            gWarningDisplayed = 1;
            printf("\n\n%s\n", gTitle);
            printf("%s\n\n", tinyfd_needs);
        }
        if (aTitle && strlen(aTitle)) {
            printf("\n%s\n", aTitle);
        }

        tcgetattr(0, &infoOri);
        tcgetattr(0, &info);
        info.c_lflag &= ~ICANON;
        info.c_cc[VMIN] = 1;
        info.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &info);
        if (aDialogType && !strcmp("okcancel", aDialogType)) {
            do {
                if (aMessage && strlen(aMessage)) {
                    printf("\n%s\n", aMessage);
                }
                printf("[O]kay/[C]ancel: "); fflush(stdout);
                lChar = tolower(getchar());
                printf("\n\n");
            } while (lChar != 'o' && lChar != 'c');
            lResult = lChar == 'o' ? 1 : 0;
        } else {
            if (aMessage && strlen(aMessage)) {
                printf("\n%s\n\n", aMessage);
            }
            printf("press enter to continue "); fflush(stdout);
            getchar();
            printf("\n\n");
            lResult = 1;
        }
        tcsetattr(0, TCSANOW, &infoOri);
        free(lDialogString);
        return lResult;
    }

    if (tinyfd_verbose) printf("lDialogString: %s\n", lDialogString);

    if (!(lIn = popen(lDialogString, "r"))) {
        free(lDialogString);
        return 0;
    }
    while (fgets(lBuff, sizeof(lBuff), lIn) != NULL) {
    }

    pclose(lIn);

    /* printf( "lBuff: %s len: %lu \n" , lBuff , strlen(lBuff) ) ; */
    if (lBuff[strlen(lBuff) - 1] == '\n') {
        lBuff[strlen(lBuff) - 1] = '\0';
    }
    /* printf( "lBuff1: %s len: %lu \n" , lBuff , strlen(lBuff) ) ; */

    lResult = !strcmp(lBuff, "2") ? 2 : !strcmp(lBuff, "1") ? 1 : 0;

    /* printf( "lResult: %d\n" , lResult ) ; */
    free(lDialogString);
    return lResult;
}

/* returns NULL on cancel */
static char const *selectFolderUsingInputBox(char const *const aTitle) /* NULL or "" */
{
    static char lBuff[MAX_PATH_OR_CMD];
    char *lDialogString = NULL;
    FILE *lIn;
    int lResult;
    int lWasBasicXterm = 0;
    struct termios oldt;
    struct termios newt;
    char *lEOF;
    size_t lTitleLen;

    lBuff[0] = '\0';

    lTitleLen = aTitle ? strlen(aTitle) : 0;
    lDialogString = (char *) malloc(MAX_PATH_OR_CMD + lTitleLen);

    if (gxmessagePresent() || gmessagePresent()) {
        if (gxmessagePresent()) {
            strcpy(lDialogString, "szAnswer=$(gxmessage -buttons Ok:1,Cancel:0 -center \"");
        } else {
            strcpy(lDialogString, "szAnswer=$(gmessage -buttons Ok:1,Cancel:0 -center \"");
        }

        strcat(lDialogString, "\"");
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, " -title  \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\" ");
        }
        strcat(lDialogString, " -entrytext \"");
        strcat(lDialogString, "\"");
        strcat(lDialogString, ");echo $?$szAnswer");
    } else if (gdialogPresent() || xdialogPresent()) {
        if (gdialogPresent()) {
            strcpy(lDialogString, "(gdialog ");
        } else if (xdialogPresent()) {
            strcpy(lDialogString, "(Xdialog ");
        }

        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "--title \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\" ");
        }

        strcat(lDialogString, "--inputbox");
        strcat(lDialogString, " \"\" 10 60 ");
        strcat(lDialogString, ") 2>/tmp/tinyfd.txt;\
if [ $? = 0 ];then tinyfdBool=1;else tinyfdBool=0;fi;\
tinyfdRes=$(cat /tmp/tinyfd.txt);echo $tinyfdBool$tinyfdRes");
    } else if (!isTerminalRunning() && terminalName()) {
        lWasBasicXterm = 1;
        strcpy(lDialogString, terminalName());
        strcat(lDialogString, "'");
        if (!gWarningDisplayed) {
            gWarningDisplayed = 1;
            tinyfd_messageBox(gTitle, tinyfd_needs, "ok", "warning", 0);
        }
        if (aTitle && strlen(aTitle)) {
            strcat(lDialogString, "echo \"");
            strcat(lDialogString, aTitle);
            strcat(lDialogString, "\";echo;");
        }

        strcat(lDialogString, "echo \"");
        strcat(lDialogString, "\";read ");
        strcat(lDialogString, "-s ");
        strcat(lDialogString, "-p \"");
        strcat(lDialogString, "(esc+enter to cancel): \" ANSWER ");
        strcat(lDialogString, ";echo 1$ANSWER >/tmp/tinyfd.txt';");
        strcat(lDialogString, "cat -v /tmp/tinyfd.txt");
    } else if (!gWarningDisplayed && !isTerminalRunning() && !terminalName()) {
        gWarningDisplayed = 1;
        tinyfd_messageBox(gTitle, tinyfd_needs, "ok", "warning", 0);
        return NULL;
    } else {
        if (!gWarningDisplayed) {
            gWarningDisplayed = 1;
            tinyfd_messageBox(gTitle, tinyfd_needs, "ok", "warning", 0);
        }
        if (aTitle && strlen(aTitle)) {
            printf("\n%s\n", aTitle);
        }
        printf("(esc+enter to cancel): "); fflush(stdout);
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        lEOF = fgets(lBuff, MAX_PATH_OR_CMD, stdin);
        /* printf("lbuff<%c><%d>\n",lBuff[0],lBuff[0]); */
        if (!lEOF || (lBuff[0] == '\0')) {
            free(lDialogString);
            return NULL;
        }

        if (lBuff[0] == '\n') {
            lEOF = fgets(lBuff, MAX_PATH_OR_CMD, stdin);
            /* printf("lbuff<%c><%d>\n",lBuff[0],lBuff[0]); */
            if (!lEOF || (lBuff[0] == '\0')) {
                free(lDialogString);
                return NULL;
            }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        printf("\n");
        if (strchr(lBuff, 27)) {
            free(lDialogString);
            return NULL;
        }
        if (lBuff[strlen(lBuff) - 1] == '\n') {
            lBuff[strlen(lBuff) - 1] = '\0';
        }
        free(lDialogString);
        return lBuff;
    }

    if (tinyfd_verbose) printf("lDialogString: %s\n", lDialogString);
    lIn = popen(lDialogString, "r");
    if (!lIn) {
        remove("/tmp/tinyfd.txt");
        remove("/tmp/tinyfd0.txt");
        free(lDialogString);
        return NULL;
    }
    while (fgets(lBuff, sizeof(lBuff), lIn) != NULL) {
    }

    pclose(lIn);

    remove("/tmp/tinyfd.txt");
    remove("/tmp/tinyfd0.txt");

    /* printf( "len Buff: %lu\n" , strlen(lBuff) ) ; */
    /* printf( "lBuff0: %s\n" , lBuff ) ; */
    if (lBuff[strlen(lBuff) - 1] == '\n') {
        lBuff[strlen(lBuff) - 1] = '\0';
    }
    /* printf( "lBuff1: %s len: %lu \n" , lBuff , strlen(lBuff) ) ; */
    if (lWasBasicXterm) {
        if (strstr(lBuff, "^[")) { /* esc was pressed */
            free(lDialogString);
            return NULL;
        }
    }

    lResult = strncmp(lBuff, "1", 1) ? 0 : 1;
    /* printf( "lResult: %d \n" , lResult ) ; */
    if (!lResult) {
        free(lDialogString);
        return NULL;
    }
    /* printf( "lBuff+1: %s\n" , lBuff+1 ) ; */
    free(lDialogString);

    return lBuff + 1;
}


char const *tinyfd_selectFolderDialog(char const *const aTitle)
{
    static char resultBuff[MAX_PATH_OR_CMD];
    char dialogString[MAX_PATH_OR_CMD];
    char const *p;
    resultBuff[0] = '\0';

    if (osascriptPresent()) {
        strcpy(dialogString, "osascript ");
        strcat(dialogString, " -e 'try' -e 'POSIX path of ( choose folder ");
        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, "with prompt \"");
            strcat(dialogString, aTitle);
            strcat(dialogString, "\" ");
        }
        strcat(dialogString, ")' ");
        strcat(dialogString, "-e 'on error number -128' ");
        strcat(dialogString, "-e 'end try'");
    } else if (kdialogPresent()) {
        strcpy(dialogString, "kdialog");
        strcat(dialogString, " --getexistingdirectory ");

        strcat(dialogString, "$PWD/");

        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, " --title \"");
            strcat(dialogString, aTitle);
            strcat(dialogString, "\"");
        }
    } else if (zenityPresent() || matedialogPresent() || qarmaPresent()) {
        if (zenityPresent()) {
            strcpy(dialogString, "zenity");
        } else if (matedialogPresent()) {
            strcpy(dialogString, "matedialog");
        } else {
            strcpy(dialogString, "qarma");
        }
        strcat(dialogString, " --file-selection --directory");

        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, " --title=\"");
            strcat(dialogString, aTitle);
            strcat(dialogString, "\"");
        }
        if (tinyfd_silent) strcat(dialogString, " 2>/dev/null ");
    } else if (xdialogPresent()) {
        strcpy(dialogString, "(Xdialog ");
        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, "--title \"");
            strcat(dialogString, aTitle);
            strcat(dialogString, "\" ");
        }
        strcat(dialogString, "--dselect \"./\" 0 60 ) 2>&1 ");
    } else if (tkinter2Present()) {
        strcpy(dialogString, gPython2Name);
        strcat(dialogString, " -S -c \"import Tkinter,tkFileDialog;root=Tkinter.Tk();root.withdraw();");
        strcat(dialogString, "print tkFileDialog.askdirectory(");
        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, "title='");
            strcat(dialogString, aTitle);
            strcat(dialogString, "',");
        }
        strcat(dialogString, ")\"");
    } else if (tkinter3Present()) {
        strcpy(dialogString, gPython3Name);
        strcat(dialogString,
            " -S -c \"import tkinter;from tkinter import filedialog;root=tkinter.Tk();root.withdraw();");
        strcat(dialogString, "print(filedialog.askdirectory(");
        if (aTitle && strlen(aTitle)) {
            strcat(dialogString, "title='");
            strcat(dialogString, aTitle);
            strcat(dialogString, "',");
        }
        strcat(dialogString, ") )\"");
    } else {
        p = selectFolderUsingInputBox(aTitle ? aTitle : "Select folder");
        if (!dirExists(p)) {
            return NULL;
        }
        return p;
    }
    if (tinyfd_verbose) printf("dialogString: %s\n", dialogString);
    FILE *pin = popen(dialogString, "r");
    if (!pin) {
        return NULL;
    }
    while (fgets(resultBuff, sizeof(resultBuff), pin) != NULL) {}
    pclose(pin);
    if (resultBuff[strlen(resultBuff) - 1] == '\n') {
        resultBuff[strlen(resultBuff) - 1] = '\0';
    }
    if (!dirExists(resultBuff)) {
        return NULL;
    }
    return resultBuff;
}

#endif /* _WIN32 */


#ifdef _MSC_VER
#pragma warning(default:4996)
#pragma warning(default:4100)
#pragma warning(default:4706)
#endif

#endif /* USE_TINYFILEDIALOGS */
