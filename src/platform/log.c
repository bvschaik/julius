#include "core/log.h"
#include "SDL.h"

#include <stdio.h>
#include <string.h>

#define MSG_SIZE 1000
#define MAX_OLD_MESSAGES 5

static char log_buffer[MSG_SIZE];
static struct {
    char buffer[MSG_SIZE];
    unsigned int count;
} previous_log_messages[MAX_OLD_MESSAGES];
static int old_message_index;

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

static int count_archived_message(void)
{
    for (int i = 0; i < MAX_OLD_MESSAGES; i++) {
        if (strcmp(previous_log_messages[i].buffer, log_buffer) == 0) {
            previous_log_messages[i].count++;
            return 1;
        }
    }
    if (old_message_index == MAX_OLD_MESSAGES) {
        log_repeated_messages();
    }
    strncpy(previous_log_messages[old_message_index++].buffer, log_buffer, MSG_SIZE);
    if (old_message_index < MAX_OLD_MESSAGES) {
        previous_log_messages[old_message_index].count = 0;
    }
    return 0;
}

void log_repeated_messages(void)
{
    for (int i = 0; i < MAX_OLD_MESSAGES; i++) {
        if (previous_log_messages[i].count) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s (message repeats %u %s)",
                previous_log_messages[i].buffer, previous_log_messages[i].count,
                previous_log_messages[i].count == 1 ? "time" : "times");
        }
        previous_log_messages[i].buffer[0] = 0;
        previous_log_messages[i].count = 0;
    }
    old_message_index = 0;
}

void log_info(const char *msg, const char *param_str, int param_int)
{
    build_message(msg, param_str, param_int);
    if (!count_archived_message()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
    }
}

void log_error(const char *msg, const char *param_str, int param_int)
{
    build_message(msg, param_str, param_int);
    if (!count_archived_message()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", log_buffer);
    }
}
