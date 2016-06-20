#include "Formation.h"

#include "Calc.h"
#include "Figure.h"
#include "Grid.h"
#include "PlayerMessage.h"
#include "Routing.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "WalkerAction.h"

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
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state == FigureState_Alive && WalkerIsLegion(w->type)) {
			if (w->actionState == FigureActionState_80_SoldierAtRest) {
				if (w->damage) {
					w->damage--;
				}
			}
		}
	}
}

static void tickUpdateMorale()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isHerd) {
			continue;
		}
		if (f->isLegion) {
			if (!f->isAtFort && !f->inDistantBattle) {
				if (f->morale <= 20 && !f->monthsLowMorale && !f->monthsVeryLowMorale) {
					changeMoraleOfAllLegions(-10);
					changeMoraleOfAllEnemies(10);
				}
				if (f->morale <= 10) {
					f->monthsVeryLowMorale++;
				} else if (f->morale <= 20) {
					f->monthsLowMorale++;
				}
			}
		} else { // enemy
			if (f->morale <= 20 && !f->monthsLowMorale && !f->monthsVeryLowMorale) {
				changeMoraleOfAllLegions(10);
				changeMoraleOfAllEnemies(-10);
			}
			if (f->morale <= 10) {
				f->monthsVeryLowMorale++;
			} else if (f->morale <= 20) {
				f->monthsLowMorale++;
			}
		}
	}
}

static void tickUpdateDirection()
{
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isHerd) {
			continue;
		}
		if (f->__unknown66) {
			f->__unknown66--;
		} else if (f->missileFired) {
			f->direction = Data_Walkers[f->figureIds[0]].direction;
		} else if (f->layout == FormationLayout_DoubleLine1 || f->layout == FormationLayout_SingleLine1) {
			if (f->yHome < f->prevYHome) {
				f->direction = Dir_0_Top;
			} else if (f->yHome > f->prevYHome) {
				f->direction = Dir_4_Bottom;
			}
		} else if (f->layout == FormationLayout_DoubleLine2 || f->layout == FormationLayout_SingleLine2) {
			if (f->xHome < f->prevXHome) {
				f->direction = Dir_6_Left;
			} else if (f->xHome > f->prevXHome) {
				f->direction = Dir_2_Right;
			}
		} else if (f->layout == FormationLayout_Tortoise || f->layout == FormationLayout_Column) {
			int dx = (f->xHome < f->prevXHome) ? (f->prevXHome - f->xHome) : (f->xHome - f->prevXHome);
			int dy = (f->yHome < f->prevYHome) ? (f->prevYHome - f->yHome) : (f->yHome - f->prevYHome);
			if (dx > dy) {
				if (f->xHome < f->prevXHome) {
					f->direction = Dir_6_Left;
				} else if (f->xHome > f->prevXHome) {
					f->direction = Dir_2_Right;
				}
			} else {
				if (f->yHome < f->prevYHome) {
					f->direction = Dir_0_Top;
				} else if (f->yHome > f->prevYHome) {
					f->direction = Dir_4_Bottom;
				}
			}
		}
		f->prevXHome = f->xHome;
		f->prevYHome = f->yHome;
	}
}

static void tickUpdateLegions()
{
	for (int i = 1; i <= 6; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || !f->isLegion) {
			continue;
		}
		if (f->cursedByMars) {
			f->cursedByMars--;
		}
		if (f->missileFired) {
			f->missileFired--;
		}
		if (f->missileAttackTimeout) {
			f->missileAttackTimeout--;
		}
		if (f->recentFight) {
			f->recentFight--;
		}
		if (Data_CityInfo.numEnemiesInCity <= 0) {
			f->recentFight = 0;
			f->missileAttackTimeout = 0;
			f->missileFired = 0;
		}
		for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
			if (Data_Walkers[f->figureIds[n]].actionState == FigureActionState_150_Attack) {
				f->recentFight = 6;
			}
		}
		if (f->monthsLowMorale || f->monthsVeryLowMorale) {
			// flee back to fort
			for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
				struct Data_Walker *w = &Data_Walkers[f->figureIds[n]];
				if (w->actionState != FigureActionState_150_Attack &&
					w->actionState != FigureActionState_149_Corpse &&
					w->actionState != FigureActionState_148_Fleeing) {
					w->actionState = FigureActionState_148_Fleeing;
					FigureRoute_remove(f->figureIds[n]);
				}
			}
		} else if (f->layout == FormationLayout_MopUp) {
			if (Data_Formation_Extra.numEnemyFormations +
				Data_CityInfo.numRiotersInCity +
				Data_CityInfo.numAttackingNativesInCity > 0) {
				for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
					struct Data_Walker *w = &Data_Walkers[f->figureIds[n]];
					if (f->figureIds[n] != 0 &&
						w->actionState != FigureActionState_150_Attack &&
						w->actionState != FigureActionState_149_Corpse) {
						w->actionState = FigureActionState_86_SoldierMoppingUp;
					}
				}
			} else {
				f->layout = f->layoutBeforeMopUp;
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
				int type = Data_Walkers[Data_Grid_figureIds[gridOffset]].type;
				if (WalkerIsLegion(type)) {
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
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || !f->isLegion) {
			continue;
		}
		if (f->numFigures > 0) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 7, 1);
		}
		if (f->numFigures > 3) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 6, 1);
		}
		if (f->numFigures > 6) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 5, 1);
		}
		if (f->numFigures > 9) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 4, 1);
		}
		if (f->numFigures > 12) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 3, 1);
		}
		if (f->numFigures > 15) {
			addRomanSoldierConcentration(f->xHome, f->yHome, 2, 1);
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
				int distance = Calc_distanceMaximum(
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

static void setEnemyTargetBuilding(struct Data_Formation *f)
{
	int attack = f->attackType;
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
				int distance = Calc_distanceMaximum(f->xHome, f->yHome, b->x, b->y);
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
					int distance = Calc_distanceMaximum(f->xHome, f->yHome, b->x, b->y);
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
		f->destinationX = b->x + 1;
		f->destinationY = b->y;
		f->destinationBuildingId = buildingId + 1;
	} else {
		f->destinationX = b->x;
		f->destinationY = b->y;
		f->destinationBuildingId = buildingId;
	}
}

static void enemyApproachTarget(struct Data_Formation *f)
{
	if (Routing_canTravelOverLandNonCitizen(f->xHome, f->yHome,
			f->destinationX, f->destinationY, f->destinationBuildingId, 400) ||
		Routing_canTravelThroughEverythingNonCitizen(f->xHome, f->yHome,
			f->destinationX, f->destinationY)) {
		int xTile, yTile;
		if (Routing_getClosestXYWithinRange(8, f->xHome, f->yHome,
				f->destinationX, f->destinationY, 20, &xTile, &yTile)) {
			f->destinationX = xTile;
			f->destinationY = yTile;
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
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state != FigureState_Alive) {
			continue;
		}
		if (WalkerIsEnemy(w->type) && w->type != Figure_Enemy54_Gladiator) {
			w->actionState = FigureActionState_149_Corpse;
			toKill--;
			if (!gridOffset) {
				gridOffset = w->gridOffset;
			}
		}
	}
	Data_CityInfo.godBlessingMarsEnemiesToKill = 0;
	PlayerMessage_post(1, Message_105_SpiritOfMars, 0, gridOffset);
}

static void setFormationWalkersToEnemyInitial(int formationId)
{
	struct Data_Formation *f = &Data_Formations[formationId];
	for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
		if (f->figureIds[i] > 0) {
			struct Data_Walker *w = &Data_Walkers[f->figureIds[i]];
			if (w->actionState != FigureActionState_149_Corpse &&
				w->actionState != FigureActionState_150_Attack) {
				w->actionState = FigureActionState_151_EnemyInitial;
				w->waitTicks = 0;
			}
		}
	}
}

static void updateEnemyMovement(int formationId, struct Data_Formation *f, int romanDistance)
{
	int regroup = 0;
	int halt = 0;
	int pursueTarget = 0;
	int advance = 0;
	int targetFormationId = 0;
	if (f->missileFired) {
		halt = 1;
	} else if (f->missileAttackTimeout) {
		pursueTarget = 1;
		targetFormationId = f->missileAttackFormationId;
	} else if (f->waitTicks < 32) {
		regroup = 1;
		f->durationAdvance = 4;
	} else if (Data_Formation_Invasion.ignoreRomanSoldiers[f->invasionId]) {
		halt = 0;
		regroup = 0;
		advance = 1;
	} else {
		int haltDuration, advanceDuration, regroupDuration;
		if (Data_Formation_Invasion.layout[f->invasionId] == FormationLayout_Enemy8 ||
			Data_Formation_Invasion.layout[f->invasionId] == FormationLayout_Enemy12) {
			switch (f->enemyLegionIndex) {
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
		if (f->durationHalt) {
			f->durationAdvance = 0;
			f->durationRegroup = 0;
			halt = 1;
			f->durationHalt--;
			if (f->durationHalt <= 0) {
				f->durationRegroup = regroupDuration;
				setFormationWalkersToEnemyInitial(formationId);
				regroup = 0;
				halt = 1;
			}
		} else if (f->durationRegroup) {
			f->durationAdvance = 0;
			f->durationHalt = 0;
			regroup = 1;
			f->durationRegroup--;
			if (f->durationRegroup <= 0) {
				f->durationAdvance = advanceDuration;
				setFormationWalkersToEnemyInitial(formationId);
				advance = 1;
				regroup = 0;
			}
		} else {
			f->durationRegroup = 0;
			f->durationHalt = 0;
			advance = 1;
			f->durationAdvance--;
			if (f->durationAdvance <= 0) {
				f->durationHalt = haltDuration;
				setFormationWalkersToEnemyInitial(formationId);
				halt = 1;
				advance = 0;
			}
		}
	}

	if (f->waitTicks > 32) {
		marsKillEnemies();
	}
	if (halt) {
		f->destinationX = f->xHome;
		f->destinationY = f->yHome;
	} else if (pursueTarget) {
		if (targetFormationId > 0) {
			if (Data_Formations[targetFormationId].numFigures > 0) {
				f->destinationX = Data_Formations[targetFormationId].xHome;
				f->destinationY = Data_Formations[targetFormationId].yHome;
			}
		} else {
			f->destinationX = Data_Formation_Invasion.destinationX[f->invasionId];
			f->destinationY = Data_Formation_Invasion.destinationY[f->invasionId];
		}
	} else if (regroup) {
		int layout = Data_Formation_Invasion.layout[f->invasionId];
		int xOffset = layoutOrientationLegionIndexOffsets[layout][f->orientation / 2][2 * f->enemyLegionIndex] +
			Data_Formation_Invasion.homeX[f->invasionId];
		int yOffset = layoutOrientationLegionIndexOffsets[layout][f->orientation / 2][2 * f->enemyLegionIndex + 1] +
			Data_Formation_Invasion.homeY[f->invasionId];
		int xTile, yTile;
		if (FigureAction_HerdEnemy_moveFormationTo(formationId, xOffset, yOffset, &xTile, &yTile)) {
			f->destinationX = xTile;
			f->destinationY = yTile;
		}
	} else if (advance) {
		int layout = Data_Formation_Invasion.layout[f->invasionId];
		int xOffset = layoutOrientationLegionIndexOffsets[layout][f->orientation / 2][2 * f->enemyLegionIndex] +
			Data_Formation_Invasion.destinationX[f->invasionId];
		int yOffset = layoutOrientationLegionIndexOffsets[layout][f->orientation / 2][2 * f->enemyLegionIndex + 1] +
			Data_Formation_Invasion.destinationY[f->invasionId];
		int xTile, yTile;
		if (FigureAction_HerdEnemy_moveFormationTo(formationId, xOffset, yOffset, &xTile, &yTile)) {
			f->destinationX = xTile;
			f->destinationY = yTile;
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
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isLegion || f->isHerd) {
			continue;
		}
		if (Data_Formation_Extra.numEnemySoldierStrength > 2 * Data_Formation_Extra.numLegionSoldierStrength) {
			if (f->figureType != Figure_FortJavelin) {
				Data_Formation_Invasion.ignoreRomanSoldiers[f->invasionId] = 1;
			}
		}
		if (f->missileFired) {
			f->missileFired--;
		}
		if (f->missileAttackTimeout) {
			f->missileAttackTimeout--;
		}
		if (f->recentFight) {
			f->recentFight--;
		}
		if (Data_CityInfo.numSoldiersInCity <= 0) {
			f->recentFight = 0;
			f->missileAttackTimeout = 0;
			f->missileFired = 0;
		}
		for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
			int walkerId = f->figureIds[n];
			if (Data_Walkers[walkerId].actionState == FigureActionState_150_Attack) {
				int opponentId = Data_Walkers[walkerId].opponentId;
				if (!FigureIsDead(opponentId) && WalkerIsLegion(Data_Walkers[opponentId].type)) {
					f->recentFight = 6;
				}
			}
		}
		if (f->monthsLowMorale || f->monthsVeryLowMorale) {
			for (int n = 0; n < MAX_FORMATION_FIGURES; n++) {
				struct Data_Walker *w = &Data_Walkers[f->figureIds[n]];
				if (w->actionState != FigureActionState_150_Attack &&
					w->actionState != FigureActionState_149_Corpse &&
					w->actionState != FigureActionState_148_Fleeing) {
					w->actionState = FigureActionState_148_Fleeing;
					FigureRoute_remove(f->figureIds[n]);
				}
			}
			continue;
		}
		if (f->figureIds[0] && Data_Walkers[f->figureIds[0]].state == FigureState_Alive) {
			f->xHome = Data_Walkers[f->figureIds[0]].x;
			f->yHome = Data_Walkers[f->figureIds[0]].y;
		}
		if (!Data_Formation_Invasion.formationId[f->invasionId]) {
			Data_Formation_Invasion.formationId[f->invasionId] = i;
			Data_Formation_Invasion.homeX[f->invasionId] = f->xHome;
			Data_Formation_Invasion.homeY[f->invasionId] = f->yHome;
			Data_Formation_Invasion.layout[f->invasionId] = f->layout;
			romanDistance = 0;
			Routing_canTravelOverLandNonCitizen(f->xHome, f->yHome, -2, -2, 100000, 300);
			int xTile, yTile;
			if (getHighestRomanSoldierConcentration(f->xHome, f->yHome, 16, &xTile, &yTile)) {
				romanDistance = 1;
			} else if (getHighestRomanSoldierConcentration(f->xHome, f->yHome, 32, &xTile, &yTile)) {
				romanDistance = 2;
			}
			if (Data_Formation_Invasion.ignoreRomanSoldiers[f->invasionId]) {
				romanDistance = 0;
			}
			if (romanDistance == 1) {
				// attack roman legion
				Data_Formation_Invasion.destinationX[f->invasionId] = xTile;
				Data_Formation_Invasion.destinationY[f->invasionId] = yTile;
				Data_Formation_Invasion.destinationBuildingId[f->invasionId] = 0;
			} else {
				setEnemyTargetBuilding(f);
				enemyApproachTarget(f);
				Data_Formation_Invasion.destinationX[f->invasionId] = f->destinationX;
				Data_Formation_Invasion.destinationY[f->invasionId] = f->destinationY;
				Data_Formation_Invasion.destinationBuildingId[f->invasionId] = f->destinationBuildingId;
			}
		}
		f->enemyLegionIndex = Data_Formation_Invasion.numLegions[f->invasionId]++;
		f->waitTicks++;
		f->destinationX = Data_Formation_Invasion.destinationX[f->invasionId];
		f->destinationY = Data_Formation_Invasion.destinationY[f->invasionId];
		f->destinationBuildingId = Data_Formation_Invasion.destinationBuildingId[f->invasionId];

		updateEnemyMovement(i, f, romanDistance);
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

static void moveAnimals(struct Data_Formation *f, int attackingAnimals)
{
	for (int i = 0; i < MAX_FORMATION_FIGURES; i++) {
		if (f->figureIds[i] <= 0) continue;
		int walkerId = f->figureIds[i];
		struct Data_Walker *w = &Data_Walkers[walkerId];
		if (w->actionState == FigureActionState_149_Corpse ||
			w->actionState == FigureActionState_150_Attack) {
			continue;
		}
		w->waitTicks = 401;
		if (attackingAnimals) {
			int targetId = FigureAction_CombatWolf_getTarget(w->x, w->y, 6);
			if (targetId) {
				w->actionState = FigureActionState_199_WolfAttacking;
				w->destinationX = Data_Walkers[targetId].x;
				w->destinationY = Data_Walkers[targetId].y;
				w->targetWalkerId = targetId;
				Data_Walkers[targetId].targetedByWalkerId = walkerId;
				w->targetWalkerCreatedSequence = Data_Walkers[targetId].createdSequence;
				FigureRoute_remove(walkerId);
			} else {
				w->actionState = FigureActionState_196_HerdAnimalAtRest;
			}
		} else {
			w->actionState = FigureActionState_196_HerdAnimalAtRest;
		}
	}
}

static void tickUpdateHerds()
{
	if (Data_CityInfo.numAnimalsInCity <= 0) {
		return;
	}
	for (int i = 1; i < MAX_FORMATIONS; i++) {
		struct Data_Formation *f = &Data_Formations[i];
		if (f->inUse != 1 || f->isLegion || !f->isHerd || f->numFigures <= 0) {
			continue;
		}
		if (f->numFigures < f->maxFigures && f->figureType == Figure_Wolf) {
			// spawn new wolf
			f->herdWolfSpawnDelay++;
			if (f->herdWolfSpawnDelay > 32) {
				f->herdWolfSpawnDelay = 0;
				if (!(Data_Grid_terrain[GridOffset(f->x, f->y)] & Terrain_d73f)) {
					int wolfId = Figure_create(f->figureType, f->x, f->y, Dir_0_Top);
					Data_Walkers[wolfId].actionState = FigureActionState_196_HerdAnimalAtRest;
					Data_Walkers[wolfId].formationId = i;
					Data_Walkers[wolfId].waitTicks = wolfId & 0x1f;
				}
			}
		}
		int attackingAnimals = 0;
		for (int w = 0; w < MAX_FORMATION_FIGURES; w++) {
			int walkerId = f->figureIds[i];
			if (walkerId > 0 && Data_Walkers[walkerId].actionState == FigureActionState_150_Attack) {
				attackingAnimals++;
			}
		}
		if (f->missileAttackTimeout) {
			attackingAnimals = 1;
		}
		if (f->figureIds[0] && Data_Walkers[f->figureIds[0]].state == FigureState_Alive) {
			f->xHome = Data_Walkers[f->figureIds[0]].x;
			f->yHome = Data_Walkers[f->figureIds[0]].y;
		}
		int roamDistance;
		int roamDelay;
		int allowNegativeDesirability;
		switch (f->figureType) {
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
			f->waitTicks = roamDelay + 1;
		}
		f->waitTicks++;
		if (f->waitTicks > roamDelay) {
			f->waitTicks = 0;
			if (attackingAnimals) {
				f->destinationX = f->xHome;
				f->destinationY = f->yHome;
				moveAnimals(f, attackingAnimals);
			} else {
				int xTile, yTile;
				if (getHerdRoamingDestination(i, allowNegativeDesirability, f->xHome, f->yHome, roamDistance, f->herdDirection, &xTile, &yTile)) {
					f->herdDirection = 0;
					if (FigureAction_HerdEnemy_moveFormationTo(i, xTile, yTile, &xTile, &yTile)) {
						f->destinationX = xTile;
						f->destinationY = yTile;
						if (f->figureType == Figure_Wolf) {
							Data_CityInfo.soundMarchWolf--;
							if (Data_CityInfo.soundMarchWolf <= 0) {
								Data_CityInfo.soundMarchWolf = 12;
								Sound_Effects_playChannel(SoundChannel_WolfHowl);
							}
						}
						moveAnimals(f, attackingAnimals);
					}
				}
			}
		}
	}
}

void Formation_Tick_updateAll(int secondTime)
{
	Formation_calculateLegionTotals();
	Formation_calculateWalkers();
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
