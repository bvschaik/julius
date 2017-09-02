#include "Graphics.h"
#include "Graphics_Footprint.h"
#include "Loader.h"

#include "Data/Graphics.h"
#include "Data/Screen.h"
#include "Data/Types.h"
#include "Data/Constants.h"

#include <stdio.h> // remove later
#include <stdlib.h> // remove later
#include <string.h>

typedef enum {
	ColorType_Set,
	ColorType_And,
	ColorType_None,
	ColorType_Blend
} ColorType;

static struct ClipRectangle {
	int xStart;
	int xEnd;
	int yStart;
	int yEnd;
} clipRectangle = {0, 800, 0, 600};

static GraphicsClipInfo clipInfo;

static void drawImageUncompressed(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, Color color, ColorType type);
static void drawImageCompressed(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height);
static void drawImageCompressedSet(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color);
static void drawImageCompressedAnd(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color);
static void drawImageCompressedBlend(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color);
static void setClipX(int xOffset, int width);
static void setClipY(int yOffset, int height);


void Graphics_clearScreen()
{
	memset(Data_Screen.drawBuffer, 0, sizeof(ScreenColor) * Data_Screen.width * Data_Screen.height);
}

static void drawDot(int x, int y, Color color)
{
	if (x >= clipRectangle.xStart && x < clipRectangle.xEnd) {
		if (y >= clipRectangle.yStart && y < clipRectangle.yEnd) {
			ScreenPixel(x, y) = color;
		}
	}
}

void Graphics_drawLine(int x1, int y1, int x2, int y2, Color color)
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

void Graphics_drawRect(int x, int y, int width, int height, Color color)
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
	Graphics_drawLine(x, y, x + width - 1, y, Color_InsetDark);
	Graphics_drawLine(x + width - 1, y, x + width - 1, y + height - 1, Color_InsetLight);
	Graphics_drawLine(x, y + height - 1, x + width - 1, y + height - 1, Color_InsetLight);
	Graphics_drawLine(x, y, x, y + height - 1, Color_InsetDark);
}

void Graphics_fillRect(int x, int y, int width, int height, Color color)
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
	for (int yy = y; yy < y + height; yy++) {
		for (int xx = x; xx < x + width; xx++) {
			ScreenColor pixel = ScreenPixel(xx, yy);
			int r = (pixel & 0xff0000) >> 16;
			int g = (pixel & 0xff00) >> 8;
			int b = (pixel & 0xff);
			int grey = (r + g + b) / 3 >> darkness;
			Color newPixel = (Color) (grey << 16 | grey << 8 | grey);
			ScreenPixel(xx, yy) = newPixel;
		}
	}
}

void Graphics_drawIsometricFootprint(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	if (index->type == 30) { // isometric
		switch (index->width) {
			case 58:
				Graphics_Footprint_drawSize1(graphicId, xOffset, yOffset, colorMask);
				break;
			case 118:
				Graphics_Footprint_drawSize2(graphicId, xOffset, yOffset, colorMask);
				break;
			case 178:
				Graphics_Footprint_drawSize3(graphicId, xOffset, yOffset, colorMask);
				break;
			case 238:
				Graphics_Footprint_drawSize4(graphicId, xOffset, yOffset, colorMask);
				break;
			case 298:
				Graphics_Footprint_drawSize5(graphicId, xOffset, yOffset, colorMask);
				break;
		}
	} else {
		printf("ERROR: %d trying to draw a non-isometric tile using drawIsometricFootprint\n", graphicId);
	}
}

void Graphics_drawIsometricTop(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	if (index->type != 30) { // isometric
		printf("ERROR: %d trying to draw a non-isometric tile using drawIsometricTop\n", graphicId);
		return;
	}
	if (!index->hasCompressedPart) {
		return;
	}
	const Color *data = &Data_Graphics_PixelData.main[index->offset] + index->uncompressedLength;

	int height = index->height;
	switch (index->width) {
		case 58:
			yOffset -= index->height - 30;
			height -= 16;
			break;
		case 118:
			xOffset -= 30;
			yOffset -= index->height - 60;
			height -= 31;
			break;
		case 178:
			xOffset -= 60;
			yOffset -= index->height - 90;
			height -= 46;
			break;
		case 238:
			xOffset -= 90;
			yOffset -= index->height - 120;
			height -= 61;
			break;
		case 298:
			xOffset -= 120;
			yOffset -= index->height - 150;
			height -= 76;
			break;
	}
	if (!colorMask) {
		drawImageCompressed(index, data, xOffset, yOffset, height);
	} else {
		drawImageCompressedAnd(index, data, xOffset, yOffset, height, colorMask);
	}
}

void Graphics_drawFullScreenImage(int graphicId)
{
	if (Data_Screen.width > 1024 || Data_Screen.height > 768) {
		Graphics_clearScreen();
	}
	Graphics_drawImage(graphicId, (Data_Screen.width - 1024) / 2, (Data_Screen.height - 768) / 2);
}

void Graphics_drawImage(int graphicId, int xOffset, int yOffset)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	const Color *data;
	if (index->isExternal) {
		data = Loader_Graphics_loadExternalImagePixelData(graphicId);
		if (!data) {
			printf("ERR: unable to load external image %d\n", graphicId);
			return;
		}
	} else {
		data = &Data_Graphics_PixelData.main[index->offset];
	}

	if (index->isFullyCompressed) {
		drawImageCompressed(index, data, xOffset, yOffset, index->height);
	} else {
		drawImageUncompressed(index, data, xOffset, yOffset, 0, ColorType_None);
	}
}

void Graphics_drawImageMasked(int graphicId, int xOffset, int yOffset, Color colorMask)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	const Color *data;
	if (index->isExternal) {
		data = Loader_Graphics_loadExternalImagePixelData(graphicId);
		if (!data) {
			printf("ERR: unable to load external image %d\n", graphicId);
			return;
		}
	} else {
		data = &Data_Graphics_PixelData.main[index->offset];
	}

	if (index->type == 30) { // isometric
		printf("ERROR: use Graphics_drawIsometricFootprint for isometric!\n");
		return;
	}

	if (index->isFullyCompressed) {
		if (!colorMask) {
			drawImageCompressed(index, data, xOffset, yOffset, index->height);
		} else {
			drawImageCompressedAnd(index, data, xOffset, yOffset, index->height, colorMask);
		}
	} else {
		drawImageUncompressed(index, (Color*)data, xOffset, yOffset,
			colorMask, colorMask ? ColorType_And : ColorType_None);
	}
}

void Graphics_drawImageBlend(int graphicId, int xOffset, int yOffset, Color color)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	const Color *data;
	if (index->isExternal) {
		data = Loader_Graphics_loadExternalImagePixelData(graphicId);
		if (!data) {
			printf("ERR: unable to load external image %d\n", graphicId);
			return;
		}
	} else {
		data = &Data_Graphics_PixelData.main[index->offset];
	}

	if (index->type == 30) { // isometric
		printf("ERROR: use Graphics_drawIsometricFootprint for isometric!\n");
		return;
	}

	if (index->isFullyCompressed) {
		drawImageCompressedBlend(index, data, xOffset, yOffset, index->height, color);
	} else {
		drawImageUncompressed(index, data, xOffset, yOffset, color, ColorType_Blend);
	}
}

void Graphics_drawLetter(int graphicId, int xOffset, int yOffset, Color color)
{
	struct Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	const Color *data;
	if (index->isExternal) {
		data = Loader_Graphics_loadExternalImagePixelData(graphicId);
		if (!data) {
			printf("ERR: unable to load external image %d\n", graphicId);
			return;
		}
	} else {
		data = &Data_Graphics_PixelData.main[index->offset];
	}

	if (index->isFullyCompressed) {
		if (color) {
			drawImageCompressedSet(index, data, xOffset, yOffset, index->height, color);
		} else {
			drawImageCompressed(index, data, xOffset, yOffset, index->height);
		}
	} else {
		drawImageUncompressed(index, data, xOffset, yOffset,
			color, color ? ColorType_Set : ColorType_None);
	}
}

static void drawImageUncompressed(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, Color color, ColorType type)
{
	GraphicsClipInfo *clip = Graphics_getClipInfo(
		xOffset, yOffset, index->width, index->height);
	if (!clip->isVisible) {
		return;
	}
	data += index->width * clip->clippedPixelsTop;
	for (int y = clip->clippedPixelsTop; y < index->height - clip->clippedPixelsBottom; y++) {
		data += clip->clippedPixelsLeft;
		ScreenColor *dst = &ScreenPixel(xOffset + clip->clippedPixelsLeft, yOffset + y);
		int xMax = index->width - clip->clippedPixelsRight;
        if (type == ColorType_None) {
            if (index->type == 0) { // can be transparent
                for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
                    if (*data != Color_Transparent) {
                        *dst = *data;
                    }
                    data++;
                }
            } else {
                int num_pixels = xMax - clip->clippedPixelsLeft;
                memcpy(dst, data, num_pixels * sizeof(Color));
                data += num_pixels;
            }
		} else if (type == ColorType_Set) {
			for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
				if (*data != Color_Transparent) {
					*dst = color;
				}
				data++;
			}
		} else if (type == ColorType_And) {
			for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
				if (*data != Color_Transparent) {
					*dst = *data & color;
				}
				data++;
			}
		} else if (type == ColorType_Blend) {
			for (int x = clip->clippedPixelsLeft; x < xMax; x++, dst++) {
				if (*data != Color_Transparent) {
					*dst &= color;
				}
				data++;
			}
		}
		data += clip->clippedPixelsRight;
	}
}

static void drawImageCompressed(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height)
{
	GraphicsClipInfo *clip = Graphics_getClipInfo(
		xOffset, yOffset, index->width, height);
	if (!clip->isVisible) {
		return;
	}
	int unclipped = clip->clipX == ClipNone;

	for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
		int x = 0;
		while (x < index->width) {
			Color b = *data;
			data++;
			if (b == 255) {
				// transparent pixels to skip
				x += *data;
				data++;
			} else if (y < clip->clippedPixelsTop) {
				data += b;
				x += b;
			} else {
				// number of concrete pixels
				const Color *pixels = data;
				data += b;
				ScreenColor *dst = &ScreenPixel(xOffset + x, yOffset + y);
				if (unclipped) {
					x += b;
					memcpy(dst, pixels, b * sizeof(Color));
				} else {
					while (b) {
						if (x >= clip->clippedPixelsLeft && x < index->width - clip->clippedPixelsRight) {
							*dst = *pixels;
						}
						dst++;
						x++;
						pixels++;
						b--;
					}
				}
			}
		}
	}
}

static void drawImageCompressedSet(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color)
{
	GraphicsClipInfo *clip = Graphics_getClipInfo(
		xOffset, yOffset, index->width, height);
	if (!clip->isVisible) {
		return;
	}
	int unclipped = clip->clipX == ClipNone;

	ScreenColor screenColor = color;
	for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
		int x = 0;
		while (x < index->width) {
			Color b = *data;
			data++;
			if (b == 255) {
				// transparent pixels to skip
				x += *data;
				data++;
			} else if (y < clip->clippedPixelsTop) {
				data += b;
				x += b;
			} else {
				// number of concrete pixels
				const Color *pixels = data;
				data += b;
				ScreenColor *dst = &ScreenPixel(xOffset + x, yOffset + y);
				if (unclipped) {
					x += b;
					while (b) {
						*dst = screenColor;
						dst++;
						pixels++;
						b--;
					}
				} else {
					while (b) {
						if (x >= clip->clippedPixelsLeft && x < index->width - clip->clippedPixelsRight) {
							*dst = screenColor;
						}
						dst++;
						x++;
						pixels++;
						b--;
					}
				}
			}
		}
	}
}

static void drawImageCompressedAnd(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color)
{
	GraphicsClipInfo *clip = Graphics_getClipInfo(
		xOffset, yOffset, index->width, height);
	if (!clip->isVisible) {
		return;
	}
	int unclipped = clip->clipX == ClipNone;

	for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
		int x = 0;
		while (x < index->width) {
			Color b = *data;
			data++;
			if (b == 255) {
				// transparent pixels to skip
				x += *data;
				data++;
			} else if (y < clip->clippedPixelsTop) {
				data += b;
				x += b;
			} else {
				// number of concrete pixels
				const Color *pixels = data;
				data += b;
				ScreenColor *dst = &ScreenPixel(xOffset + x, yOffset + y);
				if (unclipped) {
					x += b;
					while (b) {
						*dst = *pixels & color;
						dst++;
						pixels++;
						b--;
					}
				} else {
					while (b) {
						if (x >= clip->clippedPixelsLeft && x < index->width - clip->clippedPixelsRight) {
							*dst = *pixels & color;
						}
						dst++;
						x++;
						pixels++;
						b--;
					}
				}
			}
		}
	}
}

static void drawImageCompressedBlend(struct Data_Graphics_Index *index, const Color *data, int xOffset, int yOffset, int height, Color color)
{
	GraphicsClipInfo *clip = Graphics_getClipInfo(
		xOffset, yOffset, index->width, height);
	if (!clip->isVisible) {
		return;
	}
	int unclipped = clip->clipX == ClipNone;

	ScreenColor screenColor = color;
	for (int y = 0; y < height - clip->clippedPixelsBottom; y++) {
		int x = 0;
		while (x < index->width) {
			Color b = *data;
			data++;
			if (b == 255) {
				// transparent pixels to skip
				x += *data;
				data++;
			} else if (y < clip->clippedPixelsTop) {
				data += b;
				x += b;
			} else {
				// number of concrete pixels
				const Color *pixels = data;
				data += b;
				ScreenColor *dst = &ScreenPixel(xOffset + x, yOffset + y);
				if (unclipped) {
					x += b;
					while (b) {
						*dst &= screenColor;
						dst++;
						pixels++;
						b--;
					}
				} else {
					while (b) {
						if (x >= clip->clippedPixelsLeft && x < index->width - clip->clippedPixelsRight) {
							*dst &= screenColor;
						}
						dst++;
						x++;
						pixels++;
						b--;
					}
				}
			}
		}
	}
}

void Graphics_setClipRectangle(int x, int y, int width, int height)
{
	clipRectangle.xStart = x;
	clipRectangle.xEnd = x + width;
	clipRectangle.yStart = y;
	clipRectangle.yEnd = y + height;
	if (clipRectangle.xEnd > Data_Screen.width) {
		printf("ERROR: clip rectangle x end (%d) > screen width\n", clipRectangle.xEnd);
		clipRectangle.xEnd = Data_Screen.width;
	}
	if (clipRectangle.yEnd > Data_Screen.height) {
		printf("ERROR: clip rectangle y end (%d) > screen height\n", clipRectangle.yEnd);
		clipRectangle.yEnd = Data_Screen.height-15;
	}
}

void Graphics_resetClipRectangle()
{
	clipRectangle.xStart = 0;
	clipRectangle.xEnd = Data_Screen.width;
	clipRectangle.yStart = 0;
	clipRectangle.yEnd = Data_Screen.height;
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

void Graphics_drawEnemyImage(int graphicId, int xOffset, int yOffset)
{
	if (graphicId <= 0 || graphicId >= 801) {
		return;
	}
	struct Data_Graphics_Index *index = &Data_Graphics_Enemy.index[graphicId];
	if (index->offset > 0) {
		drawImageCompressed(index, &Data_Graphics_PixelData.enemy[index->offset],
			xOffset, yOffset, index->height);
	}
}

void Graphics_saveToBuffer(int x, int y, int width, int height, ScreenColor *buffer)
{
	for (int dy = 0; dy < height; dy++) {
		memcpy(&buffer[dy * height], &ScreenPixel(x, y + dy), sizeof(ScreenColor) * width);
	}
}

void Graphics_loadFromBuffer(int x, int y, int width, int height, const ScreenColor *buffer)
{
	for (int dy = 0; dy < height; dy++) {
		memcpy(&ScreenPixel(x, y + dy), &buffer[dy * height], sizeof(ScreenColor) * width);
	}
}

/////debug/////

static void pixel(ScreenColor input, unsigned char *r, unsigned char *g, unsigned char *b)
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
			pixel(((ScreenColor*)Data_Screen.drawBuffer)[y*Data_Screen.width+x],
				&pixels[3*x+2], &pixels[3*x+1], &pixels[3*x]);
		}
		fwrite(pixels, 1, Data_Screen.width * 3, fp);
	}
	fclose(fp);
	free(pixels);
}

