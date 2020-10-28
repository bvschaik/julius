#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include "core/image.h"
#include "graphics/color.h"
#include "graphics/font.h"

void image_draw(int image_id, int x, int y);
void image_draw_enemy(int image_id, int x, int y);

void image_draw_masked(int image_id, int x, int y, color_t color_mask);
void image_draw_blend(int image_id, int x, int y, color_t color);
void image_draw_blend_alpha(int image_id, int x, int y, color_t color);
void image_draw_letter(font_t font, int letter_id, int x, int y, color_t color);

void image_draw_fullscreen_background(int image_id);

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask);
void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask);

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask);
void image_draw_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask);

void image_draw_scaled(int image_id, int x_offset, int y_offset, double scale_factor);

#endif // GRAPHICS_IMAGE_H
