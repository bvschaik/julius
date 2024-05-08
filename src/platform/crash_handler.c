#include "game/system.h"

#include "platform/platform.h"
#include "platform/screen.h"
#include "core/log.h"

#include "SDL.h"

#if (defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && \
   !defined(__vita__) && !defined(__SWITCH__) && !defined(__ANDROID__) && \
   !defined(__HAIKU__) && !defined(__EMSCRIPTEN__)) || \
    (defined(_WIN32) && (defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64)))
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

static const char *fetch_signal_name(int sig)
{
    switch (sig) {
        case SIGILL:
            return "Illegal Instruction";
        case SIGBUS:
            return "Bus Error";
        case SIGFPE:
            return "Floating point exception";
        case SIGSEGV:
            return "Segmentation fault";
        case SIGCHLD:
            return "Child process has stopped";
        case SIGXCPU:
            return "CPU time limit exceeded";
        case SIGSYS:
            return "Bad system call";
        default:
            return "Another signal type";
    }
}

static void crash_handler(int sig)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Oops, crashed :(");
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Signal %d: %s", sig, fetch_signal_name(sig));
    backtrace_print();
    display_crash_message();
    exit_with_status(1);
}

void system_setup_crash_handler(void)
{
    signal(SIGSEGV, crash_handler);
}

#elif defined(_WIN32)

// This define is a hack to easily remove the full Github Actions path from the source file,
// but the source compilation path on Github Actions has been stable since forever
#define GITHUB_ACTIONS_SOURCE_PATH "D:\\a\\augustus\\augustus\\"

#include <windows.h>

#include <imagehlp.h>
#include <shlwapi.h>
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
    char modname[MAX_PATH];

    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));

    CONTEXT context = *e->ContextRecord;

#ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_ARM64
    image = IMAGE_FILE_MACHINE_ARM64;
    stackframe.AddrPC.Offset = context.Pc;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Fp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Sp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#endif

    // Record exception info
    log_info_sprintf("Exception: %s (0x%08x)", print_exception_name(e->ExceptionRecord->ExceptionCode),
        (unsigned int) e->ExceptionRecord->ExceptionCode);
    log_info_sprintf("Exception Address: 0x%p", e->ExceptionRecord->ExceptionAddress);

    // Record stacktrace
    log_info_sprintf("Stacktrace:");

    for (int frame = 0; frame < 512; frame++) {
        BOOL more = StackWalk64(image, GetCurrentProcess(), GetCurrentThread(),
            &stackframe, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL);

        if (!more) {
            break;
        }

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO) buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        DWORD64 displacement64;
        DWORD displacement;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        // This is the code path taken if debugging syms are found
        if (SymFromAddr(GetCurrentProcess(), stackframe.AddrPC.Offset, &displacement64, symbol)) {
            int has_line = SymGetLineFromAddr64(GetCurrentProcess(), stackframe.AddrPC.Offset, &displacement, &line);
            const char *relative_file_name;
            if (!has_line) {
                relative_file_name = "UNKNOWN FILE";
            } else if (strncmp(line.FileName, GITHUB_ACTIONS_SOURCE_PATH, sizeof(GITHUB_ACTIONS_SOURCE_PATH) - 1) == 0) {
                relative_file_name = line.FileName + sizeof(GITHUB_ACTIONS_SOURCE_PATH) - 1;
            } else {
                relative_file_name = line.FileName;
            }
            log_info_sprintf("(%d) %s L:%lu(%s+%#0lx) [0x%p]\n", frame, relative_file_name,
                has_line ? line.LineNumber : 0, symbol->Name, displacement, (void *) stackframe.AddrPC.Offset);

            // No need to go further up the stack
            if (strcmp(symbol->Name, "SDL_main") == 0) {
                break;
            }
        // This is the code path taken if no debugging syms are found.
        } else {
            DWORD64 mod_base = SymGetModuleBase64(GetCurrentProcess(), stackframe.AddrPC.Offset);
            if (mod_base) {
                GetModuleFileName((HINSTANCE) mod_base, modname, MAX_PATH);
            } else {
                snprintf(modname, MAX_PATH, "Unknown");
            }
            log_info_sprintf("(%d) %s [0x%p]\n", frame, modname, (void *) stackframe.AddrPC.Offset);
        }
    }
}

#endif

/** Called by windows if an exception happens. */
static LONG CALLBACK exception_handler(LPEXCEPTION_POINTERS e)
{
    // Prologue.
    log_error("Oops, crashed :(", 0, 0);

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(0, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    SetEnvironmentVariableW(L"_NT_SYMBOL_PATH", path);

    // Initialize IMAGEHLP.DLL.
    SymInitialize(GetCurrentProcess(), 0, TRUE);

    print_stacktrace(e);

    // Unintialize IMAGEHLP.DLL
    SymCleanup(GetCurrentProcess());

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
