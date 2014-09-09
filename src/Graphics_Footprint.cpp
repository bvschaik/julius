#include "Graphics_Footprint.h"
#include "Graphics.h"
#include "Graphics_private.h"

#include "Data/Graphics.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#define FOOTPRINT_WIDTH 58
#define FOOTPRINT_HEIGHT 30
#define FOOTPRINT_DATA_LENGTH 1800

#define GETDATA(graphicId) Color *data = (Color*) &Data_Graphics_PixelData.main[Data_Graphics_Main.index[graphicId].offset]
#define DATA(index) &data[900 * index]

static void drawFootprintTile(Color *data, int xOffset, int yOffset, Color colorMask)
{
	if (!colorMask) {
		colorMask = Color_NoMask;
	}
	GraphicsClipInfo *clip = Graphics_getClipInfo(xOffset, yOffset, 58, 30);
	if (!clip->isVisible) {
		return;
	}
	// footprints are ALWAYS clipped in half, if they are clipped
	int clipLeft = clip->clipX == ClipLeft;
	int clipRight = clip->clipX == ClipRight;
	if (clip->clipY != ClipTop) {
		Color *src = data;
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
			ScreenColor *buffer = &ScreenPixel(xOffset + xStart, yOffset + y);
			if (colorMask == Color_NoMask) {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src];
				}
			} else {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src & colorMask];
				}
			}
			if (clipRight) {
				src += xMax + 2;
			}
		}
	}
	if (clip->clipY != ClipBottom) {
		Color *src = &data[900 / 2];
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
			ScreenColor *buffer = &ScreenPixel(xOffset + xStart, 15 + yOffset + y);
			if (colorMask == Color_NoMask) {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src];
				}
			} else {
				for (int x = 0; x < xMax; x++, buffer++, src++) {
					*buffer = ColorLookup[*src & colorMask];
				}
			}
			if (clipRight) {
				src += xMax + 2;
			}
		}
	}
}

int Graphics_Footprint_drawSize1(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

	drawFootprintTile(DATA(0), xOffset, yOffset, colorMask);

	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize2(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);
	
	int index = 0;
	drawFootprintTile(DATA(index++), xOffset, yOffset, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset - 30, yOffset + 15, colorMask);
	drawFootprintTile(DATA(index++), xOffset + 30, yOffset + 15, colorMask);
	
	drawFootprintTile(DATA(index++), xOffset, yOffset + 30, colorMask);
	
	return FOOTPRINT_WIDTH;
}

int Graphics_Footprint_drawSize3(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);
	
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

int Graphics_Footprint_drawSize4(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

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

int Graphics_Footprint_drawSize5(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	GETDATA(graphicId);

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
