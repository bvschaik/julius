#include "Figure.h"

#include "Building.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Grid.h"

#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "figure/type.h"
#include "graphics/image.h"
#include "game/resource.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/random.h"

#define SET_LABOR_PROBLEM(b) if (b->housesCovered <= 0) b->showOnProblemOverlay = 2
#define SPAWN_LABOR_SEEKER(t) if (b->housesCovered <= t) generateLaborSeeker(buildingId, b, xRoad, yRoad);
#define EXIT_IF_FIGURE(t) if (buildingHasFigureOfType(buildingId, t, 0)) return;
#define EXIT_IF_FIGURES(t1,t2) if (buildingHasFigureOfType(buildingId, t1, t2)) return;

static int worker_percentage(struct Data_Building *b)
{
    return calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
}

static void generateLaborSeeker(int buildingId, struct Data_Building *b, int x, int y)
{
	if (Data_CityInfo.population <= 0) {
		return;
	}
	if (b->figureId2) {
		figure *f = figure_get(b->figureId2);
		if (!f->state || f->type != FIGURE_LABOR_SEEKER || f->buildingId != buildingId) {
			b->figureId2 = 0;
		}
	} else {
		figure *f = figure_create(FIGURE_LABOR_SEEKER, x, y, DIR_0_TOP);
		f->actionState = FigureActionState_125_Roaming;
		f->buildingId = buildingId;
		b->figureId2 = f->id;
		FigureMovement_initRoaming(f);
	}
}

static int buildingHasFigureOfType(int buildingId, int type1, int type2)
{
	int figureId = Data_Buildings[buildingId].figureId;
	if (figureId <= 0) {
		return 0;
	}
	figure *f = figure_get(figureId);
	if (f->state && f->buildingId == buildingId && (f->type == type1 || f->type == type2)) {
		return 1;
	} else {
		Data_Buildings[buildingId].figureId = 0;
		return 0;
	}
}

static void spawnFigurePatrician(int buildingId, struct Data_Building *b, int *patricianSpawned)
{
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > 40 && !*patricianSpawned) {
			*patricianSpawned = 1;
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_PATRICIAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureWarehouse(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int spaceId = buildingId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		if (spaceId) {
			Data_Buildings[spaceId].showOnProblemOverlay = b->showOnProblemOverlay;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad) ||
		Terrain_hasRoadAccess(b->x, b->y, 3, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
		EXIT_IF_FIGURE(FIGURE_WAREHOUSEMAN);
		int resource;
		int task = Resource_determineWarehouseWorkerTask(buildingId, &resource);
		if (task >= 0) {
			figure *f = figure_create(FIGURE_WAREHOUSEMAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			if (task == StorageFigureTask_Getting) {
				f->collectingItemId = resource;
			}
			b->figureId = f->id;
			f->buildingId = buildingId;
		}
	}
}

static void spawnFigureGranary(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessGranary(b->x, b->y, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
		EXIT_IF_FIGURE(FIGURE_WAREHOUSEMAN);
		int task = Resource_determineGranaryWorkerTask(buildingId);
		if (task >= 0) {
			figure *f = figure_create(FIGURE_WAREHOUSEMAN, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			b->figureId = f->id;
			f->buildingId = buildingId;
		}
	}
}

static void spawnFigureTower(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		if (b->numWorkers <= 0) {
			return;
		}
		if (!b->figureId4 && b->figureId) { // has sentry but no ballista -> create
			figure *f = figure_create(FIGURE_BALLISTA, b->x, b->y, DIR_0_TOP);
			b->figureId4 = f->id;
			f->buildingId = buildingId;
			f->actionState = FigureActionState_180_BallistaCreated;
		}
		buildingHasFigureOfType(buildingId, FIGURE_TOWER_SENTRY, 0);
		if (b->figureId <= 0) {
			Data_Buildings_Extra.barracksTowerSentryRequested = 2;
		}
	}
}

static void spawnFigureEngineersPost(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_ENGINEER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigurePrefecture(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_PREFECT);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureActorColony(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureGladiatorSchool(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureLionHouse(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureChariotMaker(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureAmphitheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURES(FIGURE_ACTOR, FIGURE_GLADIATOR);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureTheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_ACTOR);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureHippodrome(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	if (b->prevPartBuildingId) {
		return;
	}
	int partId = buildingId;
	for (int i = 0; i < 2; i++) {
		partId = Data_Buildings[partId].nextPartBuildingId;
		if (partId) {
			Data_Buildings[partId].showOnProblemOverlay = b->showOnProblemOverlay;
		}
	}
	EXIT_IF_FIGURE(FIGURE_CHARIOTEER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessHippodrome(b->x, b->y, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);

			if (!Data_CityInfo.entertainmentHippodromeHasShow) {
				// create mini-horses
				figure *horse1 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 1, DIR_2_RIGHT);
				horse1->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
				horse1->buildingId = buildingId;
				horse1->resourceId = 0;
				horse1->speedMultiplier = 3;

				figure *horse2 = figure_create(FIGURE_HIPPODROME_HORSES, b->x + 2, b->y + 2, DIR_2_RIGHT);
				horse2->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
				horse2->buildingId = buildingId;
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

static void spawnFigureColosseum(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURES(FIGURE_GLADIATOR, FIGURE_LION_TAMER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
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
			f->buildingId = buildingId;
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

static void setMarketGraphic(int buildingId, struct Data_Building *b)
{
	if (!BuildingIsInUse(buildingId)) {
		return;
	}
	if (map_desirability_get(b->gridOffset) <= 30) {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_MARKET), Terrain_Building);
	} else {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_MARKET_FANCY), Terrain_Building);
	}
}

static void spawnFigureMarket(int buildingId, struct Data_Building *b)
{
	setMarketGraphic(buildingId, b);
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
		if (!buildingHasFigureOfType(buildingId, FIGURE_MARKET_TRADER, 0)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay <= spawnDelay) {
				return;
			}
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_MARKET_TRADER, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
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
			int dstBuildingId = Building_Market_getDestinationGranaryWarehouse(buildingId);
			if (dstBuildingId > 0) {
				figure *f = figure_create(FIGURE_MARKET_BUYER, xRoad, yRoad, DIR_0_TOP);
				f->actionState = FigureActionState_145_MarketBuyerGoingToStorage;
				f->buildingId = buildingId;
				b->figureId2 = f->id;
				f->destinationBuildingId = dstBuildingId;
				f->collectingItemId = b->data.market.fetchInventoryId;
				struct Data_Building *bDst = &Data_Buildings[dstBuildingId];
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

static void setBathhouseGraphic(int buildingId, struct Data_Building *b)
{
	if (!BuildingIsInUse(buildingId)) {
		return;
	}
	if (Terrain_existsTileWithinAreaWithType(b->x, b->y, b->size, Terrain_ReservoirRange)) {
		b->hasWaterAccess = 1;
	} else {
		b->hasWaterAccess = 0;
	}
	if (b->hasWaterAccess && b->numWorkers) {
		if (map_desirability_get(b->gridOffset) <= 30) {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_WATER), Terrain_Building);
		} else {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_FANCY_WATER), Terrain_Building);
		}
	} else {
		if (map_desirability_get(b->gridOffset) <= 30) {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_NO_WATER), Terrain_Building);
		} else {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				image_group(GROUP_BUILDING_BATHHOUSE_FANCY_NO_WATER), Terrain_Building);
		}
	}
}

static void spawnFigureBathhouse(int buildingId, struct Data_Building *b)
{
	setBathhouseGraphic(buildingId, b);
	SET_LABOR_PROBLEM(b);
	if (!b->hasWaterAccess) {
		b->showOnProblemOverlay = 2;
	}
	EXIT_IF_FIGURE(FIGURE_BATHHOUSE_WORKER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad) && b->hasWaterAccess) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureSchool(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_SCHOOL_CHILD);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			child1->buildingId = buildingId;
			b->figureId = child1->id;
			FigureMovement_initRoaming(child1);

			figure *child2 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child2->actionState = FigureActionState_125_Roaming;
			child2->buildingId = buildingId;
			FigureMovement_initRoaming(child2);

			figure *child3 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child3->actionState = FigureActionState_125_Roaming;
			child3->buildingId = buildingId;
			FigureMovement_initRoaming(child3);

			figure *child4 = figure_create(FIGURE_SCHOOL_CHILD, xRoad, yRoad, DIR_0_TOP);
			child4->actionState = FigureActionState_125_Roaming;
			child4->buildingId = buildingId;
			FigureMovement_initRoaming(child4);
		}
	}
}

static void spawnFigureLibrary(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_LIBRARIAN);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			figure *f = figure_create(FIGURE_LIBRARIAN, xRoad, yRoad, DIR_0_TOP);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureAcademy(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_TEACHER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureBarber(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_BARBER);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureDoctor(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_DOCTOR);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureHospital(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_SURGEON);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void spawnFigureTemple(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_PRIEST);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
			FigureMovement_initRoaming(f);
		}
	}
}

static void setSenateGraphic(int buildingId, struct Data_Building *b)
{
	if (!BuildingIsInUse(buildingId)) {
		return;
	}
	if (map_desirability_get(b->gridOffset) <= 30) {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_SENATE), Terrain_Building);
	} else {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			image_group(GROUP_BUILDING_SENATE_FANCY), Terrain_Building);
	}
}

static void spawnFigureSenateForum(int buildingId, struct Data_Building *b)
{
	if (b->type == BUILDING_SENATE_UPGRADED) {
		setSenateGraphic(buildingId, b);
	}
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(FIGURE_TAX_COLLECTOR);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureMissionPost(int buildingId, struct Data_Building *b)
{
	EXIT_IF_FIGURE(FIGURE_MISSIONARY);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (Data_CityInfo.population > 0) {
			Data_CityInfo.nativeMissionPostOperational = 1;
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 1) {
				b->figureSpawnDelay = 0;
				figure *f = figure_create(FIGURE_MISSIONARY, xRoad, yRoad, DIR_0_TOP);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				b->figureId = f->id;
				FigureMovement_initRoaming(f);
			}
		}
	}
}

static void spawnFigureIndustry(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_FIGURE(FIGURE_CART_PUSHER);
		if (Building_Industry_hasProducedResource(buildingId)) {
			Building_Industry_startNewProduction(buildingId);
			figure *f = figure_create(FIGURE_CART_PUSHER, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = b->outputResourceId;
			f->buildingId = buildingId;
			b->figureId = f->id;
			f->waitTicks = 30;
		}
	}
}

static void spawnFigureWharf(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	if (b->data.other.boatFigureId) {
		figure *f = figure_get(b->data.other.boatFigureId);
		if (f->state != FigureState_Alive || f->type != FIGURE_FISHING_BOAT) {
			b->data.other.boatFigureId = 0;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_FIGURE(FIGURE_CART_PUSHER);
		if (b->figureSpawnDelay) {
			b->figureSpawnDelay = 0;
			b->data.other.fishingBoatHasFish = 0;
			b->outputResourceId = RESOURCE_MEAT;
			figure *f = figure_create(FIGURE_CART_PUSHER, xRoad, yRoad, DIR_4_BOTTOM);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = RESOURCE_MEAT;
			f->buildingId = buildingId;
			b->figureId = f->id;
			f->waitTicks = 30;
		}
	}
}

static void spawnFigureShipyard(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_FIGURE(FIGURE_FISHING_BOAT);
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
				f->buildingId = buildingId;
				b->figureId = f->id;
			}
		}
	}
}

static void spawnFigureDock(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
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
			f->buildingId = buildingId;
			for (int i = 0; i < 3; i++) {
				if (!b->data.other.dockFigureIds[i]) {
					b->data.other.dockFigureIds[i] = f->id;
					break;
				}
			}
		}
	}
}

static void spawnFigureNativeHut(int buildingId, struct Data_Building *b)
{
	map_image_set(b->gridOffset, image_group(GROUP_BUILDING_NATIVE) + (map_random_get(b->gridOffset) & 1));
	EXIT_IF_FIGURE(FIGURE_INDIGENOUS_NATIVE);
	int xOut, yOut;
	if (b->subtype.nativeMeetingCenterId > 0 && Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > 4) {
			b->figureSpawnDelay = 0;
			figure *f = figure_create(FIGURE_INDIGENOUS_NATIVE, xOut, yOut, DIR_0_TOP);
			f->actionState = FigureActionState_158_NativeCreated;
			f->buildingId = buildingId;
			b->figureId = f->id;
		}
	}
}

static void spawnFigureNativeMeeting(int buildingId, struct Data_Building *b)
{
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, 2,
		image_group(GROUP_BUILDING_NATIVE) + 2, Terrain_Building);
	if (Data_CityInfo.nativeMissionPostOperational > 0 &&
		!buildingHasFigureOfType(buildingId, FIGURE_NATIVE_TRADER, 0)) {
		int xOut, yOut;
		if (Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 8) {
				b->figureSpawnDelay = 0;
				figure *f = figure_create(FIGURE_NATIVE_TRADER, xOut, yOut, DIR_0_TOP);
				f->actionState = FigureActionState_162_NativeTraderCreated;
				f->buildingId = buildingId;
				b->figureId = f->id;
			}
		}
	}
}

static void spawnFigureBarracks(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
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
			if (!Figure_createTowerSentryFromBarracks(buildingId, xRoad, yRoad)) {
				Figure_createSoldierFromBarracks(buildingId, xRoad, yRoad);
			}
		}
	}
}

static void spawnFigureMilitaryAcademy(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
	}
}

void FigureGeneration_generateFiguresForBuildings()
{
	int patricianGenerated = 0;
	if (Data_Buildings_Extra.barracksTowerSentryRequested > 0) {
		Data_Buildings_Extra.barracksTowerSentryRequested--;
	}
	for (int i = 1; i <= Data_Buildings_Extra.highestBuildingIdInUse; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i)) {
			continue;
		}
		if (b->type == BUILDING_WAREHOUSE_SPACE || (b->type == BUILDING_HIPPODROME && b->prevPartBuildingId)) {
			continue;
		}
		b->showOnProblemOverlay = 0;
		// range of building types
		if (b->type >= BUILDING_HOUSE_SMALL_VILLA && b->type <= BUILDING_HOUSE_LUXURY_PALACE) {
			spawnFigurePatrician(i, b, &patricianGenerated);
		} else if (b->type >= BUILDING_WHEAT_FARM && b->type <= BUILDING_POTTERY_WORKSHOP) {
			spawnFigureIndustry(i, b);
		} else if (b->type >= BUILDING_SENATE && b->type <= BUILDING_FORUM_UPGRADED) {
			spawnFigureSenateForum(i, b);
		} else if (b->type >= BUILDING_SMALL_TEMPLE_CERES && b->type <= BUILDING_LARGE_TEMPLE_VENUS) {
			spawnFigureTemple(i, b);
		} else {
			// single building type
			switch (b->type) {
				case BUILDING_WAREHOUSE:
					spawnFigureWarehouse(i, b);
					break;
				case BUILDING_GRANARY:
					spawnFigureGranary(i, b);
					break;
				case BUILDING_TOWER:
					spawnFigureTower(i, b);
					break;
				case BUILDING_ENGINEERS_POST:
					spawnFigureEngineersPost(i, b);
					break;
				case BUILDING_PREFECTURE:
					spawnFigurePrefecture(i, b);
					break;
				case BUILDING_ACTOR_COLONY:
					spawnFigureActorColony(i, b);
					break;
				case BUILDING_GLADIATOR_SCHOOL:
					spawnFigureGladiatorSchool(i, b);
					break;
				case BUILDING_LION_HOUSE:
					spawnFigureLionHouse(i, b);
					break;
				case BUILDING_CHARIOT_MAKER:
					spawnFigureChariotMaker(i, b);
					break;
				case BUILDING_AMPHITHEATER:
					spawnFigureAmphitheater(i, b);
					break;
				case BUILDING_THEATER:
					spawnFigureTheater(i, b);
					break;
				case BUILDING_HIPPODROME:
					spawnFigureHippodrome(i, b);
					break;
				case BUILDING_COLOSSEUM:
					spawnFigureColosseum(i, b);
					break;
				case BUILDING_MARKET:
					spawnFigureMarket(i, b);
					break;
				case BUILDING_BATHHOUSE:
					spawnFigureBathhouse(i, b);
					break;
				case BUILDING_SCHOOL:
					spawnFigureSchool(i, b);
					break;
				case BUILDING_LIBRARY:
					spawnFigureLibrary(i, b);
					break;
				case BUILDING_ACADEMY:
					spawnFigureAcademy(i, b);
					break;
				case BUILDING_BARBER:
					spawnFigureBarber(i, b);
					break;
				case BUILDING_DOCTOR:
					spawnFigureDoctor(i, b);
					break;
				case BUILDING_HOSPITAL:
					spawnFigureHospital(i, b);
					break;
				case BUILDING_MISSION_POST:
					spawnFigureMissionPost(i, b);
					break;
				case BUILDING_DOCK:
					spawnFigureDock(i, b);
					break;
				case BUILDING_WHARF:
					spawnFigureWharf(i, b);
					break;
				case BUILDING_SHIPYARD:
					spawnFigureShipyard(i, b);
					break;
				case BUILDING_NATIVE_HUT:
					spawnFigureNativeHut(i, b);
					break;
				case BUILDING_NATIVE_MEETING:
					spawnFigureNativeMeeting(i, b);
					break;
				case BUILDING_NATIVE_CROPS:
					TerrainGraphics_updateNativeCropProgress(i);
					break;
				case BUILDING_FORT:
					Formation_setNewSoldierRequest(i);
					break;
				case BUILDING_BARRACKS:
					spawnFigureBarracks(i, b);
					break;
				case BUILDING_MILITARY_ACADEMY:
					spawnFigureMilitaryAcademy(i, b);
					break;
			}
		}
	}
}

