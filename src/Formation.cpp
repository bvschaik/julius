#include "Formation.h"

#include "Calc.h"
#include "Sound.h"
#include "Util.h"
#include "Walker.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Walker.h"

#include <string.h>

static void changeMorale(int formationId, int amount);
static void changeMoraleOfAllLegions(int amount);
static void changeMoraleOfAllEnemies(int amount);

void Formation_clearList()
{
	for (int i = 0; i < MAX_FORMATIONS; i++) {
		memset(&Data_Formations[i], 0, sizeof(struct Data_Formation));
	}
	Data_Formation_Extra.idLastInUse = 0;
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numLegions = 0;
}

void Formation_clearInvasionInfo()
{
	// TODO
}

void Formation_deleteFortAndBanner(int formationId)
{
	if (formationId > 0 && Data_Formations[formationId].inUse) {
		if (Data_Formations[formationId].bannerWalkerId) {
			Walker_delete(Data_Formations[formationId].bannerWalkerId);
		}
		memset(&Data_Formations[formationId], 0, 128);
		Formation_calculateLegionTotals();
	}
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
				if (Data_Walkers[walkerId].state == WalkerState_Alive) {
					Data_Formations[i].xFort = Data_Walkers[walkerId].x;
					Data_Formations[i].yFort = Data_Walkers[walkerId].y;
				}
			}
		}
	}
}

int Formation_anyLegionNeedsSoldiers()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 &&
			Data_Formations[i].isLegion &&
			Data_Formations[i].legionRoomForRecruits > 0) {
			return 1;
		}
	}
	return 0;
}

int Formation_getClosestMilitaryAcademy(int formationId)
{
	int fortId = Data_Formations[formationId].buildingId;
	int fortX = Data_Buildings[fortId].x;
	int fortY = Data_Buildings[fortId].y;
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 &&
			Data_Buildings[i].type == Building_MilitaryAcademy &&
			Data_Buildings[i].numWorkers >= Data_Model_Buildings[Building_MilitaryAcademy].laborers) {
			int dist = Calc_distanceMaximum(fortX, fortY,
				Data_Buildings[i].x, Data_Buildings[i].y);
			if (dist < minDistance) {
				minDistance = dist;
				minBuildingId = i;
			}
		}
	}
	return minBuildingId;
}

void Formation_calculateWalkers()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		for (int w = 0; w < 16; w++) {
			Data_Formations[i].walkerIds[w] = 0;
		}
		Data_Formations[i].numWalkers = 0;
		Data_Formations[i].isAtFort = 1;
		Data_Formations[i].totalDamage = 0;
		Data_Formations[i].maxTotalDamage = 0;
	}
	for (int i = 1; i < MAX_WALKERS; i++) {
		if (Data_Walkers[i].state != WalkerState_Alive) {
			continue;
		}
		int wtype = Data_Walkers[i].type;
		if (!WalkerIsLegion(wtype) && !WalkerIsEnemy(wtype) && !WalkerIsHerd(wtype)) {
			continue;
		}
		if (wtype == Walker_Enemy54) {
			continue;
		}
		int formationId = Data_Walkers[i].formationId;
		Data_Formations[formationId].numWalkers++;
		Data_Formations[formationId].maxTotalDamage +=
			Constant_WalkerProperties[wtype].maxDamage;
		Data_Formations[formationId].totalDamage += Data_Walkers[i].damage;
		if (Data_Walkers[i].__unknown_52 != 1) {
			Data_Formations[formationId].isAtFort = 0;
		}
		for (int w = 0; w < 16; w++) {
			if (!Data_Formations[formationId].walkerIds[w]) {
				Data_Formations[formationId].walkerIds[w] = i;
				Data_Walkers[i].indexInFormation = w;
				break;
			}
		}
	}
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse != 1 || Data_Formations[i].isHerd) {
			continue;
		}
		if (Data_Formations[i].isLegion) {
			if (Data_Formations[i].numWalkers > 0) {
				int wasHalted = Data_Formations[i].isHalted;
				for (int w = 0; w < Data_Formations[i].numWalkers; w++) {
					int walkerId = Data_Formations[i].walkerIds[w];
					if (walkerId && Data_Walkers[walkerId].direction != Direction_None) {
						Data_Formations[i].isHalted = 0;
					}
				}
				Data_Formation_Extra.numLegionFormations++;
				Data_Formation_Extra.numLegionSoldierStrength +=
					Data_Formations[i].numWalkers;
				if (Data_Formations[i].walkerType == Walker_FortLegionary) {
					if (!wasHalted && Data_Formations[i].isHalted) {
						Sound_Effects_playChannel(SoundChannel_FormationShield);
					}
					Data_Formation_Extra.numLegionSoldierStrength +=
						Data_Formations[i].numWalkers / 2;
				}
			}
		} else {
			// enemy
			if (Data_Formations[i].numWalkers <= 0) {
				memset(&Data_Formations[i], 0, 128);
			} else {
				Data_Formation_Extra.numEnemyFormations++;
				Data_Formation_Extra.numEnemySoldierStrength +=
					Data_Formations[i].numWalkers;
			}
		}
	}
	Data_CityInfo.militaryTotalLegionsEmpireService = 0;
	Data_CityInfo.militaryTotalSoldiers = 0;
	Data_CityInfo.militaryTotalLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			Data_CityInfo.militaryTotalLegions++;
			Data_CityInfo.militaryTotalSoldiers += Data_Formations[i].numWalkers;
			if (Data_Formations[i].empireService && Data_Formations[i].numWalkers > 0) {
				Data_CityInfo.militaryTotalLegionsEmpireService++;
			}
		}
	}
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
	} else if (f->walkerType == Walker_EnemyCaesarLegionary) {
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

void Formation_dispatchLegionsToDistantBattle()
{
	Data_CityInfo.distantBattleRomanStrength = 0;
	int numLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse == 1 && f->isLegion && f->empireService && f->numWalkers > 0) {
			f->inDistantBattle = 1;
			f->isAtFort = 0;
			numLegions++;
			int strengthFactor;
			if (f->hasMilitaryTraining) {
				strengthFactor = f->walkerType == Walker_FortLegionary ? 3 : 2;
			} else {
				strengthFactor = f->walkerType == Walker_FortLegionary ? 2 : 1;
			}
			Data_CityInfo.distantBattleRomanStrength += strengthFactor * f->numWalkers;
			for (int w = 0; w < f->numWalkers; w++) {
				int walkerId = f->walkerIds[w];
				if (walkerId > 0 &&
					Data_Walkers[walkerId].state == WalkerState_Alive &&
					Data_Walkers[walkerId].actionState != WalkerActionState_149_Corpse) {
					Data_Walkers[walkerId].actionState = WalkerActionState_87_ToDistantBattle;
				}
			}
		}
	}
	if (numLegions > 0) {
		Data_CityInfo.distantBattleRomanMonthsToTravel = Data_Scenario.distantBattleTravelMonthsRoman;
	}
}

void Formation_legionsReturnFromDistantBattle()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse == 1 && f->isLegion && f->inDistantBattle) {
			f->inDistantBattle = 0;
			for (int w = 0; w < f->numWalkers; w++) {
				int walkerId = f->walkerIds[w];
				if (walkerId > 0 &&
					Data_Walkers[walkerId].state == WalkerState_Alive &&
					Data_Walkers[walkerId].actionState != WalkerActionState_149_Corpse) {
					Data_Walkers[walkerId].actionState = WalkerActionState_88_FromDistantBattle;
					Data_Walkers[walkerId].__unknown_52 = 1;
				}
			}
		}
	}
}

void Formation_legionKillSoldiersInDistantBattle(int killPercentage)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse != 1 || !Data_Formations[i].isLegion ||
			!Data_Formations[i].inDistantBattle) {
			continue;
		}
		struct Data_Formation *f = &Data_Formations[i];
		changeMorale(i, -75);

		int numSoldiersTotal = 0;
		for (int w = 0; w < f->numWalkers; w++) {
			int walkerId = f->walkerIds[w];
			if (walkerId > 0 && Data_Walkers[walkerId].state == WalkerState_Alive &&
				Data_Walkers[walkerId].actionState != WalkerActionState_149_Corpse) {
				numSoldiersTotal++;
			}
		}
		int numSoldiersToKill = Calc_adjustWithPercentage(numSoldiersTotal, killPercentage);
		if (numSoldiersToKill >= numSoldiersTotal) {
			f->isAtFort = 1;
			f->inDistantBattle = 0;
		}
		for (int w = 0; w < f->numWalkers; w++) {
			int walkerId = f->walkerIds[w];
			if (walkerId > 0 && Data_Walkers[walkerId].state == WalkerState_Alive &&
				Data_Walkers[walkerId].actionState != WalkerActionState_149_Corpse) {
				if (numSoldiersToKill) {
					numSoldiersToKill--;
					Data_Walkers[walkerId].state = WalkerState_Dead;
				}
			}
		}
	}
}
