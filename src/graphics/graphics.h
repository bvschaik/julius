#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

#include "graphics/color.h"

typedef enum {
    CANVAS_UI = 0,
    CANVAS_CITY = 1
} canvas_type;

typedef enum {
    CLIP_NONE,
    CLIP_LEFT,
    CLIP_RIGHT,
    CLIP_TOP,
    CLIP_BOTTOM,
    CLIP_BOTH,
    CLIP_INVISIBLE
} clip_code;

typedef struct {
    clip_code clip_x;
    clip_code clip_y;
    int clipped_pixels_left;
    int clipped_pixels_right;
    int clipped_pixels_top;
    int clipped_pixels_bottom;
    int visible_pixels_x;
    int visible_pixels_y;
    int is_visible;
} clip_info;

void graphics_init_canvas(int width, int height);
const void *graphics_canvas(canvas_type type);
void graphics_set_active_canvas(canvas_type type);

void graphics_in_dialog(void);
void graphics_reset_dialog(void);

void graphics_set_clip_rectangle(int x, int y, int width, int height);
void graphics_reset_clip_rectangle(void);
const clip_info *graphics_get_clip_info(int x, int y, int width, int height);

void graphics_save_to_buffer(int x, int y, int width, int height, color_t *buffer);
void graphics_draw_from_buffer(int x, int y, int width, int height, const color_t *buffer);

color_t *graphics_get_pixel(int x, int y);

void graphics_clear_screen(canvas_type type);
void graphics_clear_city_viewport(void);
void graphics_clear_screens(void);

void graphics_draw_vertical_line(int x, int y1, int y2, color_t color);
void graphics_draw_horizontal_line(int x1, int x2, int y, color_t color);

void graphics_draw_rect(int x, int y, int width, int height, color_t color);
void graphics_draw_inset_rect(int x, int y, int width, int height);

void graphics_fill_rect(int x, int y, int width, int height, color_t color);
void graphics_shade_rect(int x, int y, int width, int height, int darkness);

#endif // GRAPHICS_GRAPHICS_H
