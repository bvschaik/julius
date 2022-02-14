#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

int platform_screen_create(const char *title, int dispay_scale_percentage);
void platform_screen_destroy(void);

int platform_screen_resize(int pixel_width, int pixel_height, int save);
void platform_screen_move(int x, int y);

void platform_screen_set_fullscreen(void);
void platform_screen_set_windowed(void);
void platform_screen_set_window_size(int logical_width, int logical_height);
void platform_screen_center_window(void);

#ifdef _WIN32
void platform_screen_recreate_texture(void);
#endif

int platform_screen_get_scale(void);

void platform_screen_show_error_message_box(const char *title, const char *message);

#endif // PLATFORM_SCREEN_H
