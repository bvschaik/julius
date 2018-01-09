#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

#include "graphics/color.h"

enum ClipCode {
    ClipNone,
    ClipLeft,
    ClipRight,
    ClipTop,
    ClipBottom,
    ClipBoth,
    ClipInvisible
};

typedef struct {
    enum ClipCode clipX;
    enum ClipCode clipY;
    int clippedPixelsLeft;
    int clippedPixelsRight;
    int clippedPixelsTop;
    int clippedPixelsBottom;
    int visiblePixelsX;
    int visiblePixelsY;
    int isVisible;
} GraphicsClipInfo;

void graphics_in_dialog();
void graphics_reset_dialog();

void Graphics_setClipRectangle(int x, int y, int width, int height);
void Graphics_resetClipRectangle();
GraphicsClipInfo *Graphics_getClipInfo(int xOffset, int yOffset, int width, int height);

color_t *Graphics_getDrawPosition(int x, int y);

void Graphics_clearScreen();

void Graphics_drawPixel(int x, int y, color_t color);

void Graphics_drawLine(int x1, int y1, int x2, int y2, color_t color);

void Graphics_drawRect(int x, int y, int width, int height, color_t color);
void Graphics_drawInsetRect(int x, int y, int width, int height);

void Graphics_fillRect(int x, int y, int width, int height, color_t color);
void Graphics_shadeRect(int x, int y, int width, int height, int darkness);

void Graphics_setGlobalTranslation(int x, int y);


void Graphics_saveToBuffer(int x, int y, int width, int height, color_t *buffer);
void Graphics_loadFromBuffer(int x, int y, int width, int height, const color_t *buffer);

void Graphics_saveScreenshot(const char *filename);

#endif // GRAPHICS_GRAPHICS_H
