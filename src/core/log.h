#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H

/**
 * @file
 * Debug logging
 */

/**
 * Logs a debug message
 * @param msg Message
 * @param param_str Extra info (string)
 * @param param_int Extra info (integer)
 */
void debug_log(const char *msg, const char *param_str, int param_int);

#endif // CORE_DEBUG_H
