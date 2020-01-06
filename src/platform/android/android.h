#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#ifdef __ANDROID__

#include <jni.h>

int android_check_rw_permissions(void);
void android_request_rw_permissions(void);
void android_toast_message(const char *messge);
void android_show_c3_path_dialog(void);
const char* android_get_c3_path(void);
float android_get_screen_scale(void);

#endif // __ANDROID__
#endif // PLATFORM_ANDROID_H
