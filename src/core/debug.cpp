#include "core/debug.h"

#include "debug/logger.hpp"

void debug_log(const char *msg, const char *param_str, int param_int)
{
    Logger::debug(msg);
    if (param_str)
        Logger::debug(" %s", param_str);
    if (param_int)
        Logger::debug(" %d", param_int);
}
