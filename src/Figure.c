#include "Figure.h"

#include "FigureAction.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Terrain.h"
#include "Trader.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"

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

static void create_fishing_point(int x, int y)
{
    random_generate_next();
    figure *fish = figure_create(FIGURE_FISH_GULLS, x, y, DIR_0_TOP);
    fish->graphicOffset = random_byte() & 0x1f;
    fish->progressOnTile = random_byte() & 7;
    FigureMovement_crossCountrySetDirection(fish,
        fish->crossCountryX, fish->crossCountryY,
        15 * fish->destinationX, 15 * fish->destinationY, 0);
}

void Figure_createFishingPoints()
{
    scenario_map_foreach_fishing_point(create_fishing_point);
}

static void create_herd(int x, int y)
{
    figure_type herd_type;
    int numAnimals;
    switch (scenario_property_climate()) {
        case CLIMATE_CENTRAL:
            herd_type = FIGURE_SHEEP;
            numAnimals = 10;
            break;
        case CLIMATE_NORTHERN:
            herd_type = FIGURE_WOLF;
            numAnimals = 8;
            break;
        case CLIMATE_DESERT:
            herd_type = FIGURE_ZEBRA;
            numAnimals = 12;
            break;
        default:
            return;
    }
    int formationId = formation_create_herd(herd_type, x, y, numAnimals);
    if (formationId > 0) {
        for (int fig = 0; fig < numAnimals; fig++) {
            random_generate_next();
            figure *f = figure_create(herd_type, x, y, DIR_0_TOP);
            f->actionState = FigureActionState_196_HerdAnimalAtRest;
            f->formationId = formationId;
            f->waitTicks = f->id & 0x1f;
        }
    }
}

void Figure_createHerds()
{
    scenario_map_foreach_herd_point(create_herd);
}

void Figure_createFlotsam()
{
    if (!scenario_map_has_river_entry() || !scenario_map_has_river_exit() || !scenario_map_has_flotsam()) {
		return;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (f->state && f->type == FIGURE_FLOTSAM) {
			figure_delete(f);
		}
	}
	const int resourceIds[] = {3, 1, 3, 2, 1, 3, 2, 3, 2, 1, 3, 3, 2, 3, 3, 3, 1, 2, 0, 1};
	const int waitTicks[] = {10, 50, 100, 130, 200, 250, 400, 430, 500, 600, 70, 750, 820, 830, 900, 980, 1010, 1030, 1200, 1300};
	map_point river_entry = scenario_map_river_entry();
	for (int i = 0; i < 20; i++) {
		figure *f = figure_create(FIGURE_FLOTSAM, river_entry.x, river_entry.y, DIR_0_TOP);
		f->actionState = FigureActionState_128_FlotsamCreated;
		f->resourceId = resourceIds[i];
		f->waitTicks = waitTicks[i];
	}
}

struct state {
    const int building_id;
    const int has_weapons;
    int recruit_type;
    int formation_id;
    int min_distance;
};

static void get_closest_fort_needing_soldiers(const formation *formation, void *data)
{
    struct state *state = (struct state*) data;
    if (formation->in_distant_battle || !formation->legion_recruit_type) {
        return;
    }
    if (formation->legion_recruit_type == LEGION_RECRUIT_LEGIONARY && !state->has_weapons) {
        return;
    }
    int dist = calc_maximum_distance(
        Data_Buildings[state->building_id].x, Data_Buildings[state->building_id].y,
        Data_Buildings[formation->building_id].x, Data_Buildings[formation->building_id].y);
    if (formation->legion_recruit_type > state->recruit_type ||
        (formation->legion_recruit_type == state->recruit_type && dist < state->min_distance)) {
        state->recruit_type = formation->legion_recruit_type;
        state->formation_id = formation->id;
        state->min_distance = dist;
    }
}

int Figure_createSoldierFromBarracks(int buildingId, int x, int y)
{
    struct state state = {buildingId, Data_Buildings[buildingId].loadsStored > 0, 0, 0, 10000};
    formation_foreach_legion(get_closest_fort_needing_soldiers, &state);
	if (state.formation_id > 0) {
		const formation *m = formation_get(state.formation_id);
		figure *f = figure_create(m->figure_type, x, y, DIR_0_TOP);
		f->formationId = state.formation_id;
		f->formationAtRest = 1;
		if (m->figure_type == FIGURE_FORT_LEGIONARY) {
			if (Data_Buildings[buildingId].loadsStored > 0) {
				Data_Buildings[buildingId].loadsStored--;
			}
		}
		int academyId = Formation_getClosestMilitaryAcademy(state.formation_id);
		if (academyId) {
			int xRoad, yRoad;
			if (Terrain_hasRoadAccess(Data_Buildings[academyId].x,
				Data_Buildings[academyId].y, Data_Buildings[academyId].size, &xRoad, &yRoad)) {
				f->actionState = FigureActionState_85_SoldierGoingToMilitaryAcademy;
				f->destinationX = xRoad;
				f->destinationY = yRoad;
				f->destinationGridOffsetSoldier = map_grid_offset(f->destinationX, f->destinationY);
			} else {
				f->actionState = FigureActionState_81_SoldierGoingToFort;
			}
		} else {
			f->actionState = FigureActionState_81_SoldierGoingToFort;
		}
	}
	Formation_calculateFigures();
	return state.formation_id ? 1 : 0;
}

int Figure_createTowerSentryFromBarracks(int buildingId, int x, int y)
{
	if (Data_Buildings_Extra.barracksTowerSentryRequested <= 0) {
		return 0;
	}
	int towerId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->type == BUILDING_TOWER && b->numWorkers > 0 &&
			!b->figureId && b->roadNetworkId == Data_Buildings[buildingId].roadNetworkId) {
			towerId = i;
			break;
		}
	}
	if (!towerId) {
		return 0;
	}
	struct Data_Building *tower = &Data_Buildings[towerId];
	figure *f = figure_create(FIGURE_TOWER_SENTRY, x, y, DIR_0_TOP);
	f->actionState = FigureActionState_174_TowerSentryGoingToTower;
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(tower->x, tower->y, tower->size, &xRoad, &yRoad)) {
		f->destinationX = xRoad;
		f->destinationY = yRoad;
	} else {
		f->state = FigureState_Dead;
	}
	tower->figureId = f->id;
	f->buildingId = towerId;
	return 1;
}

void Figure_killTowerSentriesAt(int x, int y)
{
	for (int i = 0; i < MAX_FIGURES; i++) {
        figure *f = figure_get(i);
		if (!figure_is_dead(f) && f->type == FIGURE_TOWER_SENTRY) {
			if (calc_maximum_distance(f->x, f->y, x, y) <= 1) {
				f->state = FigureState_Dead;
			}
		}
	}
}

void Figure_sinkAllShips()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		figure *f = figure_get(i);
		if (f->state != FigureState_Alive) {
			continue;
		}
		int buildingId;
		if (f->type == FIGURE_TRADE_SHIP) {
			buildingId = f->destinationBuildingId;
		} else if (f->type == FIGURE_FISHING_BOAT) {
			buildingId = f->buildingId;
		} else {
			continue;
		}
		Data_Buildings[buildingId].data.other.boatFigureId = 0;
		f->buildingId = 0;
		f->type = FIGURE_SHIPWRECK;
		f->waitTicks = 0;
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
    if (f->type == FIGURE_INDIGENOUS_NATIVE && f->actionState == FigureActionState_159_NativeAttacking) {
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
