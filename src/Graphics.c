#include "Graphics.h"

#include "Data/Screen.h"

#include "graphics/image.h"

#include <stdio.h> // remove later
#include <stdlib.h> // remove later
#include <string.h>

static struct ClipRectangle {
	int xStart;
	int xEnd;
	int yStart;
	int yEnd;
} clipRectangle = {0, 800, 0, 600};

static struct {
    int x;
    int y;
} translation = {0, 0};

static GraphicsClipInfo clipInfo;

static void setClipX(int xOffset, int width);
static void setClipY(int yOffset, int height);

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
	if (x >= clipRectangle.xStart && x < clipRectangle.xEnd) {
		if (y >= clipRectangle.yStart && y < clipRectangle.yEnd) {
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

static void translate_clip(int dx, int dy)
{
    clipRectangle.xStart -= dx;
    clipRectangle.xEnd -= dx;
    clipRectangle.yStart -= dy;
    clipRectangle.yEnd -= dy;
}

void Graphics_setGlobalTranslation(int x, int y)
{
    int dx = x - translation.x;
    int dy = y - translation.y;
    translation.x = x;
    translation.y = y;
    translate_clip(dx, dy);
}

void Graphics_setClipRectangle(int x, int y, int width, int height)
{
	clipRectangle.xStart = x;
	clipRectangle.xEnd = x + width;
	clipRectangle.yStart = y;
	clipRectangle.yEnd = y + height;
    // fix clip rectangle going over the edges of the screen
    if (translation.x + clipRectangle.xStart < 0) {
        clipRectangle.xStart = -translation.x;
    }
    if (translation.y + clipRectangle.yStart < 0) {
        clipRectangle.yStart = -translation.y;
    }
	if (translation.x + clipRectangle.xEnd > Data_Screen.width) {
		printf("ERROR: clip rectangle x end (%d) > screen width\n", clipRectangle.xEnd);
		clipRectangle.xEnd = Data_Screen.width - translation.x;
	}
	if (translation.y + clipRectangle.yEnd > Data_Screen.height) {
		printf("ERROR: clip rectangle y end (%d) > screen height\n", clipRectangle.yEnd);
		clipRectangle.yEnd = Data_Screen.height-15 - translation.y;
	}
}

void Graphics_resetClipRectangle()
{
	clipRectangle.xStart = 0;
	clipRectangle.xEnd = Data_Screen.width;
	clipRectangle.yStart = 0;
	clipRectangle.yEnd = Data_Screen.height;
    translate_clip(translation.x, translation.y);
}

GraphicsClipInfo *Graphics_getClipInfo(int xOffset, int yOffset, int width, int height)
{
	setClipX(xOffset, width);
	setClipY(yOffset, height);
	if (clipInfo.clipX == ClipInvisible || clipInfo.clipY == ClipInvisible) {
		clipInfo.isVisible = 0;
	} else {
		clipInfo.isVisible = 1;
	}
	return &clipInfo;
}

static void setClipX(int xOffset, int width)
{
	clipInfo.clippedPixelsLeft = 0;
	clipInfo.clippedPixelsRight = 0;
	if (width <= 0
		|| xOffset + width <= clipRectangle.xStart
		|| xOffset >= clipRectangle.xEnd) {
		clipInfo.clipX = ClipInvisible;
		clipInfo.visiblePixelsX = 0;
		return;
	}
	if (xOffset < clipRectangle.xStart) {
		// clipped on the left
		clipInfo.clippedPixelsLeft = clipRectangle.xStart - xOffset;
		if (xOffset + width <= clipRectangle.xEnd) {
			clipInfo.clipX = ClipLeft;
		} else {
			clipInfo.clipX = ClipBoth;
			clipInfo.clippedPixelsRight = xOffset + width - clipRectangle.xEnd;
		}
	} else if (xOffset + width > clipRectangle.xEnd) {
		clipInfo.clipX = ClipRight;
		clipInfo.clippedPixelsRight = xOffset + width - clipRectangle.xEnd;
	} else {
		clipInfo.clipX = ClipNone;
	}
	clipInfo.visiblePixelsX = width - clipInfo.clippedPixelsLeft - clipInfo.clippedPixelsRight;
}

static void setClipY(int yOffset, int height)
{
	clipInfo.clippedPixelsTop = 0;
	clipInfo.clippedPixelsBottom = 0;
	if (height <= 0
		|| yOffset + height <= clipRectangle.yStart
		|| yOffset >= clipRectangle.yEnd) {
		clipInfo.clipY = ClipInvisible;
	} else if (yOffset < clipRectangle.yStart) {
		// clipped on the top
		clipInfo.clippedPixelsTop = clipRectangle.yStart - yOffset;
		if (yOffset + height <= clipRectangle.yEnd) {
			clipInfo.clipY = ClipTop;
		} else {
			clipInfo.clipY = ClipBoth;
			clipInfo.clippedPixelsBottom = yOffset + height - clipRectangle.yEnd;
		}
	} else if (yOffset + height > clipRectangle.yEnd) {
		clipInfo.clipY = ClipBottom;
		clipInfo.clippedPixelsBottom = yOffset + height - clipRectangle.yEnd;
	} else {
		clipInfo.clipY = ClipNone;
	}
	clipInfo.visiblePixelsY = height - clipInfo.clippedPixelsTop - clipInfo.clippedPixelsBottom;
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

