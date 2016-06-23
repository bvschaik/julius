#include "Event.h"

#include "Building.h"
#include "Calc.h"
#include "CityInfo.h"
#include "Figure.h"
#include "Formation.h"
#include "PlayerMessage.h"
#include "Random.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Event.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Invasion.h"
#include "Data/Model.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#include <string.h>

static int getEmpireObjectForInvasion(int pathId, int year);
static int startInvasion(int enemyType, int amount, int invasionPoint, int attackType, int invasionId);

static const int enemyIdToEnemyType[20] = {
	0, 7, 7, 10, 8, 8, 9, 5, 6, 3, 3, 3, 2, 2, 4, 4, 4, 6, 1, 6
};

static const int localUprisingNumEnemies[20] = {
	0, 0, 0, 0, 0, 3, 3, 3, 0, 6, 6, 6, 6, 6, 9, 9, 9, 9, 9, 9
};

static const struct {
	int pctType1;
	int pctType2;
	int pctType3;
	int figureTypes[3];
	int formationLayout;
} enemyProperties[12] = {
	{100, 0, 0, {49, 0, 0}, 8},
	{40, 60, 0, {49, 51, 0}, 8},
	{50, 50, 0, {50, 53, 0}, 8},
	{80, 20, 0, {50, 48, 0}, 8},
	{50, 50, 0, {49, 52, 0}, 8},
	{30, 70, 0, {44, 43, 0}, 0},
	{50, 50, 0, {44, 43, 0}, 10},
	{50, 50, 0, {45, 43, 0}, 10},
	{80, 20, 0, {45, 43, 0}, 10},
	{80, 20, 0, {44, 46, 0}, 11},
	{90, 10, 0, {45, 47, 0}, 11},
	{100, 0, 0, {57, 0, 0}, 0}
};

void Event_initInvasions()
{
	memset(Data_InvasionWarnings, 0, sizeof(Data_InvasionWarnings));
	int pathCurrent = 1;
	int pathMax = 0;
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		if (Data_Empire_Objects[i].inUse && Data_Empire_Objects[i].type == EmpireObject_BattleIcon) {
			if (Data_Empire_Objects[i].invasionPathId > pathMax) {
				pathMax = Data_Empire_Objects[i].invasionPathId;
			}
		}
	}
	if (pathMax == 0) {
		return;
	}
	struct Data_InvasionWarning *warning = &Data_InvasionWarnings[1];
	for (int i = 0; i < MAX_EVENTS; i++) {
		Random_generateNext();
		if (!Data_Scenario.invasions.type[i]) {
			continue;
		}
		Data_Scenario.invasions_month[i] = 2 + (Data_Random.random1_7bit & 7);
		if (Data_Scenario.invasions.type[i] == InvasionType_LocalUprising ||
			Data_Scenario.invasions.type[i] == InvasionType_DistantBattle) {
			continue;
		}
		for (int year = 1; year < 8; year++) {
			int objectId = getEmpireObjectForInvasion(pathCurrent, year);
			if (!objectId) {
				continue;
			}
			struct Data_Empire_Object *obj = &Data_Empire_Objects[objectId];
			warning->inUse = 1;
			warning->empireInvasionPathId = obj->invasionPathId;
			warning->warningYears = obj->invasionYears;
			warning->empireX = obj->x;
			warning->empireY = obj->y;
			warning->empireGraphicId = obj->graphicId;
			warning->invasionId = i;
			warning->empireObjectId = objectId;
			warning->gameMonthNotified = 0;
			warning->gameYearNotified = 0;
			warning->monthsToGo = 12 * Data_Scenario.invasions.year[i];
			warning->monthsToGo += Data_Scenario.invasions_month[i];
			warning->monthsToGo -= 12 * year;
			++warning;
		}
		pathCurrent++;
		if (pathCurrent > pathMax) {
			pathCurrent = 1;
		}
	}
}

static int getEmpireObjectForInvasion(int pathId, int year)
{
	for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
		struct Data_Empire_Object *obj = &Data_Empire_Objects[i];
		if (obj->inUse && obj->type == EmpireObject_BattleIcon &&
			obj->invasionPathId == pathId && obj->invasionYears == year) {
			return i;
		}
	}
	return 0;
}

void Event_handleInvasions()
{
	for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
		if (!Data_InvasionWarnings[i].inUse) {
			continue;
		}
		// update warnings
		struct Data_InvasionWarning *warning = &Data_InvasionWarnings[i];
		warning->monthsToGo--;
		if (warning->monthsToGo <= 0) {
			if (warning->handled != 1) {
				warning->handled = 1;
				warning->gameYearNotified = Data_CityInfo_Extra.gameTimeYear;
				warning->gameMonthNotified = Data_CityInfo_Extra.gameTimeMonth;
				if (warning->warningYears > 2) {
					PlayerMessage_post(0, Message_25_DistantBattle, 0, 0);
				} else if (warning->warningYears > 1) {
					PlayerMessage_post(0, Message_26_EnemiesClosing, 0, 0);
				} else {
					PlayerMessage_post(0, Message_27_EnemiesAtTheDoor, 0, 0);
				}
			}
		}
		if (Data_CityInfo_Extra.gameTimeYear >= Data_Scenario.startYear + Data_Scenario.invasions.year[warning->invasionId] &&
			Data_CityInfo_Extra.gameTimeMonth >= Data_Scenario.invasions_month[warning->invasionId]) {
			// invasion attack time has passed
			warning->inUse = 0;
			if (warning->warningYears > 1) {
				continue;
			}
			// enemy invasions
			if (Data_Scenario.invasions.type[warning->invasionId] == InvasionType_EnemyArmy) {
				int gridOffset = startInvasion(
					enemyIdToEnemyType[Data_Scenario.enemyId],
					Data_Scenario.invasions.amount[warning->invasionId],
					Data_Scenario.invasions.from[warning->invasionId],
					Data_Scenario.invasions.attackType[warning->invasionId],
					warning->invasionId);
				if (gridOffset > 0) {
					if (enemyIdToEnemyType[Data_Scenario.enemyId] > 4) {
						PlayerMessage_post(1, Message_114_EnemyArmyAttack, Data_Event.lastInternalInvasionId, gridOffset);
					} else {
						PlayerMessage_post(1, Message_23_BarbarianAttack, Data_Event.lastInternalInvasionId, gridOffset);
					}
				}
			}
			if (Data_Scenario.invasions.type[warning->invasionId] == InvasionType_Caesar) {
				int gridOffset = startInvasion(
					EnemyType_11_Caesar,
					Data_Scenario.invasions.amount[warning->invasionId],
					Data_Scenario.invasions.from[warning->invasionId],
					Data_Scenario.invasions.attackType[warning->invasionId],
					warning->invasionId);
				if (gridOffset > 0) {
					PlayerMessage_post(1, Message_24_CaesarArmyAttack, Data_Event.lastInternalInvasionId, gridOffset);
				}
			}
		}
	}
	// local uprisings
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (Data_Scenario.invasions.type[i] == InvasionType_LocalUprising) {
			if (Data_CityInfo_Extra.gameTimeYear == Data_Scenario.startYear + Data_Scenario.invasions.year[i] &&
				Data_CityInfo_Extra.gameTimeMonth == Data_Scenario.invasions_month[i]) {
				int gridOffset = startInvasion(
					EnemyType_0_Barbarian,
					Data_Scenario.invasions.amount[i],
					Data_Scenario.invasions.from[i],
					Data_Scenario.invasions.attackType[i],
					i);
				if (gridOffset > 0) {
					PlayerMessage_post(1, Message_22_LocalUprising, Data_Event.lastInternalInvasionId, gridOffset);
				}
			}
		}
	}
}

int Event_startInvasionLocalUprisingFromMars()
{
	if (Data_Settings.saveGameMissionId < 0 || Data_Settings.saveGameMissionId > 19) {
		return 0;
	}
	int amount = localUprisingNumEnemies[Data_Settings.saveGameMissionId];
	if (amount <= 0) {
		return 0;
	}
	int gridOffset = startInvasion(EnemyType_0_Barbarian, amount, 8, FormationAttackType_FoodChain, 23);
	if (gridOffset) {
		PlayerMessage_post(1, Message_121_LocalUprisingMars, Data_Event.lastInternalInvasionId, gridOffset);
	}
	return 1;
}

void Event_startInvasionFromCheat()
{
	int gridOffset = startInvasion(enemyIdToEnemyType[Data_Scenario.enemyId], 150, 8, FormationAttackType_FoodChain, 23);
	if (gridOffset) {
		if (enemyIdToEnemyType[Data_Scenario.enemyId] > 4) {
			PlayerMessage_post(1, Message_114_EnemyArmyAttack, Data_Event.lastInternalInvasionId, gridOffset);
		} else {
			PlayerMessage_post(1, Message_23_BarbarianAttack, Data_Event.lastInternalInvasionId, gridOffset);
		}
	}
}

int Event_existsUpcomingInvasion()
{
	for (int i = 0; i < MAX_INVASION_WARNINGS; i++) {
		if (Data_InvasionWarnings[i].inUse && Data_InvasionWarnings[i].handled) {
			return 1;
		}
	}
	return 0;
}

static void determineFormations(int numSoldiers, int *numFormations, int soldiersPerFormation[])
{
	if (numSoldiers > 0) {
		if (numSoldiers <= 16) {
			*numFormations = 1;
			soldiersPerFormation[0] = numSoldiers;
		} else if (numSoldiers <= 32) {
			*numFormations = 2;
			soldiersPerFormation[1] = numSoldiers / 2;
			soldiersPerFormation[0] = numSoldiers - numSoldiers / 2;
		} else {
			*numFormations = 3;
			soldiersPerFormation[2] = numSoldiers / 3;
			soldiersPerFormation[1] = numSoldiers / 3;
			soldiersPerFormation[0] = numSoldiers - 2 * (numSoldiers / 3);
		}
	}
}

static int startInvasion(int enemyType, int amount, int invasionPoint, int attackType, int invasionId)
{
	if (amount <= 0) {
		return -1;
	}
	int formationsPerType[3];
	int soldiersPerFormation[3][4];
	int x, y;
	int orientation;

	amount = Calc_adjustWithPercentage(amount,
		Data_Model_Difficulty.enemyPercentage[Data_Settings.difficulty]);
	if (amount >= 150) {
		amount = 150;
	}
	Data_Event.lastInternalInvasionId++;
	if (Data_Event.lastInternalInvasionId > 32000) {
		Data_Event.lastInternalInvasionId = 1;
	}
	// calculate soldiers per type
	int numType1 = Calc_adjustWithPercentage(amount, enemyProperties[enemyType].pctType1);
	int numType2 = Calc_adjustWithPercentage(amount, enemyProperties[enemyType].pctType2);
	int numType3 = Calc_adjustWithPercentage(amount, enemyProperties[enemyType].pctType3);
	numType1 += amount - (numType1 + numType2 + numType3); // assign leftovers to type1

	for (int t = 0; t < 3; t++) {
		formationsPerType[t] = 0;
		for (int f = 0; f < 4; f++) {
			soldiersPerFormation[t][f] = 0;
		}
	}

	// calculate number of formations
	determineFormations(numType1, &formationsPerType[0], soldiersPerFormation[0]);
	determineFormations(numType2, &formationsPerType[1], soldiersPerFormation[1]);
	determineFormations(numType3, &formationsPerType[2], soldiersPerFormation[2]);

	// determine invasion point
	if (enemyType == EnemyType_11_Caesar) {
		x = Data_Scenario.entryPoint.x;
		y = Data_Scenario.entryPoint.y;
	} else {
		int numPoints = 0;
		for (int i = 0; i < 8; i++) {
			if (Data_Scenario.invasionPoints.x[i] != -1) {
				numPoints++;
			}
		}
		if (invasionPoint == 8) {
			if (numPoints <= 2) {
				invasionPoint = Data_Random.random1_7bit & 1;
			} else if (numPoints <= 4) {
				invasionPoint = Data_Random.random1_7bit & 3;
			} else {
				invasionPoint = Data_Random.random1_7bit & 7;
			}
		}
		if (numPoints > 0) {
			while (Data_Scenario.invasionPoints.x[invasionPoint] == -1) {
				invasionPoint++;
				if (invasionPoint >= 8) {
					invasionPoint = 0;
				}
			}
		}
		x = Data_Scenario.invasionPoints.x[invasionPoint];
		y = Data_Scenario.invasionPoints.y[invasionPoint];
	}
	if (x == -1 || y == -1) {
		x = Data_Scenario.exitPoint.x;
		y = Data_Scenario.exitPoint.y;
	}
	// determine orientation
	if (y == 0) {
		orientation = Dir_4_Bottom;
	} else if (y >= Data_Scenario.mapSizeY - 1) {
		orientation = Dir_0_Top;
	} else if (x == 0) {
		orientation = Dir_2_Right;
	} else if (x >= Data_Scenario.mapSizeX - 1) {
		orientation = Dir_6_Left;
	} else {
		orientation = Dir_4_Bottom;
	}
	// check terrain
	int gridOffset = GridOffset(x, y);
	int terrain = Data_Grid_terrain[gridOffset];
	if (terrain & (Terrain_Elevation | Terrain_Rock | Terrain_Tree)) {
		return -1;
	}
	if (terrain & Terrain_Water) {
		if (!(terrain & Terrain_Road)) { // bridge
			return -1;
		}
	} else if (terrain & (Terrain_Building | Terrain_Aqueduct | Terrain_Gatehouse | Terrain_Wall)) {
		Building_destroyByEnemy(x, y, gridOffset);
	}
	// spawn the lot!
	int seq = 0;
	for (int type = 0; type < 3; type++) {
		if (formationsPerType[type] <= 0) {
			continue;
		}
		int figureType = enemyProperties[enemyType].figureTypes[type];
		for (int i = 0; i < formationsPerType[type]; i++) {
			int formationId = Formation_create(
				figureType, enemyProperties[enemyType].formationLayout,
				orientation, x, y);
			if (formationId <= 0) {
				continue;
			}
			struct Data_Formation *f = &Data_Formations[formationId];
			f->attackType = attackType;
			f->orientation = orientation;
			f->enemyType = enemyType;
			f->invasionId = invasionId;
			f->invasionSeq = Data_Event.lastInternalInvasionId;
			for (int fig = 0; fig < soldiersPerFormation[type][i]; fig++) {
				int figureId = Figure_create(figureType, x, y, orientation);
				Data_Figures[figureId].isFriendly = 0;
				Data_Figures[figureId].actionState = FigureActionState_151_EnemyInitial;
				Data_Figures[figureId].waitTicks = 200 * seq + 10 * fig + 10;
				Data_Figures[figureId].formationId = formationId;
				FigureName_set(figureId);
				Data_Figures[figureId].isGhost = 1;
			}
			seq++;
		}
	}
	return gridOffset;
}

static void caesarInvasionPause()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].figureType == Figure_EnemyCaesarLegionary) {
			Data_Formations[i].waitTicks = 20;
		}
	}
}

static void caesarInvasionRetreat()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].figureType == Figure_EnemyCaesarLegionary) {
			Data_Formations[i].monthsLowMorale = 1;
		}
	}
	if (!Data_CityInfo.caesarInvasionRetreatMessageShown) {
		Data_CityInfo.caesarInvasionRetreatMessageShown = 1;
		PlayerMessage_post(1, Message_21_CaesarArmyRetreat, 0, 0);
	}
}

static void updateDebtState()
{
	if (Data_CityInfo.treasury >= 0) {
		Data_CityInfo.monthsInDebt = -1;
		return;
	}
	if (Data_CityInfo.debtState == 0) {
		// provide bailout
		int rescueLoan = Calc_adjustWithPercentage(Data_Scenario.rescueLoan,
			Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
		Data_CityInfo.treasury += rescueLoan;
		Data_CityInfo.financeDonatedThisYear += rescueLoan;
		
		CityInfo_Finance_calculateTotals();
		
		Data_CityInfo.debtState = 1;
		Data_CityInfo.monthsInDebt = 0;
		PlayerMessage_post(1, Message_16_CityInDebt, 0, 0);
		CityInfo_Ratings_reduceProsperityAfterBailout();
	} else if (Data_CityInfo.debtState == 1) {
		Data_CityInfo.debtState = 2;
		Data_CityInfo.monthsInDebt = 0;
		PlayerMessage_post(1, Message_17_CityInDebtAgain, 0, 0);
		CityInfo_Ratings_changeFavor(-5);
	} else if (Data_CityInfo.debtState == 2) {
		if (Data_CityInfo.monthsInDebt == -1) {
			PlayerMessage_post(1, Message_17_CityInDebtAgain, 0, 0);
			Data_CityInfo.monthsInDebt = 0;
		}
		if (Data_CityInfo_Extra.gameTimeDay == 0) {
			Data_CityInfo.monthsInDebt++;
		}
		if (Data_CityInfo.monthsInDebt >= 12) {
			Data_CityInfo.debtState = 3;
			Data_CityInfo.monthsInDebt = 0;
			if (!Data_CityInfo.numImperialSoldiersInCity) {
				PlayerMessage_post(1, Message_18_CityStillInDebt, 0, 0);
				CityInfo_Ratings_changeFavor(-10);
			}
		}
	} else if (Data_CityInfo.debtState == 3) {
		if (Data_CityInfo.monthsInDebt == -1) {
			PlayerMessage_post(1, Message_18_CityStillInDebt, 0, 0);
			Data_CityInfo.monthsInDebt = 0;
		}
		if (Data_CityInfo_Extra.gameTimeDay == 0) {
			Data_CityInfo.monthsInDebt++;
		}
		if (Data_CityInfo.monthsInDebt >= 12) {
			Data_CityInfo.debtState = 4;
			Data_CityInfo.monthsInDebt = 0;
			if (!Data_CityInfo.numImperialSoldiersInCity) {
				CityInfo_Ratings_setMaxFavor(10);
			}
		}
	}
}

static void updateCaesarInvasion()
{
	if (Data_CityInfo.numImperialSoldiersInCity) {
		// caesar invasion in progress
		Data_CityInfo.caesarInvasionDurationDayCountdown--;
		if (Data_CityInfo.ratingFavor >= 35 && Data_CityInfo.caesarInvasionDurationDayCountdown < 176) {
			caesarInvasionRetreat();
		} else if (Data_CityInfo.ratingFavor >= 22) {
			if (Data_CityInfo.caesarInvasionDurationDayCountdown > 0) {
				caesarInvasionPause();
			} else if (Data_CityInfo.caesarInvasionDurationDayCountdown == 0) {
				PlayerMessage_post(1, Message_20_CaesarArmyContinue, 0, 0); // a year has passed (11 months), siege goes on
			}
		}
	} else if (Data_CityInfo.caesarInvasionSoldiersDied && Data_CityInfo.caesarInvasionSoldiersDied >= Data_CityInfo.caesarInvasionSize) {
		// player defeated caesar army
		Data_CityInfo.caesarInvasionSize = 0;
		Data_CityInfo.caesarInvasionSoldiersDied = 0;
		if (Data_CityInfo.ratingFavor < 35) {
			CityInfo_Ratings_changeFavor(10);
			if (Data_CityInfo.caesarInvasionCount < 2) {
				PlayerMessage_post(1, Message_106_CaesarRespect1, 0, 0);
			} else if (Data_CityInfo.caesarInvasionCount < 3) {
				PlayerMessage_post(1, Message_107_CaesarRespect2, 0, 0);
			} else {
				PlayerMessage_post(1, Message_108_CaesarRespect3, 0, 0);
			}
		}
	} else if (Data_CityInfo.caesarInvasionDaysUntilInvasion <= 0) {
		if (Data_CityInfo.ratingFavor <= 10) {
			// warn player that caesar is angry and will invade in a year
			Data_CityInfo.caesarInvasionWarningsGiven++;
			Data_CityInfo.caesarInvasionDaysUntilInvasion = 192;
			if (Data_CityInfo.caesarInvasionWarningsGiven <= 1) {
				PlayerMessage_post(1, Message_19_CaesarWrath, 0, 0);
			}
		}
	} else {
		Data_CityInfo.caesarInvasionDaysUntilInvasion--;
		if (Data_CityInfo.caesarInvasionDaysUntilInvasion == 0) {
			// invade!
			int size;
			if (Data_CityInfo.caesarInvasionCount == 0) {
				size = 32;
			} else if (Data_CityInfo.caesarInvasionCount == 1) {
				size = 64;
			} else if (Data_CityInfo.caesarInvasionCount == 2) {
				size = 96;
			} else {
				size = 144;
			}
			int invasionId = startInvasion(
				EnemyType_11_Caesar, size, 0, FormationAttackType_BestBuildings, 24);
			if (invasionId > 0) {
				Data_CityInfo.caesarInvasionCount++;
				Data_CityInfo.caesarInvasionDurationDayCountdown = 192;
				Data_CityInfo.caesarInvasionRetreatMessageShown = 0;
				PlayerMessage_post(1, Message_24_CaesarArmyAttack, Data_Event.lastInternalInvasionId, invasionId);
				Data_CityInfo.caesarInvasionSize = size;
				Data_CityInfo.caesarInvasionSoldiersDied = 0;
			}
		}
	}
}

void Event_Caesar_update()
{
	updateDebtState();
	updateCaesarInvasion();
}
