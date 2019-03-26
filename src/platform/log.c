#include "core/log.h"
#include "SDL.h"

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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", build_message(msg, param_str, param_int));
}

void log_error(const char *msg, const char *param_str, int param_int)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", build_message(msg, param_str, param_int));
}
