// This file is part of Julius.
//
// Julius is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Julius is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Julius.  If not, see <http://www.gnu.org/licenses/>.
//
// Timo Bingmann from http://idlebox.net/
// Copyright 2012-2017 Dalerank, dalerankn8@gmail.com

#include "stacktrace.hpp"

#include <stdlib.h>
#if !defined(JULIUS_PLATFORM_WIN) && !defined(JULIUS_PLATFORM_ANDROID)
  #include <execinfo.h>
#endif

#if !defined(JULIUS_PLATFORM_HAIKU) && !defined(JULIUS_PLATFORM_ANDROID) && !defined(_MSC_VER)
  #include <cxxabi.h>
#endif

#include "scopedptr.hpp"
#include "bytearray.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "vfs/directory.hpp"
#include "osystem.hpp"
#include <signal.h>

#ifdef JULIUS_PLATFORM_WIN
#include <windows.h>
#include <imagehlp.h>
#endif

namespace internal
{

#ifdef JULIUS_PLATFORM_WIN
/** Convert exception code to human readable string. */
static const char *ExceptionName(DWORD exceptionCode)
{
  switch (exceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION:         return "Access violation";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return "Datatype misalignment";
    case EXCEPTION_BREAKPOINT:               return "Breakpoint";
    case EXCEPTION_SINGLE_STEP:              return "Single step";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "Array bounds exceeded";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return "Float denormal operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "Float divide by zero";
    case EXCEPTION_FLT_INEXACT_RESULT:       return "Float inexact result";
    case EXCEPTION_FLT_INVALID_OPERATION:    return "Float invalid operation";
    case EXCEPTION_FLT_OVERFLOW:             return "Float overflow";
    case EXCEPTION_FLT_STACK_CHECK:          return "Float stack check";
    case EXCEPTION_FLT_UNDERFLOW:            return "Float underflow";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "Integer divide by zero";
    case EXCEPTION_INT_OVERFLOW:             return "Integer overflow";
    case EXCEPTION_PRIV_INSTRUCTION:         return "Privileged instruction";
    case EXCEPTION_IN_PAGE_ERROR:            return "In page error";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return "Illegal instruction";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Noncontinuable exception";
    case EXCEPTION_STACK_OVERFLOW:           return "Stack overflow";
    case EXCEPTION_INVALID_DISPOSITION:      return "Invalid disposition";
    case EXCEPTION_GUARD_PAGE:               return "Guard page";
    case EXCEPTION_INVALID_HANDLE:           return "Invalid handle";
  }
  return "Unknown exception";
}

/** Print out a stacktrace. */
static void stacktrace(LPEXCEPTION_POINTERS e)
{
  /*PIMAGEHLP_SYMBOL pSym;
  STACKFRAME sf;
  HANDLE process, thread;
  DWORD dwModBase, Disp;
  BOOL more = FALSE;
  int count = 0;
  char modname[MAX_PATH];

  pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, 16384);

  ZeroMemory(&sf, sizeof(sf));
  sf.AddrPC.Offset = e->ContextRecord->Eip;
  sf.AddrStack.Offset = e->ContextRecord->Esp;
  sf.AddrFrame.Offset = e->ContextRecord->Ebp;
  sf.AddrPC.Mode = AddrModeFlat;
  sf.AddrStack.Mode = AddrModeFlat;
  sf.AddrFrame.Mode = AddrModeFlat;

  process = GetCurrentProcess();
  thread = GetCurrentThread();

  while(1) {
    more = StackWalk(
      IMAGE_FILE_MACHINE_I386, // TODO: fix this for 64 bit windows?
      process,
      thread,
      &sf,
      e->ContextRecord,
      NULL,
      SymFunctionTableAccess,
      SymGetModuleBase,
      NULL
    );
    if(!more || sf.AddrFrame.Offset == 0) {
      break;
    }

    dwModBase = SymGetModuleBase(process, sf.AddrPC.Offset);

    if(dwModBase) {
      GetModuleFileName((HINSTANCE)dwModBase, modname, MAX_PATH);
    } else {
      strcpy(modname, "Unknown");
    }

    pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pSym->MaxNameLength = MAX_PATH;

    if(SymGetSymFromAddr(process, sf.AddrPC.Offset, &Disp, pSym)) {
      // This is the code path taken on VC if debugging syms are found.
      Logger::warning("(%d) %s(%s+%#0x) [0x%08X]\n", count, modname, pSym->Name, Disp, sf.AddrPC.Offset);
    } else {
      // This is the code path taken on MinGW, and VC if no debugging syms are found.
      Logger::warning("(%d) %s [0x%08X]\n", count, modname, sf.AddrPC.Offset);
    }
    ++count;
  }
  GlobalFree(pSym);*/
}

/** Callback for SymEnumerateModules */
#if _MSC_VER >= 1500
static BOOL CALLBACK enum_modules(PCSTR moduleName, ULONG baseOfDll, PVOID userContext)
#else
static BOOL CALLBACK EnumModules(LPSTR moduleName, DWORD baseOfDll, PVOID userContext)
#endif
{
  Logger::warning("0x%08x\t%s\n", baseOfDll, moduleName);
  return TRUE;
}

/** Called by windows if an exception happens. */
static LONG CALLBACK exception_handler(LPEXCEPTION_POINTERS e)
{
  // Prologue.
  Logger::warning("julius {} has crashed.", GAME_BUILD_NUMBER );
  // Initialize IMAGEHLP.DLL.
  SymInitialize(GetCurrentProcess(), ".", TRUE);

  // Record exception info.
  Logger::warning("Exception: %s (0x%08x)\n", ExceptionName(e->ExceptionRecord->ExceptionCode), e->ExceptionRecord->ExceptionCode);
  Logger::warning("Exception Address: 0x%08x\n", e->ExceptionRecord->ExceptionAddress);

  // Record list of loaded DLLs.
  //Logger::warning("DLL information:\n");
  //SymEnumerateModules(GetCurrentProcess(), EnumModules, NULL);

  // Record stacktrace.
  Logger::warning("Stacktrace:\n");
  Stacktrace(e);

  // Unintialize IMAGEHLP.DLL
  SymCleanup(GetCurrentProcess());

  // Cleanup.
  // FIXME: update closing of demo to new netcode

  // Inform user.
  char dir[MAX_PATH];
  GetCurrentDirectory(sizeof(dir) - 1, dir);
  std::string msg = utils::format( 0xff,
    "julius has crashed.\n\n"
    "A stacktrace has been written to:\n"
    "%s\\stdout.txt", dir);
  OSystem::error( "julius: Unhandled exception", msg );

  // this seems to silently close the application
  return EXCEPTION_EXECUTE_HANDLER;

  // this triggers the microsoft "application has crashed" error dialog
  //return EXCEPTION_CONTINUE_SEARCH;

  // in practice, 100% CPU usage but no continuation of execution
  // (tested segmentation fault and division by zero)
  //return EXCEPTION_CONTINUE_EXECUTION;
}

#endif

void crashHandler_handle_crash(int signum)
{
  switch(signum)
  {
    case SIGABRT: Logger::warning("SIGABRT: abort() called somewhere in the program."); break;
    case SIGSEGV: Logger::warning("SIGSEGV: Illegal memory access."); break;
    case SIGILL: Logger::warning("SIGILL: Executing a malformed instruction. (possibly invalid pointer)"); break;
    case SIGFPE: Logger::warning("SIGFPE: Illegal floating point instruction (possibly division by zero)."); break;
  }

  printstack();
  exit(signum);
}

}//end namespace internal

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void CrashHandler::printstack( bool showMessage, unsigned int starting_frame, unsigned int max_frames )
{
#if !defined(JULIUS_PLATFORM_WIN) && !defined(JULIUS_PLATFORM_ANDROID)
  if (showMessage)
  {
    std::string file = vfs::Directory::applicationDir().getFilePath("stdout.txt").toString();
    std::string msg = fmt::format( "julius has crashed.\n\n"
                                   "A stacktrace has been written to:\n"
                                   "{}", file );
    OSystem::error( "julius: Unhandled exception", msg );
  }

  Logger::warning("Stacktrace::begin :");

  // storage array for stack trace address data
  void* addrlist[max_frames+1];

  // retrieve current stack addresses
  int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

  if (addrlen == 0)
  {
    Logger::warning(" <empty, possibly corrupt>\n");
    return;
  }

  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char** symbollist = backtrace_symbols(addrlist, addrlen);

  // allocate string which will be filled with the demangled function name
  size_t funcnamesize = 256;
  ByteArray funcname(funcnamesize);

  // iterate over the returned symbol lines. skip the first, it is the
  // address of this function.
  for (int i = starting_frame + 1; i < addrlen; i++)
  {
    char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

    // find parentheses and +address offset surrounding the mangled name:
    // ./module(function+0x15c) [0x8048a6d]
    for (char *p = symbollist[i]; *p; ++p)
    {
      if (*p == '(') begin_name = p;
      else if (*p == '+') begin_offset = p;
      else if (*p == ')' && begin_offset)
      {
        end_offset = p;
        break;
      }
    }

    if (begin_name && begin_offset && end_offset
    && begin_name < begin_offset)
    {
      *begin_name++ = '\0';
      *begin_offset++ = '\0';
      *end_offset = '\0';

      // mangled name is now in [begin_name, begin_offset) and caller
      // offset in [begin_offset, end_offset). now apply
      // __cxa_demangle():

      int status;
      char* ret = abi::__cxa_demangle(begin_name, funcname.data(), &funcnamesize, &status);
      if (status == 0)
      {
        funcname = ret; // use possibly realloc()-ed string
        Logger::warning(" {0} : {1}+{2}", symbollist[i], funcname.data(), begin_offset);
      }
      else
      {
        // demangling failed. Output function name as a C function with
        // no arguments.
        Logger::warning(" {0} : {1}+{2}", symbollist[i], begin_name, begin_offset);
      }
    }
    else
    {
      // couldn't parse the line? print the whole line.
      Logger::warning(" {}", symbollist[i]);
    }
  }

  //free(funcname);
  free(symbollist);
#elif defined(JULIUS_PLATFORM_WIN)
  Logger::warning( "Game: stacktrace finished." );
#else
  Logger::warning("Stack trace not available");

#endif // JULIUS_PLATFORM_LINUX
}

/** Install crash handler. */

void CrashHandler::install()
{
#ifdef JULIUS_PLATFORM_WIN
  SetUnhandledExceptionFilter(internal::exception_handler);
#else
  signal( SIGABRT, internal::crashHandler_handle_crash);
  signal( SIGSEGV, internal::crashHandler_handle_crash);
  signal( SIGILL , internal::crashHandler_handle_crash);
  signal( SIGFPE , internal::crashHandler_handle_crash);
#endif
}

/** Uninstall crash handler. */
void CrashHandler::uninstall()
{
#ifdef JULIUS_PLATFORM_WIN
  SetUnhandledExceptionFilter(nullptr);
#endif
}



