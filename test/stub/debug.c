#include "core/debug.h"

#include <stdio.h>

void debug_log(const char *msg, const char *param_str, int param_int)
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
