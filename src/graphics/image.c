#include "image.h"

#include "core/log.h"
#include "graphics/graphics.h"
#include "graphics/screen.h"

#include <string.h>

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30
#define FOOTPRINT_HALF_HEIGHT 15

typedef enum {
    DRAW_TYPE_SET,
    DRAW_TYPE_AND,
    DRAW_TYPE_NONE,
    DRAW_TYPE_BLEND,
    DRAW_TYPE_BLEND_ALPHA
} draw_type;

static const int FOOTPRINT_X_START_PER_HEIGHT[] = {
    28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0,
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28
};

static const int FOOTPRINT_OFFSET_PER_HEIGHT[] = {
    0, 2, 8, 18, 32, 50, 72, 98, 128, 162, 200, 242, 288, 338, 392, 450,
    508, 562, 612, 658, 700, 738, 772, 802, 828, 850, 868, 882, 892, 898
};

static int get_visible_footprint_pixels_per_row(int tiles, int width, int height, int row)
{
    int base_height = tiles * FOOTPRINT_HEIGHT;
    int footprint_row = row - (height - base_height);
    if (footprint_row < 0) {
        return 0;
    } else if (footprint_row < tiles * FOOTPRINT_HALF_HEIGHT) {
        return 2 + 4 * footprint_row;
    } else {
        return 2 + 4 * (base_height - 1 - footprint_row);
    }
}

static void draw_modded_footprint(int image_id, int x_offset, int y_offset, color_t color)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }
    int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    int y_top_offset = img->height - FOOTPRINT_HEIGHT * tiles;
    y_offset -= y_top_offset + FOOTPRINT_HALF_HEIGHT * tiles - FOOTPRINT_HALF_HEIGHT;
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset + y_top_offset, img->width, img->height - y_top_offset);
    if (!clip->is_visible) {
        return;
    }
    data += img->width * (clip->clipped_pixels_top + y_top_offset);
    for (int y = clip->clipped_pixels_top + y_top_offset; y < img->height - clip->clipped_pixels_bottom; y++) {
        int visible_pixels_per_row = get_visible_footprint_pixels_per_row(tiles, img->width, img->height, y);
        int x_start = (img->width - visible_pixels_per_row) / 2;
        int x_max = img->width - x_start;
        if (x_start < clip->clipped_pixels_left) {
            x_start = clip->clipped_pixels_left;
        }
        if (x_max > img->width - clip->clipped_pixels_right) {
            x_max = img->width - clip->clipped_pixels_right;
        }
        if (x_start >= x_max) {
            data += img->width;
            continue;
        }
        color_t *dst = graphics_get_pixel(x_offset + x_start, y_offset + y);
        data += x_start;
        if (color && color != COLOR_MASK_NONE) {
            for (int x = x_start; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data & color;
                }
                data++;
            }
        } else {
            for (int x = x_start; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data;
                }
                data++;
            }
        }
        data += img->width - x_max;
    }
}

static void draw_modded_top(int image_id, int x_offset, int y_offset, color_t color)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }
    int tiles = (img->width + 2) / (FOOTPRINT_WIDTH + 2);
    int y_top_offset = img->height - FOOTPRINT_HEIGHT * tiles;
    y_top_offset += FOOTPRINT_HALF_HEIGHT * tiles - FOOTPRINT_HALF_HEIGHT;
    y_offset -= y_top_offset;
    int height = img->height - FOOTPRINT_HALF_HEIGHT * tiles;
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    data += img->width * clip->clipped_pixels_top;
    for (int y = clip->clipped_pixels_top; y < height - clip->clipped_pixels_bottom; y++) {
        int visible_pixels_per_row = get_visible_footprint_pixels_per_row(tiles, img->width, img->height, y);
        int half_width = img->width / 2;
        int half_visible_pixels = visible_pixels_per_row / 2;
        int x_start = clip->clipped_pixels_left;
        if (x_start < half_width) {
            color_t *dst = graphics_get_pixel(x_offset + x_start, y_offset + y);
            int x_max = half_width - half_visible_pixels;
            if (x_start > x_max) {
                x_start = x_max;
            }
            data += x_start;
            int half_image_only = 0;
            if (img->width - clip->clipped_pixels_right < x_max) {
                x_max = img->width - clip->clipped_pixels_right;
                half_image_only = 1;
            }
            if (color && color != COLOR_MASK_NONE) {
                for (int x = x_start; x < x_max; x++, dst++) {
                    color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                    if (alpha == ALPHA_OPAQUE) {
                        *dst = *data & color;
                    } else if (alpha != ALPHA_TRANSPARENT) {
                        *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA) & color;
                    }
                    data++;
                }
            } else {
                for (int x = x_start; x < x_max; x++, dst++) {
                    color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                    if (alpha == ALPHA_OPAQUE) {
                        *dst = *data;
                    } else if (alpha != ALPHA_TRANSPARENT) {
                        *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA);
                    }
                    data++;
                }
            }
            if (half_image_only) {
                data += clip->clipped_pixels_right;
                continue;
            }
            data += half_width + half_visible_pixels - x_max;
            x_start = half_width + half_visible_pixels;
        } else {
            x_start = half_width + half_visible_pixels;
            if (x_start < clip->clipped_pixels_left) {
                x_start = clip->clipped_pixels_left;
            }
            data += x_start;
        }
        int x_max = img->width - clip->clipped_pixels_right;
        color_t *dst = graphics_get_pixel(x_offset + x_start, y_offset + y);
        if (color && color != COLOR_MASK_NONE) {
            for (int x = x_start; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data & color;
                } else if (alpha != ALPHA_TRANSPARENT) {
                    *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA) & color;
                }
                data++;
            }
        } else {
            for (int x = x_start; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data;
                } else if (alpha != ALPHA_TRANSPARENT) {
                    *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA);
                }
                data++;
            }
        }
        if (x_start > x_max) {
            data -= x_start - x_max;
        }
        data += clip->clipped_pixels_right;
    }
}

static void draw_modded_image(const image *img, const color_t *data, int x_offset, int y_offset, color_t color)
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
        if (color && color != COLOR_MASK_NONE) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data & color;
                } else if (alpha != ALPHA_TRANSPARENT) {
                    *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA) & color;
                }
                data++;
            }
        } else {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                color_t alpha = *data & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = *data;
                } else if (alpha != ALPHA_TRANSPARENT) {
                    *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(*data, *dst, alpha >> COLOR_BITSHIFT_ALPHA);
                }
                data++;
            }
        }
        data += clip->clipped_pixels_right;
    }
}

static void draw_uncompressed(const image *img, const color_t *data, int x_offset, int y_offset, color_t color, draw_type type)
{
    if (img->draw.type == IMAGE_TYPE_MOD) {
        draw_modded_image(img, data, x_offset, y_offset, color);
        return;
    }
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
            if (img->draw.type == IMAGE_TYPE_WITH_TRANSPARENCY || img->draw.is_external) { // can be transparent
                for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                    if (*data != COLOR_SG2_TRANSPARENT) {
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
                if (*data != COLOR_SG2_TRANSPARENT) {
                    *dst = color;
                }
                data++;
            }
        } else if (type == DRAW_TYPE_AND) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_SG2_TRANSPARENT) {
                    *dst = *data & color;
                }
                data++;
            }
        } else if (type == DRAW_TYPE_BLEND) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_SG2_TRANSPARENT) {
                    *dst &= color;
                }
                data++;
            }
        } else if (type == DRAW_TYPE_BLEND_ALPHA) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                if (*data != COLOR_SG2_TRANSPARENT) {
                    color_t alpha = COLOR_COMPONENT(*data, COLOR_BITSHIFT_ALPHA);
                    if (alpha == 255) {
                        *dst = color;
                    } else {
                        color_t s = color;
                        color_t d = *dst;
                        *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(s, d, alpha);
                    }
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
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    while (b) {
                        *dst = color;
                        dst++;
                        b--;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst = color;
                        }
                        dst++;
                        x++;
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
                data += b;
                color_t *dst = graphics_get_pixel(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    while (b) {
                        *dst &= color;
                        dst++;
                        b--;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            *dst &= color;
                        }
                        dst++;
                        x++;
                        b--;
                    }
                }
            }
        }
    }
}

static void draw_compressed_blend_alpha(const image *img, const color_t *data, int x_offset, int y_offset, int height, color_t color)
{
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, img->width, height);
    if (!clip->is_visible) {
        return;
    }
    color_t alpha = COLOR_COMPONENT(color, COLOR_BITSHIFT_ALPHA);
    if (!alpha) {
        return;
    }
    if (alpha == 255) {
        draw_compressed_set(img, data, x_offset, y_offset, height, color);
        return;
    }
    color_t alpha_dst = 256 - alpha;
    color_t src_rb = (color & 0xff00ff) * alpha;
    color_t src_g = (color & 0x00ff00) * alpha;
    int unclipped = clip->clip_x == CLIP_NONE;

    for (int y = 0; y < height - clip->clipped_pixels_bottom; y++) {
        int x = 0;
        color_t *dst = graphics_get_pixel(x_offset, y_offset + y);
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                dst += *data;
                data++;
            } else if (y < clip->clipped_pixels_top) {
                data += b;
                x += b;
                dst += b;
            } else {
                data += b;
                if (unclipped) {
                    x += b;
                    while (b) {
                        color_t d = *dst;
                        *dst = (((src_rb + (d & 0xff00ff) * alpha_dst) & 0xff00ff00) |
                                ((src_g  + (d & 0x00ff00) * alpha_dst) & 0x00ff0000)) >> 8;
                        b--;
                        dst++;
                    }
                } else {
                    while (b) {
                        if (x >= clip->clipped_pixels_left && x < img->width - clip->clipped_pixels_right) {
                            color_t d = *dst;
                            *dst = (((src_rb + (d & 0xff00ff) * alpha_dst) & 0xff00ff00) |
                                   ((src_g  + (d & 0x00ff00) * alpha_dst) & 0x00ff0000)) >> 8;
                        }
                        dst++;
                        x++;
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
        color_mask = COLOR_MASK_NONE;
    }
    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, FOOTPRINT_WIDTH, FOOTPRINT_HEIGHT);
    if (!clip->is_visible) {
        return;
    }
    // If the current tile neither clipped nor color masked, just draw it normally
    if (clip->clip_y == CLIP_NONE && clip->clip_x == CLIP_NONE && color_mask == COLOR_MASK_NONE) {
        draw_footprint_simple(data, x_offset, y_offset);
        return;
    }
    int clip_left = clip->clip_x == CLIP_LEFT || clip->clip_x == CLIP_BOTH;
    int clip_right = clip->clip_x == CLIP_RIGHT || clip->clip_x == CLIP_BOTH;
    const color_t *src = &data[FOOTPRINT_OFFSET_PER_HEIGHT[clip->clipped_pixels_top]];
    for (int y = clip->clipped_pixels_top; y < clip->clipped_pixels_top + clip->visible_pixels_y; y++) {
        int x_start = FOOTPRINT_X_START_PER_HEIGHT[y];
        int x_max = 58 - x_start * 2;
        int x_pixel_advance = 0;
        if (clip_left) {
            if (clip->clipped_pixels_left + clip->visible_pixels_x < x_start) {
                src += x_max;
                continue;
            }
            if (clip->clipped_pixels_left > x_start) {
                int pixels_to_reduce = clip->clipped_pixels_left - x_start;
                if (pixels_to_reduce >= x_max) {
                    src += x_max;
                    continue;
                }
                src += pixels_to_reduce;
                x_max -= pixels_to_reduce;
                x_start = clip->clipped_pixels_left;
            }
        }
        if (clip_right) {
            int clip_x = 58 - clip->clipped_pixels_right;
            if (clip_x < x_start) {
                src += x_max;
                continue;
            }
            if (x_start + x_max > clip_x) {
                int temp_x_max = clip_x - x_start;
                x_pixel_advance = x_max - temp_x_max;
                x_max = temp_x_max;
            }
        }
        color_t *buffer = graphics_get_pixel(x_offset + x_start, y_offset + y);
        if (color_mask == COLOR_MASK_NONE) {
            memcpy(buffer, src, x_max * sizeof(color_t));
            src += x_max + x_pixel_advance;
        } else {
            for (int x = 0; x < x_max; x++, buffer++, src++) {
                *buffer = *src & color_mask;
            }
            src += x_pixel_advance;
        }
    }
}

static const color_t *tile_data(const color_t *data, int index)
{
    return &data[900 * index];
}

static void draw_footprint_size1(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    draw_footprint_tile(tile_data(data, 0), x, y, color_mask);
}

static void draw_footprint_size2(int image_id, int x, int y, color_t color_mask)
{
    const color_t *data = image_data(image_id);

    int index = 0;
    draw_footprint_tile(tile_data(data, index++), x, y, color_mask);

    draw_footprint_tile(tile_data(data, index++), x - 30, y + 15, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 15, color_mask);

    draw_footprint_tile(tile_data(data, index++), x, y + 30, color_mask);
}

static void draw_footprint_size3(int image_id, int x, int y, color_t color_mask)
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
}

static void draw_footprint_size4(int image_id, int x, int y, color_t color_mask)
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
}

static void draw_footprint_size5(int image_id, int x, int y, color_t color_mask)
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
}

static void draw_footprint_size7(int image_id, int x, int y, color_t color_mask)
{
    const color_t* data = image_data(image_id);

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

    draw_footprint_tile(tile_data(data, index++), x-150, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+150, y + 75, color_mask);
    draw_footprint_tile(tile_data(data, index++), x-120, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+120, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x-90, y + 105, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+90, y + 105, color_mask);
    draw_footprint_tile(tile_data(data, index++), x-60, y + 120, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+60, y + 120, color_mask);
    draw_footprint_tile(tile_data(data, index++), x-30, y + 135, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+30, y + 135, color_mask);
    draw_footprint_tile(tile_data(data, index++), x-180, y + 150, color_mask);
    draw_footprint_tile(tile_data(data, index++), x+180, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 150, y + 90, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 150, y + 105, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 120, y + 105, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 120, y + 120, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 90, y + 120, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 90, y + 135, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 60, y + 135, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 60, y + 150, color_mask);
    draw_footprint_tile(tile_data(data, index++), x - 30, y + 165, color_mask);
    draw_footprint_tile(tile_data(data, index++), x + 30, y + 165, color_mask);
    draw_footprint_tile(tile_data(data, index++), x, y + 180, color_mask);

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

    if (img->draw.type == IMAGE_TYPE_ISOMETRIC) {
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

    if (img->draw.type == IMAGE_TYPE_ISOMETRIC) {
        return;
    }

    if (img->draw.is_fully_compressed) {
        draw_compressed_blend(img, data, x, y, img->height, color);
    } else {
        draw_uncompressed(img, data, x, y, color, DRAW_TYPE_BLEND);
    }
}

void image_draw_blend_alpha(int image_id, int x, int y, color_t color)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
        return;
    }

    if (img->draw.type == IMAGE_TYPE_ISOMETRIC) {
        return;
    }

    if (img->draw.is_fully_compressed) {
        draw_compressed_blend_alpha(img, data, x, y, img->height, color);
    } else {
        draw_uncompressed(img, data, x, y, color, DRAW_TYPE_BLEND_ALPHA);
    }
}

static void draw_multibyte_letter(font_t font, const image *img, const color_t *data, int x, int y, color_t color)
{
    switch (font) {
        case FONT_NORMAL_WHITE:
            draw_uncompressed(img, data, x + 1, y + 1, 0x311c10, DRAW_TYPE_BLEND_ALPHA);
            draw_uncompressed(img, data, x, y, COLOR_WHITE, DRAW_TYPE_BLEND_ALPHA);
            break;
        case FONT_NORMAL_RED:
            draw_uncompressed(img, data, x + 1, y + 1, 0xe7cfad, DRAW_TYPE_BLEND_ALPHA);
            draw_uncompressed(img, data, x, y, 0x731408, DRAW_TYPE_BLEND_ALPHA);
            break;
        case FONT_NORMAL_GREEN:
            draw_uncompressed(img, data, x + 1, y + 1, 0xe7cfad, DRAW_TYPE_BLEND_ALPHA);
            draw_uncompressed(img, data, x, y, 0x311c10, DRAW_TYPE_BLEND_ALPHA);
            break;
        case FONT_NORMAL_PLAIN:
            draw_uncompressed(img, data, x, y + 2, color, DRAW_TYPE_BLEND_ALPHA);
            break;
        case FONT_NORMAL_BLACK:
        case FONT_LARGE_BLACK:
            draw_uncompressed(img, data, x + 1, y + 1, 0xcead9c, DRAW_TYPE_BLEND_ALPHA);
            draw_uncompressed(img, data, x, y, color, DRAW_TYPE_BLEND_ALPHA);
            break;
        default:
            draw_uncompressed(img, data, x, y, color, DRAW_TYPE_BLEND_ALPHA);
            break;
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
    graphics_clear_screens();
    int s_width = screen_width();
    int s_height = screen_height();
    const image *img = image_get(image_id);
    double scale_w = screen_width() / (double) img->width;
    double scale_h = screen_height() / (double) img->height;
    double scale = scale_w > scale_h ? scale_w : scale_h;

    if (scale <= 1.0f) {
        image_draw(image_id, (s_width - img->width) / 2, (s_height - img->height) / 2);
    } else {
        image_draw_scaled(image_id, (int) ((s_width - img->width * scale) / 2), (int) ((s_height - img->height * scale) / 2), scale);
    }
}

void image_draw_isometric_footprint(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != IMAGE_TYPE_ISOMETRIC) {
        if (img->draw.type == IMAGE_TYPE_MOD) {
            draw_modded_footprint(image_id, x, y, color_mask);
        }
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
        case 418:
            draw_footprint_size7(image_id, x, y, color_mask);
            break;
    }
}

void image_draw_isometric_footprint_from_draw_tile(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != IMAGE_TYPE_ISOMETRIC) {
        if (img->draw.type == IMAGE_TYPE_MOD) {
            draw_modded_footprint(image_id, x, y, color_mask);
        }
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
        case 418:
            draw_footprint_size7(image_id, x + 150, y - 75, color_mask);
            break;

    }
}

void image_draw_isometric_top(int image_id, int x, int y, color_t color_mask)
{
    const image *img = image_get(image_id);
    if (img->draw.type != IMAGE_TYPE_ISOMETRIC) {
        if (img->draw.type == IMAGE_TYPE_MOD) {
            draw_modded_top(image_id, x, y, color_mask);
        }
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
    if (img->draw.type != IMAGE_TYPE_ISOMETRIC) {
        if (img->draw.type == IMAGE_TYPE_MOD) {
            draw_modded_top(image_id, x, y, color_mask);
        }
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

void image_draw_scaled(int image_id, int x_offset, int y_offset, double scale_factor)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);

    if (!data || img->draw.type == IMAGE_TYPE_ISOMETRIC || img->draw.is_fully_compressed || !scale_factor) {
        return;
    }

    int width = (int) (img->width * scale_factor);
    int height = (int) (img->height * scale_factor);

    const clip_info *clip = graphics_get_clip_info(x_offset, y_offset, width, height);
    if (!clip->is_visible) {
        return;
    }
    for (int y = clip->clipped_pixels_top; y < height - clip->clipped_pixels_bottom; y++) {
        color_t *dst = graphics_get_pixel(x_offset + clip->clipped_pixels_left, y_offset + y);
        int x_max = width - clip->clipped_pixels_right;
        int image_y_offset = (int) (y / scale_factor) * img->width;
        if (img->draw.type == IMAGE_TYPE_MOD) {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                color_t pixel = data[(int) (image_y_offset + x / scale_factor)];
                color_t alpha = pixel & COLOR_CHANNEL_ALPHA;
                if (alpha == ALPHA_OPAQUE) {
                    *dst = pixel;
                } else if (alpha != ALPHA_TRANSPARENT) {
                    *dst = COLOR_BLEND_ALPHA_TO_OPAQUE(pixel, *dst, alpha >> COLOR_BITSHIFT_ALPHA);
                }
            }
        } else {
            for (int x = clip->clipped_pixels_left; x < x_max; x++, dst++) {
                color_t pixel = data[(int) (image_y_offset + x / scale_factor)];
                if (pixel != COLOR_SG2_TRANSPARENT) {
                    *dst = pixel;
                }
            }
        }
    }
}
