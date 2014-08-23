#include "Formation.h"

#include "Calc.h"
#include "Routing.h"
#include "Sound.h"
#include "Util.h"
#include "Walker.h"
#include "UI/Warning.h"

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

int Formation_createLegion(int buildingId)
{
	Formation_calculateLegionTotals();
	int formationId = -1;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (!Data_Formations[i].inUse) {
			formationId = i;
			break;
		}
	}
	if (formationId < 0) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	struct Data_Formation *f = &Data_Formations[formationId];
	f->inUse = 1;
	f->isLegion = 1;
	f->ciid = 1;
	f->walkerType = b->subtype.fortWalkerType;
	f->buildingId = buildingId;
	f->layout = FormationLayout_DoubleLine1;
	f->morale = 50;
	f->isAtFort = 1;
	f->legionId = formationId - 1;
	f->xHome = f->xStandard = f->x = b->x + 3;
	f->yHome = f->yStandard = f->x = b->y - 1;
	int standardId = Walker_create(Walker_FortStandard, 0, 0, 0);
	Data_Walkers[standardId].buildingId = buildingId;
	f->standardWalkerId = standardId;
	
	Data_Formation_Extra.numLegions++; // numForts
	if (formationId > Data_Formation_Extra.idLastInUse) { // formationId_lastInUse
		Data_Formation_Extra.idLastInUse = formationId;
	}
	return formationId;
}

int Formation_create(int walkerType, int layout, int orientation, int x, int y)
{
	int formationId = 0;
	for (int i = 10; i < MAX_FORMATIONS; i++) {
		if (!Data_Formations[i].inUse) {
			formationId = i;
			break;
		}
	}
	if (!formationId) {
		return 0;
	}
	struct Data_Formation *f = &Data_Formations[formationId];
	f->x = x;
	f->y = y;
	f->inUse = 1;
	f->isLegion = 0;
	f->ciid = 0;
	f->walkerType = walkerType;
	f->legionId = formationId - 10;
	if (layout == 10) {
		if (orientation == 0 || orientation == 4) {
			f->layout = FormationLayout_DoubleLine1;
		} else {
			f->layout = FormationLayout_DoubleLine2;
		}
	} else {
		f->layout = layout;
	}
	f->morale = 100;
	return formationId;
}

void Formation_deleteFortAndBanner(int formationId)
{
	if (formationId > 0 && Data_Formations[formationId].inUse) {
		if (Data_Formations[formationId].standardWalkerId) {
			Walker_delete(Data_Formations[formationId].standardWalkerId);
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

int Formation_getLegionFormationAtGridOffset(int gridOffset)
{
	for (int walkerId = Data_Grid_walkerIds[gridOffset];
		walkerId && walkerId != Data_Walkers[walkerId].nextWalkerIdOnSameTile;
		walkerId = Data_Walkers[walkerId].nextWalkerIdOnSameTile) {
		if (WalkerIsLegion(Data_Walkers[walkerId].type)) {
			return Data_Walkers[walkerId].formationId;
		}
	}
	return 0;
}

int Formation_getFormationForBuilding(int gridOffset)
{
	int buildingId = Data_Grid_buildingIds[gridOffset];
	if (buildingId > 0) {
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->inUse == 1 && (b->type == Building_FortGround__ || b->type == Building_FortGround)) {
			return b->formationId;
		}
	}
	return 0;
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

void Formation_legionMoveTo(int formationId, int x, int y)
{
	struct Data_Formation *f = &Data_Formations[formationId];
	Routing_getDistance(f->xHome, f->yHome);
	if (Data_Grid_routingDistance[GridOffset(x, y)] <= 0) {
		return; // unable to route there
	}
	if (x == f->xHome && y == f->yHome) {
		return; // use legionReturnHome
	}
	if (f->cursedByMars) {
		return;
	}
	f->xStandard = x;
	f->yStandard = y;
	f->isAtFort = 0;
	if (f->morale <= 20) {
		UI_Warning_show(Warning_LegionMoraleTooLow);
	}
	for (int i = 0; i < 16 && f->walkerIds[i]; i++) {
		int walkerId = f->walkerIds[i];
		struct Data_Walker *w = &Data_Walkers[walkerId];
		if (w->actionState == WalkerActionState_149_Corpse ||
			w->actionState == WalkerActionState_150_Attack) {
			continue;
		}
		if (f->monthsVeryLowMorale || f->monthsLowMorale > 1) {
			continue;
		}
		if (f->monthsLowMorale == 1) {
			Formation_changeMorale(formationId, 10); // yay we can move?
		}
		w->alternativeLocationIndex = 0;
		w->actionState = WalkerActionState_83_SoldierGoingToStandard;
		WalkerRoute_remove(walkerId);
	}
}

void Formation_legionReturnHome(int formationId)
{
	struct Data_Formation *f = &Data_Formations[formationId];
	Routing_getDistance(f->xHome, f->yHome);
	if (Data_Grid_routingDistance[GridOffset(f->x, f->y)] <= 0) {
		return; // unable to route home
	}
	if (f->cursedByMars) {
		return;
	}
	f->isAtFort = 1;
	if (f->layout == FormationLayout_MopUp) {
		f->layout = f->layoutBeforeMopUp;
	}
	for (int i = 0; i < 16 && f->walkerIds[i]; i++) {
		int walkerId = f->walkerIds[i];
		struct Data_Walker *w = &Data_Walkers[walkerId];
		if (w->actionState == WalkerActionState_149_Corpse ||
			w->actionState == WalkerActionState_150_Attack) {
			continue;
		}
		if (f->monthsVeryLowMorale || f->monthsLowMorale > 1) {
			continue;
		}
		if (f->monthsLowMorale == 1) {
			Formation_changeMorale(formationId, 10); // yay we can move?
		}
		w->actionState = WalkerActionState_81_SoldierGoingToFort;
		WalkerRoute_remove(walkerId);
	}
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
			if (Data_Formations[i].missileAttackTimeout <= 0 &&
				Data_Formations[i].walkerIds[0]) {
				int walkerId = Data_Formations[i].walkerIds[0];
				if (Data_Walkers[walkerId].state == WalkerState_Alive) {
					Data_Formations[i].xHome = Data_Walkers[walkerId].x;
					Data_Formations[i].yHome = Data_Walkers[walkerId].y;
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
			Data_Formations[i].legionRecruitType > 0) {
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

void Formation_setNewSoldierRequest(int buildingId)
{
	struct Data_Formation *f = &Data_Formations[Data_Buildings[buildingId].formationId];
	f->legionRecruitType = 0;
	if (!f->isAtFort || f->cursedByMars || f->numWalkers == f->maxWalkers) {
		return;
	}
	if (f->numWalkers < f->maxWalkers) {
		int type = Data_Buildings[buildingId].subtype.fortWalkerType;
		if (type == Walker_FortLegionary) {
			f->legionRecruitType = 3;
		} else if (type == Walker_FortJavelin) {
			f->legionRecruitType = 2;
		} else if (type == Walker_FortMounted) {
			f->legionRecruitType = 1;
		}
	} else { // too many walkers
		int tooMany = f->numWalkers - f->maxWalkers;
		for (int i = 15; i >= 0 && tooMany > 0; i--) {
			if (f->walkerIds[i]) {
				Data_Walkers[f->walkerIds[i]].actionState = WalkerActionState_82_SoldierReturningToBarracks;
				tooMany--;
			}
		}
		Formation_calculateWalkers();
	}
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
		if (wtype == Walker_Enemy54_Gladiator) {
			continue;
		}
		int formationId = Data_Walkers[i].formationId;
		Data_Formations[formationId].numWalkers++;
		Data_Formations[formationId].maxTotalDamage +=
			Constant_WalkerProperties[wtype].maxDamage;
		Data_Formations[formationId].totalDamage += Data_Walkers[i].damage;
		if (Data_Walkers[i].formationAtRest != 1) {
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
	Formation_changeMorale(formationId, morale);
}

void Formation_changeMorale(int formationId, int amount)
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

int Formation_getInvasionGridOffset(int invasionSeq)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1) {
			if (!Data_Formations[i].isHerd && !Data_Formations[i].isLegion) {
				if (Data_Formations[i].invasionSeq == invasionSeq) {
					return GridOffset(Data_Formations[i].xHome, Data_Formations[i].yHome);
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
					Data_Walkers[walkerId].actionState = WalkerActionState_87_SoldierGoingToDistantBattle;
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
					Data_Walkers[walkerId].actionState = WalkerActionState_88_SoldierReturningFromDistantBattle;
					Data_Walkers[walkerId].formationAtRest = 1;
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
		Formation_changeMorale(i, -75);

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

void Formation_moveHerdsAwayFrom(int x, int y)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isLegion || !f->isHerd || f->numWalkers <= 0) {
			continue;
		}
		if (Calc_distanceMaximum(x, y, f->xHome, f->yHome) <= 6) {
			f->__unknown46 = 50;
			f->herdDirection = Routing_getGeneralDirection(x, y, f->xHome, f->yHome);
		}
	}
}

void Formation_Tick_updateRestMorale()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isHerd) {
			continue;
		}
		if (f->isLegion) {
			if (f->isAtFort) {
				f->monthsFromHome = 0;
				f->monthsVeryLowMorale = 0;
				f->monthsLowMorale = 0;
				Formation_changeMorale(i, 5);
				if (f->layout == FormationLayout_MopUp) {
					f->layout = f->layoutBeforeMopUp;
				}
			} else if (!f->recentFight) {
				f->monthsFromHome++;
				if (f->monthsFromHome > 3) {
					if (f->monthsFromHome > 100) {
						f->monthsFromHome = 100;
					}
					Formation_changeMorale(i, -5);
				}
			}
		} else {
			Formation_changeMorale(i, 0);
		}
	}
}
