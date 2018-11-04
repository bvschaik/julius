#ifndef GRAPHICS_SCREEN_H
#define GRAPHICS_SCREEN_H

#include "graphics/color.h"

void screen_set_resolution(int width, int height);

color_t *screen_pixel(int x, int y);

int screen_width(void);

int screen_height(void);

int screen_dialog_offset_x(void);

int screen_dialog_offset_y(void);

#endif // GRAPHICS_SCREEN_H
