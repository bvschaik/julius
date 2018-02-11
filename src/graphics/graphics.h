#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

#include "graphics/color.h"

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
    clip_code clipX;
    clip_code clipY;
    int clippedPixelsLeft;
    int clippedPixelsRight;
    int clippedPixelsTop;
    int clippedPixelsBottom;
    int visiblePixelsX;
    int visiblePixelsY;
    int isVisible;
} clip_info;

void graphics_init_canvas(int width, int height);
const void *graphics_canvas();

void graphics_in_dialog();
void graphics_reset_dialog();

void graphics_set_clip_rectangle(int x, int y, int width, int height);
void graphics_reset_clip_rectangle();
const clip_info *graphics_get_clip_info(int x, int y, int width, int height);

void graphics_save_to_buffer(int x, int y, int width, int height, color_t *buffer);
void graphics_draw_from_buffer(int x, int y, int width, int height, const color_t *buffer);

color_t *graphics_get_pixel(int x, int y);

void graphics_clear_screen();

void graphics_draw_pixel(int x, int y, color_t color);

void graphics_draw_line(int x1, int y1, int x2, int y2, color_t color);

void graphics_draw_rect(int x, int y, int width, int height, color_t color);
void graphics_draw_inset_rect(int x, int y, int width, int height);

void graphics_fill_rect(int x, int y, int width, int height, color_t color);
void graphics_shade_rect(int x, int y, int width, int height, int darkness);

void graphics_save_screenshot(const char *filename);

#endif // GRAPHICS_GRAPHICS_H
