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
		int dataIndex = 0;
		for (int y = 0; y < 15; y++) {
			ScreenColor *buffer = &ScreenPixel(xOffset, yOffset + y);
			int xMax = 4 * y + 2;
			int xOffset = 29 - 1 - 2 * y;
			if (clipLeft || clipRight) {
				xMax = 2 * y;
			}
			if (clipLeft) {
				xOffset = 30;
				dataIndex += xMax + 2;
			}
			for (int x = 0; x < xMax; x++) {
				buffer[xOffset + x] = ColorLookup[data[dataIndex++] & colorMask];
			}
			if (clipRight) {
				dataIndex += xMax + 2;
			}
		}
	}
	if (clip->clipY != ClipBottom) {
		int dataIndex = 900 / 2;
		for (int y = 0; y < 15; y++) {
			ScreenColor *buffer = &ScreenPixel(xOffset, 15 + y + yOffset);
			int xMax = 4 * (15 - 1 - y) + 2;
			int xOffset = 2 * y;
			if (clipLeft || clipRight) {
				xMax = xMax / 2 - 1;
			}
			if (clipLeft) {
				xOffset = 30;
				dataIndex += xMax + 2;
			}
			for (int x = 0; x < xMax; x++) {
				buffer[xOffset + x] = ColorLookup[data[dataIndex++] & colorMask];
			}
			if (clipRight) {
				dataIndex += xMax + 2;
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
