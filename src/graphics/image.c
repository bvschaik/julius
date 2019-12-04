#include "image.h"

#include "core/log.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include <string.h>

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30

typedef enum {
    DRAW_TYPE_SET,
    DRAW_TYPE_AND,
    DRAW_TYPE_NONE,
    DRAW_TYPE_BLEND
} draw_type;

static void draw_uncompressed(const image *img, const color_t *data, int x_offset, int y_offset, color_t color, draw_type type)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, img->height);
    if (!clip->is_visible) {
        return;
    }
    data += img->width * clip->clipped_pixels_top;
    for (int y = clip->clipped_pixels_top; y < img->height - clip->clipped_pixels_bottom; y++) {
        data += clip->clipped_pixels_left;
        color_t *dst = graphics_get_pixel(x_offset + clip->clipped_pixels_left, y_offset + y);
        int x_max = img->width - clip->clipped_pixels_right;
        if (type == DRAW_TYPE_NONE) {
            if (img->draw.type == 0 || img->draw.is_external) { // can be transparent
                for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                    if (*data != COLOR_TRANSPARENT) {
                        *dst = *data;
                    }
                    data++;
                }
            } else {
                int num_pixels = x_max - clip->clipped_pixels_left;
                memcpy(dst, data, num_pixels * sizeof(color_t));
                data += num_pixels;
            }
        } else if (type == DRAW_TYPE_SET) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst = color;
                }
                data++;
            }
        } else if (type == DRAW_TYPE_AND) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst = *data & color;
                }
                data++;
            }
        } else if (type == DRAW_TYPE_BLEND) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst &= color;
                }
                data++;
            }
        }
        data += clip->clipped_pixels_right;
    }
}

static void draw_compressed(const image *img, const color_t *data, int x_offset, int y_offset, int height)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    int unclipped = clip->clip_x == CLIP_NONE;

    for (int y = 0; y < height - clip->clipped_pixels_bottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clipped_pixels_top) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    memcpy(dst, pixels, b * sizeof(color_t));
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst = *pixels;
                        }
                        dst++;
                        x++;
                        pixels++;
                        b--;
                    }
                }
            }
        }
    }
}

static void draw_compressed_set(const image *img, const color_t *data, int x_offset, int y_offset, int height, color_t color)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    int unclipped = clip->clip_x == CLIP_NONE;

    for (int y = 0; y < height - clip->clipped_pixels_bottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clipped_pixels_top) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    while (b) {
                        *dst = color;
                        dst++;
                        pixels++;
                        b--;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst = color;
                        }
                        dst++;
                        x++;
                        pixels++;
                        b--;
                    }
                }
            }
        }
    }
}

static void draw_compressed_and(const image *img, const color_t *data, int x_offset, int y_offset, int height, color_t color)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    int unclipped = clip->clip_x == CLIP_NONE;

    for (int y = 0; y < height - clip->clipped_pixels_bottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clipped_pixels_top) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    while (b) {
                        *dst = *pixels & color;
                        dst++;
                        pixels++;
                        b--;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst = *pixels & color;
                        }
                        dst++;
                        x++;
                        pixels++;
                        b--;
                    }
                }
            }
        }
    }
}

static void draw_compressed_blend(const image *img, const color_t *data, int x_offset, int y_offset, int height, color_t color)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    int unclipped = clip->clip_x == CLIP_NONE;

    for (int y = 0; y < height - clip->clipped_pixels_bottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clipped_pixels_top) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    while (b) {
                        *dst &= color;
                        dst++;
                        pixels++;
                        b--;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst &= color;
                        }
                        dst++;
                        x++;
                        pixels++;
                        b--;
                    }
                }
            }
        }
    }
}

static void draw_footprint_simple(const color_t *src, int x, int y)
{
    memcpy(graphics_get_pixel(x + 28, y + 0), &src[0], 2 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 26, y + 1), &src[2], 6 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 24, y + 2), &src[8], 10 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 22, y + 3), &src[18], 14 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 20, y + 4), &src[32], 18 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 18, y + 5), &src[50], 22 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 16, y + 6), &src[72], 26 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 14, y + 7), &src[98], 30 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 12, y + 8), &src[128], 34 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 10, y + 9), &src[162], 38 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 8, y + 10), &src[200], 42 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 6, y + 11), &src[242], 46 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 4, y + 12), &src[288], 50 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 2, y + 13), &src[338], 54 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 0, y + 14), &src[392], 58 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 0, y + 15), &src[450], 58 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 2, y + 16), &src[508], 54 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 4, y + 17), &src[562], 50 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 6, y + 18), &src[612], 46 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 8, y + 19), &src[658], 42 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 10, y + 20), &src[700], 38 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 12, y + 21), &src[738], 34 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 14, y + 22), &src[772], 30 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 16, y + 23), &src[802], 26 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 18, y + 24), &src[828], 22 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 20, y + 25), &src[850], 18 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 22, y + 26), &src[868], 14 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 24, y + 27), &src[882], 10 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 26, y + 28), &src[892], 6 * sizeof(color_t));
    memcpy(graphics_get_pixel(x + 28, y + 29), &src[898], 2 * sizeof(color_t));
}

static void draw_footprint_tile(const color_t *data, int x_offset, int y_offset, color_t color_mask)
{
    if (!color_mask) {
        color_mask = COLOR_NO_MASK;
    }
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, FOOTPRINT_WIDTH, FOOTPRINT_HEIGHT);
    if (!clip->is_visible) {
        return;
    }
    // footprints are ALWAYS clipped in half, if they are clipped
    if (clip->clip_y == CLIP_NONE && clip->clip_x == CLIP_NONE && color_mask == COLOR_NO_MASK) {
        draw_footprint_simple(data, x_offset, y_offset);
        return;
    }
    int clip_left = clip->clip_x == CLIP_LEFT;
    int clip_right = clip->clip_x == CLIP_RIGHT;
    if (clip->clip_y != CLIP_TOP) {
        const color_t *src = data;
        for (int y = 0; y < 15; y++) {
            int x_max = 4 * y + 2;
            int x_start = 29 - 1 - 2 * y;
            if (clip_left || clip_right) {
                x_max = 2 * y;
            }
            if (clip_left) {
                x_start = 30;
                src += x_max + 2;
            }
            color_t *buffer = graphics_get_pixel(x_offset + x_start, y_offset + y);
            if (color_mask == COLOR_NO_MASK) {
                memcpy(buffer, src, x_max * sizeof(color_t));
                src += x_max;
            } else {
                for (int x = 0; x < x_max; x++, buffer++, src++) {
                    *buffer = *src & color_mask;
                }
            }
            if (clip_right) {
                src += x_max + 2;
            }
        }
    }
    if (clip->clip_y != CLIP_BOTTOM) {
        const color_t *src = &data[900 / 2];
        for (int y = 0; y < 15; y++) {
            int x_max = 4 * (15 - 1 - y) + 2;
            int x_start = 2 * y;
            if (clip_left || clip_right) {
                x_max = x_max / 2 - 1;
            }
            if (clip_left) {
                x_start = 30;
                src += x_max + 2;
            }
            color_t *buffer = graphics_get_pixel(x_offset + x_start, 15 + y_offset + y);
            if (color_mask == COLOR_NO_MASK) {
                memcpy(buffer, src, x_max * sizeof(color_t));
                src += x_max;
            } else {
                for (int x = 0; x < x_max; x++, buffer++, src++) {
                    *buffer = *src & color_mask;
                }
            }
            if (clip_right) {
                src += x_max + 2;
            }
        }
    }
}

static const color_t *tile_data(const color_t *data, int index)
{
    return &data[900 * index];
}

static int draw_footprint_size1(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    draw_footprint_tile(tile_data(data, 0), x, y, color_mask);

    return FOOTPRINT_WIDTH;
}

static int draw_footprint_size2(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    int index = 0;
    draw_footprint_tile(tile_data(data, index++), x, y, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 15, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 15, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x, y + 30, color_mask);
    
    return FOOTPRINT_WIDTH;
}

static int draw_footprint_size3(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);
    
    int index = 0;
    draw_footprint_tile(tile_data(data, index++), x, y, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 30, y + 15, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 15, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x - 60, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 30, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 45, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x, y + 60, color_mask);

    return FOOTPRINT_WIDTH;
}

static int draw_footprint_size4(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    int index = 0;
    draw_footprint_tile(tile_data(data, index++), x, y, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 30, y + 15, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 15, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 60, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 30, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 90, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 90, y + 45, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 60, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 60, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 75, color_mask);

    draw_footprint_tile(tile_data(data, index++), x, y + 90, color_mask);
    
    return FOOTPRINT_WIDTH;
}

static int draw_footprint_size5(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    int index = 0;
    draw_footprint_tile(tile_data(data, index++), x, y, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 30, y + 15, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 15, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 60, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 30, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 30, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 90, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 45, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 90, y + 45, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 120, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 60, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 60, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 120, y + 60, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 90, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 90, y + 75, color_mask);
    
    draw_footprint_tile(tile_data(data, index++), x - 60, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 90, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 30, y + 105, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 105, color_mask);

    draw_footprint_tile(tile_data(data, index++), x, y + 120, color_mask);
    
    return FOOTPRINT_WIDTH;
}


void image_draw(int image_id, int x, int y)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }

    if (img->draw.is_fully_compressed) {
        draw_compressed(img, data, x, y, img->height);
    } else {
        draw_uncompressed(img, data, x, y, 0, DRAW_TYPE_NONE);
    }
}

void image_draw_enemy(int image_id, int x, int y)
{
    if (image_id <= 0 || image_id >= 801) {
        return;
    }
    const image *img = image_get_enemy(image_id);
    const color_t *data = image_data_enemy(image_id);
    if (data) {
        draw_compressed(img, data, x, y, img->height);
    }
}

void image_draw_masked(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }

    if (img->draw.type == 30) { // isometric
        log_error("use image_draw_isometric_footprint for isometric!", 0, image_id);
        return;
    }

    if (img->draw.is_fully_compressed) {
        if (!color_mask) {
            draw_compressed(img, data, x, y, img->height);
        } else {
            draw_compressed_and(img, data, x, y, img->height, color_mask);
        }
    } else {
        draw_uncompressed(img, data, x, y,
                              color_mask, color_mask ? DRAW_TYPE_AND : DRAW_TYPE_NONE);
    }
}

void image_draw_blend(int image_id, int x, int y, color_t color)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }

    if (img->draw.type == 30) { // isometric
        return;
    }

    if (img->draw.is_fully_compressed) {
        draw_compressed_blend(img, data, x, y, img->height, color);
    } else {
        draw_uncompressed(img, data, x, y, color, DRAW_TYPE_BLEND);
    }
}

static void draw_multibyte_letter(font_t font, const image *img, const color_t *data, int x, int y, color_t color)
{
    switch (font) {
        case FONT_NORMAL_WHITE:
            draw_uncompressed(img, data, x + 1, y + 1, 0x311c10, DRAW_TYPE_SET);
            draw_uncompressed(img, data, x, y, COLOR_WHITE, DRAW_TYPE_SET);
            break;
        case FONT_NORMAL_RED:
            draw_uncompressed(img, data, x + 1, y + 1, 0xe7cfad, DRAW_TYPE_SET);
            draw_uncompressed(img, data, x, y, 0x731408, DRAW_TYPE_SET);
            break;
        case FONT_NORMAL_GREEN:
            draw_uncompressed(img, data, x + 1, y + 1, 0xe7cfad, DRAW_TYPE_SET);
            draw_uncompressed(img, data, x, y, 0x311c10, DRAW_TYPE_SET);
            break;

        default:
            draw_uncompressed(img, data, x, y,
                color, color ? DRAW_TYPE_SET : DRAW_TYPE_NONE);
    }
}

void image_draw_letter(font_t font, int letter_id, int x, int y, color_t color)
{
    const image *img = image_letter(letter_id);
    const color_t *data = image_data_letter(letter_id);
    if (!data) {
        return;
    }
    if (letter_id >= IMAGE_FONT_MULTIBYTE_OFFSET) {
        draw_multibyte_letter(font, img, data, x, y, color);
        return;
    }

    if (img->draw.is_fully_compressed) {
        if (color) {
            draw_compressed_set(img, data, x, y, img->height, color);
        } else {
            draw_compressed(img, data, x, y, img->height);
        }
    } else {
        draw_uncompressed(img, data, x, y,
            color, color ? DRAW_TYPE_SET : DRAW_TYPE_NONE);
    }
}

void image_draw_fullscreen_background(int image_id)
{
    int s_width = screen_width();
    int s_height = screen_height();
    if (s_width > 1024 || s_height > 768) {
        graphics_clear_screen();
    }
    image_draw(image_id, (s_width - 1024) / 2, (s_height - 768) / 2);
}

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != 30) { // isometric
        return;
    }
    switch (img->width) {
        case 58:
            draw_footprint_size1(image_id, x, y, color_mask);
            break;
        case 118:
            draw_footprint_size2(image_id, x, y, color_mask);
            break;
        case 178:
            draw_footprint_size3(image_id, x, y, color_mask);
            break;
        case 238:
            draw_footprint_size4(image_id, x, y, color_mask);
            break;
        case 298:
            draw_footprint_size5(image_id, x, y, color_mask);
            break;
    }
}

void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != 30) { // isometric
        return;
    }
    switch (img->width) {
        case 58:
            draw_footprint_size1(image_id, x, y, color_mask);
            break;
        case 118:
            draw_footprint_size2(image_id, x + 30, y - 15, color_mask);
            break;
        case 178:
            draw_footprint_size3(image_id, x + 60, y - 30, color_mask);
            break;
        case 238:
            draw_footprint_size4(image_id, x + 90, y - 45, color_mask);
            break;
        case 298:
            draw_footprint_size5(image_id, x + 120, y - 60, color_mask);
            break;
    }
}

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != 30) { // isometric
        return;
    }
    if (!img->draw.has_compressed_part) {
        return;
    }
    const color_t *data = &image_data(image_id)[img->draw.uncompressed_length];

    int height = img->height;
    switch (img->width) {
        case 58:
            y -= img->height - 30;
            height -= 16;
            break;
        case 118:
            x -= 30;
            y -= img->height - 60;
            height -= 31;
            break;
        case 178:
            x -= 60;
            y -= img->height - 90;
            height -= 46;
            break;
        case 238:
            x -= 90;
            y -= img->height - 120;
            height -= 61;
            break;
        case 298:
            x -= 120;
            y -= img->height - 150;
            height -= 76;
            break;
    }
    if (!color_mask) {
        draw_compressed(img, data, x, y, height);
    } else {
        draw_compressed_and(img, data, x, y, height, color_mask);
    }
}

void image_draw_isometric_top_from_draw_tile(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != 30) { // isometric
        return;
    }
    if (!img->draw.has_compressed_part) {
        return;
    }
    const color_t *data = &image_data(image_id)[img->draw.uncompressed_length];

    int height = img->height;
    switch (img->width) {
        case 58:
            y -= img->height - 30;
            height -= 16;
            break;
        case 118:
            y -= img->height - 45;
            height -= 31;
            break;
        case 178:
            y -= img->height - 60;
            height -= 46;
            break;
        case 238:
            y -= img->height - 75;
            height -= 61;
            break;
        case 298:
            y -= img->height - 90;
            height -= 76;
            break;
    }
    if (!color_mask) {
        draw_compressed(img, data, x, y, height);
    } else {
        draw_compressed_and(img, data, x, y, height, color_mask);
    }
}
