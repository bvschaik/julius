#include "Graphics_Footprint.h"
#include "Graphics.h"

#include "Data/Graphics.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30
#define FOOTPRINT_DATA_LENGTH 1800

#define DATA(index) &data[900 * index]

#include <string.h>

static void drawFootprintTopFullNoMask(const color_t *src, int xOffset, int yOffset)
{
    memcpy(&ScreenPixel(xOffset + 28, yOffset + 0), &src[0], 2 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 26, yOffset + 1), &src[2], 6 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 24, yOffset + 2), &src[8], 10 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 22, yOffset + 3), &src[18], 14 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 20, yOffset + 4), &src[32], 18 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 18, yOffset + 5), &src[50], 22 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 16, yOffset + 6), &src[72], 26 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 14, yOffset + 7), &src[98], 30 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 12, yOffset + 8), &src[128], 34 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 10, yOffset + 9), &src[162], 38 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 8, yOffset + 10), &src[200], 42 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 6, yOffset + 11), &src[242], 46 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 4, yOffset + 12), &src[288], 50 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 2, yOffset + 13), &src[338], 54 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 0, yOffset + 14), &src[392], 58 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 0, yOffset + 15), &src[450], 58 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 2, yOffset + 16), &src[508], 54 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 4, yOffset + 17), &src[562], 50 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 6, yOffset + 18), &src[612], 46 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 8, yOffset + 19), &src[658], 42 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 10, yOffset + 20), &src[700], 38 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 12, yOffset + 21), &src[738], 34 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 14, yOffset + 22), &src[772], 30 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 16, yOffset + 23), &src[802], 26 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 18, yOffset + 24), &src[828], 22 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 20, yOffset + 25), &src[850], 18 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 22, yOffset + 26), &src[868], 14 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 24, yOffset + 27), &src[882], 10 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 26, yOffset + 28), &src[892], 6 * sizeof(color_t));
    memcpy(&ScreenPixel(xOffset + 28, yOffset + 29), &src[898], 2 * sizeof(color_t));
}

static void drawFootprintTile(const color_t *data, int xOffset, int yOffset, color_t colorMask)
{
	if (!colorMask) {
		colorMask = COLOR_NO_MASK;
	}
	GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, 58, 30);
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
			color_t *buffer = &ScreenPixel(xOffset + xStart, yOffset + y);
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
			color_t *buffer = &ScreenPixel(xOffset + xStart, 15 + yOffset + y);
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
