#ifndef PLATFORM_ANDROID_H
#define PLATFORM_ANDROID_H

#ifdef __ANDROID__

#include <stdint.h>

void android_toast_message(const char *messge);
const char *android_show_c3_path_dialog(void);
float android_get_screen_scale(void);
int android_get_file_descriptor(const char *filename, const char *mode);
int android_set_base_path(const char *path);
int android_get_directory_contents(const char *dir, int type, const char *extension, int (*callback)(const char *));
int android_remove_file(const char *filename);

#define PLATFORM_USE_VIRTUAL_KEYBOARD
void platform_show_virtual_keyboard(const uint8_t *text, int max_length);
void platform_hide_virtual_keyboard(void);

#endif // __ANDROID__
#endif // PLATFORM_ANDROID_H
