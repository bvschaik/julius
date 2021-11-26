#ifndef LOG_H
#define LOG_H

#include <stdio.h>

static inline void log_internal(const char *type, const char *msg, const char *param_str, int param_int)
{
	if (!param_str && !param_int) {
		printf("%s: %s\n", type, msg);
	} else if (param_str && !param_int) {
		printf("%s: %s %s\n", type, msg, param_str);
	} else if (!param_str && param_int) {
		printf("%s: %s %d\n", type, msg, param_int);
	} else {
		printf("%s: %s %s %d\n", type, msg, param_str, param_int);
	}
}

const char *config_get_string(int key)
{
	return 0;
}

void log_info(const char *msg, const char *param_str, int param_int)
{
	log_internal("Info", msg, param_str, param_int);
}

void log_error(const char *msg, const char *param_str, int param_int)
{
	log_internal("ERROR", msg, param_str, param_int);
}

#endif // LOG_H
