#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Data/Types.h"
#include "Data/Graphics.h"

void Graphics_clearScreen();

void Graphics_drawLine(int x1, int y1, int x2, int y2, color_t color);

void Graphics_drawRect(int x, int y, int width, int height, color_t color);
void Graphics_drawInsetRect(int x, int y, int width, int height);

void Graphics_fillRect(int x, int y, int width, int height, color_t color);
void Graphics_shadeRect(int x, int y, int width, int height, int darkness);

void Graphics_setClipRectangle(int x, int y, int width, int height);
void Graphics_resetClipRectangle();

GraphicsClipInfo *Graphics_getClipInfo(int xOffset, int yOffset, int width, int height);

void Graphics_drawImage(int graphicId, int xOffset, int yOffset);
void Graphics_drawImageMasked(int graphicId, int xOffset, int yOffset, color_t colorMask);
void Graphics_drawImageBlend(int graphicId, int xOffset, int yOffset, color_t color);
void Graphics_drawFullScreenImage(int graphicId);
void Graphics_drawLetter(int graphicId, int xOffset, int yOffset, color_t color);
void Graphics_drawIsometricFootprint(int graphicId, int xOffset, int yOffset, color_t colorMask);
void Graphics_drawIsometricTop(int graphicId, int xOffset, int yOffset, color_t colorMask);

void Graphics_drawEnemyImage(int graphicId, int xOffset, int yOffset);

void Graphics_saveToBuffer(int x, int y, int width, int height, color_t *buffer);
void Graphics_loadFromBuffer(int x, int y, int width, int height, const color_t *buffer);

void Graphics_saveScreenshot(const char *filename);

#endif
