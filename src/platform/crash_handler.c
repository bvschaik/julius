#include "game/system.h"

#include "platform/platform.h"
#include "platform/screen.h"
#include "core/log.h"

#include "SDL.h"

#if (defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && \
   !defined(__vita__) && !defined(__SWITCH__) && !defined(__ANDROID__) && \
   !defined(__HAIKU__) && !defined(__EMSCRIPTEN__)) || \
    (defined(_WIN32) && defined(_M_X64))
#define HAS_STACK_TRACE
#endif

static void display_crash_message(void)
{
    platform_screen_show_error_message_box("Augustus has crashed :(",
        "There was an unrecoverable error in Augustus, which will now close.\n\n"
#ifdef HAS_STACK_TRACE
        "The piece of code that caused the crash has been saved to augustus-log.txt.\n\n"
#endif
        "If you can, please create an issue by going to:\n\n"
        "https://github.com/Keriew/augustus/issues/new \n\n"
        "Please attach "
#ifdef HAS_STACK_TRACE
        "augustus-log.txt and "
#endif
        "your city save to the issue report.\n\n"
        "Also, please describe what you were doing when the game crashed.\n\n"
        "With your help, we can avoid this crash in the future.\n\n"
        "Thanks!\n\n"
        "- The Augustus dev team");
}

#if defined(__GNUC__) && !defined(_WIN32)

#include <signal.h>

#ifdef HAS_STACK_TRACE

#include <execinfo.h>

static void backtrace_print(void)
{
    void *array[100];
    int size = backtrace(array, 100);

    char **stack = backtrace_symbols(array, size);

    for (int i = 0; i < size; i++) {
        log_info("", stack[i], 0);
    }
}
#else
static void backtrace_print(void)
{
    log_info("No stack trace available", 0, 0);
}
#endif

static void crash_handler(int sig)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Oops, crashed with signal %d :(", sig);
    backtrace_print();
    display_crash_message();
    exit_with_status(1);
}

void system_setup_crash_handler(void)
{
    signal(SIGSEGV, crash_handler);
}

#elif defined(_WIN32)

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

#ifdef HAS_STACK_TRACE

#define log_info_sprintf(...) \
    snprintf(crash_info, 256, __VA_ARGS__); \
    log_info(crash_info, 0, 0)


static const char *print_exception_name(DWORD exception_code)
{
    switch (exception_code) {
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

    // Record exception info
    log_info_sprintf("Exception: %s (0x%08x)", print_exception_name(e->ExceptionRecord->ExceptionCode),
        (unsigned int) e->ExceptionRecord->ExceptionCode);
    log_info_sprintf("Exception Address: 0x%p", e->ExceptionRecord->ExceptionAddress);

    // Record stacktrace
    log_info_sprintf("Stacktrace:");

    while (1) {
        more = StackWalk(machine_type, GetCurrentProcess(), GetCurrentThread(), &sf, context, NULL,
            SymFunctionTableAccess64, SymGetModuleBase64, NULL);
        if (!more || sf.AddrFrame.Offset == 0) {
            break;
        }
        dwModBase = SymGetModuleBase64(GetCurrentProcess(), sf.AddrPC.Offset);
        if (dwModBase) {
            GetModuleFileName((HINSTANCE) dwModBase, modname, MAX_PATH);
        } else {
            strcpy(modname, "Unknown");
        }

        pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSym->MaxNameLength = MAX_PATH;

        // unwind callstack
        if (SymGetSymFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &Disp64, pSym)) {
            filelineinfo_ok = SymGetLineFromAddr64(GetCurrentProcess(), sf.AddrPC.Offset, &Disp, &line);
            filename = !!filelineinfo_ok ? line.FileName : "UNKNOWN FILE";
            linenum = !!filelineinfo_ok ? line.LineNumber : 0;

            // This is the code path taken on VC if debugging syms are found
            log_info_sprintf("(%d) %s L:%u(%s+%#0lx) [0x%08X]\n", count, filename, linenum, pSym->Name,
                Disp, (unsigned int)sf.AddrPC.Offset);
        } else {
            // This is the code path taken on MinGW, and VC if no debugging syms are found.
            log_info_sprintf("(%d) %s [0x%08X]\n", count, modname, (unsigned int)sf.AddrPC.Offset);
        }
        ++count;
    }
    GlobalFree(pSym);
}

#endif

/** Called by windows if an exception happens. */
static LONG CALLBACK exception_handler(LPEXCEPTION_POINTERS e)
{
    // Prologue.
    log_error("Oops, crashed :(", 0, 0);

#ifdef _M_X64

    // Initialize IMAGEHLP.DLL.
    SymInitialize(GetCurrentProcess(), ".", TRUE);

    print_stacktrace(e);

    // Unintialize IMAGEHLP.DLL
    SymCleanup(GetCurrentProcess());

#else
    log_info("No stack trace available", 0, 0);
#endif

    // Inform user
    display_crash_message();

    // this seems to silently close the application
    return EXCEPTION_EXECUTE_HANDLER;
}

void system_setup_crash_handler(void)
{
    SetUnhandledExceptionFilter(exception_handler);
}

#else // fallback

void system_setup_crash_handler(void)
{}

#endif
