#include "Formation.h"

#include "Calc.h"
#include "Util.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

static void changeMorale(int formationId, int amount);
static void changeMoraleOfAllLegions(int amount);
static void changeMoraleOfAllEnemies(int amount);

void Formation_clearList()
{
	for (int i = 0; i < MAX_FORMATIONS; i++) {
		memset(&Data_Formations[i], sizeof(struct Data_Formation), 0);
	}
	Data_Formation_Extra.idLastInUse = 0;
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numLegions = 0;
}

void Formation_deleteFortAndBanner(int formationId)
{
	// TODO
}

void Formation_setMaxSoldierPerLegion()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			Data_Formations[i].maxWalkers = 16;
		}
	}
}

int Formation_getNumLegions()
{
	int numLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			numLegions++;
		}
	}
	return numLegions;
}

int Formation_getLegionFormationId(int legionIndex)
{
	int index = 1;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			if (index == legionIndex) {
				return i;
			}
			index++;
		}
	}
	return 0;
}

void Formation_legionReturnHome(int formationId)
{
	// TODO
}

void Formation_calculateLegionTotals()
{
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numLegions = 0;
	Data_CityInfo.militaryLegionaryLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1) {
			if (Data_Formations[i].isLegion) {
				Data_Formation_Extra.idLastLegion = i;
				Data_Formation_Extra.numLegions++;
				if (Data_Formations[i].walkerType == Walker_FortLegionary) {
					Data_CityInfo.militaryLegionaryLegions++;
				}
			}
			if (Data_Formations[i].__unknown56 <= 0 &&
				Data_Formations[i].walkerIds[0]) {
				int walkerId = Data_Formations[i].walkerIds[0];
				if (Data_Walkers[walkerId].state == WalkerState_1) {
					Data_Formations[i].xFort = Data_Walkers[walkerId].x;
					Data_Formations[i].yFort = Data_Walkers[walkerId].y;
				}
			}
		}
	}
}

void Formation_calculateWalkers()
{
	// TODO
}

void Formation_updateAfterDeath(int formationId)
{
	Formation_calculateWalkers();
	int pctDead = Calc_getPercentage(1, Data_Formations[formationId].numWalkers);
	int morale;
	if (pctDead < 8) {
		morale = -5;
	} else if (pctDead < 10) {
		morale = -7;
	} else if (pctDead < 14) {
		morale = -10;
	} else if (pctDead < 20) {
		morale = -12;
	} else if (pctDead < 30) {
		morale = -15;
	} else {
		morale = -20;
	}
	changeMorale(formationId, morale);
}

static void changeMorale(int formationId, int amount)
{
	Data_Formation *f = &Data_Formations[formationId];
	int maxMorale;
	if (f->walkerType == Walker_FortLegionary) {
		maxMorale = f->hasMilitaryTraining ? 100 : 80;
	} else if (f->walkerType == Walker_Enemy57_Legionary) {
		maxMorale = 100;
	} else if (f->walkerType == Walker_FortJavelin || f->walkerType == Walker_FortMounted) {
		maxMorale = f->hasMilitaryTraining ? 80 : 60;
	} else {
		switch (f->enemyType) {
			case EnemyType_Numidian:
			case EnemyType_Gaul:
			case EnemyType_Celt:
			case EnemyType_Goth:
				maxMorale = 80;
				break;
			case EnemyType_Greek:
			case EnemyType_Carthaginian:
				maxMorale = 90;
				break;
			default:
				maxMorale = 70;
				break;
		}
	}
	BOUND(f->morale, 0, maxMorale);
}

static void changeMoraleOfAllLegions(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (Data_Formations[i].isLegion) {
				changeMorale(i, amount);
			}
		}
	}
}

static void changeMoraleOfAllEnemies(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (!Data_Formations[i].isLegion) {
				changeMorale(i, amount);
			}
		}
	}
}

int Formation_getInvasionGridOffset(int invasionSeq)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1) {
			if (!Data_Formations[i].isHerd && !Data_Formations[i].isLegion) {
				if (Data_Formations[i].invasionSeq == invasionSeq) {
					return Data_Settings_Map.gridStartOffset +
						GRID_SIZE * Data_Formations[i].yFort + Data_Formations[i].xFort;
				}
			}
		}
	}
	return 0;
}
