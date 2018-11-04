#ifndef PLATFORM_SCREEN_H
#define PLATFORM_SCREEN_H

#include "SDL.h"

void platform_screen_create(const char *title);

void platform_screen_resize(int width, int height);

void platform_screen_set_fullscreen();
void platform_screen_set_windowed();
void platform_screen_set_window_size(int width, int height);

void platform_screen_render();

#endif // PLATFORM_SCREEN_H
