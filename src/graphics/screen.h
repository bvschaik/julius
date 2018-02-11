#ifndef GRAPHICS_SCREEN_H
#define GRAPHICS_SCREEN_H

#include "graphics/color.h"

void screen_set_resolution(int width, int height);

color_t *screen_pixel(int x, int y);

int screen_width();

int screen_height();

int screen_dialog_offset_x();

int screen_dialog_offset_y();

#endif // GRAPHICS_SCREEN_H
