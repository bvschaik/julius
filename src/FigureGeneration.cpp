#include "Figure.h"

#include "Building.h"
#include "Calc.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Figure.h"

#define SET_LABOR_PROBLEM(b) if (b->housesCovered <= 0) b->showOnProblemOverlay = 2
#define SPAWN_LABOR_SEEKER(t) if (b->housesCovered <= t) generateLaborSeeker(buildingId, b, xRoad, yRoad);
#define EXIT_IF_FIGURE(t) if (buildingHasFigureOfType(buildingId, t, 0)) return;
#define EXIT_IF_FIGURES(t1,t2) if (buildingHasFigureOfType(buildingId, t1, t2)) return;

#define WORKER_PERCENTAGE(b) Calc_getPercentage(b->numWorkers, Data_Model_Buildings[b->type].laborers)

#define CREATE_FIGURE(t,x,y,d) \
	int figureId = Figure_create(t, x, y, d);\
	struct Data_Figure *f = &Data_Figures[figureId];

static void generateLaborSeeker(int buildingId, struct Data_Building *b, int x, int y)
{
	if (Data_CityInfo.population <= 0) {
		return;
	}
	if (b->figureId2) {
		struct Data_Figure *f = &Data_Figures[b->figureId2];
		if (!f->state || f->type != Figure_LaborSeeker || f->buildingId != buildingId) {
			b->figureId2 = 0;
		}
	} else {
		CREATE_FIGURE(Figure_LaborSeeker, x, y, Dir_0_Top);
		f->actionState = FigureActionState_125_Roaming;
		f->buildingId = buildingId;
		b->figureId2 = figureId;
		FigureMovement_initRoaming(figureId);
	}
}

static int buildingHasFigureOfType(int buildingId, int type1, int type2)
{
	int figureId = Data_Buildings[buildingId].figureId;
	if (figureId <= 0) {
		return 0;
	}
	struct Data_Figure *f = &Data_Figures[figureId];
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
			CREATE_FIGURE(Figure_Patrician, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			FigureMovement_initRoaming(figureId);
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
		EXIT_IF_FIGURE(Figure_Warehouseman);
		int resource;
		int task = Resource_determineWarehouseWorkerTask(buildingId, &resource);
		if (task >= 0) {
			CREATE_FIGURE(Figure_Warehouseman, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			if (task == StorageFigureTask_Getting) {
				f->collectingItemId = resource;
			}
			b->figureId = figureId;
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
		EXIT_IF_FIGURE(Figure_Warehouseman);
		int task = Resource_determineGranaryWorkerTask(buildingId);
		if (task >= 0) {
			CREATE_FIGURE(Figure_Warehouseman, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_50_WarehousemanCreated;
			f->resourceId = task;
			b->figureId = figureId;
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
			CREATE_FIGURE(Figure_Ballista, b->x, b->y, Dir_0_Top);
			b->figureId4 = figureId;
			f->buildingId = buildingId;
			f->actionState = FigureActionState_180_BallistaCreated;
		}
		buildingHasFigureOfType(buildingId, Figure_TowerSentry, 0);
		if (b->figureId <= 0) {
			Data_Buildings_Extra.barracksTowerSentryRequested = 2;
		}
	}
}

static void spawnFigureEngineersPost(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Engineer);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Engineer, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_60_EngineerCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigurePrefecture(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Prefect);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Prefect, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_70_PrefectCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureActorColony(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Actor, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureGladiatorSchool(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Gladiator, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureLionHouse(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_LionTamer, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureChariotMaker(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Charioteer, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_90_EntertainerAtSchoolCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureAmphitheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURES(Figure_Actor, Figure_Gladiator);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
		}
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			int figureId;
			if (b->data.entertainment.days1 > 0) {
				figureId = Figure_create(Figure_Gladiator, xRoad, yRoad, Dir_0_Top);
			} else {
				figureId = Figure_create(Figure_Actor, xRoad, yRoad, Dir_0_Top);
			}
			struct Data_Figure *f = &Data_Figures[figureId];
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureTheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Actor);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
		}
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Actor, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
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
	EXIT_IF_FIGURE(Figure_Charioteer);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessHippodrome(b->x, b->y, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 || b->data.entertainment.days1 <= 0) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
		}
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			{
				CREATE_FIGURE(Figure_Charioteer, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_94_EntertainerRoaming;
				f->buildingId = buildingId;
				b->figureId = figureId;
				FigureMovement_initRoaming(figureId);
			}
			if (!Data_CityInfo.entertainmentHippodromeHasShow) {
				// create mini-horses
				{
					CREATE_FIGURE(Figure_HippodromeMiniHorses, b->x + 2, b->y + 1, Dir_2_Right);
					f->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
					f->buildingId = buildingId;
					f->resourceId = 0;
					f->speedMultiplier = 3;
				}
				{
					CREATE_FIGURE(Figure_HippodromeMiniHorses, b->x + 2, b->y + 2, Dir_2_Right);
					f->actionState = FigureActionState_200_HippodromeMiniHorseCreated;
					f->buildingId = buildingId;
					f->resourceId = 1;
					f->speedMultiplier = 2;
				}
				if (b->data.entertainment.days1 > 0) {
					if (!Data_CityInfo.messageShownHippodrome) {
						Data_CityInfo.messageShownHippodrome = 1;
						PlayerMessage_post(1, Message_109_WorkingHippodrome, 0, 0);
					}
				}
			}
		}
	}
}

static void spawnFigureColosseum(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURES(Figure_Gladiator, Figure_LionTamer);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (b->housesCovered <= 50 ||
			(b->data.entertainment.days1 <= 0 && b->data.entertainment.days2 <= 0)) {
			generateLaborSeeker(buildingId, b, xRoad, yRoad);
		}
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			int figureId;
			if (b->data.entertainment.days1 > 0) {
				figureId = Figure_create(Figure_LionTamer, xRoad, yRoad, Dir_0_Top);
			} else {
				figureId = Figure_create(Figure_Gladiator, xRoad, yRoad, Dir_0_Top);
			}
			struct Data_Figure *f = &Data_Figures[figureId];
			f->actionState = FigureActionState_94_EntertainerRoaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
			if (b->data.entertainment.days1 > 0 || b->data.entertainment.days2 > 0) {
				if (!Data_CityInfo.messageShownColosseum) {
					Data_CityInfo.messageShownColosseum = 1;
					PlayerMessage_post(1, Message_110_WorkingColosseum, 0, 0);
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
	if (Data_Grid_desirability[b->gridOffset] <= 30) {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			GraphicId(ID_Graphic_Market), Terrain_Building);
	} else {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			GraphicId(ID_Graphic_MarketFancy), Terrain_Building);
	}
}

static void spawnFigureMarket(int buildingId, struct Data_Building *b)
{
	setMarketGraphic(buildingId, b);
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
		if (!buildingHasFigureOfType(buildingId, Figure_MarketTrader, 0)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay <= spawnDelay) {
				return;
			}
			b->figureSpawnDelay = 0;
			CREATE_FIGURE(Figure_MarketTrader, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
		// market buyer or labor seeker
		if (b->figureId2) {
			struct Data_Figure *f = &Data_Figures[b->figureId2];
			if (f->state != FigureState_Alive || (f->type != Figure_MarketBuyer && f->type != Figure_LaborSeeker)) {
				b->figureId2 = 0;
			}
		} else {
			Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad);
			int dstBuildingId = Building_Market_getDestinationGranaryWarehouse(buildingId);
			if (dstBuildingId > 0) {
				CREATE_FIGURE(Figure_MarketBuyer, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_145_MarketBuyerGoingToStorage;
				f->buildingId = buildingId;
				b->figureId2 = figureId;
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
		if (Data_Grid_desirability[b->gridOffset] <= 30) {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				GraphicId(ID_Graphic_BathhouseWater), Terrain_Building);
		} else {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				GraphicId(ID_Graphic_BathhouseFancyWater), Terrain_Building);
		}
	} else {
		if (Data_Grid_desirability[b->gridOffset] <= 30) {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				GraphicId(ID_Graphic_BathhouseNoWater), Terrain_Building);
		} else {
			Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
				GraphicId(ID_Graphic_BathhouseFancyNoWater), Terrain_Building);
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
	EXIT_IF_FIGURE(Figure_BathhouseWorker);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad) && b->hasWaterAccess) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_BathhouseWorker, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureSchool(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_SchoolChild);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			{
				CREATE_FIGURE(Figure_SchoolChild, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				b->figureId = figureId;
				FigureMovement_initRoaming(figureId);
			}
			{
				CREATE_FIGURE(Figure_SchoolChild, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				FigureMovement_initRoaming(figureId);
			}
			{
				CREATE_FIGURE(Figure_SchoolChild, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				FigureMovement_initRoaming(figureId);
			}
			{
				CREATE_FIGURE(Figure_SchoolChild, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				FigureMovement_initRoaming(figureId);
			}
		}
	}
}

static void spawnFigureLibrary(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Librarian);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Librarian, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureAcademy(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Teacher);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Teacher, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureBarber(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Barber);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Barber, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureDoctor(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Doctor);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Doctor, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureHospital(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Surgeon);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_Surgeon, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void spawnFigureTemple(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_Priest);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
		int spawnDelay;
		if (Data_Model_Buildings[b->type].laborers <= 0) {
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
			CREATE_FIGURE(Figure_Priest, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_125_Roaming;
			f->buildingId = buildingId;
			b->figureId = figureId;
			FigureMovement_initRoaming(figureId);
		}
	}
}

static void setSenateGraphic(int buildingId, struct Data_Building *b)
{
	if (!BuildingIsInUse(buildingId)) {
		return;
	}
	if (Data_Grid_desirability[b->gridOffset] <= 30) {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			GraphicId(ID_Graphic_Senate), Terrain_Building);
	} else {
		Terrain_addBuildingToGrids(buildingId, b->x, b->y, b->size,
			GraphicId(ID_Graphic_SenateFancy), Terrain_Building);
	}
}

static void spawnFigureSenateForum(int buildingId, struct Data_Building *b)
{
	if (b->type == Building_SenateUpgraded) {
		setSenateGraphic(buildingId, b);
	}
	SET_LABOR_PROBLEM(b);
	EXIT_IF_FIGURE(Figure_TaxCollector);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
			CREATE_FIGURE(Figure_TaxCollector, xRoad, yRoad, Dir_0_Top);
			f->actionState = FigureActionState_40_TaxCollectorCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureMissionPost(int buildingId, struct Data_Building *b)
{
	EXIT_IF_FIGURE(Figure_Missionary);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (Data_CityInfo.population > 0) {
			Data_CityInfo.nativeMissionPostOperational = 1;
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 1) {
				b->figureSpawnDelay = 0;
				CREATE_FIGURE(Figure_Missionary, xRoad, yRoad, Dir_0_Top);
				f->actionState = FigureActionState_125_Roaming;
				f->buildingId = buildingId;
				b->figureId = figureId;
				FigureMovement_initRoaming(figureId);
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
		EXIT_IF_FIGURE(Figure_CartPusher);
		if (Building_Industry_hasProducedResource(buildingId)) {
			Building_Industry_startNewProduction(buildingId);
			CREATE_FIGURE(Figure_CartPusher, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = b->outputResourceId;
			f->buildingId = buildingId;
			b->figureId = figureId;
			f->waitTicks = 30;
		}
	}
}

static void spawnFigureWharf(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	if (b->data.other.boatFigureId) {
		struct Data_Figure *f = &Data_Figures[b->data.other.boatFigureId];
		if (f->state != FigureState_Alive || f->type != Figure_FishingBoat) {
			b->data.other.boatFigureId = 0;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_FIGURE(Figure_CartPusher);
		if (b->figureSpawnDelay) {
			b->figureSpawnDelay = 0;
			b->data.other.fishingBoatHasFish = 0;
			b->outputResourceId = Resource_Meat;
			CREATE_FIGURE(Figure_CartPusher, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_20_CartpusherInitial;
			f->resourceId = Resource_Meat;
			f->buildingId = buildingId;
			b->figureId = figureId;
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
		EXIT_IF_FIGURE(Figure_FishingBoat);
		int pctWorkers = WORKER_PERCENTAGE(b);
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
				CREATE_FIGURE(Figure_FishingBoat, xBoat, yBoat, Dir_0_Top);
				f->actionState = FigureActionState_190_FishingBoatCreated;
				f->buildingId = buildingId;
				b->figureId = figureId;
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
		int pctWorkers = WORKER_PERCENTAGE(b);
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
				if (Data_Figures[b->data.other.dockFigureIds[i]].type == Figure_Dockman) {
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
					Data_Figures[b->data.other.dockFigureIds[i]].state = FigureState_Dead;
					break;
				}
			}
		} else if (existingDockers < maxDockers) {
			CREATE_FIGURE(Figure_Dockman, xRoad, yRoad, Dir_4_Bottom);
			f->actionState = FigureActionState_132_DockerIdling;
			f->buildingId = buildingId;
			for (int i = 0; i < 3; i++) {
				if (!b->data.other.dockFigureIds[i]) {
					b->data.other.dockFigureIds[i] = figureId;
					break;
				}
			}
		}
	}
}

static void spawnFigureNativeHut(int buildingId, struct Data_Building *b)
{
	Data_Grid_graphicIds[b->gridOffset] =
		GraphicId(ID_Graphic_NativeBuilding) + (Data_Grid_random[b->gridOffset] & 1);
	EXIT_IF_FIGURE(Figure_IndigenousNative);
	int xOut, yOut;
	if (b->subtype.nativeMeetingCenterId > 0 && Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
		b->figureSpawnDelay++;
		if (b->figureSpawnDelay > 4) {
			b->figureSpawnDelay = 0;
			CREATE_FIGURE(Figure_IndigenousNative, xOut, yOut, Dir_0_Top);
			f->actionState = FigureActionState_158_NativeCreated;
			f->buildingId = buildingId;
			b->figureId = figureId;
		}
	}
}

static void spawnFigureNativeMeeting(int buildingId, struct Data_Building *b)
{
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, 2,
		GraphicId(ID_Graphic_NativeBuilding) + 2, Terrain_Building);
	if (Data_CityInfo.nativeMissionPostOperational > 0 &&
		!buildingHasFigureOfType(buildingId, Figure_NativeTrader, 0)) {
		int xOut, yOut;
		if (Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
			b->figureSpawnDelay++;
			if (b->figureSpawnDelay > 8) {
				b->figureSpawnDelay = 0;
				CREATE_FIGURE(Figure_NativeTrader, xOut, yOut, Dir_0_Top);
				f->actionState = FigureActionState_162_NativeTraderCreated;
				f->buildingId = buildingId;
				b->figureId = figureId;
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
		int pctWorkers = WORKER_PERCENTAGE(b);
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
		if (b->type == Building_WarehouseSpace || (b->type == Building_Hippodrome && b->prevPartBuildingId)) {
			continue;
		}
		b->showOnProblemOverlay = 0;
		// range of building types
		if (b->type >= Building_HouseSmallVilla && b->type <= Building_HouseLuxuryPalace) {
			spawnFigurePatrician(i, b, &patricianGenerated);
		} else if (b->type >= Building_WheatFarm && b->type <= Building_PotteryWorkshop) {
			spawnFigureIndustry(i, b);
		} else if (b->type >= Building_Senate && b->type <= Building_ForumUpgraded) {
			spawnFigureSenateForum(i, b);
		} else if (b->type >= Building_SmallTempleCeres && b->type <= Building_LargeTempleVenus) {
			spawnFigureTemple(i, b);
		} else {
			// single building type
			switch (b->type) {
				case Building_Warehouse:
					spawnFigureWarehouse(i, b);
					break;
				case Building_Granary:
					spawnFigureGranary(i, b);
					break;
				case Building_Tower:
					spawnFigureTower(i, b);
					break;
				case Building_EngineersPost:
					spawnFigureEngineersPost(i, b);
					break;
				case Building_Prefecture:
					spawnFigurePrefecture(i, b);
					break;
				case Building_ActorColony:
					spawnFigureActorColony(i, b);
					break;
				case Building_GladiatorSchool:
					spawnFigureGladiatorSchool(i, b);
					break;
				case Building_LionHouse:
					spawnFigureLionHouse(i, b);
					break;
				case Building_ChariotMaker:
					spawnFigureChariotMaker(i, b);
					break;
				case Building_Amphitheater:
					spawnFigureAmphitheater(i, b);
					break;
				case Building_Theater:
					spawnFigureTheater(i, b);
					break;
				case Building_Hippodrome:
					spawnFigureHippodrome(i, b);
					break;
				case Building_Colosseum:
					spawnFigureColosseum(i, b);
					break;
				case Building_Market:
					spawnFigureMarket(i, b);
					break;
				case Building_Bathhouse:
					spawnFigureBathhouse(i, b);
					break;
				case Building_School:
					spawnFigureSchool(i, b);
					break;
				case Building_Library:
					spawnFigureLibrary(i, b);
					break;
				case Building_Academy:
					spawnFigureAcademy(i, b);
					break;
				case Building_Barber:
					spawnFigureBarber(i, b);
					break;
				case Building_Doctor:
					spawnFigureDoctor(i, b);
					break;
				case Building_Hospital:
					spawnFigureHospital(i, b);
					break;
				case Building_MissionPost:
					spawnFigureMissionPost(i, b);
					break;
				case Building_Dock:
					spawnFigureDock(i, b);
					break;
				case Building_Wharf:
					spawnFigureWharf(i, b);
					break;
				case Building_Shipyard:
					spawnFigureShipyard(i, b);
					break;
				case Building_NativeHut:
					spawnFigureNativeHut(i, b);
					break;
				case Building_NativeMeeting:
					spawnFigureNativeMeeting(i, b);
					break;
				case Building_NativeCrops:
					TerrainGraphics_updateNativeCropProgress(i);
					break;
				case Building_Fort:
					Formation_setNewSoldierRequest(i);
					break;
				case Building_Barracks:
					spawnFigureBarracks(i, b);
					break;
				case Building_MilitaryAcademy:
					spawnFigureMilitaryAcademy(i, b);
					break;
			}
		}
	}
}

