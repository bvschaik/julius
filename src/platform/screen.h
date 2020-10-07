#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

void platform_screen_get_scaled_params(int* width, int* height);
void platform_screen_set_scale(int display_scale_percentage);
int platform_screen_get_scale(void);
int platform_screen_create(const char *title, int dispay_scale_percentage);
void platform_screen_destroy(void);

int platform_screen_resize(int pixel_width, int pixel_height, int save);
void platform_screen_move(int x, int y);

void platform_screen_set_fullscreen(void);
void platform_screen_set_windowed(void);
void platform_screen_set_window_size(int logical_width, int logical_height);
void platform_screen_center_window(void);

void platform_screen_render(void);

#endif // PLATFORM_SCREEN_H
