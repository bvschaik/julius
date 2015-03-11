#include "Walker.h"

#include "Building.h"
#include "Calc.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "WalkerMovement.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Walker.h"

#define SET_LABOR_PROBLEM(b) if (b->housesCovered <= 0) b->showOnProblemOverlay = 2
#define SPAWN_LABOR_SEEKER(t) if (b->housesCovered <= t) generateLaborSeeker(buildingId, b, xRoad, yRoad);
#define EXIT_IF_WALKER(t) if (buildingHasWalkerOfType(buildingId, t, 0)) return;
#define EXIT_IF_WALKERS(t1,t2) if (buildingHasWalkerOfType(buildingId, t1, t2)) return;

#define WORKER_PERCENTAGE(b) Calc_getPercentage(b->numWorkers, Data_Model_Buildings[b->type].laborers)

#define CREATE_WALKER(t,x,y,d) \
	int walkerId = Walker_create(t, x, y, d);\
	struct Data_Walker *w = &Data_Walkers[walkerId];

static void generateLaborSeeker(int buildingId, struct Data_Building *b, int x, int y)
{
	if (Data_CityInfo.population <= 0) {
		return;
	}
	if (b->walkerId2) {
		struct Data_Walker *w = &Data_Walkers[b->walkerId2];
		if (!w->state || w->type != Walker_LaborSeeker || w->buildingId != buildingId) {
			b->walkerId2 = 0;
		}
	} else {
		CREATE_WALKER(Walker_LaborSeeker, x, y, 0);
		w->actionState = WalkerActionState_125_Roaming;
		w->buildingId = buildingId;
		b->walkerId2 = walkerId;
		WalkerMovement_initRoaming(walkerId);
	}
}

static int buildingHasWalkerOfType(int buildingId, int type1, int type2)
{
	int walkerId = Data_Buildings[buildingId].walkerId;
	if (walkerId <= 0) {
		return 0;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];
	if (w->state && w->buildingId == buildingId && (w->type == type1 || w->type == type2)) {
		return 1;
	} else {
		Data_Buildings[buildingId].walkerId = 0;
		return 0;
	}
}

static void spawnWalkerPatrician(int buildingId, struct Data_Building *b, int *patricianSpawned)
{
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > 40 && !*patricianSpawned) {
			*patricianSpawned = 1;
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Patrician, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerWarehouse(int buildingId, struct Data_Building *b)
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
		EXIT_IF_WALKER(Walker_Warehouseman);
		int resource;
		int task = Resource_determineWarehouseWorkerTask(buildingId, &resource);
		if (task >= 0) {
			CREATE_WALKER(Walker_Warehouseman, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_50_WarehousemanCreated;
			w->resourceId = task;
			if (task == StorageWalkerTask_Getting) {
				w->collectingItemId = resource;
			}
			b->walkerId = walkerId;
			w->buildingId = buildingId;
		}
	}
}

static void spawnWalkerGranary(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccessGranary(b->x, b->y, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
		EXIT_IF_WALKER(Walker_Warehouseman);
		int task = Resource_determineGranaryWorkerTask(buildingId);
		if (task >= 0) {
			CREATE_WALKER(Walker_Warehouseman, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_50_WarehousemanCreated;
			w->resourceId = task;
			b->walkerId = walkerId;
			w->buildingId = buildingId;
		}
	}
}

static void spawnWalkerTower(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		if (b->numWorkers <= 0) {
			return;
		}
		if (!b->walkerId4 && b->walkerId) { // has sentry but no ballista -> create
			CREATE_WALKER(Walker_Ballista, b->x, b->y, 0);
			b->walkerId4 = walkerId;
			w->buildingId = buildingId;
			w->actionState = WalkerActionState_180_BallistaCreated;
		}
		buildingHasWalkerOfType(buildingId, Walker_TowerSentry, 0);
		if (b->walkerId <= 0) {
			Data_Buildings_Extra.barracksTowerSentryRequested = 2;
		}
	}
}

static void spawnWalkerEngineersPost(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Engineer);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Engineer, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_60_EngineerCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerPrefecture(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Prefect);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Prefect, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_70_PrefectCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerActorColony(int buildingId, struct Data_Building *b)
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Actor, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_90_EntertainerAtSchoolCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerGladiatorSchool(int buildingId, struct Data_Building *b)
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Gladiator, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_90_EntertainerAtSchoolCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerLionHouse(int buildingId, struct Data_Building *b)
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_LionTamer, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_90_EntertainerAtSchoolCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerChariotMaker(int buildingId, struct Data_Building *b)
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Charioteer, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_90_EntertainerAtSchoolCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerAmphitheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKERS(Walker_Actor, Walker_Gladiator);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			int walkerId;
			if (b->data.entertainment.days1 > 0) {
				walkerId = Walker_create(Walker_Gladiator, xRoad, yRoad, 0);
			} else {
				walkerId = Walker_create(Walker_Actor, xRoad, yRoad, 0);
			}
			struct Data_Walker *w = &Data_Walkers[walkerId];
			w->actionState = WalkerActionState_94_EntertainerRoaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerTheater(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Actor);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Actor, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_94_EntertainerRoaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerHippodrome(int buildingId, struct Data_Building *b)
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
	EXIT_IF_WALKER(Walker_Charioteer);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			{
				CREATE_WALKER(Walker_Charioteer, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_94_EntertainerRoaming;
				w->buildingId = buildingId;
				b->walkerId = walkerId;
				WalkerMovement_initRoaming(walkerId);
			}
			if (!Data_CityInfo.entertainmentHippodromeHasShow) {
				// create mini-horses
				{
					CREATE_WALKER(Walker_HippodromeMiniHorses, b->x + 2, b->y + 1, 2);
					w->actionState = WalkerActionState_200_HippodromeMiniHorseCreated;
					w->buildingId = buildingId;
					w->resourceId = 0;
					w->speedMultiplier = 3;
				}
				{
					CREATE_WALKER(Walker_HippodromeMiniHorses, b->x + 2, b->y + 2, 2);
					w->actionState = WalkerActionState_200_HippodromeMiniHorseCreated;
					w->buildingId = buildingId;
					w->resourceId = 1;
					w->speedMultiplier = 2;
				}
				if (b->data.entertainment.days1 > 0) {
					if (!Data_CityInfo.messageShownHippodrome) {
						Data_CityInfo.messageShownHippodrome = 1;
						PlayerMessage_post(1, 109, 0, 0);
					}
				}
			}
		}
	}
}

static void spawnWalkerColosseum(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKERS(Walker_Gladiator, Walker_LionTamer);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			int walkerId;
			if (b->data.entertainment.days1 > 0) {
				walkerId = Walker_create(Walker_LionTamer, xRoad, yRoad, 0);
			} else {
				walkerId = Walker_create(Walker_Gladiator, xRoad, yRoad, 0);
			}
			struct Data_Walker *w = &Data_Walkers[walkerId];
			w->actionState = WalkerActionState_94_EntertainerRoaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
			if (b->data.entertainment.days1 > 0 || b->data.entertainment.days2 > 0) {
				if (!Data_CityInfo.messageShownColosseum) {
					Data_CityInfo.messageShownColosseum = 1;
					PlayerMessage_post(1, 110, 0, 0);
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

static void spawnWalkerMarket(int buildingId, struct Data_Building *b)
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
		if (!buildingHasWalkerOfType(buildingId, Walker_MarketTrader, 0)) {
			b->walkerSpawnDelay++;
			if (b->walkerSpawnDelay <= spawnDelay) {
				return;
			}
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_MarketTrader, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
		// market buyer or labor seeker
		if (b->walkerId2) {
			struct Data_Walker *w = &Data_Walkers[b->walkerId2];
			if (w->state != WalkerState_Alive || (w->type != Walker_MarketBuyer && w->type != Walker_LaborSeeker)) {
				b->walkerId2 = 0;
			}
		} else {
			Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad);
			int dstBuildingId = Building_Market_getDestinationGranaryWarehouse(buildingId);
			if (dstBuildingId > 0) {
				CREATE_WALKER(Walker_MarketBuyer, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_145_MarketBuyerGoingToStorage;
				w->buildingId = buildingId;
				b->walkerId2 = walkerId;
				w->destinationBuildingId = dstBuildingId;
				w->collectingItemId = b->data.market.fetchInventoryId;
				struct Data_Building *bDst = &Data_Buildings[dstBuildingId];
				if (Terrain_hasRoadAccess(bDst->x, bDst->y, bDst->size, &xRoad, &yRoad) ||
					Terrain_hasRoadAccess(bDst->x, bDst->y, 3, &xRoad, &yRoad)) {
					w->destinationX = xRoad;
					w->destinationY = yRoad;
				} else {
					w->actionState = WalkerActionState_146_MarketBuyerReturning;
					w->destinationX = w->x;
					w->destinationY = w->y;
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

static void spawnWalkerBathhouse(int buildingId, struct Data_Building *b)
{
	setBathhouseGraphic(buildingId, b);
	SET_LABOR_PROBLEM(b);
	if (!b->hasWaterAccess) {
		b->showOnProblemOverlay = 2;
	}
	EXIT_IF_WALKER(Walker_BathhouseWorker);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_BathhouseWorker, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerSchool(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_SchoolChild);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			{
				CREATE_WALKER(Walker_SchoolChild, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_125_Roaming;
				w->buildingId = buildingId;
				b->walkerId = walkerId;
				WalkerMovement_initRoaming(walkerId);
			}
			{
				CREATE_WALKER(Walker_SchoolChild, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_125_Roaming;
				w->buildingId = buildingId;
				WalkerMovement_initRoaming(walkerId);
			}
			{
				CREATE_WALKER(Walker_SchoolChild, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_125_Roaming;
				w->buildingId = buildingId;
				WalkerMovement_initRoaming(walkerId);
			}
			{
				CREATE_WALKER(Walker_SchoolChild, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_125_Roaming;
				w->buildingId = buildingId;
				WalkerMovement_initRoaming(walkerId);
			}
		}
	}
}

static void spawnWalkerLibrary(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Librarian);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Librarian, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerAcademy(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Teacher);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Teacher, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerBarber(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Barber);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Barber, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerDoctor(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Doctor);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Doctor, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerHospital(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Surgeon);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Surgeon, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
		}
	}
}

static void spawnWalkerTemple(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_Priest);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_Priest, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_125_Roaming;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			WalkerMovement_initRoaming(walkerId);
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

static void spawnWalkerSenateForum(int buildingId, struct Data_Building *b)
{
	if (b->type == Building_SenateUpgraded) {
		setSenateGraphic(buildingId, b);
	}
	SET_LABOR_PROBLEM(b);
	EXIT_IF_WALKER(Walker_TaxCollector);
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_TaxCollector, xRoad, yRoad, 0);
			w->actionState = WalkerActionState_40_TaxCollectorCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerMissionPost(int buildingId, struct Data_Building *b)
{
	EXIT_IF_WALKER(Walker_Missionary);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		if (Data_CityInfo.population > 0) {
			Data_CityInfo.nativeMissionPostOperational = 1;
			b->walkerSpawnDelay++;
			if (b->walkerSpawnDelay > 1) {
				b->walkerSpawnDelay = 0;
				CREATE_WALKER(Walker_Missionary, xRoad, yRoad, 0);
				w->actionState = WalkerActionState_125_Roaming;
				w->buildingId = buildingId;
				b->walkerId = walkerId;
				WalkerMovement_initRoaming(walkerId);
			}
		}
	}
}

static void spawnWalkerIndustry(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_WALKER(Walker_CartPusher);
		if (Building_Industry_hasProducedResource(buildingId)) {
			Building_Industry_startNewProduction(buildingId);
			CREATE_WALKER(Walker_CartPusher, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_20_CartpusherInitial;
			w->resourceId = b->outputResourceId;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			w->waitTicks = 30;
		}
	}
}

static void spawnWalkerWharf(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	if (b->data.other.boatWalkerId) {
		struct Data_Walker *w = &Data_Walkers[b->data.other.boatWalkerId];
		if (w->state != WalkerState_Alive || w->type != Walker_FishingBoat) {
			b->data.other.boatWalkerId = 0;
		}
	}
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_WALKER(Walker_CartPusher);
		if (b->walkerSpawnDelay) {
			b->walkerSpawnDelay = 0;
			b->data.other.fishingBoatHasFish = 0;
			b->outputResourceId = Resource_Meat;
			CREATE_WALKER(Walker_CartPusher, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_20_CartpusherInitial;
			w->resourceId = Resource_Meat;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
			w->waitTicks = 30;
		}
	}
}

static void spawnWalkerShipyard(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(50);
		EXIT_IF_WALKER(Walker_FishingBoat);
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
				CREATE_WALKER(Walker_FishingBoat, xBoat, yBoat, 0);
				w->actionState = WalkerActionState_190_FishingBoatCreated;
				w->buildingId = buildingId;
				b->walkerId = walkerId;
			}
		}
	}
}

static void spawnWalkerDock(int buildingId, struct Data_Building *b)
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
			if (b->data.other.dockWalkerIds[i]) {
				if (Data_Walkers[b->data.other.dockWalkerIds[i]].type == Walker_Dockman) {
					existingDockers++;
				} else {
					b->data.other.dockWalkerIds[i] = 0;
				}
			}
		}
		if (existingDockers > maxDockers) {
			// too many dockers, kill one of them
			for (int i = 2; i >= 0; i--) {
				if (b->data.other.dockWalkerIds[i]) {
					Data_Walkers[b->data.other.dockWalkerIds[i]].state = WalkerState_Dead;
					break;
				}
			}
		} else if (existingDockers < maxDockers) {
			CREATE_WALKER(Walker_Dockman, xRoad, yRoad, 4);
			w->actionState = WalkerActionState_132_DockerIdling;
			w->buildingId = buildingId;
			for (int i = 0; i < 3; i++) {
				if (!b->data.other.dockWalkerIds[i]) {
					b->data.other.dockWalkerIds[i] = walkerId;
					break;
				}
			}
		}
	}
}

static void spawnWalkerNativeHut(int buildingId, struct Data_Building *b)
{
	Data_Grid_graphicIds[b->gridOffset] =
		GraphicId(ID_Graphic_NativeBuilding) + (Data_Grid_random[b->gridOffset] & 1);
	EXIT_IF_WALKER(Walker_IndigenousNative);
	int xOut, yOut;
	if (b->subtype.nativeMeetingCenterId > 0 && Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > 4) {
			b->walkerSpawnDelay = 0;
			CREATE_WALKER(Walker_IndigenousNative, xOut, yOut, 0);
			w->actionState = WalkerActionState_158_NativeCreated;
			w->buildingId = buildingId;
			b->walkerId = walkerId;
		}
	}
}

static void spawnWalkerNativeMeeting(int buildingId, struct Data_Building *b)
{
	Terrain_addBuildingToGrids(buildingId, b->x, b->y, 2,
		GraphicId(ID_Graphic_NativeBuilding) + 2, Terrain_Building);
	if (Data_CityInfo.nativeMissionPostOperational > 0 &&
		!buildingHasWalkerOfType(buildingId, Walker_NativeTrader, 0)) {
		int xOut, yOut;
		if (Terrain_getAdjacentRoadOrClearLand(b->x, b->y, b->size, &xOut, &yOut)) {
			b->walkerSpawnDelay++;
			if (b->walkerSpawnDelay > 8) {
				b->walkerSpawnDelay = 0;
				CREATE_WALKER(Walker_NativeTrader, xOut, yOut, 0);
				w->actionState = WalkerActionState_162_NativeTraderCreated;
				w->buildingId = buildingId;
				b->walkerId = walkerId;
			}
		}
	}
}

static void spawnWalkerBarracks(int buildingId, struct Data_Building *b)
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
		b->walkerSpawnDelay++;
		if (b->walkerSpawnDelay > spawnDelay) {
			b->walkerSpawnDelay = 0;
			Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad);
			if (!Walker_createTowerSentryFromBarracks(buildingId, xRoad, yRoad)) {
				Walker_createSoldierFromBarracks(buildingId, xRoad, yRoad);
			}
		}
	}
}

static void spawnWalkerMilitaryAcademy(int buildingId, struct Data_Building *b)
{
	SET_LABOR_PROBLEM(b);
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(b->x, b->y, b->size, &xRoad, &yRoad)) {
		SPAWN_LABOR_SEEKER(100);
	}
}

void WalkerGeneration_generateWalkersForBuildings()
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
			spawnWalkerPatrician(i, b, &patricianGenerated);
		} else if (b->type >= Building_WheatFarm && b->type <= Building_PotteryWorkshop) {
			spawnWalkerIndustry(i, b);
		} else if (b->type >= Building_Senate && b->type <= Building_ForumUpgraded) {
			spawnWalkerSenateForum(i, b);
		} else if (b->type >= Building_SmallTempleCeres && b->type <= Building_LargeTempleVenus) {
			spawnWalkerTemple(i, b);
		} else {
			// single building type
			switch (b->type) {
				case Building_Warehouse:
					spawnWalkerWarehouse(i, b);
					break;
				case Building_Granary:
					spawnWalkerGranary(i, b);
					break;
				case Building_Tower:
					spawnWalkerTower(i, b);
					break;
				case Building_EngineersPost:
					spawnWalkerEngineersPost(i, b);
					break;
				case Building_Prefecture:
					spawnWalkerPrefecture(i, b);
					break;
				case Building_ActorColony:
					spawnWalkerActorColony(i, b);
					break;
				case Building_GladiatorSchool:
					spawnWalkerGladiatorSchool(i, b);
					break;
				case Building_LionHouse:
					spawnWalkerLionHouse(i, b);
					break;
				case Building_ChariotMaker:
					spawnWalkerChariotMaker(i, b);
					break;
				case Building_Amphitheater:
					spawnWalkerAmphitheater(i, b);
					break;
				case Building_Theater:
					spawnWalkerTheater(i, b);
					break;
				case Building_Hippodrome:
					spawnWalkerHippodrome(i, b);
					break;
				case Building_Colosseum:
					spawnWalkerColosseum(i, b);
					break;
				case Building_Market:
					spawnWalkerMarket(i, b);
					break;
				case Building_Bathhouse:
					spawnWalkerBathhouse(i, b);
					break;
				case Building_School:
					spawnWalkerSchool(i, b);
					break;
				case Building_Library:
					spawnWalkerLibrary(i, b);
					break;
				case Building_Academy:
					spawnWalkerAcademy(i, b);
					break;
				case Building_Barber:
					spawnWalkerBarber(i, b);
					break;
				case Building_Doctor:
					spawnWalkerDoctor(i, b);
					break;
				case Building_Hospital:
					spawnWalkerHospital(i, b);
					break;
				case Building_MissionPost:
					spawnWalkerMissionPost(i, b);
					break;
				case Building_Dock:
					spawnWalkerDock(i, b);
					break;
				case Building_Wharf:
					spawnWalkerWharf(i, b);
					break;
				case Building_Shipyard:
					spawnWalkerShipyard(i, b);
					break;
				case Building_NativeHut:
					spawnWalkerNativeHut(i, b);
					break;
				case Building_NativeMeeting:
					spawnWalkerNativeMeeting(i, b);
					break;
				case Building_NativeCrops:
					TerrainGraphics_updateNativeCropProgress(i);
					break;
				case Building_FortGround__:
					Formation_setNewSoldierRequest(i);
					break;
				case Building_Barracks:
					spawnWalkerBarracks(i, b);
					break;
				case Building_MilitaryAcademy:
					spawnWalkerMilitaryAcademy(i, b);
					break;
			}
		}
	}
}

