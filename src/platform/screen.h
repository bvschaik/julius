#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

int platform_screen_create(const char *title);
void platform_screen_destroy(void);

int platform_screen_resize(int width, int height);

void platform_screen_set_fullscreen(void);
void platform_screen_set_windowed(void);
void platform_screen_set_window_size(int width, int height);
void platform_screen_center_window(void);

void platform_screen_render(void);

#endif // PLATFORM_SCREEN_H
