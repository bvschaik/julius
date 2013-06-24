#include "Data/Graphics.h"

#include <stdlib.h>
#include <string.h>

#include "FileSystem.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#include <stdio.h> // TODO remove

static const char mainGraphicsSg2[][32] = {
	"C3.sg2",
	"C3_North.sg2",
	"C3_south.sg2"
};
static const char mainGraphics555[][32] = {
	"C3.555",
	"C3_North.555",
	"c3_South.555"
};
static const char enemyGraphics555[][32] = {
	"TODO",
};
static const char enemyGraphicsSg2[][32] = {
	"TODO",
};

static int currentClimate = -1;

static void prepareMainGraphics();
static void prepareEnemyGraphics();
static void convertGraphicToSurfaceFormat(void *data, int length);
static void convertCompressedGraphicToSurfaceFormat(void *data, int length);

static char *imagesScratchSpace;

int Loader_Graphics_initGraphics()
{
	char *ptr = (char *) malloc(17000000);
	Data_Graphics_PixelData.main = ptr;
	Data_Graphics_PixelData.enemy = &ptr[12100000];
	imagesScratchSpace = (char*) malloc(2000 * 1000 * 2);
	return 1;
}

int Loader_Graphics_loadMainGraphics(int climate)
{
	if (climate == currentClimate) {
		return 0;
	}

	const char *filename555 = mainGraphics555[climate];
	const char *filenameSg2 = mainGraphicsSg2[climate];
	if (!FileSystem_fileExists(filename555) ||
		!FileSystem_fileExists(filenameSg2)) {
		return 2;
	}
	
	FileSystem_readFileIntoBuffer(filenameSg2, &Data_Graphics_Main);
	FileSystem_readFileIntoBuffer(filename555, Data_Graphics_PixelData.main);

	prepareMainGraphics();
	currentClimate = climate;
	return 0;
}

int Loader_Graphics_loadEnemyGraphics(int enemyId)
{
	const char *filename555 = enemyGraphics555[enemyId];
	const char *filenameSg2 = enemyGraphicsSg2[enemyId];
	if (!FileSystem_fileExists(filename555) ||
		!FileSystem_fileExists(filenameSg2)) {
		return 2;
	}

	FileSystem_readFilePartIntoBuffer(filenameSg2, &Data_Graphics_Enemy, 51264, 20680);
	FileSystem_readFileIntoBuffer(filename555, Data_Graphics_PixelData.enemy);

	prepareEnemyGraphics();
	return 0;
}

const char *Loader_Graphics_loadExternalImagePixelData(int graphicId)
{
	Data_Graphics_Index *index = &Data_Graphics_Main.index[graphicId];
	char filename[200] = "555/";
	strcpy(&filename[4], Data_Graphics_Main.bitmaps[(int)index->bitmapId]);
	FileSystem_changeExtension(filename, "555");
	if (!FileSystem_readFilePartIntoBuffer(
			&filename[4], imagesScratchSpace,
			index->dataLength, index->offset - 1)) {
		// try in 555 dir
		if (!FileSystem_readFilePartIntoBuffer(
				filename, imagesScratchSpace,
				index->dataLength, index->offset - 1)) {
			return 0;
		}
	}
	if (index->isFullyCompressed) {
		convertCompressedGraphicToSurfaceFormat(imagesScratchSpace, index->dataLength);
	} else {
		convertGraphicToSurfaceFormat(imagesScratchSpace, index->dataLength);
	}
	return imagesScratchSpace;
}

static void prepareMainGraphics()
{
	int offset = 4;
	for (int i = 1; i < 10000; i++) {
		Data_Graphics_Index *index = &Data_Graphics_Main.index[i];
		if (index->isExternal) {
			if (!index->offset) {
				index->offset = 1;
			}
			continue;
		}
		// internal graphic
		index->offset = offset;
		offset += index->dataLength;
		if (index->dataLength <= 0) {
			continue;
		}
		if (index->type == 30) { // isometric
			if (index->hasCompressedPart) {
				convertGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.main[index->offset],
					index->uncompressedLength);
				convertCompressedGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.main[index->offset + index->uncompressedLength],
					index->dataLength - index->uncompressedLength);
			} else {
				convertGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.main[index->offset],
					index->dataLength);
			}
		} else {
			if (index->isFullyCompressed) {
				convertCompressedGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.main[index->offset],
					index->dataLength);
			} else {
				convertGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.main[index->offset],
					index->dataLength);
			}
		}
	}
}

static void prepareEnemyGraphics()
{
	int offset = 4;
	for (int i = 1; i <= 800; i++) {
		Data_Graphics_Index *index = &Data_Graphics_Main.index[i];
		if (index->isExternal) {
			if (!index->offset) {
				index->offset = 1;
			}
			continue;
		}
		// internal graphic
		index->offset = offset;
		offset += index->dataLength;
		if (index->dataLength > 0) {
			if (index->isFullyCompressed) {
				convertCompressedGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.enemy[index->offset],
					index->dataLength);
			} else {
				convertGraphicToSurfaceFormat(
					&Data_Graphics_PixelData.enemy[index->offset],
					index->dataLength);
			}
		}
	}
}

static void convertGraphicToSurfaceFormat(void *data, int lengthInBytes)
{
	if (Data_Screen.format == 565) {
		Color *color = (Color*) data;
		for (int i = 0; i < lengthInBytes; i += 2) {
			if (*color != Color_Transparent) {
				*color = (*color & 0x1f) | ((*color & 0xffe0) << 1);
			}
			++color;
		}
	}
}

static void convertCompressedGraphicToSurfaceFormat(void *data, int lengthInBytes)
{
	if (Data_Screen.format == 565) {
		char *ptr = (char *) data;
		int bytesToGo = lengthInBytes;
		while (bytesToGo > 0) {
			signed char length = *ptr;
			if (length == -1) {
				ptr += 2;
				bytesToGo -= 2;
			} else {
				if (length < 0) {
					printf("Length negative: %d with %d bytes to go\n", length, bytesToGo);
					return;
				}
				ptr++;
				bytesToGo--;
				Color *color = (Color*) ptr;
				for (int i = 0; i < length; i++) {
					*color = (*color & 0x1f) | ((*color & 0xffe0) << 1);
					++color;
				}
				ptr += length * 2;
				bytesToGo -= length * 2;
			}
		}
	}
}
