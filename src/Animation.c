#include "Animation.h"

#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Building.h"
#include "Data/Model.h"

#include "core/calc.h"
#include "Time.h"

#define MAX_ANIM_TIMERS 51

static unsigned int lastUpdate[MAX_ANIM_TIMERS];
static int shouldUpdate[MAX_ANIM_TIMERS];

void Animation_resetTimers()
{
	for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
		lastUpdate[i] = 0;
		shouldUpdate[i] = 0;
	}
}

void Animation_updateTimers()
{
	TimeMillis currentTimeMillis = Time_getMillis();
	for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
		shouldUpdate[i] = 0;
	}
	int delayMillis = 0;
	for (int i = 0; i < MAX_ANIM_TIMERS; i++) {
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
		if (b->loadsStored <= 0 || b->numWorkers <= 0) {
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
			Data_Grid_spriteOffsets[gridOffset] = 1;
			return 1;
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
		int pctDone = calc_percentage(b->data.industry.progress, 400);
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
					newSprite = 12;
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
	if (Data_Graphics_Main.index[graphicId].animationCanReverse) {
		int isReverse = 0;
		if (currentIndex & 0x80) {
			isReverse = 1;
		}
		int currentSprite = currentIndex & 0x7f;
		if (isReverse) {
			currentIndex = currentSprite - 1;
			if (currentIndex < 1) {
				currentIndex = 1;
				isReverse = 0;
			}
		} else {
			currentIndex = currentSprite + 1;
			if (currentIndex > GraphicNumAnimationSprites(graphicId)) {
				currentIndex = GraphicNumAnimationSprites(graphicId);
				isReverse = 1;
			}
		}
		if (isReverse) {
			currentIndex = currentIndex | 0x80;
		}
	} else {
		// Absolutely normal case
		currentIndex++;
		if (currentIndex > GraphicNumAnimationSprites(graphicId)) {
			currentIndex = 1;
		}
	}
	return currentIndex;
}
