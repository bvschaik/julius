#include "CityInfoUpdater.h"
#include "Calc.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Building.h"
#include "Data/Model.h"

static int buildingTypeToLaborCategory[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//0
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//10
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,//20
	6, 6, 6, 6, 6, 6, 6, 6, -1, -1,//30
	-1, -1, -1, -1, -1, -1, 7, 7, 7, 7,//40
	0, 7, 7, 7, -1, 4, -1, 5, 5, 5,//50
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,//60
	0, 1, 0, -1, 0, 0, 0, -1, -1, -1,//70
	7, 2, -1, -1, 8, 8, 8, 8, -1, -1,//80
	-1, 3, -1, -1, 5, 5, -1, -1, 8, -1,//90
	1, 1, 1, 0, 0, 1, 0, 0, 0, 0,//100
	0, 0, 0, 0, 0, -1, -1, -1, -1, -1,//110
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1//120
};

static void setWorkerPercentages();
static void allocateWorkersToWater();
static void allocateWorkersToBuildings();


static int isIndustryDisabled(int buildingId) {
	if (Data_Buildings[buildingId].type < Building_WheatFarm) {
		return 0;
	}
	if (Data_Buildings[buildingId].type > Building_PotteryWorkshop) {
		return 0;
	}
	if (Data_CityInfo.resourceIndustryMothballed[Data_Buildings[buildingId].outputResourceId]) {
		return 1;
	}
	return 0;
}

void CityInfoUpdater_Labor_calculateWorkersNeededPerCategory()
{
	for (int cat = 0; cat < 10; cat++) {
		Data_CityInfo.laborCategory[cat].buildings = 0;
		Data_CityInfo.laborCategory[cat].totalHousesCovered = 0;
		Data_CityInfo.laborCategory[cat].workersAllocated = 0;
		Data_CityInfo.laborCategory[cat].workersNeeded = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1) {
			continue;
		}
		int category = buildingTypeToLaborCategory[Data_Buildings[i].type];
		Data_Buildings[i].laborCategory = category;
		if (category < 0) {
			continue;
		}
		if (category == LaborCategory_Entertainment) {
			if (Data_Buildings[i].type == Building_Hippodrome && Data_Buildings[i].prevPartBuildingId) {
				continue;
			}
			if (!Data_Buildings[i].housesCovered) {
				continue;
			}
		} else if (category == LaborCategory_FoodProduction || category == LaborCategory_IndustryCommerce) {
			if (isIndustryDisabled(i)) {
				continue;
			}
		}
		Data_CityInfo.laborCategory[category].workersNeeded +=
			Data_Model_Buildings[Data_Buildings[i].type].laborers;
		Data_CityInfo.laborCategory[category].totalHousesCovered +=
			Data_Buildings[i].housesCovered;
		Data_CityInfo.laborCategory[category].buildings++;
	}
}

void CityInfoUpdater_Labor_calculateUnemployment()
{
	// TODO
}

void CityInfoUpdater_Labor_allocateWorkersToBuildings()
{
	setWorkerPercentages();
	allocateWorkersToWater();
	allocateWorkersToBuildings();
}

static void setWorkerPercentages()
{
	int waterPct = Calc_getPercentage(100, Data_CityInfo.laborCategory[LaborCategory_Water].priority);
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1) {
			continue;
		}
		int cat = buildingTypeToLaborCategory[Data_Buildings[i].type];
		if (cat == LaborCategory_Water) {
			Data_Buildings[i].percentageWorkers = waterPct;
		} else if (cat >= 0) {
			Data_Buildings[i].percentageWorkers = 0;
			if (Data_Buildings[i].housesCovered) {
				Data_Buildings[i].percentageWorkers =
					Calc_getPercentage(100 * Data_Buildings[i].housesCovered,
						Data_CityInfo.laborCategory[cat].totalHousesCovered);
			}
		}
	}
}

static void allocateWorkersToWater()
{
	// TODO
}

static void allocateWorkersToBuildings()
{
	// TODO
}
