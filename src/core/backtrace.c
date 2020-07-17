#include "backtrace.h"

#include "core/log.h"

#if defined(__GNUC__) && !defined(__MINGW32__) && !defined(__OpenBSD__) && !defined(__vita__) && !defined(__SWITCH__) && !defined(__ANDROID__)

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
