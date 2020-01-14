#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#ifdef __ANDROID__

#include <jni.h>

void android_toast_message(const char *messge);
const char* android_show_c3_path_dialog(void);
float android_get_screen_scale(void);
int android_get_file_descriptor(const char *filename, const char *mode);
int android_set_base_path(const char *path);
int android_get_directory_contents_by_extension(char **list, int *count, const char *extension, int max_files);

#endif // __ANDROID__
#endif // PLATFORM_ANDROID_H
