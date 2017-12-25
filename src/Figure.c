#include "Figure.h"

#include "FigureAction.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Terrain.h"
#include "Trader.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "core/calc.h"
#include "core/random.h"
#include "empire/city.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "map/figure.h"
#include "map/grid.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/effect.h"

#include <string.h>

static const int dustCloudTileOffsets[] = {0, 0, 0, 1, 1, 2};
static const int dustCloudCCOffsets[] = {0, 7, 14, 7, 14, 7};
static const int dustCloudDirectionX[] = {
	0, -2, -4, -5, -6, -5, -4, -2, 0, -2, -4, -5, -6, -5, -4, -2
};
static const int dustCloudDirectionY[] = {
	-6, -5, -4, -2, 0, -2, -4, -5, -6, -5, -4, -2, 0, -2, -4, -5
};
static const int dustCloudSpeed[] = {
	1, 2, 1, 3, 2, 1, 3, 2, 1, 1, 2, 1, 2, 1, 3, 1
};
void Figure_createDustCloud(int x, int y, int size)
{
	int tileOffset = dustCloudTileOffsets[size];
	int ccOffset = dustCloudCCOffsets[size];
	for (int i = 0; i < 16; i++) {
		figure *f = figure_create(FIGURE_EXPLOSION,
			x + tileOffset, y + tileOffset, DIR_0_TOP);
		if (f->id) {
			f->crossCountryX += ccOffset;
			f->crossCountryY += ccOffset;
			f->destinationX += dustCloudDirectionX[i];
			f->destinationY += dustCloudDirectionY[i];
			FigureMovement_crossCountrySetDirection(f,
				f->crossCountryX, f->crossCountryY,
				15 * f->destinationX + ccOffset,
				15 * f->destinationY + ccOffset, 0);
			f->speedMultiplier = dustCloudSpeed[i];
		}
	}
	sound_effect_play(SOUND_EFFECT_EXPLOSION);
}

void Figure_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type)
{
	figure *f = figure_create(type, x, y, DIR_0_TOP);
	if (f->id) {
		f->missileDamage = (type == FIGURE_BOLT) ? 60 : 10;
		f->buildingId = buildingId;
		f->destinationX = xDst;
		f->destinationY = yDst;
		FigureMovement_crossCountrySetDirection(
			f, f->crossCountryX, f->crossCountryY,
			15 * xDst, 15 * yDst, 1);
	}
}

static int is_citizen(figure *f)
{
    if (f->actionState != FigureActionState_149_Corpse) {
        if (f->type && f->type != FIGURE_EXPLOSION && f->type != FIGURE_FORT_STANDARD &&
            f->type != FIGURE_MAP_FLAG && f->type != FIGURE_FLOTSAM && f->type < FIGURE_INDIGENOUS_NATIVE) {
            return f->id;
        }
    }
    return 0;
}

int Figure_getCitizenOnSameTile(int figureId)
{
    return map_figure_foreach_until(figure_get(figureId)->gridOffset, is_citizen);
}

static int is_non_citizen(figure *f)
{
    if (f->actionState == FigureActionState_149_Corpse) {
        return 0;
    }
    if (FigureIsEnemy(f->type)) {
        return f->id;
    }
    if (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FIGURE_ACTION_159_NATIVE_ATTACKING) {
        return f->id;
    }
    if (f->type == FIGURE_WOLF || f->type == FIGURE_SHEEP || f->type == FIGURE_ZEBRA) {
        return f->id;
    }
    return 0;
}

int Figure_getNonCitizenOnSameTile(int figureId)
{
    return map_figure_foreach_until(figure_get(figureId)->gridOffset, is_non_citizen);
}

int Figure_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd)
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->state != FigureState_Alive || !FigureIsEnemy(f->type)) {
			continue;
		}
		int dx = (f->x > xStart) ? (f->x - xStart) : (xStart - f->x);
		int dy = (f->y > yStart) ? (f->y - yStart) : (yStart - f->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
		dx = (f->x > xEnd) ? (f->x - xEnd) : (xEnd - f->x);
		dy = (f->y > yEnd) ? (f->y - yEnd) : (yEnd - f->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
	}
	return 0;
}
