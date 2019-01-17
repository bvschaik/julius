#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

#include "SDL.h"

typedef enum {
    NORMAL_EVENT = 0,
    THREADED_EVENT = 1
} platform_screen_event_mode;

int platform_screen_create(const char *title);
void platform_screen_destroy(void);

int platform_screen_resize(int width, int height, platform_screen_event_mode mode);
int platform_screen_rescale(void);
void platform_screen_move(int x, int y, platform_screen_event_mode mode);

void platform_screen_set_fullscreen(void);
void platform_screen_set_windowed(void);
void platform_screen_set_window_size(int width, int height);
void platform_screen_center_window(void);

void platform_screen_render(void);

#endif // PLATFORM_SCREEN_H
