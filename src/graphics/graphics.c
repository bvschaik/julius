#include "graphics.h"

#include "city/view.h"
#include "core/config.h"
#include "graphics/color.h"
#include "graphics/menu.h"
#include "graphics/screen.h"

#include <stdlib.h>
#include <string.h>
#ifdef __vita__
#include <vita2d.h>
#endif

static struct {
    color_t *pixels;
    int width;
    int height;
} canvas[2];

static struct {
    int x_start;
    int x_end;
    int y_start;
    int y_end;
} clip_rectangle = {0, 800, 0, 600};

static struct {
    int x;
    int y;
} translation;

static clip_info clip;
static canvas_type active_canvas;

#ifdef __vita__
extern vita2d_texture *tex_buffer_ui;
extern vita2d_texture * tex_buffer_city;
#endif

void graphics_init_canvas(int width, int height)
{
#ifdef __vita__
    canvas[CANVAS_UI].pixels = vita2d_texture_get_datap(tex_buffer_ui);
    if (config_get(CONFIG_UI_ZOOM)) {
        canvas[CANVAS_CITY].pixels = vita2d_texture_get_datap(tex_buffer_city);
    } else {
        canvas[CANVAS_CITY].pixels = 0;
    }
#else
    free(canvas[CANVAS_UI].pixels);
    free(canvas[CANVAS_CITY].pixels);
    canvas[CANVAS_UI].pixels = (color_t *) malloc((size_t) width * height * sizeof(color_t));
    if (config_get(CONFIG_UI_ZOOM)) {
        canvas[CANVAS_CITY].pixels = (color_t *) malloc((size_t) width * height * 4 * sizeof(color_t));
    } else {
        canvas[CANVAS_CITY].pixels = 0;
    }
#endif
    canvas[CANVAS_UI].width = width;
    canvas[CANVAS_UI].height = height;
    canvas[CANVAS_CITY].width = width * 2;
    canvas[CANVAS_CITY].height = height * 2;

    graphics_clear_screens();
    graphics_set_clip_rectangle(0, 0, width, height);
}

const void *graphics_canvas(canvas_type type)
{
    if (!config_get(CONFIG_UI_ZOOM)) {
        return canvas[CANVAS_UI].pixels;
    }
    return canvas[type].pixels;
}

void graphics_set_active_canvas(canvas_type type)
{
    active_canvas = type;
    graphics_reset_clip_rectangle();
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
    if (translation.x + clip_rectangle.x_end > canvas[active_canvas].width) {
        clip_rectangle.x_end = canvas[active_canvas].width - translation.x;
    }
    if (translation.y + clip_rectangle.y_end > canvas[active_canvas].height) {
        clip_rectangle.y_end = canvas[active_canvas].height - translation.y;
    }
}

void graphics_reset_clip_rectangle(void)
{
    clip_rectangle.x_start = 0;
    clip_rectangle.x_end = canvas[active_canvas].width;
    clip_rectangle.y_start = 0;
    clip_rectangle.y_end = canvas[active_canvas].height;
    if (active_canvas == CANVAS_UI) {
        translate_clip(translation.x, translation.y);
    }
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
    const clip_info *clip = graphics_get_clip_info(x, y, width, height);
    if (!clip->is_visible) {
        return;
    }
    int min_x = x + clip->clipped_pixels_left;
    int min_dy = clip->clipped_pixels_top;
    int max_dy = height - clip->clipped_pixels_bottom;
    for (int dy = min_dy; dy < max_dy; dy++) {
        memcpy(&buffer[dy * width], graphics_get_pixel(min_x, y + dy), sizeof(color_t) * clip->visible_pixels_x);
    }
}

void graphics_draw_from_buffer(int x, int y, int width, int height, const color_t *buffer)
{
    const clip_info *clip = graphics_get_clip_info(x, y, width, height);
    if (!clip->is_visible) {
        return;
    }
    int min_x = x + clip->clipped_pixels_left;
    int min_dy = clip->clipped_pixels_top;
    int max_dy = height - clip->clipped_pixels_bottom;
    for (int dy = min_dy; dy < max_dy; dy++) {
        memcpy(graphics_get_pixel(min_x, y + dy), &buffer[dy * width], sizeof(color_t) * clip->visible_pixels_x);
    }
}

color_t *graphics_get_pixel(int x, int y)
{
    if (active_canvas == CANVAS_UI) {
        return &canvas[CANVAS_UI].pixels[(translation.y + y) * canvas[CANVAS_UI].width + translation.x + x];
    } else {
        return &canvas[CANVAS_CITY].pixels[y * canvas[CANVAS_CITY].width + x];
    }
}

void graphics_clear_screen(canvas_type type)
{
    memset(canvas[type].pixels, 0, sizeof(color_t) * canvas[type].width * canvas[type].height);
}

void graphics_clear_city_viewport(void)
{
    int x, y, width, height;
    city_view_get_unscaled_viewport(&x, &y, &width, &height);
    while (y < height) {
        memset(graphics_get_pixel(0, y + TOP_MENU_HEIGHT), 0, width * sizeof(color_t));
        y++;
    }
}

void graphics_clear_screens(void)
{
    graphics_clear_screen(CANVAS_UI);
    if (config_get(CONFIG_UI_ZOOM)) {
        graphics_clear_screen(CANVAS_CITY);
    }
}

void graphics_draw_vertical_line(int x, int y1, int y2, color_t color)
{
    if (x < clip_rectangle.x_start || x >= clip_rectangle.x_end) {
        return;
    }
    int y_min = y1 < y2 ? y1 : y2;
    int y_max = y1 < y2 ? y2 : y1;
    y_min = y_min < clip_rectangle.y_start ? clip_rectangle.y_start : y_min;
    y_max = y_max >= clip_rectangle.y_end ? clip_rectangle.y_end - 1 : y_max;
    color_t *pixel = graphics_get_pixel(x, y_min);
    color_t *end_pixel = pixel + ((y_max - y_min) * canvas[active_canvas].width);
    while (pixel <= end_pixel) {
        *pixel = color;
        pixel += canvas[active_canvas].width;
    }
}

void graphics_draw_horizontal_line(int x1, int x2, int y, color_t color)
{
    if (y < clip_rectangle.y_start || y >= clip_rectangle.y_end) {
        return;
    }
    int x_min = x1 < x2 ? x1 : x2;
    int x_max = x1 < x2 ? x2 : x1;
    x_min = x_min < clip_rectangle.x_start ? clip_rectangle.x_start : x_min;
    x_max = x_max >= clip_rectangle.x_end ? clip_rectangle.x_end - 1 : x_max;
    color_t *pixel = graphics_get_pixel(x_min, y);
    color_t *end_pixel = pixel + (x_max - x_min);
    while (pixel <= end_pixel) {
        *pixel = color;
        ++pixel;
    }
}

void graphics_draw_rect(int x, int y, int width, int height, color_t color)
{
    graphics_draw_horizontal_line(x, x + width - 1, y, color);
    graphics_draw_horizontal_line(x, x + width - 1, y + height - 1, color);
    graphics_draw_vertical_line(x, y, y + height - 1, color);
    graphics_draw_vertical_line(x + width - 1, y, y + height - 1, color);
}

void graphics_draw_inset_rect(int x, int y, int width, int height)
{
    graphics_draw_horizontal_line(x, x + width - 1, y, COLOR_INSET_DARK);
    graphics_draw_vertical_line(x + width - 1, y, y + height - 1, COLOR_INSET_LIGHT);
    graphics_draw_horizontal_line(x, x + width - 1, y + height - 1, COLOR_INSET_LIGHT);
    graphics_draw_vertical_line(x, y, y + height - 1, COLOR_INSET_DARK);
}

void graphics_fill_rect(int x, int y, int width, int height, color_t color)
{
    for (int yy = y; yy < height + y; yy++) {
        graphics_draw_horizontal_line(x, x + width - 1, yy, color);
    }
}

void graphics_shade_rect(int x, int y, int width, int height, int darkness)
{
    const clip_info *cur_clip = graphics_get_clip_info(x, y, width, height);
    if (!cur_clip->is_visible) {
        return;
    }
    for (int yy = y + cur_clip->clipped_pixels_top; yy < y + height - cur_clip->clipped_pixels_bottom; yy++) {
        for (int xx = x + cur_clip->clipped_pixels_left; xx < x + width - cur_clip->clipped_pixels_right; xx++) {
            color_t *pixel = graphics_get_pixel(xx, yy);
            int r = (*pixel & 0xff0000) >> 16;
            int g = (*pixel & 0xff00) >> 8;
            int b = (*pixel & 0xff);
            int grey = (r + g + b) / 3 >> darkness;
            color_t new_pixel = (color_t) (ALPHA_OPAQUE | grey << 16 | grey << 8 | grey);
            *pixel = new_pixel;
        }
    }
}
