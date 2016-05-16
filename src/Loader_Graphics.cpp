#include "Data/Graphics.h"

#include <stdlib.h>
#include <string.h>

#include "FileSystem.h"
#include "Graphics.h"
#include "Data/Screen.h"
#include "Data/Constants.h"

#define MAIN_SIZE 12100000
#define ENEMY_SIZE 4900000

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
static const char enemyGraphicsSg2[][32] = {
	"goths.sg2",
	"Etruscan.sg2",
	"Etruscan.sg2",
	"carthage.sg2",
	"Greek.sg2",
	"Greek.sg2",
	"egyptians.sg2",
	"Persians.sg2",
	"Phoenician.sg2",
	"celts.sg2",
	"celts.sg2",
	"celts.sg2",
	"Gaul.sg2",
	"Gaul.sg2",
	"goths.sg2",
	"goths.sg2",
	"goths.sg2",
	"Phoenician.sg2",
	"North African.sg2",
	"Phoenician.sg2",
};
static const char enemyGraphics555[][32] = {
	"goths.555",
	"Etruscan.555",
	"Etruscan.555",
	"carthage.555",
	"Greek.555",
	"Greek.555",
	"egyptians.555",
	"Persians.555",
	"Phoenician.555",
	"celts.555",
	"celts.555",
	"celts.555",
	"Gaul.555",
	"Gaul.555",
	"goths.555",
	"goths.555",
	"goths.555",
	"Phoenician.555",
	"North African.555",
	"Phoenician.555",
};

static int currentClimate = -1;

static void prepareMainGraphics();
static void prepareEnemyGraphics();

static char *imagesScratchSpace;

int Loader_Graphics_initGraphics()
{
	Data_Graphics_PixelData.enemy = (char*) malloc(ENEMY_SIZE);
	Data_Graphics_PixelData.main = (char*) malloc(MAIN_SIZE);
	imagesScratchSpace = (char*) malloc(2000 * 1000 * 2);
	if (!Data_Graphics_PixelData.main || !Data_Graphics_PixelData.enemy || !imagesScratchSpace) {
		return 0;
	}
	Graphics_initialize();
	return 1;
}

#include <cstdio>
int Loader_Graphics_loadMainGraphics(int climate)
{
	if (climate == currentClimate) {
		return 1;
	}

	const char *filename555 = mainGraphics555[climate];
	const char *filenameSg2 = mainGraphicsSg2[climate];
	if (!FileSystem_fileExists(filename555) ||
		!FileSystem_fileExists(filenameSg2)) {
		return 0;
	}
	
	if (!FileSystem_readFileIntoBuffer(filenameSg2, &Data_Graphics_Main, sizeof(Data_Graphics_Main))) {
		return 0;
	}
	if (!FileSystem_readFileIntoBuffer(filename555, Data_Graphics_PixelData.main, MAIN_SIZE)) {
		return 0;
	}

	printf("Preparing main graphics..\n");
	prepareMainGraphics();
	printf("Prepared main graphics\n");
	currentClimate = climate;
	return 1;
}

int Loader_Graphics_loadEnemyGraphics(int enemyId)
{
	const char *filename555 = enemyGraphics555[enemyId];
	const char *filenameSg2 = enemyGraphicsSg2[enemyId];
	if (!FileSystem_fileExists(filename555) ||
		!FileSystem_fileExists(filenameSg2)) {
		return 0;
	}

	FileSystem_readFilePartIntoBuffer(filenameSg2, &Data_Graphics_Enemy, 51264, 20680);
	FileSystem_readFileIntoBuffer(filename555, Data_Graphics_PixelData.enemy, ENEMY_SIZE);

	prepareEnemyGraphics();
	return 1;
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
	}
}

static void prepareEnemyGraphics()
{
	int offset = 4;
	for (int i = 1; i <= 800; i++) {
		Data_Graphics_Index *index = &Data_Graphics_Enemy.index[i];
		if (index->isExternal) {
			if (!index->offset) {
				index->offset = 1;
			}
			continue;
		}
		// internal graphic
		index->offset = offset;
		offset += index->dataLength;
	}
}
