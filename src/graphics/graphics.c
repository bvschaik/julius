#include "graphics.h"

#include "graphics/image.h"
#include "graphics/screen.h"

#include "Data/Screen.h"

#include <stdio.h> // remove later
#include <stdlib.h> // remove later
#include <string.h>

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

static GraphicsClipInfo clip_info;

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

void graphics_in_dialog()
{
    set_translation(screen_dialog_offset_x(), screen_dialog_offset_y());
}

void graphics_reset_dialog()
{
    set_translation(0, 0);
}

void Graphics_setClipRectangle(int x, int y, int width, int height)
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
    if (translation.x + clip_rectangle.x_end > Data_Screen.width) {
        clip_rectangle.x_end = Data_Screen.width - translation.x;
    }
    if (translation.y + clip_rectangle.y_end > Data_Screen.height) {
        clip_rectangle.y_end = Data_Screen.height-15 - translation.y;
    }
}

void Graphics_resetClipRectangle()
{
    clip_rectangle.x_start = 0;
    clip_rectangle.x_end = Data_Screen.width;
    clip_rectangle.y_start = 0;
    clip_rectangle.y_end = Data_Screen.height;
    translate_clip(translation.x, translation.y);
}

static void setClipX(int xOffset, int width)
{
    clip_info.clippedPixelsLeft = 0;
    clip_info.clippedPixelsRight = 0;
    if (width <= 0
        || xOffset + width <= clip_rectangle.x_start
        || xOffset >= clip_rectangle.x_end) {
        clip_info.clipX = ClipInvisible;
        clip_info.visiblePixelsX = 0;
        return;
    }
    if (xOffset < clip_rectangle.x_start) {
        // clipped on the left
        clip_info.clippedPixelsLeft = clip_rectangle.x_start - xOffset;
        if (xOffset + width <= clip_rectangle.x_end) {
            clip_info.clipX = ClipLeft;
        } else {
            clip_info.clipX = ClipBoth;
            clip_info.clippedPixelsRight = xOffset + width - clip_rectangle.x_end;
        }
    } else if (xOffset + width > clip_rectangle.x_end) {
        clip_info.clipX = ClipRight;
        clip_info.clippedPixelsRight = xOffset + width - clip_rectangle.x_end;
    } else {
        clip_info.clipX = ClipNone;
    }
    clip_info.visiblePixelsX = width - clip_info.clippedPixelsLeft - clip_info.clippedPixelsRight;
}

static void setClipY(int yOffset, int height)
{
    clip_info.clippedPixelsTop = 0;
    clip_info.clippedPixelsBottom = 0;
    if (height <= 0
        || yOffset + height <= clip_rectangle.y_start
        || yOffset >= clip_rectangle.y_end) {
        clip_info.clipY = ClipInvisible;
    } else if (yOffset < clip_rectangle.y_start) {
        // clipped on the top
        clip_info.clippedPixelsTop = clip_rectangle.y_start - yOffset;
        if (yOffset + height <= clip_rectangle.y_end) {
            clip_info.clipY = ClipTop;
        } else {
            clip_info.clipY = ClipBoth;
            clip_info.clippedPixelsBottom = yOffset + height - clip_rectangle.y_end;
        }
    } else if (yOffset + height > clip_rectangle.y_end) {
        clip_info.clipY = ClipBottom;
        clip_info.clippedPixelsBottom = yOffset + height - clip_rectangle.y_end;
    } else {
        clip_info.clipY = ClipNone;
    }
    clip_info.visiblePixelsY = height - clip_info.clippedPixelsTop - clip_info.clippedPixelsBottom;
}

const GraphicsClipInfo *Graphics_getClipInfo(int x, int y, int width, int height)
{
    setClipX(x, width);
    setClipY(y, height);
    if (clip_info.clipX == ClipInvisible || clip_info.clipY == ClipInvisible) {
        clip_info.isVisible = 0;
    } else {
        clip_info.isVisible = 1;
    }
    return &clip_info;
}

color_t *Graphics_getDrawPosition(int x, int y)
{
    return &ScreenPixel(translation.x + x, translation.y + y);
}

void Graphics_clearScreen()
{
    memset(Data_Screen.drawBuffer, 0, sizeof(color_t) * Data_Screen.width * Data_Screen.height);
}

static void drawDot(int x, int y, color_t color)
{
    if (x >= clip_rectangle.x_start && x < clip_rectangle.x_end) {
        if (y >= clip_rectangle.y_start && y < clip_rectangle.y_end) {
            *Graphics_getDrawPosition(x, y) = color;
        }
    }
}

void Graphics_drawPixel(int x, int y, color_t color)
{
    drawDot(x, y, color);
}

void Graphics_drawLine(int x1, int y1, int x2, int y2, color_t color)
{
    if (x1 == x2) {
        int yMin = y1 < y2 ? y1 : y2;
        int yMax = y1 < y2 ? y2 : y1;
        for (int y = yMin; y <= yMax; y++) {
            drawDot(x1, y, color);
        }
    } else if (y1 == y2) {
        int xMin = x1 < x2 ? x1 : x2;
        int xMax = x1 < x2 ? x2 : x1;
        for (int x = xMin; x <= xMax; x++) {
            drawDot(x, y1, color);
        }
    } else {
        // non-straight line: ignore
    }
}

void Graphics_drawRect(int x, int y, int width, int height, color_t color)
{
    if (x < 0) {
        x = 0;
    }
    if (width + x >= Data_Screen.width) {
        width = Data_Screen.width - x;
    }
    if (y < 0) {
        y = 0;
    }
    if (height + y >= Data_Screen.height) {
        height = Data_Screen.height - y;
    }
    Graphics_drawLine(x, y, x + width - 1, y, color);
    Graphics_drawLine(x, y + height - 1, x + width - 1, y + height - 1, color);
    Graphics_drawLine(x, y, x, y + height - 1, color);
    Graphics_drawLine(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void Graphics_drawInsetRect(int x, int y, int width, int height)
{
    if (x < 0) {
        x = 0;
    }
    if (width + x >= Data_Screen.width) {
        width = Data_Screen.width - x;
    }
    if (y < 0) {
        y = 0;
    }
    if (height + y >= Data_Screen.height) {
        height = Data_Screen.height - y;
    }
    Graphics_drawLine(x, y, x + width - 1, y, COLOR_INSET_DARK);
    Graphics_drawLine(x + width - 1, y, x + width - 1, y + height - 1, COLOR_INSET_LIGHT);
    Graphics_drawLine(x, y + height - 1, x + width - 1, y + height - 1, COLOR_INSET_LIGHT);
    Graphics_drawLine(x, y, x, y + height - 1, COLOR_INSET_DARK);
}

void Graphics_fillRect(int x, int y, int width, int height, color_t color)
{
    if (x < 0) {
        x = 0;
    }
    if (width + x >= Data_Screen.width) {
        width = Data_Screen.width - x;
    }
    if (y < 0) {
        y = 0;
    }
    if (height + y >= Data_Screen.height) {
        height = Data_Screen.height - y;
    }
    for (int yy = y; yy < height + y; yy++) {
        Graphics_drawLine(x, yy, x + width - 1, yy, color);
    }
}

void Graphics_shadeRect(int x, int y, int width, int height, int darkness)
{
    GraphicsClipInfo *clip = Graphics_getClipInfo(x, y, width, height);
    if (!clip->isVisible) {
        return;
    }
    for (int yy = y + clip->clippedPixelsTop; yy < y + height - clip->clippedPixelsBottom; yy++) {
        for (int xx = x + clip->clippedPixelsLeft; xx < x + width - clip->clippedPixelsRight; xx++) {
            color_t *pixel = Graphics_getDrawPosition(xx, yy);
            int r = (*pixel & 0xff0000) >> 16;
            int g = (*pixel & 0xff00) >> 8;
            int b = (*pixel & 0xff);
            int grey = (r + g + b) / 3 >> darkness;
            color_t newPixel = (color_t) (grey << 16 | grey << 8 | grey);
            *pixel = newPixel;
        }
    }
}

void Graphics_saveToBuffer(int x, int y, int width, int height, color_t *buffer)
{
    for (int dy = 0; dy < height; dy++) {
        memcpy(&buffer[dy * height], Graphics_getDrawPosition(x, y + dy), sizeof(color_t) * width);
    }
}

void Graphics_loadFromBuffer(int x, int y, int width, int height, const color_t *buffer)
{
    for (int dy = 0; dy < height; dy++) {
        memcpy(Graphics_getDrawPosition(x, y + dy), &buffer[dy * height], sizeof(color_t) * width);
    }
}

/////debug/////

static void pixel(color_t input, unsigned char *r, unsigned char *g, unsigned char *b)
{
    int rr = (input & 0xff0000) >> 16;
    int gg = (input & 0x00ff00) >> 8;
    int bb = (input & 0x0000ff) >> 0;
    *r = (unsigned char) rr;
    *g = (unsigned char) gg;
    *b = (unsigned char) bb;
}

void Graphics_saveScreenshot(const char *filename)
{
    struct {
        char __filler1;
        char __filler2;
        char B;
        char M;
        int filesize;
        int reserved;
        int dataOffset;

        int dibSize;
        short width;
        short height;
        short planes;
        short bpp;
    } header = {
        0, 0, 'B', 'M', 26 + Data_Screen.width * Data_Screen.height * 3, 0, 26,
        12, (short) Data_Screen.width, (short) Data_Screen.height, 1, 24
    };
    unsigned char *pixels = (unsigned char*) malloc(Data_Screen.width * 3);
    FILE *fp = fopen(filename, "wb");
    fwrite(&header.B, 1, 26, fp);
    for (int y = Data_Screen.height - 1; y >= 0; y--) {
        for (int x = 0; x < Data_Screen.width; x++) {
            pixel(((color_t*)Data_Screen.drawBuffer)[y*Data_Screen.width+x],
                &pixels[3*x+2], &pixels[3*x+1], &pixels[3*x]);
        }
        fwrite(pixels, 1, Data_Screen.width * 3, fp);
    }
    fclose(fp);
    free(pixels);
}
