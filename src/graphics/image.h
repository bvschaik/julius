#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include "core/image.h"
#include "graphics/color.h"
#include "graphics/font.h"

#define SCALE_NONE 1.0f

void image_draw(int image_id, int x, int y, color_t color, float scale);

void image_draw_enemy(int image_id, int x, int y, float scale);

void image_draw_letter(font_t font, int letter_id, int x, int y, color_t color, float scale);

void image_draw_fullscreen_background(int image_id);

void image_draw_border(int base_image_id, int x, int y, color_t color);

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask, float scale);
void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask, float scale);
void image_blend_footprint_color(int x, int y, color_t color, float scale);

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask, float scale);
void image_draw_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask, float scale);
void image_draw_set_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask, float scale);

#endif // GRAPHICS_IMAGE_H
