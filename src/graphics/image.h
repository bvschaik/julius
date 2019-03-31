#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include "core/image.h"
#include "graphics/color.h"

void image_draw(int image_id, int x, int y);
void image_draw_enemy(int image_id, int x, int y);

void image_draw_masked(int image_id, int x, int y, color_t color_mask);
void image_draw_blend(int image_id, int x, int y, color_t color);
void image_draw_letter(int letter_id, int x, int y, color_t color);

void image_draw_fullscreen_background(int image_id);

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask);
void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask);

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask);
void image_draw_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask);

#endif // GRAPHICS_IMAGE_H
