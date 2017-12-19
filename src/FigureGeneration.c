#include "Figure.h"

#include "Building.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/industry.h"
#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "figure/type.h"
#include "graphics/image.h"
#include "game/resource.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/random.h"
#include "map/terrain.h"

static int worker_percentage(const building *b)
{
    return calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
}

static void check_labor_problem(building *b)
{
    if (b->housesCovered <= 0) {
        b->showOnProblemOverlay = 2;
    }
}

static void generate_labor_seeker(building *b, int x, int y)
{
    if (Data_CityInfo.population <= 0) {
        return;
    }
    if (b->figureId2) {
        figure *f = figure_get(b->figureId2);
        if (!f->state || f->type != FIGURE_LABOR_SEEKER || f->buildingId != b->id) {
            b->figureId2 = 0;
        }
    } else {
        figure *f = figure_create(FIGURE_LABOR_SEEKER, x, y, DIR_0_TOP);
        f->actionState = FigureActionState_125_Roaming;
        f->buildingId = b->id;
        b->figureId2 = f->id;
        FigureMovement_initRoaming(f);
    }
}

static void spawn_labor_seeker(building *b, int x, int y, int min_houses)
{
    if (b->housesCovered <= min_houses) {
        generate_labor_seeker(b, x, y);
    }
}

static int has_figure_of_types(building *b, figure_type type1, figure_type type2)
{
	int figureId = b->figureId;
	if (figureId <= 0) {
		return 0;
	}
	figure *f = figure_get(figureId);
	if (f->state && f->buildingId == b->id && (f->type == type1 || f->type == type2)) {
		return 1;
	} else {
		b->figureId = 0;
		return 0;
	}
}

static int has_figure_of_type(building *b, figure_type type)
{
    return has_figure_of_types(b, type, 0);
}

static void spawnFigurePatrician(building *b, int *patricianSpawned)
{
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > 40 && !*patricianSpawned) {
			*patricianSpawned = 1;
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_PATRICIAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureWarehouse(building *b)
{
	check_labor_problem(b);
	building *space = b;
	for (int i = 0; i < 8; i++) {
		space = building_next(space);
		if (space->id) {
			space->showOnProblemOverlay = b->showOnProblemOverlay;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad) ||
		Terrain_hasRoadAccess(b->x, b->y, 3, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
		if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
		int resource;
		int task = Resource_determineWarehouseWorkerTask(b, &resource);
		if (task >= 0) {
			figure *f = figure_create(FIGURE_WAREHOUSEMAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			if (task == StorageFigureTask_Getting) {
				f->collectingItemId = resource;
			}
			b->figureId = f->id;
			f->buildingId = b->id;
		}
	}
}

static void spawnFigureGranary(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessGranary(b->x, b->y, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
		if (has_figure_of_type(b, FIGURE_WAREHOUSEMAN)) {
            return;
        }
		int task = Resource_determineGranaryWorkerTask(b);
		if (task >= 0) {
			figure *f = figure_create(FIGURE_WAREHOUSEMAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			b->figureId = f->id;
			f->buildingId = b->id;
		}
	}
}

static void spawnFigureTower(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		if (b->numWorkers <= 0) {
			return;
		}
		if (!b->figureId4 && b->figureId) { // has sentry but no ballista -> create
			figure *f = figure_create(FIGURE_BALLISTA, b->x, b->y, DIR_0_TOP);
			b->figureId4 = f->id;
			f->buildingId = b->id;
			f->actionState = FigureActionState_180_BallistaCreated;
		}
		has_figure_of_type(b, FIGURE_TOWER_SENTRY);
		if (b->figureId <= 0) {
			Data_Buildings_Extra.barracksTowerSentryRequested = 2;
		}
	}
}

static void spawnFigureEngineersPost(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_ENGINEER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 0;
		} else if (pctWorkers >= 75) {
			spawnDelay = 1;
		} else if (pctWorkers >= 50) {
			spawnDelay = 3;
		} else if (pctWorkers >= 25) {
			spawnDelay = 7;
		} else if (pctWorkers >= 1) {
			spawnDelay = 15;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_ENGINEER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_60_EngineerCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigurePrefecture(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_PREFECT)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 0;
		} else if (pctWorkers >= 75) {
			spawnDelay = 1;
		} else if (pctWorkers >= 50) {
			spawnDelay = 3;
		} else if (pctWorkers >= 25) {
			spawnDelay = 7;
		} else if (pctWorkers >= 1) {
			spawnDelay = 15;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_PREFECT, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_70_PrefectCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureActorColony(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_ACTOR, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureGladiatorSchool(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_GLADIATOR, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureLionHouse(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 5;
		} else if (pctWorkers >= 75) {
			spawnDelay = 10;
		} else if (pctWorkers >= 50) {
			spawnDelay = 20;
		} else if (pctWorkers >= 25) {
			spawnDelay = 35;
		} else if (pctWorkers >= 1) {
			spawnDelay = 60;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_LION_TAMER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureChariotMaker(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 7;
		} else if (pctWorkers >= 75) {
			spawnDelay = 15;
		} else if (pctWorkers >= 50) {
			spawnDelay = 30;
		} else if (pctWorkers >= 25) {
			spawnDelay = 60;
		} else if (pctWorkers >= 1) {
			spawnDelay = 90;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_CHARIOTEER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureAmphitheater(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_types(b, FIGURE_ACTOR, FIGURE_GLADIATOR)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generate_labor_seeker(b, xRoad, yRoad);
		}
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f;
			if (b->data.entertainment.days1 > 0) {
				f = figure_create(FIGURE_GLADIATOR, xRoad, yRoad, DIR_0_TOP);
			} else {
				f = figure_create(FIGURE_ACTOR, xRoad, yRoad, DIR_0_TOP);
			}
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureTheater(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_ACTOR)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generate_labor_seeker(b, xRoad, yRoad);
		}
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_ACTOR, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureHippodrome(building *b)
{
	check_labor_problem(b);
	if (b->prevPartBuildingId) {
		return;
	}
	building *part = b;
	for (int i = 0; i < 2; i++) {
		part = building_next(part);
		if (part->id) {
			part->showOnProblemOverlay = b->showOnProblemOverlay;
		}
	}
	if (has_figure_of_type(b, FIGURE_CHARIOTEER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessHippodrome(b->x, b->y, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generate_labor_seeker(b, xRoad, yRoad);
		}
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 7;
		} else if (pctWorkers >= 75) {
			spawnDelay = 15;
		} else if (pctWorkers >= 50) {
			spawnDelay = 30;
		} else if (pctWorkers >= 25) {
			spawnDelay = 50;
		} else if (pctWorkers >= 1) {
			spawnDelay = 80;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_CHARIOTEER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);

			if (!Data_CityInfo.entertainmentHippodromeHasShow) {
				// create mini-horses
				figure *horse1 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 1, DIR_2_RIGHT);
				horse1->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
				horse1->buildingId = b->id;
				horse1->resourceId = 0;
				horse1->speedMultiplier = 3;

				figure *horse2 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 2, DIR_2_RIGHT);
				horse2->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
				horse2->buildingId = b->id;
				horse2->resourceId = 1;
				horse2->speedMultiplier = 2;

				if (b->data.entertainment.days1 > 0) {
					if (!Data_CityInfo.messageShownHippodrome) {
						Data_CityInfo.messageShownHippodrome = 1;
						city_message_post(1, MESSAGE_WORKING_HIPPODROME, 0, 0);
					}
				}
			}
		}
	}
}

static void spawnFigureColosseum(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_types(b, FIGURE_GLADIATOR, FIGURE_LION_TAMER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generate_labor_seeker(b, xRoad, yRoad);
		}
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 6;
		} else if (pctWorkers >= 75) {
			spawnDelay = 12;
		} else if (pctWorkers >= 50) {
			spawnDelay = 20;
		} else if (pctWorkers >= 25) {
			spawnDelay = 40;
		} else if (pctWorkers >= 1) {
			spawnDelay = 70;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f;
			if (b->data.entertainment.days1 > 0) {
				f = figure_create(FIGURE_LION_TAMER, xRoad, yRoad, DIR_0_TOP);
			} else {
				f = figure_create(FIGURE_GLADIATOR, xRoad, yRoad, DIR_0_TOP);
			}
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
			if (b->data.entertainment.days1 > 0 || b->data.entertainment.days2 > 0) {
				if (!Data_CityInfo.messageShownColosseum) {
					Data_CityInfo.messageShownColosseum = 1;
					city_message_post(1, MESSAGE_WORKING_COLOSSEUM, 0, 0);
				}
			}
		}
	}
}

static void setMarketGraphic(building *b)
{
	if (!BuildingIsInUse(b)) {
		return;
	}
	if (map_desirability_get(b->gridOffset) <= 30) {
		Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_MARKET), TERRAIN_BUILDING);
	} else {
		Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_MARKET_FANCY), TERRAIN_BUILDING);
	}
}

static void spawnFigureMarket(building *b)
{
	setMarketGraphic(b);
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 2;
		} else if (pctWorkers >= 75) {
			spawnDelay = 5;
		} else if (pctWorkers >= 50) {
			spawnDelay = 10;
		} else if (pctWorkers >= 25) {
			spawnDelay = 20;
		} else if (pctWorkers >= 1) {
			spawnDelay = 30;
		} else {
			return;
		}
		// market trader
		if (!has_figure_of_type(b, FIGURE_MARKET_TRADER)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay <= spawnDelay) {
				return;
			}
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_MARKET_TRADER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
		// market buyer or labor seeker
		if (b->figureId2) {
			figure *f = figure_get(b->figureId2);
			if (f->state != FigureState_Alive || (f->type != FIGURE_MARKET_BUYER && f->type != FIGURE_LABOR_SEEKER)) {
				b->figureId2 = 0;
			}
		} else {
			Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad);
			int dstBuildingId = Building_Market_getDestinationGranaryWarehouse(b);
			if (dstBuildingId > 0) {
				figure *f = figure_create(FIGURE_MARKET_BUYER, xRoad, yRoad, DIR_0_TOP);
				f->actionState = FigureActionState_145_MarketBuyerGoingToStorage;
				f->buildingId = b->id;
				b->figureId2 = f->id;
				f->destinationBuildingId = dstBuildingId;
				f->collectingItemId = b->data.market.fetchInventoryId;
				building *bDst = building_get(dstBuildingId);
				if (Terrain_hasRoadAccess(bDst->x, bDst->y, bDst->size, &xRoad, &yRoad) ||
					Terrain_hasRoadAccess(bDst->x, bDst->y, 3, &xRoad, &yRoad)) {
					f->destinationX = xRoad;
					f->destinationY = yRoad;
				} else {
					f->actionState = FigureActionState_146_MarketBuyerReturning;
					f->destinationX = f->x;
					f->destinationY = f->y;
				}
			}
		}
	}
}

static void setBathhouseGraphic(building *b)
{
	if (!BuildingIsInUse(b)) {
		return;
	}
	if (Terrain_existsTileWithinAreaWithType(b->x, b->y, b->size, TERRAIN_RESERVOIR_RANGE)) {
		b->hasWaterAccess = 1;
	} else {
		b->hasWaterAccess = 0;
	}
	if (b->hasWaterAccess && b->numWorkers) {
		if (map_desirability_get(b->gridOffset) <= 30) {
			Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_WATER), TERRAIN_BUILDING);
		} else {
			Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_FANCY_WATER), TERRAIN_BUILDING);
		}
	} else {
		if (map_desirability_get(b->gridOffset) <= 30) {
			Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER), TERRAIN_BUILDING);
		} else {
			Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
                image_group(GROUP_BUILDING_BATHHOUSE_FANCY_NO_WATER), TERRAIN_BUILDING);
		}
	}
}

static void spawnFigureBathhouse(building *b)
{
	setBathhouseGraphic(b);
	check_labor_problem(b);
	if (!b->hasWaterAccess) {
		b->showOnProblemOverlay = 2;
	}
	if (has_figure_of_type(b, FIGURE_BATHHOUSE_WORKER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad) && b->hasWaterAccess) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_BATHHOUSE_WORKER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureSchool(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_SCHOOL_CHILD)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;

			figure *child1 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child1->actionState = FigureActionState_125_Roaming;
			child1->buildingId = b->id;
			b->figureId = child1->id;
			FigureMovement_initRoaming(child1);

			figure *child2 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child2->actionState = FigureActionState_125_Roaming;
			child2->buildingId = b->id;
			FigureMovement_initRoaming(child2);

			figure *child3 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child3->actionState = FigureActionState_125_Roaming;
			child3->buildingId = b->id;
			FigureMovement_initRoaming(child3);

			figure *child4 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child4->actionState = FigureActionState_125_Roaming;
			child4->buildingId = b->id;
			FigureMovement_initRoaming(child4);
		}
	}
}

static void spawnFigureLibrary(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_LIBRARIAN)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_LIBRARIAN, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureAcademy(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_TEACHER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_TEACHER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureBarber(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_BARBER)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_BARBER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureDoctor(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_DOCTOR)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_DOCTOR, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureHospital(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_SURGEON)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 15;
		} else if (pctWorkers >= 25) {
			spawnDelay = 29;
		} else if (pctWorkers >= 1) {
			spawnDelay = 44;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_SURGEON, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureTemple(building *b)
{
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_PRIEST)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (model_get_building(b->type)->laborers <= 0) {
			spawnDelay = 7;
		} else if (pctWorkers >= 100) {
			spawnDelay = 3;
		} else if (pctWorkers >= 75) {
			spawnDelay = 7;
		} else if (pctWorkers >= 50) {
			spawnDelay = 10;
		} else if (pctWorkers >= 25) {
			spawnDelay = 15;
		} else if (pctWorkers >= 1) {
			spawnDelay = 20;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_PRIEST, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = b->id;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void setSenateGraphic(building *b)
{
	if (!BuildingIsInUse(b)) {
		return;
	}
	if (map_desirability_get(b->gridOffset) <= 30) {
		Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_SENATE), TERRAIN_BUILDING);
	} else {
		Terrain_addBuildingToGrids(b->id, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_SENATE_FANCY), TERRAIN_BUILDING);
	}
}

static void spawnFigureSenateForum(building *b)
{
	if (b->type == BUILDING_SENATE_UPGRADED) {
		setSenateGraphic(b);
	}
	check_labor_problem(b);
	if (has_figure_of_type(b, FIGURE_TAX_COLLECTOR)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 0;
		} else if (pctWorkers >= 75) {
			spawnDelay = 1;
		} else if (pctWorkers >= 50) {
			spawnDelay = 3;
		} else if (pctWorkers >= 25) {
			spawnDelay = 7;
		} else if (pctWorkers >= 1) {
			spawnDelay = 15;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_TAX_COLLECTOR, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_40_TaxCollectorCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureMissionPost(building *b)
{
	if (has_figure_of_type(b, FIGURE_MISSIONARY)) {
        return;
    }
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (Data_CityInfo.population > 0) {
			Data_CityInfo.nativeMissionPostOperational = 1;
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 1) {
				b->figureSpawnDelay = 0;
				figure *f = figure_create(FIGURE_MISSIONARY, xRoad, yRoad, DIR_0_TOP);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = b->id;
				b->figureId = f->id;
				FigureMovement_initRoaming(f);
			}
		}
	}
}

static void spawnFigureIndustry(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
		if (building_industry_has_produced_resource(b)) {
			building_industry_start_new_production(b);
			figure *f = figure_create(FIGURE_CART_PUSHER, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = b->outputResourceId;
			f->buildingId = b->id;
			b->figureId = f->id;
			f->waitTicks = 30;
		}
	}
}

static void spawnFigureWharf(building *b)
{
	check_labor_problem(b);
	if (b->data.other.boatFigureId) {
		figure *f = figure_get(b->data.other.boatFigureId);
		if (f->state != FigureState_Alive || f->type != FIGURE_FISHING_BOAT) {
			b->data.other.boatFigureId = 0;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		if (has_figure_of_type(b, FIGURE_CART_PUSHER)) {
            return;
        }
		if (b->figureSpawnDelay) {
			b->figureSpawnDelay = 0;
			b->data.other.fishingBoatHasFish = 0;
			b->outputResourceId = RESOURCE_MEAT;
			figure *f = figure_create(FIGURE_CART_PUSHER, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = RESOURCE_MEAT;
			f->buildingId = b->id;
			b->figureId = f->id;
			f->waitTicks = 30;
		}
	}
}

static void spawnFigureShipyard(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		if (has_figure_of_type(b, FIGURE_FISHING_BOAT)) {
            return;
        }
		int pctWorkers = worker_percentage(b);
		if (pctWorkers >= 100) {
			b->data.industry.progress += 10;
		} else if (pctWorkers >= 75) {
			b->data.industry.progress += 8;
		} else if (pctWorkers >= 50) {
			b->data.industry.progress += 6;
		} else if (pctWorkers >= 25) {
			b->data.industry.progress += 4;
		} else if (pctWorkers >= 1) {
			b->data.industry.progress += 2;
		}
		if (b->data.industry.progress >= 160) {
			b->data.industry.progress = 0;
			int xBoat, yBoat;
			if (Terrain_canSpawnFishingBoatInWater(b->x, b->y, b->size, &xBoat, &yBoat)) {
				figure *f = figure_create(FIGURE_FISHING_BOAT, xBoat, yBoat, DIR_0_TOP);
				f->actionState = FigureActionState_190_FishingBoatCreated;
				f->buildingId = b->id;
				b->figureId = f->id;
			}
		}
	}
}

static void spawnFigureDock(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 50);
		int pctWorkers = worker_percentage(b);
		int maxDockers;
		if (pctWorkers >= 75) {
			maxDockers = 3;
		} else if (pctWorkers >= 50) {
			maxDockers = 2;
		} else if (pctWorkers > 0) {
			maxDockers = 1;
		} else {
			maxDockers = 0;
		}
		// count existing dockers
		int existingDockers = 0;
		for (int i = 0; i < 3; i++) {
			if (b->data.other.dockFigureIds[i]) {
				if (figure_get(b->data.other.dockFigureIds[i])->type == FIGURE_DOCKER) {
					existingDockers++;
				} else {
					b->data.other.dockFigureIds[i] = 0;
				}
			}
		}
		if (existingDockers > maxDockers) {
			// too many dockers, kill one of them
			for (int i = 2; i >= 0; i--) {
				if (b->data.other.dockFigureIds[i]) {
					figure_get(b->data.other.dockFigureIds[i])->state = FigureState_Dead;
					break;
				}
			}
		} else if (existingDockers < maxDockers) {
			figure *f = figure_create(FIGURE_DOCKER, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_132_DockerIdling;
			f->buildingId = b->id;
			for (int i = 0; i < 3; i++) {
				if (!b->data.other.dockFigureIds[i]) {
					b->data.other.dockFigureIds[i] = f->id;
					break;
				}
			}
		}
	}
}

static void spawnFigureNativeHut(building *b)
{
	map_image_set(b->gridOffset, image_group(GROUP_BUILDING_NATIVE) + (map_random_get(b->gridOffset) & 1));
	if (has_figure_of_type(b, FIGURE_INDIGENOUS_NATIVE)) {
        return;
    }
	int xOut, yOut;
	if (b->subtype.nativeMeetingCenterId > 0 && Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > 4) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_INDIGENOUS_NATIVE, xOut, yOut, DIR_0_TOP);
			f->actionState = FigureActionState_158_NativeCreated;
			f->buildingId = b->id;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureNativeMeeting(building *b)
{
	Terrain_addBuildingToGrids(b->id, b->x, b->y, 2,
		image_group(GROUP_BUILDING_NATIVE) + 2, TERRAIN_BUILDING);
	if (Data_CityInfo.nativeMissionPostOperational > 0 &&
		!has_figure_of_type(b, FIGURE_NATIVE_TRADER)) {
        int xOut, yOut;
        if (Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 8) {
				b->figureSpawnDelay = 0;
				figure *f = figure_create(FIGURE_NATIVE_TRADER, xOut, yOut, DIR_0_TOP);
				f->actionState = FigureActionState_162_NativeTraderCreated;
				f->buildingId = b->id;
				b->figureId = f->id;
			}
		}
	}
}

static void spawnFigureBarracks(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
		int pctWorkers = worker_percentage(b);
		int spawnDelay;
		if (pctWorkers >= 100) {
			spawnDelay = 8;
		} else if (pctWorkers >= 75) {
			spawnDelay = 12;
		} else if (pctWorkers >= 50) {
			spawnDelay = 16;
		} else if (pctWorkers >= 25) {
			spawnDelay = 32;
		} else if (pctWorkers >= 1) {
			spawnDelay = 48;
		} else {
			return;
		}
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > spawnDelay) {
			b->figureSpawnDelay = 0;
			Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad);
			if (!Figure_createTowerSentryFromBarracks(b, xRoad, yRoad)) {
				Figure_createSoldierFromBarracks(b, xRoad, yRoad);
			}
		}
	}
}

static void spawnFigureMilitaryAcademy(building *b)
{
	check_labor_problem(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		spawn_labor_seeker(b, xRoad, yRoad, 100);
	}
}

void FigureGeneration_generateFiguresForBuildings()
{
	int patricianGenerated = 0;
	if (Data_Buildings_Extra.barracksTowerSentryRequested > 0) {
		Data_Buildings_Extra.barracksTowerSentryRequested--;
	}
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b)) {
			continue;
		}
		if (b->type == BUILDING_WAREHOUSE_SPACE || (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId)) {
			continue;
		}
		b->showOnProblemOverlay = 0;
		// range of building types
		if (b->type >= BUILDING_HOUSE_SMALL_VILLA && b->type <= BUILDING_HOUSE_LUXURY_PALACE) {
			spawnFigurePatrician(b, &patricianGenerated);
		} else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_POTTERY_WORKSHOP) {
			spawnFigureIndustry(b);
		} else if (b->type >= BUILDING_SENATE && b->type <= BUILDING_FORUM_UPGRADED) {
			spawnFigureSenateForum(b);
		} else if (b->type >= BUILDING_SMALL_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) {
			spawnFigureTemple(b);
		} else {
			// single building type
			switch (b->type) {
				case BUILDING_WAREHOUSE:
					spawnFigureWarehouse(b);
					break;
				case BUILDING_GRANARY:
					spawnFigureGranary(b);
					break;
				case BUILDING_TOWER:
					spawnFigureTower(b);
					break;
				case BUILDING_ENGINEERS_POST:
					spawnFigureEngineersPost(b);
					break;
				case BUILDING_PREFECTURE:
					spawnFigurePrefecture(b);
					break;
				case BUILDING_ACTOR_COLONY:
					spawnFigureActorColony(b);
					break;
				case BUILDING_GLADIATOR_SCHOOL:
					spawnFigureGladiatorSchool(b);
					break;
				case BUILDING_LION_HOUSE:
					spawnFigureLionHouse(b);
					break;
				case BUILDING_CHARIOT_MAKER:
					spawnFigureChariotMaker(b);
					break;
				case BUILDING_AMPHITHEATER:
					spawnFigureAmphitheater(b);
					break;
				case BUILDING_THEATER:
					spawnFigureTheater(b);
					break;
				case BUILDING_HIPPODROME:
					spawnFigureHippodrome(b);
					break;
				case BUILDING_COLOSSEUM:
					spawnFigureColosseum(b);
					break;
				case BUILDING_MARKET:
					spawnFigureMarket(b);
					break;
				case BUILDING_BATHHOUSE:
					spawnFigureBathhouse(b);
					break;
				case BUILDING_SCHOOL:
					spawnFigureSchool(b);
					break;
				case BUILDING_LIBRARY:
					spawnFigureLibrary(b);
					break;
				case BUILDING_ACADEMY:
					spawnFigureAcademy(b);
					break;
				case BUILDING_BARBER:
					spawnFigureBarber(b);
					break;
				case BUILDING_DOCTOR:
					spawnFigureDoctor(b);
					break;
				case BUILDING_HOSPITAL:
					spawnFigureHospital(b);
					break;
				case BUILDING_MISSION_POST:
					spawnFigureMissionPost(b);
					break;
				case BUILDING_DOCK:
					spawnFigureDock(b);
					break;
				case BUILDING_WHARF:
					spawnFigureWharf(b);
					break;
				case BUILDING_SHIPYARD:
					spawnFigureShipyard(b);
					break;
				case BUILDING_NATIVE_HUT:
					spawnFigureNativeHut(b);
					break;
				case BUILDING_NATIVE_MEETING:
					spawnFigureNativeMeeting(b);
					break;
				case BUILDING_NATIVE_CROPS:
					TerrainGraphics_updateNativeCropProgress(b);
					break;
				case BUILDING_FORT:
					Formation_setNewSoldierRequest(b);
					break;
				case BUILDING_BARRACKS:
					spawnFigureBarracks(b);
					break;
				case BUILDING_MILITARY_ACADEMY:
					spawnFigureMilitaryAcademy(b);
					break;
			}
		}
	}
}

