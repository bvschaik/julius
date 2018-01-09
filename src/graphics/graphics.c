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

static clip_info clip;

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
    if (translation.x + clip_rectangle.x_end > Data_Screen.width) {
        clip_rectangle.x_end = Data_Screen.width - translation.x;
    }
    if (translation.y + clip_rectangle.y_end > Data_Screen.height) {
        clip_rectangle.y_end = Data_Screen.height-15 - translation.y;
    }
}

void graphics_reset_clip_rectangle()
{
    clip_rectangle.x_start = 0;
    clip_rectangle.x_end = Data_Screen.width;
    clip_rectangle.y_start = 0;
    clip_rectangle.y_end = Data_Screen.height;
    translate_clip(translation.x, translation.y);
}

static void setClipX(int xOffset, int width)
{
    clip.clippedPixelsLeft = 0;
    clip.clippedPixelsRight = 0;
    if (width <= 0
        || xOffset + width <= clip_rectangle.x_start
        || xOffset >= clip_rectangle.x_end) {
        clip.clipX = CLIP_INVISIBLE;
        clip.visiblePixelsX = 0;
        return;
    }
    if (xOffset < clip_rectangle.x_start) {
        // clipped on the left
        clip.clippedPixelsLeft = clip_rectangle.x_start - xOffset;
        if (xOffset + width <= clip_rectangle.x_end) {
            clip.clipX = CLIP_LEFT;
        } else {
            clip.clipX = CLIP_BOTH;
            clip.clippedPixelsRight = xOffset + width - clip_rectangle.x_end;
        }
    } else if (xOffset + width > clip_rectangle.x_end) {
        clip.clipX = CLIP_RIGHT;
        clip.clippedPixelsRight = xOffset + width - clip_rectangle.x_end;
    } else {
        clip.clipX = CLIP_NONE;
    }
    clip.visiblePixelsX = width - clip.clippedPixelsLeft - clip.clippedPixelsRight;
}

static void setClipY(int yOffset, int height)
{
    clip.clippedPixelsTop = 0;
    clip.clippedPixelsBottom = 0;
    if (height <= 0
        || yOffset + height <= clip_rectangle.y_start
        || yOffset >= clip_rectangle.y_end) {
        clip.clipY = CLIP_INVISIBLE;
    } else if (yOffset < clip_rectangle.y_start) {
        // clipped on the top
        clip.clippedPixelsTop = clip_rectangle.y_start - yOffset;
        if (yOffset + height <= clip_rectangle.y_end) {
            clip.clipY = CLIP_TOP;
        } else {
            clip.clipY = CLIP_BOTH;
            clip.clippedPixelsBottom = yOffset + height - clip_rectangle.y_end;
        }
    } else if (yOffset + height > clip_rectangle.y_end) {
        clip.clipY = CLIP_BOTTOM;
        clip.clippedPixelsBottom = yOffset + height - clip_rectangle.y_end;
    } else {
        clip.clipY = CLIP_NONE;
    }
    clip.visiblePixelsY = height - clip.clippedPixelsTop - clip.clippedPixelsBottom;
}

const clip_info *graphics_get_clip_info(int x, int y, int width, int height)
{
    setClipX(x, width);
    setClipY(y, height);
    if (clip.clipX == CLIP_INVISIBLE || clip.clipY == CLIP_INVISIBLE) {
        clip.isVisible = 0;
    } else {
        clip.isVisible = 1;
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
    return &(((color_t*)Data_Screen.drawBuffer)[(translation.y + y) * Data_Screen.width + (translation.x + x)]);
}

void graphics_clear_screen()
{
    memset(Data_Screen.drawBuffer, 0, sizeof(color_t) * Data_Screen.width * Data_Screen.height);
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
        int yMin = y1 < y2 ? y1 : y2;
        int yMax = y1 < y2 ? y2 : y1;
        for (int y = yMin; y <= yMax; y++) {
            graphics_draw_pixel(x1, y, color);
        }
    } else if (y1 == y2) {
        int xMin = x1 < x2 ? x1 : x2;
        int xMax = x1 < x2 ? x2 : x1;
        for (int x = xMin; x <= xMax; x++) {
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
    if (!clip->isVisible) {
        return;
    }
    for (int yy = y + clip->clippedPixelsTop; yy < y + height - clip->clippedPixelsBottom; yy++) {
        for (int xx = x + clip->clippedPixelsLeft; xx < x + width - clip->clippedPixelsRight; xx++) {
            color_t *pixel = graphics_get_pixel(xx, yy);
            int r = (*pixel & 0xff0000) >> 16;
            int g = (*pixel & 0xff00) >> 8;
            int b = (*pixel & 0xff);
            int grey = (r + g + b) / 3 >> darkness;
            color_t newPixel = (color_t) (grey << 16 | grey << 8 | grey);
            *pixel = newPixel;
        }
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

void graphics_save_screenshot(const char *filename)
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
