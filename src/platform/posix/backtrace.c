#include "platform/backtrace.h"

#include "platform/platform.h"
#include "core/log.h"

#include "SDL.h"

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && \
    !defined(__vita__) && !defined(__SWITCH__) && !defined(__ANDROID__) && \
    !defined(__HAIKU__) && !defined(__EMSCRIPTEN__)

#include <execinfo.h>
#include <signal.h>

void backtrace_print(void)
{
    void *array[100];
    int size = backtrace(array, 100);

    char **stack = backtrace_symbols(array, size);

    for (int i = 0; i < size; i++) {
        log_info("", stack[i], 0);
    }
}

static void crashhandler(int sig)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Oops, crashed with signal %d :(", sig);
    backtrace_print();
    exit_with_status(1);
}

void install_game_crashhandler() {
    signal(SIGSEGV, crashhandler);
}

#else // fallback

void install_game_crashhandler() {}
void backtrace_print(void) {}

#endif
