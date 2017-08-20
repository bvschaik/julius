#include "Formation.h"

#include "core/calc.h"
#include "Figure.h"
#include "FigureMovement.h"
#include "Routing.h"
#include "Sound.h"
#include "UI/Warning.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

#include "building/model.h"

#include <string.h>

void Formation_clearList()
{
	for (int i = 0; i < MAX_FORMATIONS; i++) {
		memset(&Data_Formations[i], 0, sizeof(struct Data_Formation));
	}
	Data_Formation_Extra.idLastInUse = 0;
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numForts = 0;
}

void Formation_clearInvasionInfo()
{
	for (int i = 0; i < MAX_INVASION_FORMATIONS; i++) {
		Data_Formation_Invasion.formationId[i] = 0;
		Data_Formation_Invasion.homeX[i] = 0;
		Data_Formation_Invasion.homeY[i] = 0;
		Data_Formation_Invasion.layout[i] = 0;
		Data_Formation_Invasion.destinationX[i] = 0;
		Data_Formation_Invasion.destinationY[i] = 0;
		Data_Formation_Invasion.destinationBuildingId[i] = 0;
		Data_Formation_Invasion.ignoreRomanSoldiers[i] = 0;
	}
	Data_Formation_Extra.numEnemyFormations = 0;
	Data_Formation_Extra.numEnemySoldierStrength = 0;
	Data_Formation_Extra.numLegionFormations = 0;
	Data_Formation_Extra.numLegionSoldierStrength = 0;
	Data_Formation_Extra.daysSinceRomanSoldierConcentration = 0;
}

int Formation_createLegion(int buildingId)
{
	Formation_calculateLegionTotals();
	int formationId = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (!Data_Formations[i].inUse) {
			formationId = i;
			break;
		}
	}
	if (!formationId) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	struct Data_Formation *m = &Data_Formations[formationId];
	m->inUse = 1;
	m->isLegion = 1;
	m->ciid = 1;
	m->figureType = b->subtype.fortFigureType;
	m->buildingId = buildingId;
	m->layout = FormationLayout_DoubleLine1;
	m->morale = 50;
	m->isAtFort = 1;
	m->legionId = formationId - 1;
	m->xHome = m->xStandard = m->x = b->x + 3;
	m->yHome = m->yStandard = m->y = b->y - 1;
	int standardId = Figure_create(Figure_FortStandard, 0, 0, 0);
	Data_Figures[standardId].buildingId = buildingId;
	Data_Figures[standardId].formationId = formationId;
	m->standardFigureId = standardId;
	
	Data_Formation_Extra.numForts++;
	if (formationId > Data_Formation_Extra.idLastInUse) {
		Data_Formation_Extra.idLastInUse = formationId;
	}
	return formationId;
}

int Formation_create(int figureType, int layout, int orientation, int x, int y)
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
	struct Data_Formation *m = &Data_Formations[formationId];
	m->x = x;
	m->y = y;
	m->inUse = 1;
	m->isLegion = 0;
	m->ciid = 0;
	m->figureType = figureType;
	m->legionId = formationId - 10;
	if (layout == FormationLayout_Enemy10) {
		if (orientation == Dir_0_Top || orientation == Dir_4_Bottom) {
			m->layout = FormationLayout_DoubleLine1;
		} else {
			m->layout = FormationLayout_DoubleLine2;
		}
	} else {
		m->layout = layout;
	}
	m->morale = 100;
	return formationId;
}

void Formation_deleteFortAndBanner(int formationId)
{
	if (formationId > 0 && Data_Formations[formationId].inUse) {
		if (Data_Formations[formationId].standardFigureId) {
			Figure_delete(Data_Formations[formationId].standardFigureId);
		}
		memset(&Data_Formations[formationId], 0, sizeof(struct Data_Formation));
		Formation_calculateLegionTotals();
	}
}

void Formation_setMaxSoldierPerLegion()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			Data_Formations[i].maxFigures = 16;
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
	for (int figureId = Data_Grid_figureIds[gridOffset];
		figureId && figureId != Data_Figures[figureId].nextFigureIdOnSameTile;
		figureId = Data_Figures[figureId].nextFigureIdOnSameTile) {
		if (FigureIsLegion(Data_Figures[figureId].type) ||
			Data_Figures[figureId].type == Figure_FortStandard) {
			return Data_Figures[figureId].formationId;
		}
	}
	return 0;
}

int Formation_getFormationForBuilding(int gridOffset)
{
	int buildingId = Data_Grid_buildingIds[gridOffset];
	if (buildingId > 0) {
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (BuildingIsInUse(buildingId) && (b->type == BUILDING_FORT || b->type == BUILDING_FORT_GROUND)) {
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
	struct Data_Formation *m = &Data_Formations[formationId];
	Routing_getDistance(m->xHome, m->yHome);
	if (Data_Grid_routingDistance[GridOffset(x, y)] <= 0) {
		return; // unable to route there
	}
	if (x == m->xHome && y == m->yHome) {
		return; // use legionReturnHome
	}
	if (m->cursedByMars) {
		return;
	}
	m->xStandard = x;
	m->yStandard = y;
	m->isAtFort = 0;
	if (m->morale <= 20) {
		UI_Warning_show(Warning_LegionMoraleTooLow);
	}
	for (int i = 0; i < MAX_FORMATION_FIGURES && m->figureIds[i]; i++) {
		int figureId = m->figureIds[i];
		struct Data_Figure *f = &Data_Figures[figureId];
		if (f->actionState == FigureActionState_149_Corpse ||
			f->actionState == FigureActionState_150_Attack) {
			continue;
		}
		if (m->monthsVeryLowMorale || m->monthsLowMorale > 1) {
			continue;
		}
		if (m->monthsLowMorale == 1) {
			Formation_changeMorale(formationId, 10); // yay we can move?
		}
		f->alternativeLocationIndex = 0;
		f->actionState = FigureActionState_83_SoldierGoingToStandard;
		FigureRoute_remove(figureId);
	}
}

void Formation_legionReturnHome(int formationId)
{
	struct Data_Formation *m = &Data_Formations[formationId];
	Routing_getDistance(m->xHome, m->yHome);
	if (Data_Grid_routingDistance[GridOffset(m->x, m->y)] <= 0) {
		return; // unable to route home
	}
	if (m->cursedByMars) {
		return;
	}
	m->isAtFort = 1;
	if (m->layout == FormationLayout_MopUp) {
		m->layout = m->layoutBeforeMopUp;
	}
	for (int i = 0; i < MAX_FORMATION_FIGURES && m->figureIds[i]; i++) {
		int figureId = m->figureIds[i];
		struct Data_Figure *f = &Data_Figures[figureId];
		if (f->actionState == FigureActionState_149_Corpse ||
			f->actionState == FigureActionState_150_Attack) {
			continue;
		}
		if (m->monthsVeryLowMorale || m->monthsLowMorale > 1) {
			continue;
		}
		if (m->monthsLowMorale == 1) {
			Formation_changeMorale(formationId, 10); // yay we can go home?
		}
		f->actionState = FigureActionState_81_SoldierGoingToFort;
		FigureRoute_remove(figureId);
	}
}

void Formation_calculateLegionTotals()
{
	Data_Formation_Extra.idLastLegion = 0;
	Data_Formation_Extra.numForts = 0;
	Data_CityInfo.militaryLegionaryLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse == 1) {
			if (m->isLegion) {
				Data_Formation_Extra.idLastLegion = i;
				Data_Formation_Extra.numForts++;
				if (m->figureType == Figure_FortLegionary) {
					Data_CityInfo.militaryLegionaryLegions++;
				}
			}
			if (m->missileAttackTimeout <= 0 && m->figureIds[0]) {
				int figureId = m->figureIds[0];
				if (Data_Figures[figureId].state == FigureState_Alive) {
					m->xHome = Data_Figures[figureId].x;
					m->yHome = Data_Figures[figureId].y;
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
		if (BuildingIsInUse(i) &&
			Data_Buildings[i].type == BUILDING_MILITARY_ACADEMY &&
			Data_Buildings[i].numWorkers >= model_get_building(BUILDING_MILITARY_ACADEMY)->laborers) {
			int dist = calc_maximum_distance(fortX, fortY, Data_Buildings[i].x, Data_Buildings[i].y);
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
	struct Data_Formation *m = &Data_Formations[Data_Buildings[buildingId].formationId];
	m->legionRecruitType = 0;
	if (!m->isAtFort || m->cursedByMars || m->numFigures == m->maxFigures) {
		return;
	}
	if (m->numFigures < m->maxFigures) {
		int type = Data_Buildings[buildingId].subtype.fortFigureType;
		if (type == Figure_FortLegionary) {
			m->legionRecruitType = 3;
		} else if (type == Figure_FortJavelin) {
			m->legionRecruitType = 2;
		} else if (type == Figure_FortMounted) {
			m->legionRecruitType = 1;
		}
	} else { // too many figures
		int tooMany = m->numFigures - m->maxFigures;
		for (int i = 15; i >= 0 && tooMany > 0; i--) {
			if (m->figureIds[i]) {
				Data_Figures[m->figureIds[i]].actionState = FigureActionState_82_SoldierReturningToBarracks;
				tooMany--;
			}
		}
		Formation_calculateFigures();
	}
}

void Formation_calculateFigures()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
			Data_Formations[i].figureIds[fig] = 0;
		}
		Data_Formations[i].numFigures = 0;
		Data_Formations[i].isAtFort = 1;
		Data_Formations[i].totalDamage = 0;
		Data_Formations[i].maxTotalDamage = 0;
	}
	for (int i = 1; i < MAX_FIGURES; i++) {
		if (Data_Figures[i].state != FigureState_Alive) {
			continue;
		}
		int figtype = Data_Figures[i].type;
		if (!FigureIsLegion(figtype) && !FigureIsEnemy(figtype) && !FigureIsHerd(figtype)) {
			continue;
		}
		if (figtype == Figure_Enemy54_Gladiator) {
			continue;
		}
		int formationId = Data_Figures[i].formationId;
		Data_Formations[formationId].numFigures++;
		Data_Formations[formationId].maxTotalDamage += Constant_FigureProperties[figtype].maxDamage;
		Data_Formations[formationId].totalDamage += Data_Figures[i].damage;
		if (Data_Figures[i].formationAtRest != 1) {
			Data_Formations[formationId].isAtFort = 0;
		}
		for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
			if (!Data_Formations[formationId].figureIds[fig]) {
				Data_Formations[formationId].figureIds[fig] = i;
				Data_Figures[i].indexInFormation = fig;
				break;
			}
		}
	}
	Data_Formation_Extra.numEnemyFormations = 0;
	Data_Formation_Extra.numEnemySoldierStrength = 0;
	Data_Formation_Extra.numLegionFormations = 0;
	Data_Formation_Extra.numLegionSoldierStrength = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isHerd) {
			continue;
		}
		if (m->isLegion) {
			if (m->numFigures > 0) {
				int wasHalted = m->isHalted;
				m->isHalted = 1;
				for (int fig = 0; fig < m->numFigures; fig++) {
					int figureId = m->figureIds[fig];
					if (figureId && Data_Figures[figureId].direction != Dir_8_None) {
						m->isHalted = 0;
					}
				}
				Data_Formation_Extra.numLegionFormations++;
				Data_Formation_Extra.numLegionSoldierStrength += m->numFigures;
				if (m->figureType == Figure_FortLegionary) {
					if (!wasHalted && m->isHalted) {
						Sound_Effects_playChannel(SoundChannel_FormationShield);
					}
					Data_Formation_Extra.numLegionSoldierStrength += m->numFigures / 2;
				}
			}
		} else {
			// enemy
			if (m->numFigures <= 0) {
				memset(&Data_Formations[i], 0, 128);
			} else {
				Data_Formation_Extra.numEnemyFormations++;
				Data_Formation_Extra.numEnemySoldierStrength += m->numFigures;
			}
		}
	}
	Data_CityInfo.militaryTotalLegionsEmpireService = 0;
	Data_CityInfo.militaryTotalSoldiers = 0;
	Data_CityInfo.militaryTotalLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && Data_Formations[i].isLegion) {
			Data_CityInfo.militaryTotalLegions++;
			Data_CityInfo.militaryTotalSoldiers += Data_Formations[i].numFigures;
			if (Data_Formations[i].empireService && Data_Formations[i].numFigures > 0) {
				Data_CityInfo.militaryTotalLegionsEmpireService++;
			}
		}
	}
}

void Formation_updateAfterDeath(int formationId)
{
	Formation_calculateFigures();
	int pctDead = calc_percentage(1, Data_Formations[formationId].numFigures);
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
	struct Data_Formation *m = &Data_Formations[formationId];
	int maxMorale;
	if (m->figureType == Figure_FortLegionary) {
		maxMorale = m->hasMilitaryTraining ? 100 : 80;
	} else if (m->figureType == Figure_EnemyCaesarLegionary) {
		maxMorale = 100;
	} else if (m->figureType == Figure_FortJavelin || m->figureType == Figure_FortMounted) {
		maxMorale = m->hasMilitaryTraining ? 80 : 60;
	} else {
		switch (m->enemyType) {
			case 0:
			case EnemyType_1_Numidian:
			case EnemyType_2_Gaul:
			case EnemyType_3_Celt:
			case EnemyType_4_Goth:
				maxMorale = 80;
				break;
			case EnemyType_8_Greek:
			case EnemyType_10_Carthaginian:
				maxMorale = 90;
				break;
			default:
				maxMorale = 70;
				break;
		}
	}
	m->morale = calc_bound(m->morale + amount, 0, maxMorale);
}

int Formation_getInvasionGridOffset(int invasionSeq)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse == 1 && !m->isLegion && !m->isHerd && m->invasionSeq == invasionSeq) {
			if (m->xHome > 0 || m->yHome > 0) {
				return GridOffset(m->xHome, m->yHome);
			}
			return 0;
		}
	}
	return 0;
}

void Formation_dispatchLegionsToDistantBattle()
{
	Data_CityInfo.distantBattleRomanStrength = 0;
	int numLegions = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse == 1 && m->isLegion && m->empireService && m->numFigures > 0) {
			m->inDistantBattle = 1;
			m->isAtFort = 0;
			numLegions++;
			int strengthFactor;
			if (m->hasMilitaryTraining) {
				strengthFactor = m->figureType == Figure_FortLegionary ? 3 : 2;
			} else {
				strengthFactor = m->figureType == Figure_FortLegionary ? 2 : 1;
			}
			Data_CityInfo.distantBattleRomanStrength += strengthFactor * m->numFigures;
			for (int fig = 0; fig < m->numFigures; fig++) {
				int figureId = m->figureIds[fig];
				if (figureId > 0 &&
					Data_Figures[figureId].state == FigureState_Alive &&
					Data_Figures[figureId].actionState != FigureActionState_149_Corpse) {
					Data_Figures[figureId].actionState = FigureActionState_87_SoldierGoingToDistantBattle;
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
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse == 1 && m->isLegion && m->inDistantBattle) {
			m->inDistantBattle = 0;
			for (int fig = 0; fig < m->numFigures; fig++) {
				int figureId = m->figureIds[fig];
				if (figureId > 0 &&
					Data_Figures[figureId].state == FigureState_Alive &&
					Data_Figures[figureId].actionState != FigureActionState_149_Corpse) {
					Data_Figures[figureId].actionState = FigureActionState_88_SoldierReturningFromDistantBattle;
					Data_Figures[figureId].formationAtRest = 1;
				}
			}
		}
	}
}

void Formation_legionKillSoldiersInDistantBattle(int killPercentage)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || !m->isLegion || !m->inDistantBattle) {
			continue;
		}
		Formation_changeMorale(i, -75);

		int numSoldiersTotal = 0;
		for (int fig = 0; fig < m->numFigures; fig++) {
			int figureId = m->figureIds[fig];
			if (figureId > 0 && Data_Figures[figureId].state == FigureState_Alive &&
				Data_Figures[figureId].actionState != FigureActionState_149_Corpse) {
				numSoldiersTotal++;
			}
		}
		int numSoldiersToKill = calc_adjust_with_percentage(numSoldiersTotal, killPercentage);
		if (numSoldiersToKill >= numSoldiersTotal) {
			m->isAtFort = 1;
			m->inDistantBattle = 0;
		}
		for (int fig = 0; fig < m->numFigures; fig++) {
			int figureId = m->figureIds[fig];
			if (figureId > 0 && Data_Figures[figureId].state == FigureState_Alive &&
				Data_Figures[figureId].actionState != FigureActionState_149_Corpse) {
				if (numSoldiersToKill) {
					numSoldiersToKill--;
					Data_Figures[figureId].state = FigureState_Dead;
				}
			}
		}
	}
}

void Formation_moveHerdsAwayFrom(int x, int y)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isLegion || !m->isHerd || m->numFigures <= 0) {
			continue;
		}
		if (calc_maximum_distance(x, y, m->xHome, m->yHome) <= 6) {
			m->waitTicks = 50;
			m->herdDirection = Routing_getGeneralDirection(x, y, m->xHome, m->yHome);
		}
	}
}

int Formation_marsCurseFort()
{
	int bestLegionId = 0;
	int bestLegionWeight = 0;
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse == 1 && m->isLegion) {
			int weight = m->numFigures;
			if (m->figureType == Figure_FortLegionary) {
				weight *= 2;
			}
			if (weight > bestLegionWeight) {
				bestLegionWeight = weight;
				bestLegionId = i;
			}
		}
	}
	if (bestLegionId <= 0) {
		return 0;
	}
	struct Data_Formation *m = &Data_Formations[bestLegionId];
	for (int i = 0; i < 15; i++) { // BUG: last figure not cursed
		if (m->figureIds[i] > 0) {
			Data_Figures[m->figureIds[i]].actionState = FigureActionState_82_SoldierReturningToBarracks;
		}
	}
	m->cursedByMars = 96;
	Formation_calculateFigures();
	return 1;
}

void Formation_Tick_updateRestMorale()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isHerd) {
			continue;
		}
		if (m->isLegion) {
			if (m->isAtFort) {
				m->monthsFromHome = 0;
				m->monthsVeryLowMorale = 0;
				m->monthsLowMorale = 0;
				Formation_changeMorale(i, 5);
				if (m->layout == FormationLayout_MopUp) {
					m->layout = m->layoutBeforeMopUp;
				}
			} else if (!m->recentFight) {
				m->monthsFromHome++;
				if (m->monthsFromHome > 3) {
					if (m->monthsFromHome > 100) {
						m->monthsFromHome = 100;
					}
					Formation_changeMorale(i, -5);
				}
			}
		} else {
			Formation_changeMorale(i, 0);
		}
	}
}
