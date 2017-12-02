#include "app.hpp"
#include "core/dir.h"
#include "debug/logger.hpp"
#include "core/stringarray.hpp"

#if defined(JULIUS_PLATFORM_UNIX)
#include <unistd.h>
#endif

Application app;

//! Returns the string of the current working directory
std::string Application::workdir()
{
    if (_workdir.empty())
    {
#if defined(JULIUS_PLATFORM_WIN)
        char tmp[_MAX_PATH];
        _getcwd(tmp, _MAX_PATH);
        _d->workingDirectory[type] = utils::replace( tmp, "\\", "/" );
#elif defined(JULIUS_PLATFORM_UNIX)
        // getting the CWD is rather complex as we do not know the size
        // so try it until the call was successful
        // Note that neither the first nor the second parameter may be 0 according to POSIX
        std::array<char, 2048> tmpPath;

        getcwd(tmpPath.data(), tmpPath.size());
        _workdir = tmpPath.data();
#endif //JULIUS_PLATFORM_UNIX
    }

    return _workdir;
}

#if defined(JULIUS_PLATFORM_UNIX) || defined(JULIUS_PLATFORM_HAIKU)
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

#ifdef JULIUS_PLATFORM_LINUX
#include <cstdlib>
#include <string.h>
#elif defined(JULIUS_PLATFORM_MACOSX)
#include <cstdlib>
#elif defined(JULIUS_PLATFORM_WIN)
#include <windows.h>
#endif

const char* getDialogCommand()
{
    if (::system(NULL))
    {
        if(::system("which gdialog") == 0)
            return "gdialog";

        else if (::system("which kdialog") == 0)
            return "kdialog";
    }
    return NULL;
}

void Application::errordlg(std::string title, std::string text)
{
#if defined(JULIUS_PLATFORM_LINUX)
    const char * dialogCommand = getDialogCommand();
    if (dialogCommand)
    {
        std::string command = dialogCommand;
        command += " --title \"" + title + "\" --msgbox \"" + text + "\"";
        int syserror = ::system(command.c_str());
        if( syserror )
        {
            Logger::error( "WARNING: Cant execute command " + command );
        }
    }

    // fail-safe method here, using stdio perhaps, depends on your application
#elif defined(JULIUS_PLATFORM_WIN)
    MessageBox(NULL, text.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
#endif
}

bool Application::Platform::is(int type)
{
    switch( type )
    {
    case platform_windows:
#ifdef JULIUS_PLATFORM_WIN
        return true;
#endif
        break;
    case platform_win32:
#ifdef JULIUS_PLATFORM_WIN32
        return true;
#endif
        break;
    case platform_win64:
#ifdef JULIUS_PLATFORM_WIN64
        return true;
#endif
        break;
    case platform_linux:
#ifdef JULIUS_PLATFORM_LINUX
        return true;
#endif
        break;
    case platform_unix:
#ifdef JULIUS_PLATFORM_UNIX
        return true;
#endif
        break;
    case platform_android:
#ifdef JULIUS_PLATFORM_ANDROID
        return true;
#endif
        break;
    case platform_macos:
#ifdef JULIUS_PLATFORM_MACOSX
        return true;
#endif
        break;
    case platform_bsd:
#ifdef JULIUS_PLATFORM_XBSD
        return true;
#endif
        break;
    case platform_haiku:
#ifdef JULIUS_PLATFORM_HAIKU
        return true;
#endif
        break;
    case platform_beos:
#ifdef JULIUS_PLATFORM_BEOS
        return true;
#endif
        break;
    default:
        break;
    }

    return false;
}

bool Application::Platform::android()
{
    return is( platform_android );
}
bool Application::Platform::linux()
{
    return is( platform_linux );
}
bool Application::Platform::unix()
{
    return is( platform_unix );
}
bool Application::Platform::mac()
{
    return is( platform_macos );
}
bool Application::Platform::windows()
{
    return is( platform_windows );
}
