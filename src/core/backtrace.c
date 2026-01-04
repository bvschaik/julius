#include "backtrace.h"

#include "core/log.h"

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && \
    !defined(__vita__) && !defined(__SWITCH__) && !defined(__ANDROID__) && \
    !defined(__HAIKU__) && !defined(__EMSCRIPTEN__) && !defined(__CYGWIN__) && \
    (!defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050)

#include <execinfo.h>

void backtrace_print(void)
{
    void *array[100];
    int size = backtrace(array, 100);

    char **stack = backtrace_symbols(array, size);

    for (int i = 0; i < size; i++) {
        log_info("", stack[i], 0);
    }
}

#else
void backtrace_print(void) {}
#endif
