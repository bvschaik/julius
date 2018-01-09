#include "image.h"

#include "graphics/screen.h"

#include "../Graphics.h"

#include <stdio.h>
#include <string.h>

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30

#define DATA(index) &data[900 * (index)]

typedef enum {
    ColorType_Set,
    ColorType_And,
    ColorType_None,
    ColorType_Blend
} ColorType;

static void drawImageUncompressed(const image *img, const color_t *data, int xOffset, int yOffset, color_t color, ColorType type);
static void drawImageCompressed(const image *img, const color_t *data, int xOffset, int yOffset, int height);
static void drawImageCompressedSet(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color);
static void drawImageCompressedAnd(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color);
static void drawImageCompressedBlend(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color);

int Graphics_Footprint_drawSize1(int graphicId, int xOffset, int yOffset, color_t colorMask);
int Graphics_Footprint_drawSize2(int graphicId, int xOffset, int yOffset, color_t colorMask);
int Graphics_Footprint_drawSize3(int graphicId, int xOffset, int yOffset, color_t colorMask);
int Graphics_Footprint_drawSize4(int graphicId, int xOffset, int yOffset, color_t colorMask);
int Graphics_Footprint_drawSize5(int graphicId, int xOffset, int yOffset, color_t colorMask);

void Graphics_drawIsometricFootprint(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const image *img = image_get(graphicId);
    if (img->draw.type == 30) { // isometric
        switch (img->width) {
            case 58:
                Graphics_Footprint_drawSize1(graphicId, xOffset, yOffset, colorMask);
                break;
            case 118:
                Graphics_Footprint_drawSize2(graphicId, xOffset, yOffset, colorMask);
                break;
            case 178:
                Graphics_Footprint_drawSize3(graphicId, xOffset, yOffset, colorMask);
                break;
            case 238:
                Graphics_Footprint_drawSize4(graphicId, xOffset, yOffset, colorMask);
                break;
            case 298:
                Graphics_Footprint_drawSize5(graphicId, xOffset, yOffset, colorMask);
                break;
        }
    } else {
        printf("ERROR: %d trying to draw a non-isometric tile using drawIsometricFootprint\n", graphicId);
    }
}

void Graphics_drawIsometricTop(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const image *img = image_get(graphicId);
    if (img->draw.type != 30) { // isometric
        printf("ERROR: %d trying to draw a non-isometric tile using drawIsometricTop\n", graphicId);
        return;
    }
    if (!img->draw.has_compressed_part) {
        return;
    }
    const color_t *data = &image_data(graphicId)[img->draw.uncompressed_length];

    int height = img->height;
    switch (img->width) {
        case 58:
            yOffset -= img->height - 30;
            height -= 16;
            break;
        case 118:
            xOffset -= 30;
            yOffset -= img->height - 60;
            height -= 31;
            break;
        case 178:
            xOffset -= 60;
            yOffset -= img->height - 90;
            height -= 46;
            break;
        case 238:
            xOffset -= 90;
            yOffset -= img->height - 120;
            height -= 61;
            break;
        case 298:
            xOffset -= 120;
            yOffset -= img->height - 150;
            height -= 76;
            break;
    }
    if (!colorMask) {
        drawImageCompressed(img, data, xOffset, yOffset, height);
    } else {
        drawImageCompressedAnd(img, data, xOffset, yOffset, height, colorMask);
    }
}

void Graphics_drawFullScreenImage(int graphicId)
{
    int s_width = screen_width();
    int s_height = screen_height();
    if (s_width > 1024 || s_height > 768) {
        Graphics_clearScreen();
    }
    Graphics_drawImage(graphicId, (s_width - 1024) / 2, (s_height - 768) / 2);
}

void Graphics_drawImage(int graphicId, int xOffset, int yOffset)
{
    const image *img = image_get(graphicId);
    const color_t *data = image_data(graphicId);
    if (!data) {
        return;
    }

    if (img->draw.is_fully_compressed) {
        drawImageCompressed(img, data, xOffset, yOffset, img->height);
    } else {
        drawImageUncompressed(img, data, xOffset, yOffset, 0, ColorType_None);
    }
}

void Graphics_drawEnemyImage(int graphicId, int xOffset, int yOffset)
{
    if (graphicId <= 0 || graphicId >= 801) {
        return;
    }
    const image *img = image_get_enemy(graphicId);
    const color_t *data = image_data_enemy(graphicId);
    if (data) {
        drawImageCompressed(img, data, xOffset, yOffset, img->height);
    }
}

void Graphics_drawImageMasked(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const image *img = image_get(graphicId);
    const color_t *data = image_data(graphicId);
    if (!data) {
        return;
    }

    if (img->draw.type == 30) { // isometric
        printf("ERROR: use Graphics_drawIsometricFootprint for isometric!\n");
        return;
    }

    if (img->draw.is_fully_compressed) {
        if (!colorMask) {
            drawImageCompressed(img, data, xOffset, yOffset, img->height);
        } else {
            drawImageCompressedAnd(img, data, xOffset, yOffset, img->height, colorMask);
        }
    } else {
        drawImageUncompressed(img, data, xOffset, yOffset,
            colorMask, colorMask ? ColorType_And : ColorType_None);
    }
}

void Graphics_drawImageBlend(int graphicId, int xOffset, int yOffset, color_t color)
{
    const image *img = image_get(graphicId);
    const color_t *data = image_data(graphicId);
    if (!data) {
        return;
    }

    if (img->draw.type == 30) { // isometric
        printf("ERROR: use Graphics_drawIsometricFootprint for isometric!\n");
        return;
    }

    if (img->draw.is_fully_compressed) {
        drawImageCompressedBlend(img, data, xOffset, yOffset, img->height, color);
    } else {
        drawImageUncompressed(img, data, xOffset, yOffset, color, ColorType_Blend);
    }
}

void Graphics_drawLetter(int graphicId, int xOffset, int yOffset, color_t color)
{
    const image *img = image_get(graphicId);
    const color_t *data = image_data(graphicId);
    if (!data) {
        return;
    }

    if (img->draw.is_fully_compressed) {
        if (color) {
            drawImageCompressedSet(img, data, xOffset, yOffset, img->height, color);
        } else {
            drawImageCompressed(img, data, xOffset, yOffset, img->height);
        }
    } else {
        drawImageUncompressed(img, data, xOffset, yOffset,
            color, color ? ColorType_Set : ColorType_None);
    }
}

static void drawImageUncompressed(const image *img, const color_t *data, int xOffset, int yOffset, color_t color, ColorType type)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, img->width, img->height);
    if (!clip->isVisible) {
        return;
    }
    data += img->width * clip->clippedPixelsTop;
    for (int y = clip->clippedPixelsTop; y < img->height - clip->clippedPixelsBottom; y++) {
        data += clip->clippedPixelsLeft;
        color_t *dst = Graphics_getDrawPosition(xOffset + clip->clippedPixelsLeft, yOffset + y);
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

static void drawImageCompressed(const image *img, const color_t *data, int xOffset, int yOffset, int height)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, img->width, height);
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
                color_t *dst = Graphics_getDrawPosition(xOffset + x, yOffset + y);
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

static void drawImageCompressedSet(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(
        xOffset, yOffset, img->width, height);
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
                color_t *dst = Graphics_getDrawPosition(xOffset + x, yOffset + y);
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

static void drawImageCompressedAnd(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(
        xOffset, yOffset, img->width, height);
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
                color_t *dst = Graphics_getDrawPosition(xOffset + x, yOffset + y);
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

static void drawImageCompressedBlend(const image *img, const color_t *data, int xOffset, int yOffset, int height, color_t color)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(
        xOffset, yOffset, img->width, height);
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
                color_t *dst = Graphics_getDrawPosition(xOffset + x, yOffset + y);
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

static void drawFootprintTopFullNoMask(const color_t *src, int xOffset, int yOffset)
{
    memcpy(Graphics_getDrawPosition(xOffset + 28, yOffset + 0), &src[0], 2 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 26, yOffset + 1), &src[2], 6 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 24, yOffset + 2), &src[8], 10 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 22, yOffset + 3), &src[18], 14 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 20, yOffset + 4), &src[32], 18 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 18, yOffset + 5), &src[50], 22 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 16, yOffset + 6), &src[72], 26 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 14, yOffset + 7), &src[98], 30 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 12, yOffset + 8), &src[128], 34 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 10, yOffset + 9), &src[162], 38 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 8, yOffset + 10), &src[200], 42 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 6, yOffset + 11), &src[242], 46 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 4, yOffset + 12), &src[288], 50 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 2, yOffset + 13), &src[338], 54 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 0, yOffset + 14), &src[392], 58 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 0, yOffset + 15), &src[450], 58 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 2, yOffset + 16), &src[508], 54 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 4, yOffset + 17), &src[562], 50 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 6, yOffset + 18), &src[612], 46 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 8, yOffset + 19), &src[658], 42 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 10, yOffset + 20), &src[700], 38 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 12, yOffset + 21), &src[738], 34 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 14, yOffset + 22), &src[772], 30 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 16, yOffset + 23), &src[802], 26 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 18, yOffset + 24), &src[828], 22 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 20, yOffset + 25), &src[850], 18 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 22, yOffset + 26), &src[868], 14 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 24, yOffset + 27), &src[882], 10 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 26, yOffset + 28), &src[892], 6 * sizeof(color_t));
    memcpy(Graphics_getDrawPosition(xOffset + 28, yOffset + 29), &src[898], 2 * sizeof(color_t));
}

static void drawFootprintTile(const color_t *data, int xOffset, int yOffset, color_t colorMask)
{
    if (!colorMask) {
        colorMask = COLOR_NO_MASK;
    }
    GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, FOOTPRINT_WIDTH, FOOTPRINT_HEIGHT);
    if (!clip->isVisible) {
        return;
    }
    // footprints are ALWAYS clipped in half, if they are clipped
    if (clip->clipY == ClipNone && clip->clipX == ClipNone && colorMask == COLOR_NO_MASK) {
        drawFootprintTopFullNoMask(data, xOffset, yOffset);
        return;
    }
    int clipLeft = clip->clipX == ClipLeft;
    int clipRight = clip->clipX == ClipRight;
    if (clip->clipY != ClipTop) {
        const color_t *src = data;
        for (int y = 0; y < 15; y++) {
            int xMax = 4 * y + 2;
            int xStart = 29 - 1 - 2 * y;
            if (clipLeft || clipRight) {
                xMax = 2 * y;
            }
            if (clipLeft) {
                xStart = 30;
                src += xMax + 2;
            }
            color_t *buffer = Graphics_getDrawPosition(xOffset + xStart, yOffset + y);
            if (colorMask == COLOR_NO_MASK) {
                memcpy(buffer, src, xMax * sizeof(color_t));
                src += xMax;
            } else {
                for (int x = 0; x < xMax; x++, buffer++, src++) {
                    *buffer = *src & colorMask;
                }
            }
            if (clipRight) {
                src += xMax + 2;
            }
        }
    }
    if (clip->clipY != ClipBottom) {
        const color_t *src = &data[900 / 2];
        for (int y = 0; y < 15; y++) {
            int xMax = 4 * (15 - 1 - y) + 2;
            int xStart = 2 * y;
            if (clipLeft || clipRight) {
                xMax = xMax / 2 - 1;
            }
            if (clipLeft) {
                xStart = 30;
                src += xMax + 2;
            }
            color_t *buffer = Graphics_getDrawPosition(xOffset + xStart, 15 + yOffset + y);
            if (colorMask == COLOR_NO_MASK) {
                memcpy(buffer, src, xMax * sizeof(color_t));
                src += xMax;
            } else {
                for (int x = 0; x < xMax; x++, buffer++, src++) {
                    *buffer = *src & colorMask;
                }
            }
            if (clipRight) {
                src += xMax + 2;
            }
        }
    }
}

int Graphics_Footprint_drawSize1(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const color_t *data = image_data(graphicId);

    drawFootprintTile(DATA(0), xOffset, yOffset, colorMask);

    return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize2(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const color_t *data = image_data(graphicId);
    
    int index = 0;
    drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
    
    return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize3(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const color_t *data = image_data(graphicId);
    
    int index = 0;
    drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);

    return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize4(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const color_t *data = image_data(graphicId);

    int index = 0;
    drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 45, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 60, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 75, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 75, colorMask);

    drawFootprintTile(DATA(index++), xOffset, yOffset + 90, colorMask);
    
    return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize5(int graphicId, int xOffset, int yOffset, color_t colorMask)
{
    const color_t *data = image_data(graphicId);

    int index = 0;
    drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 30, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 45, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 45, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 120, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 60, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 120, yOffset + 60, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 90, yOffset + 75, colorMask);
    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 75, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 75, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 90, yOffset + 75, colorMask);
    
    drawFootprintTile(DATA(index++), xOffset - 60, yOffset + 90, colorMask);
    drawFootprintTile(DATA(index++), xOffset, yOffset + 90, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 60, yOffset + 90, colorMask);

    drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 105, colorMask);
    drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 105, colorMask);

    drawFootprintTile(DATA(index++), xOffset, yOffset + 120, colorMask);
    
    return FOOTPRINT_WIDTH;
}
