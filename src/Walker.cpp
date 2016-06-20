#include "Figure.h"

#include "Calc.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Random.h"
#include "Sound.h"
#include "Terrain.h"
#include "Trader.h"
#include "WalkerAction.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Figure.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"

#include <string.h>

void Figure_clearList()
{
	for (int i = 0; i < MAX_FIGURES; i++) {
		memset(&Data_Walkers[i], 0, sizeof(struct Data_Walker));
	}
	Data_Figure_Extra.highestFigureIdEver = 0;
}

int Figure_create(int walkerType, int x, int y, char direction)
{
	int id = 0;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (!Data_Walkers[i].state) {
			id = i;
			break;
		}
	}
	if (!id) {
		return 0;
	}
	struct Data_Walker *w = &Data_Walkers[id];
	w->state = FigureState_Alive;
	w->ciid = 1;
	w->type = walkerType;
	w->useCrossCountry = 0;
	w->isFriendly = 1;
	w->createdSequence = Data_Figure_Extra.createdSequence++;
	w->direction = direction;
	w->sourceX = w->destinationX = w->previousTileX = w->x = x;
	w->sourceY = w->destinationY = w->previousTileY = w->y = y;
	w->gridOffset = GridOffset(x, y);
	w->crossCountryX = 15 * x;
	w->crossCountryY = 15 * y;
	w->progressOnTile = 15;
	w->phraseSequenceCity = w->phraseSequenceExact = Data_Random.random1_7bit & 3;
	FigureName_set(id);
	Figure_addToTileList(id);
	if (walkerType == Figure_TradeCaravan || walkerType == Figure_TradeShip) {
		Trader_create(id);
	}
	if (id > Data_Figure_Extra.highestFigureIdEver) {
		Data_Figure_Extra.highestFigureIdEver = id;
	}
	return id;
}

void Figure_delete(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	switch (w->type) {
		case Figure_LaborSeeker:
		case Figure_MarketBuyer:
			if (w->buildingId) {
				Data_Buildings[w->buildingId].walkerId2 = 0;
			}
			break;
		case Figure_Ballista:
			Data_Buildings[w->buildingId].walkerId4 = 0;
			break;
		case Figure_Dockman:
			for (int i = 0; i < 3; i++) {
				if (Data_Buildings[w->buildingId].data.other.dockWalkerIds[i] == walkerId) {
					Data_Buildings[w->buildingId].data.other.dockWalkerIds[i] = 0;
				}
			}
			break;
		case Figure_EnemyCaesarLegionary:
			Data_CityInfo.caesarInvasionSoldiersDied++;
			break;
		case Figure_Explosion:
		case Figure_FortStandard:
		case Figure_Arrow:
		case Figure_Javelin:
		case Figure_Bolt:
		case Figure_Spear:
		case Figure_FishGulls:
		case Figure_Sheep:
		case Figure_Wolf:
		case Figure_Zebra:
		case Figure_DeliveryBoy:
		case Figure_Patrician:
			// nothing to do here
			break;
		default:
			if (w->buildingId) {
				Data_Buildings[w->buildingId].walkerId = 0;
			}
			break;
	}
	if (w->empireCityId) {
		for (int i = 0; i < 3; i++) {
			if (Data_Empire_Cities[w->empireCityId].traderWalkerIds[i] == walkerId) {
				Data_Empire_Cities[w->empireCityId].traderWalkerIds[i] = 0;
			}
		}
	}
	if (w->immigrantBuildingId) {
		Data_Buildings[w->buildingId].immigrantWalkerId = 0;
	}
	FigureRoute_remove(walkerId);
	Figure_removeFromTileList(walkerId);
	memset(w, 0, sizeof(struct Data_Walker));
}

void Figure_addToTileList(int walkerId)
{
	if (Data_Walkers[walkerId].gridOffset < 0) {
		return;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->numPreviousWalkersOnSameTile = 0;

	int next = Data_Grid_figureIds[w->gridOffset];
	if (next) {
		w->numPreviousWalkersOnSameTile++;
		while (Data_Walkers[next].nextWalkerIdOnSameTile) {
			next = Data_Walkers[next].nextWalkerIdOnSameTile;
			w->numPreviousWalkersOnSameTile++;
		}
		if (w->numPreviousWalkersOnSameTile > 20) {
			w->numPreviousWalkersOnSameTile = 20;
		}
		Data_Walkers[next].nextWalkerIdOnSameTile = walkerId;
	} else {
		Data_Grid_figureIds[w->gridOffset] = walkerId;
	}
}

void Figure_updatePositionInTileList(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->numPreviousWalkersOnSameTile = 0;
	
	int next = Data_Grid_figureIds[w->gridOffset];
	while (next) {
		if (next == walkerId) {
			return;
		}
		w->numPreviousWalkersOnSameTile++;
		next = Data_Walkers[next].nextWalkerIdOnSameTile;
	}
	if (w->numPreviousWalkersOnSameTile > 20) {
		w->numPreviousWalkersOnSameTile = 20;
	}
}

void Figure_removeFromTileList(int walkerId)
{
	if (Data_Walkers[walkerId].gridOffset < 0) {
		return;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];

	int cur = Data_Grid_figureIds[w->gridOffset];
	if (cur) {
		if (cur == walkerId) {
			Data_Grid_figureIds[w->gridOffset] = w->nextWalkerIdOnSameTile;
		} else {
			while (cur && Data_Walkers[cur].nextWalkerIdOnSameTile != walkerId) {
				cur = Data_Walkers[cur].nextWalkerIdOnSameTile;
			}
			Data_Walkers[cur].nextWalkerIdOnSameTile = w->nextWalkerIdOnSameTile;
		}
		w->nextWalkerIdOnSameTile = 0;
	}
}

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
		int walkerId = Figure_create(Figure_Explosion,
			x + tileOffset, y + tileOffset, 0);
		if (walkerId) {
			struct Data_Walker *w = &Data_Walkers[walkerId];
			w->crossCountryX += ccOffset;
			w->crossCountryY += ccOffset;
			w->destinationX += dustCloudDirectionX[i];
			w->destinationY += dustCloudDirectionY[i];
			FigureMovement_crossCountrySetDirection(walkerId,
				w->crossCountryX, w->crossCountryY,
				15 * w->destinationX + ccOffset,
				15 * w->destinationY + ccOffset, 0);
			w->speedMultiplier = dustCloudSpeed[i];
		}
	}
	Sound_Effects_playChannel(SoundChannel_Explosion);
}

int Figure_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type)
{
	int walkerId = Figure_create(type, x, y, 0);
	if (walkerId) {
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->missileDamage = (type == Figure_Bolt) ? 60 : 10;
		w->buildingId = buildingId;
		w->destinationX = xDst;
		w->destinationY = yDst;
		FigureMovement_crossCountrySetDirection(
			walkerId, w->crossCountryX, w->crossCountryY,
			15 * xDst, 15 * yDst, 1);
	}
	return walkerId;
}

void Figure_createFishingPoints()
{
	for (int i = 0; i < 8; i++) {
		if (Data_Scenario.fishingPoints.x[i] > 0) {
			Random_generateNext();
			int fishId = Figure_create(Figure_FishGulls,
				Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i], 0);
			Data_Walkers[fishId].graphicOffset = Data_Random.random1_7bit & 0x1f;
			Data_Walkers[fishId].progressOnTile = Data_Random.random1_7bit & 7;
			FigureMovement_crossCountrySetDirection(fishId,
				Data_Walkers[fishId].crossCountryX, Data_Walkers[fishId].crossCountryY,
				15 * Data_Walkers[fishId].destinationX, 15 * Data_Walkers[fishId].destinationY, 0);
		}
	}
}

void Figure_createHerds()
{
	int herdType, numAnimals;
	switch (Data_Scenario.climate) {
		case Climate_Central: herdType = Figure_Sheep; numAnimals = 10; break;
		case Climate_Northern: herdType = Figure_Wolf; numAnimals = 8; break;
		case Climate_Desert: herdType = Figure_Zebra; numAnimals = 12; break;
		default: return;
	}
	for (int i = 0; i < 4; i++) {
		if (Data_Scenario.herdPoints.x[i] > 0) {
			int formationId = Formation_create(herdType, FormationLayout_Herd, 0,
				Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i]);
			if (formationId > 0) {
				Data_Formations[formationId].isHerd = 1;
				Data_Formations[formationId].waitTicks = 24;
				Data_Formations[formationId].maxFigures = numAnimals;
				for (int w = 0; w < numAnimals; w++) {
					Random_generateNext();
					int walkerId = Figure_create(herdType,
						Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i], 0);
					Data_Walkers[walkerId].actionState = FigureActionState_196_HerdAnimalAtRest;
					Data_Walkers[walkerId].formationId = formationId;
					Data_Walkers[walkerId].waitTicks = walkerId & 0x1f;
				}
			}
		}
	}
}

void Figure_createFlotsam(int xEntry, int yEntry, int hasWater)
{
	if (!hasWater || !Data_Scenario.flotsamEnabled) {
		return;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (Data_Walkers[i].state && Data_Walkers[i].type == Figure_Flotsam) {
			Figure_delete(i);
		}
	}
	const int resourceIds[] = {3, 1, 3, 2, 1, 3, 2, 3, 2, 1, 3, 3, 2, 3, 3, 3, 1, 2, 0, 1};
	const int waitTicks[] = {10, 50, 100, 130, 200, 250, 400, 430, 500, 600, 70, 750, 820, 830, 900, 980, 1010, 1030, 1200, 1300};
	for (int i = 0; i < 20; i++) {
		int walkerId = Figure_create(Figure_Flotsam, xEntry, yEntry, 0);
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->actionState = FigureActionState_128_FlotsamCreated;
		w->resourceId = resourceIds[i];
		w->waitTicks = waitTicks[i];
	}
}

int Figure_createSoldierFromBarracks(int buildingId, int x, int y)
{
	int noWeapons = Data_Buildings[buildingId].loadsStored <= 0;
	int recruitType = 0;
	int formationId = 0;
	int minDist = 10000;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || !f->isLegion || f->inDistantBattle || !f->legionRecruitType) {
			continue;
		}
		if (f->legionRecruitType == 3 && noWeapons) {
			continue;
		}
		int dist = Calc_distanceMaximum(
			Data_Buildings[buildingId].x, Data_Buildings[buildingId].y,
			Data_Buildings[f->buildingId].x, Data_Buildings[f->buildingId].y);
		if (f->legionRecruitType > recruitType) {
			recruitType = f->legionRecruitType;
			formationId = i;
			minDist = dist;
		} else if (f->legionRecruitType == recruitType && dist < minDist) {
			recruitType = f->legionRecruitType;
			formationId = i;
			minDist = dist;
		}
	}
	if (formationId > 0) {
		struct Data_Formation *f = &Data_Formations[formationId];
		int walkerId = Figure_create(f->figureType, x, y, 0);
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->formationId = formationId;
		w->formationAtRest = 1;
		if (f->figureType == Figure_FortLegionary) {
			if (Data_Buildings[buildingId].loadsStored > 0) {
				Data_Buildings[buildingId].loadsStored--;
			}
		}
		int academyId = Formation_getClosestMilitaryAcademy(formationId);
		if (academyId) {
			int xRoad, yRoad;
			if (Terrain_hasRoadAccess(Data_Buildings[academyId].x,
				Data_Buildings[academyId].y, Data_Buildings[academyId].size, &xRoad, &yRoad)) {
				w->actionState = FigureActionState_85_SoldierGoingToMilitaryAcademy;
				w->destinationX = xRoad;
				w->destinationY = yRoad;
				w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			} else {
				w->actionState = FigureActionState_81_SoldierGoingToFort;
			}
		} else {
			w->actionState = FigureActionState_81_SoldierGoingToFort;
		}
	}
	Formation_calculateWalkers();
	return formationId ? 1 : 0;
}

int Figure_createTowerSentryFromBarracks(int buildingId, int x, int y)
{
	if (Data_Buildings_Extra.barracksTowerSentryRequested <= 0) {
		return 0;
	}
	int towerId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->type == Building_Tower && b->numWorkers > 0 &&
			!b->walkerId && b->roadNetworkId == Data_Buildings[buildingId].roadNetworkId) {
			towerId = i;
			break;
		}
	}
	if (!towerId) {
		return 0;
	}
	struct Data_Building *tower = &Data_Buildings[towerId];
	int walkerId = Figure_create(Figure_TowerSentry, x, y, 0);
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->actionState = FigureActionState_174_TowerSentryGoingToTower;
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(tower->x, tower->y, tower->size, &xRoad, &yRoad)) {
		w->destinationX = xRoad;
		w->destinationY = yRoad;
	} else {
		w->state = FigureState_Dead;
	}
	tower->walkerId = walkerId;
	w->buildingId = towerId;
	return 1;
}

void Figure_killTowerSentriesAt(int x, int y)
{
	for (int i = 0; i < MAX_FIGURES; i++) {
		if (!FigureIsDead(i) && Data_Walkers[i].type == Figure_TowerSentry) {
			if (Calc_distanceMaximum(Data_Walkers[i].x, Data_Walkers[i].y, x, y) <= 1) {
				Data_Walkers[i].state = FigureState_Dead;
			}
		}
	}
}

void Figure_sinkAllShips()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != FigureState_Alive) {
			continue;
		}
		int buildingId;
		if (w->type == Figure_TradeShip) {
			buildingId = w->destinationBuildingId;
		} else if (w->type == Figure_FishingBoat) {
			buildingId = w->buildingId;
		} else {
			continue;
		}
		Data_Buildings[buildingId].data.other.boatWalkerId = 0;
		w->buildingId = 0;
		w->type = Figure_Shipwreck;
		w->waitTicks = 0;
	}
}

int Figure_getCitizenOnSameTile(int walkerId)
{
	for (int w = Data_Grid_figureIds[Data_Walkers[walkerId].gridOffset];
		w > 0; w = Data_Walkers[w].nextWalkerIdOnSameTile) {
		if (Data_Walkers[w].actionState != FigureActionState_149_Corpse) {
			int type = Data_Walkers[w].type;
			if (type && type != Figure_Explosion && type != Figure_FortStandard &&
				type != Figure_MapFlag && type != Figure_Flotsam && type < Figure_IndigenousNative) {
				return w;
			}
		}
	}
	return 0;
}

int Figure_getNonCitizenOnSameTile(int walkerId)
{
	for (int w = Data_Grid_figureIds[Data_Walkers[walkerId].gridOffset];
		w > 0; w = Data_Walkers[w].nextWalkerIdOnSameTile) {
		if (Data_Walkers[w].actionState != FigureActionState_149_Corpse) {
			int type = Data_Walkers[w].type;
			if (WalkerIsEnemy(type)) {
				return w;
			}
			if (type == Figure_IndigenousNative && Data_Walkers[w].actionState == FigureActionState_159_NativeAttacking) {
				return w;
			}
			if (type == Figure_Wolf || type == Figure_Sheep || type == Figure_Zebra) {
				return w;
			}
		}
	}
	return 0;
}

int Figure_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd)
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != FigureState_Alive || !WalkerIsEnemy(w->type)) {
			continue;
		}
		int dx = (w->x > xStart) ? (w->x - xStart) : (xStart - w->x);
		int dy = (w->y > yStart) ? (w->y - yStart) : (yStart - w->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
		dx = (w->x > xEnd) ? (w->x - xEnd) : (xEnd - w->x);
		dy = (w->y > yEnd) ? (w->y - yEnd) : (yEnd - w->y);
		if (dx <= 12 && dy <= 12) {
			return 1;
		}
	}
	return 0;
}
