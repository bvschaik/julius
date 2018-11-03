#include "core/debug.h"
#include "SDL.h"

#include <stdio.h>

static char log_buffer[1000];

void debug_log(const char *msg, const char *param_str, int param_int)
{
    int index = 0;
    index += sprintf(&log_buffer[index], "%s", msg);
    if (param_str) {
        index += sprintf(&log_buffer[index], "  %s", param_str);
    }
    if (param_int) {
        index += sprintf(&log_buffer[index], "  %s", param_str);
    }
    SDL_Log("%s", log_buffer);
}
