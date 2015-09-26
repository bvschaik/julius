#include "Walker.h"

#include "Calc.h"
#include "Formation.h"
#include "Random.h"
#include "Sound.h"
#include "Terrain.h"
#include "Trader.h"
#include "WalkerAction.h"
#include "WalkerMovement.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

void Walker_clearList()
{
	for (int i = 0; i < MAX_WALKERS; i++) {
		memset(&Data_Walkers[i], 0, sizeof(struct Data_Walker));
	}
	Data_Walker_Extra.highestWalkerIdEver = 0;
}

int Walker_create(int walkerType, int x, int y, char direction)
{
	int id = 0;
	for (int i = 1; i < MAX_WALKERS; i++) {
		if (!Data_Walkers[i].state) {
			id = i;
			break;
		}
	}
	if (!id) {
		return 0;
	}
	struct Data_Walker *w = &Data_Walkers[id];
	w->state = WalkerState_Alive;
	w->ciid = 1;
	w->type = walkerType;
	w->useCrossCountry = 0;
	w->isFriendly = 1;
	w->createdSequence = Data_Walker_Extra.createdSequence++;
	w->direction = direction;
	w->sourceX = w->destinationX = w->previousTileX = w->x = x;
	w->sourceY = w->destinationY = w->previousTileY = w->y = y;
	w->gridOffset = GridOffset(x, y);
	w->crossCountryX = 15 * x;
	w->crossCountryY = 15 * y;
	w->progressOnTile = 15;
	w->phraseSequenceCity = w->phraseSequenceExact = Data_Random.random1_7bit & 3;
	WalkerName_set(id);
	Walker_addToTileList(id);
	if (walkerType == Walker_TradeCaravan || walkerType == Walker_TradeShip) {
		Trader_create(id);
	}
	if (id > Data_Walker_Extra.highestWalkerIdEver) {
		Data_Walker_Extra.highestWalkerIdEver = id;
	}
	return id;
}

void Walker_delete(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	switch (w->type) {
		case Walker_LaborSeeker:
		case Walker_MarketBuyer:
			if (w->buildingId) {
				Data_Buildings[w->buildingId].walkerId2 = 0;
			}
			break;
		case Walker_Ballista:
			Data_Buildings[w->buildingId].walkerId4 = 0;
			break;
		case Walker_Dockman:
			for (int i = 0; i < 3; i++) {
				if (Data_Buildings[w->buildingId].data.other.dockWalkerIds[i] == walkerId) {
					Data_Buildings[w->buildingId].data.other.dockWalkerIds[i] = 0;
				}
			}
			break;
		case Walker_EnemyCaesarLegionary:
			Data_CityInfo.caesarInvasionSoldiersDied++;
			break;
		case Walker_Explosion:
		case Walker_FortStandard:
		case Walker_Arrow:
		case Walker_Javelin:
		case Walker_Bolt:
		case Walker_Spear:
		case Walker_FishGulls:
		case Walker_Sheep:
		case Walker_Wolf:
		case Walker_Zebra:
		case Walker_DeliveryBoy:
		case Walker_Patrician:
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
	WalkerRoute_remove(walkerId);
	Walker_removeFromTileList(walkerId);
	memset(w, 0, sizeof(struct Data_Walker));
}

void Walker_addToTileList(int walkerId)
{
	if (Data_Walkers[walkerId].gridOffset < 0) {
		return;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->numPreviousWalkersOnSameTile = 0;

	int next = Data_Grid_walkerIds[w->gridOffset];
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
		Data_Grid_walkerIds[w->gridOffset] = walkerId;
	}
}

void Walker_updatePositionInTileList(int walkerId)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->numPreviousWalkersOnSameTile = 0;
	
	int next = Data_Grid_walkerIds[w->gridOffset];
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

void Walker_removeFromTileList(int walkerId)
{
	if (Data_Walkers[walkerId].gridOffset < 0) {
		return;
	}
	struct Data_Walker *w = &Data_Walkers[walkerId];

	int cur = Data_Grid_walkerIds[w->gridOffset];
	if (cur) {
		if (cur == walkerId) {
			Data_Grid_walkerIds[w->gridOffset] = w->nextWalkerIdOnSameTile;
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
void Walker_createDustCloud(int x, int y, int size)
{
	int tileOffset = dustCloudTileOffsets[size];
	int ccOffset = dustCloudCCOffsets[size];
	for (int i = 0; i < 16; i++) {
		int walkerId = Walker_create(Walker_Explosion,
			x + tileOffset, y + tileOffset, 0);
		if (walkerId) {
			struct Data_Walker *w = &Data_Walkers[walkerId];
			w->crossCountryX += ccOffset;
			w->crossCountryY += ccOffset;
			w->destinationX += dustCloudDirectionX[i];
			w->destinationY += dustCloudDirectionY[i];
			WalkerMovement_crossCountrySetDirection(walkerId,
				w->crossCountryX, w->crossCountryY,
				15 * w->destinationX + ccOffset,
				15 * w->destinationY + ccOffset, 0);
			w->speedMultiplier = dustCloudSpeed[i];
		}
	}
	Sound_Effects_playChannel(SoundChannel_Explosion);
}

int Walker_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type)
{
	int walkerId = Walker_create(type, x, y, 0);
	if (walkerId) {
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->missileDamage = (type == Walker_Bolt) ? 60 : 10;
		w->buildingId = buildingId;
		w->destinationX = xDst;
		w->destinationY = yDst;
		WalkerMovement_crossCountrySetDirection(
			walkerId, w->crossCountryX, w->crossCountryY,
			15 * xDst, 15 * yDst, 1);
	}
	return walkerId;
}

void Walker_createFishingPoints()
{
	for (int i = 0; i < 8; i++) {
		if (Data_Scenario.fishingPoints.x[i] > 0) {
			Random_generateNext();
			int fishId = Walker_create(Walker_FishGulls,
				Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i], 0);
			Data_Walkers[fishId].graphicOffset = Data_Random.random1_7bit & 0x1f;
			Data_Walkers[fishId].progressOnTile = Data_Random.random1_7bit & 7;
			WalkerMovement_crossCountrySetDirection(fishId,
				Data_Walkers[fishId].crossCountryX, Data_Walkers[fishId].crossCountryY,
				15 * Data_Walkers[fishId].destinationX, 15 * Data_Walkers[fishId].destinationY, 0);
		}
	}
}

void Walker_createHerds()
{
	int herdType, numAnimals;
	switch (Data_Scenario.climate) {
		case Climate_Central: herdType = Walker_Sheep; numAnimals = 10; break;
		case Climate_Northern: herdType = Walker_Wolf; numAnimals = 8; break;
		case Climate_Desert: herdType = Walker_Zebra; numAnimals = 12; break;
		default: return;
	}
	for (int i = 0; i < 4; i++) {
		if (Data_Scenario.herdPoints.x[i] > 0) {
			int formationId = Formation_create(herdType, FormationLayout_Herd, 0,
				Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i]);
			if (formationId > 0) {
				Data_Formations[formationId].isHerd = 1;
				Data_Formations[formationId].waitTicks = 24;
				Data_Formations[formationId].maxWalkers = numAnimals;
				for (int w = 0; w < numAnimals; w++) {
					Random_generateNext();
					int walkerId = Walker_create(herdType,
						Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i], 0);
					Data_Walkers[walkerId].actionState = WalkerActionState_196_HerdAnimalAtRest;
					Data_Walkers[walkerId].formationId = formationId;
					Data_Walkers[walkerId].waitTicks = walkerId & 0x1f;
				}
			}
		}
	}
}

void Walker_createFlotsam(int xEntry, int yEntry, int hasWater)
{
	if (!hasWater || !Data_Scenario.flotsamEnabled) {
		return;
	}
	for (int i = 1; i < MAX_WALKERS; i++) {
		if (Data_Walkers[i].state && Data_Walkers[i].type == Walker_Flotsam) {
			Walker_delete(i);
		}
	}
	const int resourceIds[] = {3, 1, 3, 2, 1, 3, 2, 3, 2, 1, 3, 3, 2, 3, 3, 3, 1, 2, 0, 1};
	const int waitTicks[] = {10, 50, 100, 130, 200, 250, 400, 430, 500, 600, 70, 750, 820, 830, 900, 980, 1010, 1030, 1200, 1300};
	for (int i = 0; i < 20; i++) {
		int walkerId = Walker_create(Walker_Flotsam, xEntry, yEntry, 0);
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->actionState = WalkerActionState_128_FlotsamCreated;
		w->resourceId = resourceIds[i];
		w->waitTicks = waitTicks[i];
	}
}

int Walker_createSoldierFromBarracks(int buildingId, int x, int y)
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
		int walkerId = Walker_create(f->walkerType, x, y, 0);
		struct Data_Walker *w = &Data_Walkers[walkerId];
		w->formationId = formationId;
		w->formationAtRest = 1;
		if (f->walkerType == Walker_FortLegionary) {
			if (Data_Buildings[buildingId].loadsStored > 0) {
				Data_Buildings[buildingId].loadsStored--;
			}
		}
		int academyId = Formation_getClosestMilitaryAcademy(formationId);
		if (academyId) {
			int xRoad, yRoad;
			if (Terrain_hasRoadAccess(Data_Buildings[academyId].x,
				Data_Buildings[academyId].y, Data_Buildings[academyId].size, &xRoad, &yRoad)) {
				w->actionState = WalkerActionState_85_SoldierGoingToMilitaryAcademy;
				w->destinationX = xRoad;
				w->destinationY = yRoad;
				w->destinationGridOffsetSoldier = GridOffset(w->destinationX, w->destinationY);
			} else {
				w->actionState = WalkerActionState_81_SoldierGoingToFort;
			}
		} else {
			w->actionState = WalkerActionState_81_SoldierGoingToFort;
		}
	}
	Formation_calculateWalkers();
	return formationId ? 1 : 0;
}

int Walker_createTowerSentryFromBarracks(int buildingId, int x, int y)
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
	int walkerId = Walker_create(Walker_TowerSentry, x, y, 0);
	struct Data_Walker *w = &Data_Walkers[walkerId];
	w->actionState = WalkerActionState_174_TowerSentryGoingToTower;
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(tower->x, tower->y, tower->size, &xRoad, &yRoad)) {
		w->destinationX = xRoad;
		w->destinationY = yRoad;
	} else {
		w->state = WalkerState_Dead;
	}
	tower->walkerId = walkerId;
	w->buildingId = towerId;
	return 1;
}

void Walker_killTowerSentriesAt(int x, int y)
{
	for (int i = 0; i < MAX_WALKERS; i++) {
		if (!WalkerIsDead(i) && Data_Walkers[i].type == Walker_TowerSentry) {
			if (Calc_distanceMaximum(Data_Walkers[i].x, Data_Walkers[i].y, x, y) <= 1) {
				Data_Walkers[i].state = WalkerState_Dead;
			}
		}
	}
}

void Walker_sinkAllShips()
{
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive) {
			continue;
		}
		int buildingId;
		if (w->type == Walker_TradeShip) {
			buildingId = w->destinationBuildingId;
		} else if (w->type == Walker_FishingBoat) {
			buildingId = w->buildingId;
		} else {
			continue;
		}
		Data_Buildings[buildingId].data.other.boatWalkerId = 0;
		w->buildingId = 0;
		w->type = Walker_Shipwreck;
		w->waitTicks = 0;
	}
}

int Walker_getCitizenOnSameTile(int walkerId)
{
	for (int w = Data_Grid_walkerIds[Data_Walkers[walkerId].gridOffset];
		w > 0; w = Data_Walkers[w].nextWalkerIdOnSameTile) {
		if (Data_Walkers[w].actionState != WalkerActionState_149_Corpse) {
			int type = Data_Walkers[w].type;
			if (type && type != Walker_Explosion && type != Walker_FortStandard &&
				type != Walker_MapFlag && type != Walker_Flotsam && type < Walker_IndigenousNative) {
				return w;
			}
		}
	}
	return 0;
}

int Walker_getNonCitizenOnSameTile(int walkerId)
{
	for (int w = Data_Grid_walkerIds[Data_Walkers[walkerId].gridOffset];
		w > 0; w = Data_Walkers[w].nextWalkerIdOnSameTile) {
		if (Data_Walkers[w].actionState != WalkerActionState_149_Corpse) {
			int type = Data_Walkers[w].type;
			if (WalkerIsEnemy(type)) {
				return w;
			}
			if (type == Walker_IndigenousNative && Data_Walkers[w].actionState == WalkerActionState_159_NativeAttacking) {
				return w;
			}
			if (type == Walker_Wolf || type == Walker_Sheep || type == Walker_Zebra) {
				return w;
			}
		}
	}
	return 0;
}

int Walker_hasNearbyEnemy(int xStart, int yStart, int xEnd, int yEnd)
{
	for (int i = 1; i < MAX_WALKERS; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != WalkerState_Alive || !WalkerIsEnemy(w->type)) {
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
