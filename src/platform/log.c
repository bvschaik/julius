#include "core/log.h"

#ifndef HEADLESS_BUILD
#include "SDL.h"
#endif

#include <stdio.h>

#define MSG_SIZE 1000

static char log_buffer[MSG_SIZE];

static const char *build_message(const char *msg, const char *param_str, int param_int)
{
    int index = 0;
    index += snprintf(&log_buffer[index], MSG_SIZE - index, "%s", msg);
    if (param_str) {
        index += snprintf(&log_buffer[index], MSG_SIZE - index, "  %s", param_str);
    }
    if (param_int) {
        index += snprintf(&log_buffer[index], MSG_SIZE - index, "  %d", param_int);
    }
    return log_buffer;
}

void log_info(const char *msg, const char *param_str, int param_int)
{
#ifdef HEADLESS_BUILD
    printf("INFO: %s\n", build_message(msg, param_str, param_int));
#else
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", build_message(msg, param_str, param_int));
#endif
}

void log_error(const char *msg, const char *param_str, int param_int)
{
#ifdef HEADLESS_BUILD
    fprintf(stderr, "ERROR: %s\n", build_message(msg, param_str, param_int));
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", build_message(msg, param_str, param_int));
#endif
}
