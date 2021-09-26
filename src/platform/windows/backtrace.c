#include "platform/backtrace.h"

#include "platform/platform.h"
#include "core/log.h"

#include "SDL.h"

#if defined(_WIN32) && defined(_M_X64)

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

const char *print_exception_name(DWORD exceptionCode)
{
    switch (exceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        return "Access violation";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        return "Datatype misalignment";
    case EXCEPTION_BREAKPOINT:
        return "Breakpoint";
    case EXCEPTION_SINGLE_STEP:
        return "Single step";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        return "Array bounds exceeded";
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        return "Float denormal operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        return "Float divide by zero";
    case EXCEPTION_FLT_INEXACT_RESULT:
        return "Float inexact result";
    case EXCEPTION_FLT_INVALID_OPERATION:
        return "Float invalid operation";
    case EXCEPTION_FLT_OVERFLOW:
        return "Float overflow";
    case EXCEPTION_FLT_STACK_CHECK:
        return "Float stack check";
    case EXCEPTION_FLT_UNDERFLOW:
        return "Float underflow";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return "Integer divide by zero";
    case EXCEPTION_INT_OVERFLOW:
        return "Integer overflow";
    case EXCEPTION_PRIV_INSTRUCTION:
        return "Privileged instruction";
    case EXCEPTION_IN_PAGE_ERROR:
        return "In page error";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        return "Illegal instruction";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        return "Noncontinuable exception";
    case EXCEPTION_STACK_OVERFLOW:
        return "Stack overflow";
    case EXCEPTION_INVALID_DISPOSITION:
        return "Invalid disposition";
    case EXCEPTION_GUARD_PAGE:
        return "Guard page";
    case EXCEPTION_INVALID_HANDLE:
        return "Invalid handle";
    }
    return "Unknown exception";
}

#define log_info_sprintf(...) \
  snprintf(crash_info, 256, __VA_ARGS__); \
  log_info(crash_info, 0, 0)

void osystem_error(const char *title, const char *text)
{
    MessageBox(NULL, text, title, MB_OK | MB_ICONERROR);
}

static void print_stacktrace(LPEXCEPTION_POINTERS e)
{
    char crash_info[256];
    int filelineinfo_ok = 0;
    PIMAGEHLP_SYMBOL64 pSym;
    STACKFRAME sf;
    DWORD64 dwModBase, Disp64;
    DWORD Disp;
    BOOL more = FALSE;
    IMAGEHLP_LINE64 line;
    const char *filename = NULL;
    unsigned int linenum = 0;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    int count = 0;
    char modname[MAX_PATH];
    pSym = (PIMAGEHLP_SYMBOL64)GlobalAlloc(GMEM_FIXED, 16384);
    ZeroMemory(&sf, sizeof(sf));
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrReturn.Mode  = AddrModeFlat;

    PCONTEXT context = e->ContextRecord;
    DWORD machine_type = 0;
    sf.AddrPC.Offset = context->Rip;
    sf.AddrFrame.Offset = context->Rbp;
    sf.AddrStack.Offset = context->Rsp;
    machine_type = IMAGE_FILE_MACHINE_AMD64;

    // Record exception info.
    log_info_sprintf("Exception: %s (0x%08x)", print_exception_name(e->ExceptionRecord->ExceptionCode), (unsigned int)e->ExceptionRecord->ExceptionCode);
    log_info_sprintf("Exception Address: 0x%p", e->ExceptionRecord->ExceptionAddress);

    // Record stacktrace.
    log_info_sprintf("Stacktrace:");

    while(1)
    {
        more = StackWalk( machine_type, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);
        if(!more || sf.AddrFrame.Offset == 0)
        {
            break;
        }
        dwModBase = SymGetModuleBase64(GetCurrentProcess(), sf.AddrPC.Offset);
        dwModBase ? (void)GetModuleFileName((HINSTANCE)dwModBase, modname, MAX_PATH) : (void)strcpy(modname, "Unknown");

        pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSym->MaxNameLength = MAX_PATH;

        // unwind callstack
        if(SymGetSymFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &Disp64, pSym))
        {
            filelineinfo_ok = SymGetLineFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &Disp, &line);
            filename = !!filelineinfo_ok ? line.FileName : "UNKNOWN FILE";
            linenum = !!filelineinfo_ok ? line.LineNumber : 0;

            // This is the code path taken on VC if debugging syms are found.
            log_info_sprintf("(%d) %s L:%d(%s+%#0x) [0x%08X]\n", count, filename, linenum, pSym->Name, Disp, (unsigned int)sf.AddrPC.Offset);
        }
        else
        {
            // This is the code path taken on MinGW, and VC if no debugging syms are found.
            log_info_sprintf("(%d) %s [0x%08X]\n", count, modname, (unsigned int)sf.AddrPC.Offset);
        }
        ++count;
    }
    GlobalFree(pSym);
}

/** Called by windows if an exception happens. */
LONG CALLBACK exception_handler(LPEXCEPTION_POINTERS e)
{
    // Prologue.
    log_info("augustus has crashed.", 0, 0);
    // Initialize IMAGEHLP.DLL.
    SymInitialize(GetCurrentProcess(), ".", TRUE);

    print_stacktrace(e);

    // Unintialize IMAGEHLP.DLL
    SymCleanup(GetCurrentProcess());

    // Inform user.
    osystem_error( "augustus: unhandled exception", "Send logfile to developers" );

    // this seems to silently close the application
    return EXCEPTION_EXECUTE_HANDLER;
}

void install_game_crashhandler()
{
    SetUnhandledExceptionFilter(exception_handler);
}

void backtrace_print(void) {}

#else // fallback

void install_game_crashhandler() {}
void backtrace_print(void) {}

#endif
