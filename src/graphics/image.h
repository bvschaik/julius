#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include "core/image.h"
#include "graphics/color.h"

void Graphics_drawImage(int graphicId, int xOffset, int yOffset);
void Graphics_drawImageMasked(int graphicId, int xOffset, int yOffset, color_t colorMask);
void Graphics_drawImageBlend(int graphicId, int xOffset, int yOffset, color_t color);
void Graphics_drawFullScreenImage(int graphicId);
void Graphics_drawLetter(int graphicId, int xOffset, int yOffset, color_t color);
void Graphics_drawIsometricFootprint(int graphicId, int xOffset, int yOffset, color_t colorMask);
void Graphics_drawIsometricTop(int graphicId, int xOffset, int yOffset, color_t colorMask);

void Graphics_drawEnemyImage(int graphicId, int xOffset, int yOffset);

#endif // GRAPHICS_IMAGE_H
