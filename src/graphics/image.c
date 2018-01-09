#include "image.h"

#include "graphics/graphics.h"
#include "graphics/screen.h"

#include <stdio.h>
#include <string.h>

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30

typedef enum {
    ColorType_Set,
    ColorType_And,
    ColorType_None,
    ColorType_Blend
} ColorType;

static void draw_uncompressed(const image *img, const color_t *data, int x_offset, int y_offset, color_t color, ColorType type)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, y_offset, img->width, img->height);
    if (!clip->isVisible) {
        return;
    }
    data += img->width * clip->clippedPixelsTop;
    for (int y = clip->clippedPixelsTop; y < img->height - clip->clippedPixelsBottom; y++) {
        data += clip->clippedPixelsLeft;
        color_t *dst = Graphics_getDrawPosition(x_offset + clip->clippedPixelsLeft, y_offset + y);
        int xMax = img->width - clip->clippedPixelsRight;
        if (type == ColorType_None) {
            if (img->draw.type == 0 || img->draw.is_external) { // can be transparent
                for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
                    if (*data != COLOR_TRANSPARENT) {
                        *dst = *data;
                    }
                    data++;
                }
            } else {
                int num_pixels = xMax - clip->clippedPixelsLeft;
                memcpy(dst, data, num_pixels * sizeof(color_t));
                data += num_pixels;
            }
        } else if (type == ColorType_Set) {
            for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst = color;
                }
                data++;
            }
        } else if (type == ColorType_And) {
            for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst = *data & color;
                }
                data++;
            }
        } else if (type == ColorType_Blend) {
            for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
                if (*data != COLOR_TRANSPARENT) {
                    *dst &= color;
                }
                data++;
            }
        }
        data += clip->clippedPixelsRight;
    }
}

static void draw_compressed(const image *img, const color_t *data, int x_offset, int y_offset, int height)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, y_offset, img->width, height);
    if (!clip->isVisible) {
        return;
    }
    int unclipped = clip->clipX == ClipNone;

    for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clippedPixelsTop) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = Graphics_getDrawPosition(x_offset + x, y_offset + y);
                if (unclipped) {
                    x += b;
                    memcpy(dst, pixels, b * sizeof(color_t));
                } else {
                    while (b) {
                        if (x >= clip->clippedPixelsLeft && x < img->width - clip->clippedPixelsRight) {
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

static void draw_compressed_set(const image *img, const color_t *data, int x_offset, int yOffset, int height, color_t color)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, yOffset, img->width, height);
    if (!clip->isVisible) {
        return;
    }
    int unclipped = clip->clipX == ClipNone;

    for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clippedPixelsTop) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = Graphics_getDrawPosition(x_offset + x, yOffset + y);
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
                        if (x >= clip->clippedPixelsLeft && x < img->width - clip->clippedPixelsRight) {
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
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, y_offset, img->width, height);
    if (!clip->isVisible) {
        return;
    }
    int unclipped = clip->clipX == ClipNone;

    for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clippedPixelsTop) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = Graphics_getDrawPosition(x_offset + x, y_offset + y);
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
                        if (x >= clip->clippedPixelsLeft && x < img->width - clip->clippedPixelsRight) {
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
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, y_offset, img->width, height);
    if (!clip->isVisible) {
        return;
    }
    int unclipped = clip->clipX == ClipNone;

    for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
        int x = 0;
        while (x < img->width) {
            color_t b = *data;
            data++;
            if (b == 255) {
                // transparent pixels to skip
                x += *data;
                data++;
            } else if (y < clip->clippedPixelsTop) {
                data += b;
                x += b;
            } else {
                // number of concrete pixels
                const color_t *pixels = data;
                data += b;
                color_t *dst = Graphics_getDrawPosition(x_offset + x, y_offset + y);
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
                        if (x >= clip->clippedPixelsLeft && x < img->width - clip->clippedPixelsRight) {
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
    memcpy(Graphics_getDrawPosition(x + 28, y + 0), &src[0], 2 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 26, y + 1), &src[2], 6 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 24, y + 2), &src[8], 10 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 22, y + 3), &src[18], 14 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 20, y + 4), &src[32], 18 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 18, y + 5), &src[50], 22 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 16, y + 6), &src[72], 26 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 14, y + 7), &src[98], 30 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 12, y + 8), &src[128], 34 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 10, y + 9), &src[162], 38 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 8, y + 10), &src[200], 42 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 6, y + 11), &src[242], 46 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 4, y + 12), &src[288], 50 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 2, y + 13), &src[338], 54 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 0, y + 14), &src[392], 58 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 0, y + 15), &src[450], 58 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 2, y + 16), &src[508], 54 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 4, y + 17), &src[562], 50 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 6, y + 18), &src[612], 46 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 8, y + 19), &src[658], 42 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 10, y + 20), &src[700], 38 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 12, y + 21), &src[738], 34 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 14, y + 22), &src[772], 30 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 16, y + 23), &src[802], 26 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 18, y + 24), &src[828], 22 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 20, y + 25), &src[850], 18 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 22, y + 26), &src[868], 14 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 24, y + 27), &src[882], 10 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 26, y + 28), &src[892], 6 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(x + 28, y + 29), &src[898], 2 * sizeof(color_t));
}

static void draw_footprint_tile(const color_t *data, int x_offset, int y_offset, color_t color_mask)
{
    if (!color_mask) {
        color_mask = COLOR_NO_MASK;
    }
    GraphicsClipInfo *clip = Graphics_getClipInfo(x_offset, y_offset, FOOTPRINT_WIDTH, FOOTPRINT_HEIGHT);
    if (!clip->isVisible) {
        return;
    }
    // footprints are ALWAYS clipped in half, if they are clipped
    if (clip->clipY == ClipNone && clip->clipX == ClipNone && color_mask == COLOR_NO_MASK) {
        draw_footprint_simple(data, x_offset, y_offset);
        return;
    }
    int clip_left = clip->clipX == ClipLeft;
    int clip_right = clip->clipX == ClipRight;
    if (clip->clipY != ClipTop) {
        const color_t *src = data;
        for (int y = 0; y < 15; y++) {
            int xMax = 4 * y + 2;
            int xStart = 29 - 1 - 2 * y;
            if (clip_left || clip_right) {
                xMax = 2 * y;
            }
            if (clip_left) {
                xStart = 30;
                src += xMax + 2;
            }
            color_t *buffer = Graphics_getDrawPosition(x_offset + xStart, y_offset + y);
            if (color_mask == COLOR_NO_MASK) {
                memcpy(buffer, src, xMax * sizeof(color_t));
                src += xMax;
            } else {
                for (int x = 0; x < xMax; x++, buffer++, src++) {
                    *buffer = *src & color_mask;
                }
            }
            if (clip_right) {
                src += xMax + 2;
            }
        }
    }
    if (clip->clipY != ClipBottom) {
        const color_t *src = &data[900 / 2];
        for (int y = 0; y < 15; y++) {
            int xMax = 4 * (15 - 1 - y) + 2;
            int xStart = 2 * y;
            if (clip_left || clip_right) {
                xMax = xMax / 2 - 1;
            }
            if (clip_left) {
                xStart = 30;
                src += xMax + 2;
            }
            color_t *buffer = Graphics_getDrawPosition(x_offset + xStart, 15 + y_offset + y);
            if (color_mask == COLOR_NO_MASK) {
                memcpy(buffer, src, xMax * sizeof(color_t));
                src += xMax;
            } else {
                for (int x = 0; x < xMax; x++, buffer++, src++) {
                    *buffer = *src & color_mask;
                }
            }
            if (clip_right) {
                src += xMax + 2;
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
        draw_uncompressed(img, data, x, y, 0, ColorType_None);
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
        printf("ERROR: use Graphics_drawIsometricFootprint for isometric!\n");
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
                              color_mask, color_mask ? ColorType_And : ColorType_None);
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
        draw_uncompressed(img, data, x, y, color, ColorType_Blend);
    }
}

void image_draw_letter(int image_id, int x, int y, color_t color)
{
    const image *img = image_get(image_id);
    const color_t *data = image_data(image_id);
    if (!data) {
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
            color, color ? ColorType_Set : ColorType_None);
    }
}

void image_draw_fullscreen_background(int image_id)
{
    int s_width = screen_width();
    int s_height = screen_height();
    if (s_width > 1024 || s_height > 768) {
        Graphics_clearScreen();
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
