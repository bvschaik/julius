#include "Figure.h"

#include "Calc.h"
#include "FigureAction.h"
#include "FigureMovement.h"
#include "Formation.h"
#include "Random.h"
#include "Sound.h"
#include "Terrain.h"
#include "Trader.h"

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
		memset(&Data_Figures[i], 0, sizeof(struct Data_Figure));
	}
	Data_Figure_Extra.highestFigureIdEver = 0;
}

int Figure_create(int figureType, int x, int y, char direction)
{
	int id = 0;
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (!Data_Figures[i].state) {
			id = i;
			break;
		}
	}
	if (!id) {
		return 0;
	}
	struct Data_Figure *f = &Data_Figures[id];
	f->state = FigureState_Alive;
	f->ciid = 1;
	f->type = figureType;
	f->useCrossCountry = 0;
	f->isFriendly = 1;
	f->createdSequence = Data_Figure_Extra.createdSequence++;
	f->direction = direction;
	f->sourceX = f->destinationX = f->previousTileX = f->x = x;
	f->sourceY = f->destinationY = f->previousTileY = f->y = y;
	f->gridOffset = GridOffset(x, y);
	f->crossCountryX = 15 * x;
	f->crossCountryY = 15 * y;
	f->progressOnTile = 15;
	f->phraseSequenceCity = f->phraseSequenceExact = Data_Random.random1_7bit & 3;
	FigureName_set(id);
	Figure_addToTileList(id);
	if (figureType == Figure_TradeCaravan || figureType == Figure_TradeShip) {
		Trader_create(id);
	}
	if (id > Data_Figure_Extra.highestFigureIdEver) {
		Data_Figure_Extra.highestFigureIdEver = id;
	}
	return id;
}

void Figure_delete(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	switch (f->type) {
		case Figure_LaborSeeker:
		case Figure_MarketBuyer:
			if (f->buildingId) {
				Data_Buildings[f->buildingId].figureId2 = 0;
			}
			break;
		case Figure_Ballista:
			Data_Buildings[f->buildingId].figureId4 = 0;
			break;
		case Figure_Dockman:
			for (int i = 0; i < 3; i++) {
				if (Data_Buildings[f->buildingId].data.other.dockFigureIds[i] == figureId) {
					Data_Buildings[f->buildingId].data.other.dockFigureIds[i] = 0;
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
			if (f->buildingId) {
				Data_Buildings[f->buildingId].figureId = 0;
			}
			break;
	}
	if (f->empireCityId) {
		for (int i = 0; i < 3; i++) {
			if (Data_Empire_Cities[f->empireCityId].traderFigureIds[i] == figureId) {
				Data_Empire_Cities[f->empireCityId].traderFigureIds[i] = 0;
			}
		}
	}
	if (f->immigrantBuildingId) {
		Data_Buildings[f->buildingId].immigrantFigureId = 0;
	}
	FigureRoute_remove(figureId);
	Figure_removeFromTileList(figureId);
	memset(f, 0, sizeof(struct Data_Figure));
}

void Figure_addToTileList(int figureId)
{
	if (Data_Figures[figureId].gridOffset < 0) {
		return;
	}
	struct Data_Figure *f = &Data_Figures[figureId];
	f->numPreviousFiguresOnSameTile = 0;

	int next = Data_Grid_figureIds[f->gridOffset];
	if (next) {
		f->numPreviousFiguresOnSameTile++;
		while (Data_Figures[next].nextFigureIdOnSameTile) {
			next = Data_Figures[next].nextFigureIdOnSameTile;
			f->numPreviousFiguresOnSameTile++;
		}
		if (f->numPreviousFiguresOnSameTile > 20) {
			f->numPreviousFiguresOnSameTile = 20;
		}
		Data_Figures[next].nextFigureIdOnSameTile = figureId;
	} else {
		Data_Grid_figureIds[f->gridOffset] = figureId;
	}
}

void Figure_updatePositionInTileList(int figureId)
{
	struct Data_Figure *f = &Data_Figures[figureId];
	f->numPreviousFiguresOnSameTile = 0;
	
	int next = Data_Grid_figureIds[f->gridOffset];
	while (next) {
		if (next == figureId) {
			return;
		}
		f->numPreviousFiguresOnSameTile++;
		next = Data_Figures[next].nextFigureIdOnSameTile;
	}
	if (f->numPreviousFiguresOnSameTile > 20) {
		f->numPreviousFiguresOnSameTile = 20;
	}
}

void Figure_removeFromTileList(int figureId)
{
	if (Data_Figures[figureId].gridOffset < 0) {
		return;
	}
	struct Data_Figure *f = &Data_Figures[figureId];

	int cur = Data_Grid_figureIds[f->gridOffset];
	if (cur) {
		if (cur == figureId) {
			Data_Grid_figureIds[f->gridOffset] = f->nextFigureIdOnSameTile;
		} else {
			while (cur && Data_Figures[cur].nextFigureIdOnSameTile != figureId) {
				cur = Data_Figures[cur].nextFigureIdOnSameTile;
			}
			Data_Figures[cur].nextFigureIdOnSameTile = f->nextFigureIdOnSameTile;
		}
		f->nextFigureIdOnSameTile = 0;
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
		int figureId = Figure_create(Figure_Explosion,
			x + tileOffset, y + tileOffset, 0);
		if (figureId) {
			struct Data_Figure *f = &Data_Figures[figureId];
			f->crossCountryX += ccOffset;
			f->crossCountryY += ccOffset;
			f->destinationX += dustCloudDirectionX[i];
			f->destinationY += dustCloudDirectionY[i];
			FigureMovement_crossCountrySetDirection(figureId,
				f->crossCountryX, f->crossCountryY,
				15 * f->destinationX + ccOffset,
				15 * f->destinationY + ccOffset, 0);
			f->speedMultiplier = dustCloudSpeed[i];
		}
	}
	Sound_Effects_playChannel(SoundChannel_Explosion);
}

int Figure_createMissile(int buildingId, int x, int y, int xDst, int yDst, int type)
{
	int figureId = Figure_create(type, x, y, 0);
	if (figureId) {
		struct Data_Figure *f = &Data_Figures[figureId];
		f->missileDamage = (type == Figure_Bolt) ? 60 : 10;
		f->buildingId = buildingId;
		f->destinationX = xDst;
		f->destinationY = yDst;
		FigureMovement_crossCountrySetDirection(
			figureId, f->crossCountryX, f->crossCountryY,
			15 * xDst, 15 * yDst, 1);
	}
	return figureId;
}

void Figure_createFishingPoints()
{
	for (int i = 0; i < 8; i++) {
		if (Data_Scenario.fishingPoints.x[i] > 0) {
			Random_generateNext();
			int fishId = Figure_create(Figure_FishGulls,
				Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i], 0);
			Data_Figures[fishId].graphicOffset = Data_Random.random1_7bit & 0x1f;
			Data_Figures[fishId].progressOnTile = Data_Random.random1_7bit & 7;
			FigureMovement_crossCountrySetDirection(fishId,
				Data_Figures[fishId].crossCountryX, Data_Figures[fishId].crossCountryY,
				15 * Data_Figures[fishId].destinationX, 15 * Data_Figures[fishId].destinationY, 0);
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
				for (int fig = 0; fig < numAnimals; fig++) {
					Random_generateNext();
					int figureId = Figure_create(herdType,
						Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i], 0);
					Data_Figures[figureId].actionState = FigureActionState_196_HerdAnimalAtRest;
					Data_Figures[figureId].formationId = formationId;
					Data_Figures[figureId].waitTicks = figureId & 0x1f;
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
		if (Data_Figures[i].state && Data_Figures[i].type == Figure_Flotsam) {
			Figure_delete(i);
		}
	}
	const int resourceIds[] = {3, 1, 3, 2, 1, 3, 2, 3, 2, 1, 3, 3, 2, 3, 3, 3, 1, 2, 0, 1};
	const int waitTicks[] = {10, 50, 100, 130, 200, 250, 400, 430, 500, 600, 70, 750, 820, 830, 900, 980, 1010, 1030, 1200, 1300};
	for (int i = 0; i < 20; i++) {
		int figureId = Figure_create(Figure_Flotsam, xEntry, yEntry, 0);
		struct Data_Figure *f = &Data_Figures[figureId];
		f->actionState = FigureActionState_128_FlotsamCreated;
		f->resourceId = resourceIds[i];
		f->waitTicks = waitTicks[i];
	}
}

int Figure_createSoldierFromBarracks(int buildingId, int x, int y)
{
	int noWeapons = Data_Buildings[buildingId].loadsStored <= 0;
	int recruitType = 0;
	int formationId = 0;
	int minDist = 10000;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || !m->isLegion || m->inDistantBattle || !m->legionRecruitType) {
			continue;
		}
		if (m->legionRecruitType == 3 && noWeapons) {
			continue;
		}
		int dist = Calc_distanceMaximum(
			Data_Buildings[buildingId].x, Data_Buildings[buildingId].y,
			Data_Buildings[m->buildingId].x, Data_Buildings[m->buildingId].y);
		if (m->legionRecruitType > recruitType) {
			recruitType = m->legionRecruitType;
			formationId = i;
			minDist = dist;
		} else if (m->legionRecruitType == recruitType && dist < minDist) {
			recruitType = m->legionRecruitType;
			formationId = i;
			minDist = dist;
		}
	}
	if (formationId > 0) {
		struct Data_Formation *m = &Data_Formations[formationId];
		int figureId = Figure_create(m->figureType, x, y, 0);
		struct Data_Figure *f = &Data_Figures[figureId];
		f->formationId = formationId;
		f->formationAtRest = 1;
		if (m->figureType == Figure_FortLegionary) {
			if (Data_Buildings[buildingId].loadsStored > 0) {
				Data_Buildings[buildingId].loadsStored--;
			}
		}
		int academyId = Formation_getClosestMilitaryAcademy(formationId);
		if (academyId) {
			int xRoad, yRoad;
			if (Terrain_hasRoadAccess(Data_Buildings[academyId].x,
				Data_Buildings[academyId].y, Data_Buildings[academyId].size, &xRoad, &yRoad)) {
				f->actionState = FigureActionState_85_SoldierGoingToMilitaryAcademy;
				f->destinationX = xRoad;
				f->destinationY = yRoad;
				f->destinationGridOffsetSoldier = GridOffset(f->destinationX, f->destinationY);
			} else {
				f->actionState = FigureActionState_81_SoldierGoingToFort;
			}
		} else {
			f->actionState = FigureActionState_81_SoldierGoingToFort;
		}
	}
	Formation_calculateFigures();
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
			!b->figureId && b->roadNetworkId == Data_Buildings[buildingId].roadNetworkId) {
			towerId = i;
			break;
		}
	}
	if (!towerId) {
		return 0;
	}
	struct Data_Building *tower = &Data_Buildings[towerId];
	int figureId = Figure_create(Figure_TowerSentry, x, y, 0);
	struct Data_Figure *f = &Data_Figures[figureId];
	f->actionState = FigureActionState_174_TowerSentryGoingToTower;
	int xRoad, yRoad;
	if (Terrain_hasRoadAccess(tower->x, tower->y, tower->size, &xRoad, &yRoad)) {
		f->destinationX = xRoad;
		f->destinationY = yRoad;
	} else {
		f->state = FigureState_Dead;
	}
	tower->figureId = figureId;
	f->buildingId = towerId;
	return 1;
}

void Figure_killTowerSentriesAt(int x, int y)
{
	for (int i = 0; i < MAX_FIGURES; i++) {
		if (!FigureIsDead(i) && Data_Figures[i].type == Figure_TowerSentry) {
			if (Calc_distanceMaximum(Data_Figures[i].x, Data_Figures[i].y, x, y) <= 1) {
				Data_Figures[i].state = FigureState_Dead;
			}
		}
	}
}

void Figure_sinkAllShips()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->state != FigureState_Alive) {
			continue;
		}
		int buildingId;
		if (f->type == Figure_TradeShip) {
			buildingId = f->destinationBuildingId;
		} else if (f->type == Figure_FishingBoat) {
			buildingId = f->buildingId;
		} else {
			continue;
		}
		Data_Buildings[buildingId].data.other.boatFigureId = 0;
		f->buildingId = 0;
		f->type = Figure_Shipwreck;
		f->waitTicks = 0;
	}
}

int Figure_getCitizenOnSameTile(int figureId)
{
	for (int w = Data_Grid_figureIds[Data_Figures[figureId].gridOffset];
		w > 0; w = Data_Figures[w].nextFigureIdOnSameTile) {
		if (Data_Figures[w].actionState != FigureActionState_149_Corpse) {
			int type = Data_Figures[w].type;
			if (type && type != Figure_Explosion && type != Figure_FortStandard &&
				type != Figure_MapFlag && type != Figure_Flotsam && type < Figure_IndigenousNative) {
				return w;
			}
		}
	}
	return 0;
}

int Figure_getNonCitizenOnSameTile(int figureId)
{
	for (int w = Data_Grid_figureIds[Data_Figures[figureId].gridOffset];
		w > 0; w = Data_Figures[w].nextFigureIdOnSameTile) {
		if (Data_Figures[w].actionState != FigureActionState_149_Corpse) {
			int type = Data_Figures[w].type;
			if (FigureIsEnemy(type)) {
				return w;
			}
			if (type == Figure_IndigenousNative && Data_Figures[w].actionState == FigureActionState_159_NativeAttacking) {
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
		struct Data_Figure *f = &Data_Figures[i];
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
