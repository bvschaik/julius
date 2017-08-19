#include "Formation.h"

#include "core/calc.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Grid.h"
#include "PlayerMessage.h"
#include "Routing.h"
#include "Sound.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Settings.h"
#include "Data/Figure.h"

static const int enemyAttackBuildingPriority[4][24] = {
	{
		Building_Granary, Building_Warehouse, Building_Market,
		Building_WheatFarm, Building_VegetableFarm, Building_FruitFarm,
		Building_OliveFarm, Building_VinesFarm, Building_PigFarm, 0
	},
	{
		Building_SenateUpgraded, Building_Senate,
		Building_ForumUpgraded, Building_Forum, 0
	},
	{
		Building_GovernorsPalace, Building_GovernorsVilla, Building_GovernorsHouse,
		Building_HouseLuxuryPalace, Building_HouseLargePalace,
		Building_HouseMediumPalace, Building_HouseSmallPalace,
		Building_HouseGrandVilla, Building_HouseLargeVilla,
		Building_HouseMediumVilla, Building_HouseSmallVilla,
		Building_HouseGrandInsula, Building_HouseLargeInsula,
		Building_HouseMediumInsula, Building_HouseSmallInsula,
		Building_HouseLargeCasa, Building_HouseSmallCasa,
		Building_HouseLargeHovel, Building_HouseSmallHovel,
		Building_HouseLargeShack, Building_HouseSmallShack,
		Building_HouseLargeTent, Building_HouseSmallTent, 0
	},
	{
		Building_MilitaryAcademy, Building_Barracks, 0
	}
};

static const int rioterAttackBuildingPriority[100] = {
	79, 78, 77, 29, 28, 27, 26, 25, 85, 84, 32, 33, 98, 65, 66, 67,
	68, 69, 87, 86, 30, 31, 47, 52, 46, 48, 53, 51, 24, 23, 22, 21,
	20, 46, 48, 114, 113, 112, 111, 110, 71, 72, 70, 74, 75, 76, 60, 61,
	62, 63, 64, 34, 36, 37, 35, 94, 19, 18, 17, 16, 15, 49, 106, 107,
	109, 108, 90, 100, 101, 102, 103, 104, 105, 55, 81, 91, 92, 14, 13, 12, 11, 10, 0
};

static const int layoutOrientationLegionIndexOffsets[13][4][40] = {
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -6, 0, 6, 0, -6, 2, 6, 2, -2, 4, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, 2, -6, 2, 6, 4, -2, 6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -6, 0, 6, 0, -6, -2, 6, -2, -4, -6, 4, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -6, 0, 6, -2, -6, -2, 6, -6, -4, -6, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -4, 0, 4, 0, -12, 0, 12, 0, -4, 12, 4, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -4, 0, 4, 0, -12, 0, 12, 12, -4, 12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -4, 0, 4, 0, -12, 0, 12, 0, -4, -12, 4, -12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -4, 0, 4, 0, -12, 0, 12, -12, -4, -12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	{
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, 8, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, 8, -3, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, -3, 0, 3, 0, -8, 0, 8, 0, -3, -8, 3, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, -3, 0, 3, 0, -8, 0, 8, -8, -3, -8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	}
};


static void changeMoraleOfAllLegions(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (Data_Formations[i].isLegion) {
				Formation_changeMorale(i, amount);
			}
		}
	}
}

static void changeMoraleOfAllEnemies(int amount)
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		if (Data_Formations[i].inUse == 1 && !Data_Formations[i].isHerd) {
			if (!Data_Formations[i].isLegion) {
				Formation_changeMorale(i, amount);
			}
		}
	}
}

static void tickDecreaseLegionDamage()
{
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->state == FigureState_Alive && FigureIsLegion(f->type)) {
			if (f->actionState == FigureActionState_80_SoldierAtRest) {
				if (f->damage) {
					f->damage--;
				}
			}
		}
	}
}

static void tickUpdateMorale()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isHerd) {
			continue;
		}
		if (m->isLegion) {
			if (!m->isAtFort && !m->inDistantBattle) {
				if (m->morale <= 20 && !m->monthsLowMorale && !m->monthsVeryLowMorale) {
					changeMoraleOfAllLegions(-10);
					changeMoraleOfAllEnemies(10);
				}
				if (m->morale <= 10) {
					m->monthsVeryLowMorale++;
				} else if (m->morale <= 20) {
					m->monthsLowMorale++;
				}
			}
		} else { // enemy
			if (m->morale <= 20 && !m->monthsLowMorale && !m->monthsVeryLowMorale) {
				changeMoraleOfAllLegions(10);
				changeMoraleOfAllEnemies(-10);
			}
			if (m->morale <= 10) {
				m->monthsVeryLowMorale++;
			} else if (m->morale <= 20) {
				m->monthsLowMorale++;
			}
		}
	}
}

static void tickUpdateDirection()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isHerd) {
			continue;
		}
		if (m->__unknown66) {
			m->__unknown66--;
		} else if (m->missileFired) {
			m->direction = Data_Figures[m->figureIds[0]].direction;
		} else if (m->layout == FormationLayout_DoubleLine1 || m->layout == FormationLayout_SingleLine1) {
			if (m->yHome < m->prevYHome) {
				m->direction = Dir_0_Top;
			} else if (m->yHome > m->prevYHome) {
				m->direction = Dir_4_Bottom;
			}
		} else if (m->layout == FormationLayout_DoubleLine2 || m->layout == FormationLayout_SingleLine2) {
			if (m->xHome < m->prevXHome) {
				m->direction = Dir_6_Left;
			} else if (m->xHome > m->prevXHome) {
				m->direction = Dir_2_Right;
			}
		} else if (m->layout == FormationLayout_Tortoise || m->layout == FormationLayout_Column) {
			int dx = (m->xHome < m->prevXHome) ? (m->prevXHome - m->xHome) : (m->xHome - m->prevXHome);
			int dy = (m->yHome < m->prevYHome) ? (m->prevYHome - m->yHome) : (m->yHome - m->prevYHome);
			if (dx > dy) {
				if (m->xHome < m->prevXHome) {
					m->direction = Dir_6_Left;
				} else if (m->xHome > m->prevXHome) {
					m->direction = Dir_2_Right;
				}
			} else {
				if (m->yHome < m->prevYHome) {
					m->direction = Dir_0_Top;
				} else if (m->yHome > m->prevYHome) {
					m->direction = Dir_4_Bottom;
				}
			}
		}
		m->prevXHome = m->xHome;
		m->prevYHome = m->yHome;
	}
}

static void tickUpdateLegions()
{
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || !m->isLegion) {
			continue;
		}
		if (m->cursedByMars) {
			m->cursedByMars--;
		}
		if (m->missileFired) {
			m->missileFired--;
		}
		if (m->missileAttackTimeout) {
			m->missileAttackTimeout--;
		}
		if (m->recentFight) {
			m->recentFight--;
		}
		if (Data_CityInfo.numEnemiesInCity <= 0) {
			m->recentFight = 0;
			m->missileAttackTimeout = 0;
			m->missileFired = 0;
		}
		for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
			if (Data_Figures[m->figureIds[n]].actionState == FigureActionState_150_Attack) {
				m->recentFight = 6;
			}
		}
		if (m->monthsLowMorale || m->monthsVeryLowMorale) {
			// flee back to fort
			for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
				struct Data_Figure *f = &Data_Figures[m->figureIds[n]];
				if (f->actionState != FigureActionState_150_Attack &&
					f->actionState != FigureActionState_149_Corpse &&
					f->actionState != FigureActionState_148_Fleeing) {
					f->actionState = FigureActionState_148_Fleeing;
					FigureRoute_remove(m->figureIds[n]);
				}
			}
		} else if (m->layout == FormationLayout_MopUp) {
			if (Data_Formation_Extra.numEnemyFormations +
				Data_CityInfo.numRiotersInCity +
				Data_CityInfo.numAttackingNativesInCity > 0) {
				for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
					struct Data_Figure *f = &Data_Figures[m->figureIds[n]];
					if (m->figureIds[n] != 0 &&
						f->actionState != FigureActionState_150_Attack &&
						f->actionState != FigureActionState_149_Corpse) {
						f->actionState = FigureActionState_86_SoldierMoppingUp;
					}
				}
			} else {
				m->layout = m->layoutBeforeMopUp;
			}
		}
	}
}

static void addRomanSoldierConcentration(int x, int y, int radius, int amount)
{
	int xMin = x - radius;
	int yMin = y - radius;
	int xMax = x + radius;
	int yMax = y + radius;
	Bound2ToMap(xMin, yMin, xMax, yMax);

	for (int yy = yMin; yy <= yMax; yy++) {
		for (int xx = xMin; xx <= xMax; xx++) {
			int gridOffset = GridOffset(xx, yy);
			Data_Grid_romanSoldierConcentration[gridOffset] += amount;
			if (Data_Grid_figureIds[gridOffset] > 0) {
				int type = Data_Figures[Data_Grid_figureIds[gridOffset]].type;
				if (FigureIsLegion(type)) {
					Data_Grid_romanSoldierConcentration[gridOffset] += 2;
				}
			}
		}
	}
}

static void calculateRomanSoldierConcentration()
{
	Grid_clearUByteGrid(Data_Grid_romanSoldierConcentration);
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || !m->isLegion) {
			continue;
		}
		if (m->numFigures > 0) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 7, 1);
		}
		if (m->numFigures > 3) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 6, 1);
		}
		if (m->numFigures > 6) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 5, 1);
		}
		if (m->numFigures > 9) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 4, 1);
		}
		if (m->numFigures > 12) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 3, 1);
		}
		if (m->numFigures > 15) {
			addRomanSoldierConcentration(m->xHome, m->yHome, 2, 1);
		}
	}
}

static int getHighestRomanSoldierConcentration(int x, int y, int radius, int *xTile, int *yTile)
{
	int xMin = x - radius;
	int yMin = y - radius;
	int xMax = x + radius;
	int yMax = y + radius;
	Bound2ToMap(xMin, yMin, xMax, yMax);

	int maxValue = 0;
	int maxX, maxY;
	for (int yy = yMin; yy <= yMax; yy++) {
		for (int xx = xMin; xx <= xMax; xx++) {
			int gridOffset = GridOffset(xx, yy);
			if (Data_Grid_routingDistance[gridOffset] > 0 &&
				Data_Grid_romanSoldierConcentration[gridOffset] > maxValue) {
				maxValue = Data_Grid_romanSoldierConcentration[gridOffset];
				maxX = xx;
				maxY = yy;
			}
		}
	}
	if (maxValue > 0) {
		*xTile = maxX;
		*yTile = maxY;
		return 1;
	}
	return 0;
}

static void setNativeTargetBuilding(int formationId)
{
	int minBuildingId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		switch (Data_Buildings[i].type) {
			case Building_MissionPost:
			case Building_NativeHut:
			case Building_NativeCrops:
			case Building_NativeMeeting:
			case Building_Warehouse:
			case Building_Fort:
				break;
			default:
				int distance = calc_maximum_distance(
					Data_CityInfo.nativeMainMeetingCenterX,
					Data_CityInfo.nativeMainMeetingCenterY,
					Data_Buildings[i].x, Data_Buildings[i].y);
				if (distance < minDistance) {
					minBuildingId = i;
					minDistance = distance;
				}
		}
	}
	if (minBuildingId > 0) {
		Data_Formations[formationId].destinationX = Data_Buildings[minBuildingId].x;
		Data_Formations[formationId].destinationY = Data_Buildings[minBuildingId].y;
		Data_Formations[formationId].destinationBuildingId = minBuildingId;
	}
}

static void setEnemyTargetBuilding(struct Data_Formation *m)
{
	int attack = m->attackType;
	if (attack == FormationAttackType_Random) {
		attack = Data_Random.random1_7bit & 3;
	}
	int bestTypeIndex = 100;
	int buildingId = 0;
	int minDistance = 10000;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || Data_Grid_romanSoldierConcentration[b->gridOffset]) {
			continue;
		}
		for (int n = 0; n < 24 && n <= bestTypeIndex && enemyAttackBuildingPriority[attack][n]; n++) {
			if (b->type == enemyAttackBuildingPriority[attack][n]) {
				int distance = calc_maximum_distance(m->xHome, m->yHome, b->x, b->y);
				if (n < bestTypeIndex) {
					bestTypeIndex = n;
					buildingId = i;
					minDistance = distance;
				} else if (distance < minDistance) {
					buildingId = i;
					minDistance = distance;
				}
				break;
			}
		}
	}
	if (buildingId <= 0) {
		// no target buildings left: take rioter attack priority
		for (int i = 1; i < MAX_BUILDINGS; i++) {
			struct Data_Building *b = &Data_Buildings[i];
			if (!BuildingIsInUse(i) || Data_Grid_romanSoldierConcentration[b->gridOffset]) {
				continue;
			}
			for (int n = 0; n < 100 && n <= bestTypeIndex && rioterAttackBuildingPriority[n]; n++) {
				if (b->type == rioterAttackBuildingPriority[n]) {
					int distance = calc_maximum_distance(m->xHome, m->yHome, b->x, b->y);
					if (n < bestTypeIndex) {
						bestTypeIndex = n;
						buildingId = i;
						minDistance = distance;
					} else if (distance < minDistance) {
						buildingId = i;
						minDistance = distance;
					}
					break;
				}
			}
		}
	}
	if (buildingId <= 0) {
		return;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type == Building_Warehouse) {
		m->destinationX = b->x + 1;
		m->destinationY = b->y;
		m->destinationBuildingId = buildingId + 1;
	} else {
		m->destinationX = b->x;
		m->destinationY = b->y;
		m->destinationBuildingId = buildingId;
	}
}

static void enemyApproachTarget(struct Data_Formation *m)
{
	if (Routing_canTravelOverLandNonCitizen(m->xHome, m->yHome,
			m->destinationX, m->destinationY, m->destinationBuildingId, 400) ||
		Routing_canTravelThroughEverythingNonCitizen(m->xHome, m->yHome,
			m->destinationX, m->destinationY)) {
		int xTile, yTile;
		if (Routing_getClosestXYWithinRange(8, m->xHome, m->yHome,
				m->destinationX, m->destinationY, 20, &xTile, &yTile)) {
			m->destinationX = xTile;
			m->destinationY = yTile;
		}
	}
}

static void marsKillEnemies()
{
	if (Data_CityInfo.godBlessingMarsEnemiesToKill <= 0) {
		return;
	}
	int toKill = Data_CityInfo.godBlessingMarsEnemiesToKill;
	int gridOffset = 0;
	for (int i = 1; i < MAX_FIGURES && toKill > 0; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->state != FigureState_Alive) {
			continue;
		}
		if (FigureIsEnemy(f->type) && f->type != Figure_Enemy54_Gladiator) {
			f->actionState = FigureActionState_149_Corpse;
			toKill--;
			if (!gridOffset) {
				gridOffset = f->gridOffset;
			}
		}
	}
	Data_CityInfo.godBlessingMarsEnemiesToKill = 0;
	PlayerMessage_post(1, Message_105_SpiritOfMars, 0, gridOffset);
}

static void setFormationFiguresToEnemyInitial(int formationId)
{
	struct Data_Formation *m = &Data_Formations[formationId];
	for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
		if (m->figureIds[i] > 0) {
			struct Data_Figure *f = &Data_Figures[m->figureIds[i]];
			if (f->actionState != FigureActionState_149_Corpse &&
				f->actionState != FigureActionState_150_Attack) {
				f->actionState = FigureActionState_151_EnemyInitial;
				f->waitTicks = 0;
			}
		}
	}
}

static void updateEnemyMovement(int formationId, struct Data_Formation *m, int romanDistance)
{
	int regroup = 0;
	int halt = 0;
	int pursueTarget = 0;
	int advance = 0;
	int targetFormationId = 0;
	if (m->missileFired) {
		halt = 1;
	} else if (m->missileAttackTimeout) {
		pursueTarget = 1;
		targetFormationId = m->missileAttackFormationId;
	} else if (m->waitTicks < 32) {
		regroup = 1;
		m->durationAdvance = 4;
	} else if (Data_Formation_Invasion.ignoreRomanSoldiers[m->invasionId]) {
		halt = 0;
		regroup = 0;
		advance = 1;
	} else {
		int haltDuration, advanceDuration, regroupDuration;
		if (Data_Formation_Invasion.layout[m->invasionId] == FormationLayout_Enemy8 ||
			Data_Formation_Invasion.layout[m->invasionId] == FormationLayout_Enemy12) {
			switch (m->enemyLegionIndex) {
				case 0:
				case 1:
					regroupDuration = 2; advanceDuration = 4; haltDuration = 2;
					break;
				case 2:
				case 3:
					regroupDuration = 2; advanceDuration = 5; haltDuration = 3;
					break;
				default:
					regroupDuration = 2; advanceDuration = 6; haltDuration = 4;
					break;
			}
			if (!romanDistance) {
				advanceDuration += 6;
				haltDuration--;
				regroupDuration--;
			}
		} else {
			if (romanDistance) {
				regroupDuration = 6;
				advanceDuration = 4;
				haltDuration = 2;
			} else {
				regroupDuration = 1;
				advanceDuration = 12;
				haltDuration = 1;
			}
		}
		if (m->durationHalt) {
			m->durationAdvance = 0;
			m->durationRegroup = 0;
			halt = 1;
			m->durationHalt--;
			if (m->durationHalt <= 0) {
				m->durationRegroup = regroupDuration;
				setFormationFiguresToEnemyInitial(formationId);
				regroup = 0;
				halt = 1;
			}
		} else if (m->durationRegroup) {
			m->durationAdvance = 0;
			m->durationHalt = 0;
			regroup = 1;
			m->durationRegroup--;
			if (m->durationRegroup <= 0) {
				m->durationAdvance = advanceDuration;
				setFormationFiguresToEnemyInitial(formationId);
				advance = 1;
				regroup = 0;
			}
		} else {
			m->durationRegroup = 0;
			m->durationHalt = 0;
			advance = 1;
			m->durationAdvance--;
			if (m->durationAdvance <= 0) {
				m->durationHalt = haltDuration;
				setFormationFiguresToEnemyInitial(formationId);
				halt = 1;
				advance = 0;
			}
		}
	}

	if (m->waitTicks > 32) {
		marsKillEnemies();
	}
	if (halt) {
		m->destinationX = m->xHome;
		m->destinationY = m->yHome;
	} else if (pursueTarget) {
		if (targetFormationId > 0) {
			if (Data_Formations[targetFormationId].numFigures > 0) {
				m->destinationX = Data_Formations[targetFormationId].xHome;
				m->destinationY = Data_Formations[targetFormationId].yHome;
			}
		} else {
			m->destinationX = Data_Formation_Invasion.destinationX[m->invasionId];
			m->destinationY = Data_Formation_Invasion.destinationY[m->invasionId];
		}
	} else if (regroup) {
		int layout = Data_Formation_Invasion.layout[m->invasionId];
		int xOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemyLegionIndex] +
			Data_Formation_Invasion.homeX[m->invasionId];
		int yOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemyLegionIndex + 1] +
			Data_Formation_Invasion.homeY[m->invasionId];
		int xTile, yTile;
		if (FigureAction_HerdEnemy_moveFormationTo(formationId, xOffset, yOffset, &xTile, &yTile)) {
			m->destinationX = xTile;
			m->destinationY = yTile;
		}
	} else if (advance) {
		int layout = Data_Formation_Invasion.layout[m->invasionId];
		int xOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemyLegionIndex] +
			Data_Formation_Invasion.destinationX[m->invasionId];
		int yOffset = layoutOrientationLegionIndexOffsets[layout][m->orientation / 2][2 * m->enemyLegionIndex + 1] +
			Data_Formation_Invasion.destinationY[m->invasionId];
		int xTile, yTile;
		if (FigureAction_HerdEnemy_moveFormationTo(formationId, xOffset, yOffset, &xTile, &yTile)) {
			m->destinationX = xTile;
			m->destinationY = yTile;
		}
	}
}

static void tickUpdateEnemies()
{
	if (Data_Formation_Extra.numEnemyFormations <= 0) {
		for (int i = 0; i < MAX_INVASION_FORMATIONS; i++) {
			Data_Formation_Invasion.ignoreRomanSoldiers[i] = 0;
		}
		setNativeTargetBuilding(0);
		return;
	}
	Data_Formation_Extra.daysSinceRomanSoldierConcentration++;
	if (Data_Formation_Extra.daysSinceRomanSoldierConcentration > 4) {
		Data_Formation_Extra.daysSinceRomanSoldierConcentration = 0;
		calculateRomanSoldierConcentration();
	}
	for (int i = 0; i < MAX_INVASION_FORMATIONS; i++) {
		Data_Formation_Invasion.formationId[i] = 0;
		Data_Formation_Invasion.numLegions[i] = 0;
	}
	int romanDistance = 0;
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isLegion || m->isHerd) {
			continue;
		}
		if (Data_Formation_Extra.numEnemySoldierStrength > 2 * Data_Formation_Extra.numLegionSoldierStrength) {
			if (m->figureType != Figure_FortJavelin) {
				Data_Formation_Invasion.ignoreRomanSoldiers[m->invasionId] = 1;
			}
		}
		if (m->missileFired) {
			m->missileFired--;
		}
		if (m->missileAttackTimeout) {
			m->missileAttackTimeout--;
		}
		if (m->recentFight) {
			m->recentFight--;
		}
		if (Data_CityInfo.numSoldiersInCity <= 0) {
			m->recentFight = 0;
			m->missileAttackTimeout = 0;
			m->missileFired = 0;
		}
		for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
			int figureId = m->figureIds[n];
			if (Data_Figures[figureId].actionState == FigureActionState_150_Attack) {
				int opponentId = Data_Figures[figureId].opponentId;
				if (!FigureIsDead(opponentId) && FigureIsLegion(Data_Figures[opponentId].type)) {
					m->recentFight = 6;
				}
			}
		}
		if (m->monthsLowMorale || m->monthsVeryLowMorale) {
			for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
				struct Data_Figure *f = &Data_Figures[m->figureIds[n]];
				if (f->actionState != FigureActionState_150_Attack &&
					f->actionState != FigureActionState_149_Corpse &&
					f->actionState != FigureActionState_148_Fleeing) {
					f->actionState = FigureActionState_148_Fleeing;
					FigureRoute_remove(m->figureIds[n]);
				}
			}
			continue;
		}
		if (m->figureIds[0] && Data_Figures[m->figureIds[0]].state == FigureState_Alive) {
			m->xHome = Data_Figures[m->figureIds[0]].x;
			m->yHome = Data_Figures[m->figureIds[0]].y;
		}
		if (!Data_Formation_Invasion.formationId[m->invasionId]) {
			Data_Formation_Invasion.formationId[m->invasionId] = i;
			Data_Formation_Invasion.homeX[m->invasionId] = m->xHome;
			Data_Formation_Invasion.homeY[m->invasionId] = m->yHome;
			Data_Formation_Invasion.layout[m->invasionId] = m->layout;
			romanDistance = 0;
			Routing_canTravelOverLandNonCitizen(m->xHome, m->yHome, -2, -2, 100000, 300);
			int xTile, yTile;
			if (getHighestRomanSoldierConcentration(m->xHome, m->yHome, 16, &xTile, &yTile)) {
				romanDistance = 1;
			} else if (getHighestRomanSoldierConcentration(m->xHome, m->yHome, 32, &xTile, &yTile)) {
				romanDistance = 2;
			}
			if (Data_Formation_Invasion.ignoreRomanSoldiers[m->invasionId]) {
				romanDistance = 0;
			}
			if (romanDistance == 1) {
				// attack roman legion
				Data_Formation_Invasion.destinationX[m->invasionId] = xTile;
				Data_Formation_Invasion.destinationY[m->invasionId] = yTile;
				Data_Formation_Invasion.destinationBuildingId[m->invasionId] = 0;
			} else {
				setEnemyTargetBuilding(m);
				enemyApproachTarget(m);
				Data_Formation_Invasion.destinationX[m->invasionId] = m->destinationX;
				Data_Formation_Invasion.destinationY[m->invasionId] = m->destinationY;
				Data_Formation_Invasion.destinationBuildingId[m->invasionId] = m->destinationBuildingId;
			}
		}
		m->enemyLegionIndex = Data_Formation_Invasion.numLegions[m->invasionId]++;
		m->waitTicks++;
		m->destinationX = Data_Formation_Invasion.destinationX[m->invasionId];
		m->destinationY = Data_Formation_Invasion.destinationY[m->invasionId];
		m->destinationBuildingId = Data_Formation_Invasion.destinationBuildingId[m->invasionId];

		updateEnemyMovement(i, m, romanDistance);
	}

	setNativeTargetBuilding(0);
}

static int getHerdRoamingDestination(int formationId, int allowNegativeDesirability,
	int x, int y, int distance, int direction, int *xTile, int *yTile)
{
	int targetDirection = (formationId + Data_Random.random1_7bit) & 6;
	if (direction) {
		targetDirection = direction;
		allowNegativeDesirability = 1;
	}
	for (int i = 0; i < 4; i++) {
		int xTarget, yTarget;
		switch (targetDirection) {
			case Dir_0_Top:         xTarget = x; yTarget = y - distance; break;
			case Dir_1_TopRight:    xTarget = x + distance; yTarget = y - distance; break;
			case Dir_2_Right:       xTarget = x + distance; yTarget = y; break;
			case Dir_3_BottomRight: xTarget = x + distance; yTarget = y + distance; break;
			case Dir_4_Bottom:      xTarget = x; yTarget = y + distance; break;
			case Dir_5_BottomLeft:  xTarget = x - distance; yTarget = y + distance; break;
			case Dir_6_Left:        xTarget = x - distance; yTarget = y; break;
			case Dir_7_TopLeft:     xTarget = x - distance; yTarget = y - distance; break;
			default: continue;
		}
		if (xTarget <= 0) {
			xTarget = 1;
		} else if (yTarget <= 0) {
			yTarget = 1;
		} else if (xTarget >= Data_Settings_Map.width - 1) {
			xTarget = Data_Settings_Map.width - 2;
		} else if (yTarget >= Data_Settings_Map.height - 1) {
			yTarget = Data_Settings_Map.height - 2;
		}
		if (TerrainGraphics_getFreeTileForHerd(xTarget, yTarget, allowNegativeDesirability, xTile, yTile)) {
			return 1;
		}
		targetDirection += 2;
		if (targetDirection > 6) {
			targetDirection = 0;
		}
	}
	return 0;
}

static void moveAnimals(struct Data_Formation *m, int attackingAnimals)
{
	for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
		if (m->figureIds[i] <= 0) continue;
		int figureId = m->figureIds[i];
		struct Data_Figure *f = &Data_Figures[figureId];
		if (f->actionState == FigureActionState_149_Corpse ||
			f->actionState == FigureActionState_150_Attack) {
			continue;
		}
		f->waitTicks = 401;
		if (attackingAnimals) {
			int targetId = FigureAction_CombatWolf_getTarget(f->x, f->y, 6);
			if (targetId) {
				f->actionState = FigureActionState_199_WolfAttacking;
				f->destinationX = Data_Figures[targetId].x;
				f->destinationY = Data_Figures[targetId].y;
				f->targetFigureId = targetId;
				Data_Figures[targetId].targetedByFigureId = figureId;
				f->targetFigureCreatedSequence = Data_Figures[targetId].createdSequence;
				FigureRoute_remove(figureId);
			} else {
				f->actionState = FigureActionState_196_HerdAnimalAtRest;
			}
		} else {
			f->actionState = FigureActionState_196_HerdAnimalAtRest;
		}
	}
}

static void tickUpdateHerds()
{
	if (Data_CityInfo.numAnimalsInCity <= 0) {
		return;
	}
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *m = &Data_Formations[i];
		if (m->inUse != 1 || m->isLegion || !m->isHerd || m->numFigures <= 0) {
			continue;
		}
		if (m->numFigures < m->maxFigures && m->figureType == Figure_Wolf) {
			// spawn new wolf
			m->herdWolfSpawnDelay++;
			if (m->herdWolfSpawnDelay > 32) {
				m->herdWolfSpawnDelay = 0;
				if (!(Data_Grid_terrain[GridOffset(m->x, m->y)] & Terrain_d73f)) {
					int wolfId = Figure_create(m->figureType, m->x, m->y, Dir_0_Top);
					Data_Figures[wolfId].actionState = FigureActionState_196_HerdAnimalAtRest;
					Data_Figures[wolfId].formationId = i;
					Data_Figures[wolfId].waitTicks = wolfId & 0x1f;
				}
			}
		}
		int attackingAnimals = 0;
		for (int fig = 0; fig < MAX_FORMATION_FIGURES; fig++) {
			int figureId = m->figureIds[fig];
			if (figureId > 0 && Data_Figures[figureId].actionState == FigureActionState_150_Attack) {
				attackingAnimals++;
			}
		}
		if (m->missileAttackTimeout) {
			attackingAnimals = 1;
		}
		if (m->figureIds[0] && Data_Figures[m->figureIds[0]].state == FigureState_Alive) {
			m->xHome = Data_Figures[m->figureIds[0]].x;
			m->yHome = Data_Figures[m->figureIds[0]].y;
		}
		int roamDistance;
		int roamDelay;
		int allowNegativeDesirability;
		switch (m->figureType) {
			case Figure_Sheep:
				roamDistance = 8;
				roamDelay = 20;
				allowNegativeDesirability = 0;
				attackingAnimals = 0;
				break;
			case Figure_Zebra:
				roamDistance = 20;
				roamDelay = 4;
				allowNegativeDesirability = 0;
				attackingAnimals = 0;
				break;
			case Figure_Wolf:
				roamDistance = 16;
				roamDelay = 6;
				allowNegativeDesirability = 1;
				break;
			default: continue;
		}
		if (attackingAnimals) {
			m->waitTicks = roamDelay + 1;
		}
		m->waitTicks++;
		if (m->waitTicks > roamDelay) {
			m->waitTicks = 0;
			if (attackingAnimals) {
				m->destinationX = m->xHome;
				m->destinationY = m->yHome;
				moveAnimals(m, attackingAnimals);
			} else {
				int xTile, yTile;
				if (getHerdRoamingDestination(i, allowNegativeDesirability, m->xHome, m->yHome, roamDistance, m->herdDirection, &xTile, &yTile)) {
					m->herdDirection = 0;
					if (FigureAction_HerdEnemy_moveFormationTo(i, xTile, yTile, &xTile, &yTile)) {
						m->destinationX = xTile;
						m->destinationY = yTile;
						if (m->figureType == Figure_Wolf) {
							Data_CityInfo.soundMarchWolf--;
							if (Data_CityInfo.soundMarchWolf <= 0) {
								Data_CityInfo.soundMarchWolf = 12;
								Sound_Effects_playChannel(SoundChannel_WolfHowl);
							}
						}
						moveAnimals(m, attackingAnimals);
					}
				}
			}
		}
	}
}

void Formation_Tick_updateAll(int secondTime)
{
	Formation_calculateLegionTotals();
	Formation_calculateFigures();
	tickUpdateDirection();
	tickDecreaseLegionDamage();
	if (!secondTime) {
		tickUpdateMorale();
	}
	Formation_setMaxSoldierPerLegion();
	tickUpdateLegions();
	tickUpdateEnemies();
	tickUpdateHerds();
}

int Formation_Rioter_getTargetBuilding(int *xTile, int *yTile)
{
	int bestTypeIndex = 100;
	int buildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		int type = Data_Buildings[i].type;
		for (int b = 0; b < 100 && b <= bestTypeIndex && rioterAttackBuildingPriority[b]; b++) {
			if (type == rioterAttackBuildingPriority[b]) {
				if (b < bestTypeIndex) {
					bestTypeIndex = b;
					buildingId = i;
				}
				break;
			}
		}
	}
	if (buildingId <= 0) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type == Building_Warehouse) {
		*xTile = b->x + 1;
		*yTile = b->y;
		return buildingId + 1;
	} else {
		*xTile = b->x;
		*yTile = b->y;
		return buildingId;
	}
}
