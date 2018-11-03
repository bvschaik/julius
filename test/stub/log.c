#include "core/log.h"

#include <stdio.h>

static void print_message(const char *msg, const char *param_str, int param_int)
{
    printf("%s", msg);
    if (param_str) {
        printf("  %s", param_str);
    }
    if (param_int) {
        printf("  %s", param_str);
    }
    printf("\n");
}

void log_info(const char *msg, const char *param_str, int param_int)
{
    printf("INFO: ");
    print_message(msg, param_str, param_int);
}

void log_error(const char *msg, const char *param_str, int param_int)
{
    printf("ERROR: ");
    print_message(msg, param_str, param_int);
}
