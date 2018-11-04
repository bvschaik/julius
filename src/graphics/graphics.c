#include "graphics.h"

#include "graphics/screen.h"

#include <stdlib.h>
#include <string.h>

static struct {
    color_t *pixels;
    int width;
    int height;
} canvas;

static struct {
    int x_start;
    int x_end;
    int y_start;
    int y_end;
} clip_rectangle = {0, 800, 0, 600};

static struct {
    int x;
    int y;
} translation = {0, 0};

static clip_info clip;

void graphics_init_canvas(int width, int height)
{
    if (canvas.pixels) {
        free(canvas.pixels);
    }
    canvas.pixels = (color_t *) malloc(width * height * sizeof(color_t));
    memset(canvas.pixels, 0, width * height * sizeof(color_t));
    canvas.width = width;
    canvas.height = height;

    graphics_set_clip_rectangle(0, 0, width, height);
}

const void *graphics_canvas(void)
{
    return canvas.pixels;
}

static void translate_clip(int dx, int dy)
{
    clip_rectangle.x_start -= dx;
    clip_rectangle.x_end -= dx;
    clip_rectangle.y_start -= dy;
    clip_rectangle.y_end -= dy;
}

static void set_translation(int x, int y)
{
    int dx = x - translation.x;
    int dy = y - translation.y;
    translation.x = x;
    translation.y = y;
    translate_clip(dx, dy);
}

void graphics_in_dialog(void)
{
    set_translation(screen_dialog_offset_x(), screen_dialog_offset_y());
}

void graphics_reset_dialog(void)
{
    set_translation(0, 0);
}

void graphics_set_clip_rectangle(int x, int y, int width, int height)
{
    clip_rectangle.x_start = x;
    clip_rectangle.x_end = x + width;
    clip_rectangle.y_start = y;
    clip_rectangle.y_end = y + height;
    // fix clip rectangle going over the edges of the screen
    if (translation.x + clip_rectangle.x_start < 0) {
        clip_rectangle.x_start = -translation.x;
    }
    if (translation.y + clip_rectangle.y_start < 0) {
        clip_rectangle.y_start = -translation.y;
    }
    if (translation.x + clip_rectangle.x_end > canvas.width) {
        clip_rectangle.x_end = canvas.width - translation.x;
    }
    if (translation.y + clip_rectangle.y_end > canvas.height) {
        clip_rectangle.y_end = canvas.height - translation.y;
    }
}

void graphics_reset_clip_rectangle(void)
{
    clip_rectangle.x_start = 0;
    clip_rectangle.x_end = canvas.width;
    clip_rectangle.y_start = 0;
    clip_rectangle.y_end = canvas.height;
    translate_clip(translation.x, translation.y);
}

static void set_clip_x(int x_offset, int width)
{
    clip.clipped_pixels_left = 0;
    clip.clipped_pixels_right = 0;
    if (width <= 0
        || x_offset + width <= clip_rectangle.x_start
        || x_offset >= clip_rectangle.x_end) {
        clip.clip_x = CLIP_INVISIBLE;
        clip.visible_pixels_x = 0;
        return;
    }
    if (x_offset < clip_rectangle.x_start) {
        // clipped on the left
        clip.clipped_pixels_left = clip_rectangle.x_start - x_offset;
        if (x_offset + width <= clip_rectangle.x_end) {
            clip.clip_x = CLIP_LEFT;
        } else {
            clip.clip_x = CLIP_BOTH;
            clip.clipped_pixels_right = x_offset + width - clip_rectangle.x_end;
        }
    } else if (x_offset + width > clip_rectangle.x_end) {
        clip.clip_x = CLIP_RIGHT;
        clip.clipped_pixels_right = x_offset + width - clip_rectangle.x_end;
    } else {
        clip.clip_x = CLIP_NONE;
    }
    clip.visible_pixels_x = width - clip.clipped_pixels_left - clip.clipped_pixels_right;
}

static void set_clip_y(int y_offset, int height)
{
    clip.clipped_pixels_top = 0;
    clip.clipped_pixels_bottom = 0;
    if (height <= 0
        || y_offset + height <= clip_rectangle.y_start
        || y_offset >= clip_rectangle.y_end) {
        clip.clip_y = CLIP_INVISIBLE;
    } else if (y_offset < clip_rectangle.y_start) {
        // clipped on the top
        clip.clipped_pixels_top = clip_rectangle.y_start - y_offset;
        if (y_offset + height <= clip_rectangle.y_end) {
            clip.clip_y = CLIP_TOP;
        } else {
            clip.clip_y = CLIP_BOTH;
            clip.clipped_pixels_bottom = y_offset + height - clip_rectangle.y_end;
        }
    } else if (y_offset + height > clip_rectangle.y_end) {
        clip.clip_y = CLIP_BOTTOM;
        clip.clipped_pixels_bottom = y_offset + height - clip_rectangle.y_end;
    } else {
        clip.clip_y = CLIP_NONE;
    }
    clip.visible_pixels_y = height - clip.clipped_pixels_top - clip.clipped_pixels_bottom;
}

const clip_info *graphics_get_clip_info(int x, int y, int width, int height)
{
    set_clip_x(x, width);
    set_clip_y(y, height);
    if (clip.clip_x == CLIP_INVISIBLE || clip.clip_y == CLIP_INVISIBLE) {
        clip.is_visible = 0;
    } else {
        clip.is_visible = 1;
    }
    return &clip;
}

void graphics_save_to_buffer(int x, int y, int width, int height, color_t *buffer)
{
    for (int dy = 0; dy < height; dy++) {
        memcpy(&buffer[dy * height], graphics_get_pixel(x, y + dy), sizeof(color_t) * width);
    }
}

void graphics_draw_from_buffer(int x, int y, int width, int height, const color_t *buffer)
{
    for (int dy = 0; dy < height; dy++) {
        memcpy(graphics_get_pixel(x, y + dy), &buffer[dy * height], sizeof(color_t) * width);
    }
}

color_t *graphics_get_pixel(int x, int y)
{
    return &canvas.pixels[(translation.y + y) * canvas.width + (translation.x + x)];
}

void graphics_clear_screen(void)
{
    memset(canvas.pixels, 0, sizeof(color_t) * canvas.width * canvas.height);
}

void graphics_draw_pixel(int x, int y, color_t color)
{
    if (x >= clip_rectangle.x_start && x < clip_rectangle.x_end) {
        if (y >= clip_rectangle.y_start && y < clip_rectangle.y_end) {
            *graphics_get_pixel(x, y) = color;
        }
    }
}

void graphics_draw_line(int x1, int y1, int x2, int y2, color_t color)
{
    if (x1 == x2) {
        int y_min = y1 < y2 ? y1 : y2;
        int y_max = y1 < y2 ? y2 : y1;
        for (int y = y_min; y <= y_max; y++) {
            graphics_draw_pixel(x1, y, color);
        }
    } else if (y1 == y2) {
        int x_min = x1 < x2 ? x1 : x2;
        int x_max = x1 < x2 ? x2 : x1;
        for (int x = x_min; x <= x_max; x++) {
            graphics_draw_pixel(x, y1, color);
        }
    } else {
        // non-straight line: ignore
    }
}

void graphics_draw_rect(int x, int y, int width, int height, color_t color)
{
    graphics_draw_line(x, y, x + width - 1, y, color);
    graphics_draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);
    graphics_draw_line(x, y, x, y + height - 1, color);
    graphics_draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void graphics_draw_inset_rect(int x, int y, int width, int height)
{
    graphics_draw_line(x, y, x + width - 1, y, COLOR_INSET_DARK);
    graphics_draw_line(x + width - 1, y, x + width - 1, y + height - 1, COLOR_INSET_LIGHT);
    graphics_draw_line(x, y + height - 1, x + width - 1, y + height - 1, COLOR_INSET_LIGHT);
    graphics_draw_line(x, y, x, y + height - 1, COLOR_INSET_DARK);
}

void graphics_fill_rect(int x, int y, int width, int height, color_t color)
{
    for (int yy = y; yy < height + y; yy++) {
        graphics_draw_line(x, yy, x + width - 1, yy, color);
    }
}

void graphics_shade_rect(int x, int y, int width, int height, int darkness)
{
    const clip_info *clip = graphics_get_clip_info(x, y, width, height);
    if (!clip->is_visible) {
        return;
    }
    for (int yy = y + clip->clipped_pixels_top; yy < y + height - clip->clipped_pixels_bottom; yy++) {
        for (int xx = x + clip->clipped_pixels_left; xx < x + width - clip->clipped_pixels_right; xx++) {
            color_t *pixel = graphics_get_pixel(xx, yy);
            int r = (*pixel & 0xff0000) >> 16;
            int g = (*pixel & 0xff00) >> 8;
            int b = (*pixel & 0xff);
            int grey = (r + g + b) / 3 >> darkness;
            color_t new_pixel = (color_t) (grey << 16 | grey << 8 | grey);
            *pixel = new_pixel;
        }
    }
}
