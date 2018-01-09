#ifndef GRAPHICS_IMAGE_H
#define GRAPHICS_IMAGE_H

#include "core/image.h"
#include "graphics/color.h"

void Graphics_drawImage(int image_id, int x, int y);
void Graphics_drawImageMasked(int image_id, int x, int y, color_t color_mask);
void Graphics_drawImageBlend(int image_id, int x, int y, color_t color);
void Graphics_drawFullScreenImage(int image_id);
void Graphics_drawLetter(int image_id, int x, int y, color_t color);
void Graphics_drawIsometricFootprint(int image_id, int x, int y, color_t color_mask);
void Graphics_drawIsometricTop(int image_id, int x, int y, color_t color_mask);

void Graphics_drawEnemyImage(int image_id, int x, int y);

#endif // GRAPHICS_IMAGE_H
