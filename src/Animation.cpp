#include "Animation.h"

#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Building.h"
#include "Data/Model.h"

#include "Calc.h"
#include "Time.h"

static unsigned int lastUpdate[51];
static int shouldUpdate[51];

void Animation_resetTimers()
{
	for (int i = 0; i < 51; i++) {
		lastUpdate[i] = 0;
		shouldUpdate[i] = 0;
	}
}

void Animation_updateTimers()
{
	TimeMillis currentTimeMillis = Time_getMillis();
	for (int i = 0; i < 51; i++) {
		shouldUpdate[i] = 0;
	}
	int delayMillis = 0;
	for (int i = 0; i < 51; i++) {
		if (currentTimeMillis >= delayMillis + lastUpdate[i]) {
			shouldUpdate[i] = 1;
			lastUpdate[i] = currentTimeMillis;
		}
		delayMillis += 20;
	}
}

int Animation_getIndexForCityBuilding(int graphicId, int gridOffset)
{
	int buildingId = Data_Grid_buildingIds[gridOffset];
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type == Building_Fountain && (b->numWorkers <= 0 || !b->hasWaterAccess)) {
		return 0;
	}
	if (b->type == Building_Reservoir && !b->hasWaterAccess) {
		return 0;
	}
	if (BuildingIsWorkshop(b->type)) {
		if (b->rawMaterialsStored <= 0 || b->numWorkers <= 0) {
			return 0;
		}
	}
	if ((b->type == Building_Prefecture || b->type == Building_EngineersPost) && b->numWorkers <= 0) {
		return 0;
	}
	if (b->type == Building_Market && b->numWorkers <= 0) {
		return 0;
	}
	if (b->type == Building_Warehouse && b->numWorkers < Data_Model_Buildings[b->type].laborers) {
		return 0;
	}
	if (b->type == Building_Dock && b->data.other.dockNumShips <= 0) {
		Data_Grid_spriteOffsets[gridOffset] = 1;
		return 1;
	}
	if (b->type == Building_MarbleQuarry && b->numWorkers <= 0) {
		Data_Grid_spriteOffsets[gridOffset] = 1;
		return 1;
	} else if ((b->type == Building_IronMine || b->type == Building_ClayPit ||
		b->type == Building_TimberYard) && b->numWorkers <= 0) {
		return 0;
	}
	if (b->type == Building_GladiatorSchool) {
		if (b->numWorkers <= 0) {
			return 0;
		}
	} else if (BuildingIsEntertainment(b->type) &&
		b->type != Building_Hippodrome && b->numWorkers <= 0) {
		return 0;
	}
	if (b->type == Building_Granary && b->numWorkers < Data_Model_Buildings[b->type].laborers) {
		return 0;
	}

	if (!shouldUpdate[Data_Graphics_Main.index[graphicId].animationSpeedId]) {
		return Data_Grid_spriteOffsets[gridOffset] & 0x7f;
	}
	// advance animation
	int newSprite = 0;
	int isReverse = 0;
	if (b->type == Building_WineWorkshop) {
		// exception for wine...
		int pctDone = Calc_getPercentage(b->data.industry.progress, 400);
		if (pctDone <= 0) {
			newSprite = 0;
		} else if (pctDone < 4) {
			newSprite = 1;
		} else if (pctDone < 8) {
			newSprite = 2;
		} else if (pctDone < 12) {
			newSprite = 3;
		} else if (pctDone < 96) {
			if (Data_Grid_spriteOffsets[gridOffset] < 4) {
				newSprite = 4;
			} else {
				newSprite = Data_Grid_spriteOffsets[gridOffset] + 1;
				if (newSprite > 8) {
					newSprite = 4;
				}
			}
		} else {
			// close to done
			if (Data_Grid_spriteOffsets[gridOffset] < 9) {
				newSprite = 9;
			} else {
				newSprite = Data_Grid_spriteOffsets[gridOffset] + 1;
				if (newSprite > 12) {
					newSprite = 9;
				}
			}
		}
	} else if (Data_Graphics_Main.index[graphicId].animationCanReverse) {
		if (Data_Grid_spriteOffsets[gridOffset] & 0x80) {
			isReverse = 1;
		}
		int currentSprite = Data_Grid_spriteOffsets[gridOffset] & 0x7f;
		if (isReverse) {
			newSprite = currentSprite - 1;
			if (newSprite < 1) {
				newSprite = 1;
				isReverse = 0;
			}
		} else {
			newSprite = currentSprite + 1;
			if (newSprite > GraphicNumAnimationSprites(graphicId)) {
				newSprite = GraphicNumAnimationSprites(graphicId);
				isReverse = 1;
			}
		}
	} else {
		// Absolutely normal case
		newSprite = Data_Grid_spriteOffsets[gridOffset] + 1;
		if (newSprite > GraphicNumAnimationSprites(graphicId)) {
			newSprite = 1;
		}
	}

	Data_Grid_spriteOffsets[gridOffset] = newSprite;
	if (isReverse) {
		Data_Grid_spriteOffsets[gridOffset] |= 0x80;
	}
	return newSprite;
}

int Animation_getIndexForEmpireMap(int graphicId, int currentIndex)
{
	if (currentIndex <= 0) {
		currentIndex = 1;
	}
	int animationSpeed = Data_Graphics_Main.index[graphicId].animationSpeedId;
	if (!shouldUpdate[animationSpeed]) {
		return currentIndex;
	}
	// assumption: the 'unused26' field of graphics index isn't set
	currentIndex++;
	if (currentIndex > Data_Graphics_Main.index[graphicId].numAnimationSprites) {
		currentIndex = 1;
	}
	return currentIndex;
}
