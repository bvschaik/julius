#include "CityInfo.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Building.h"

#include "building/model.h"
#include "city/message.h"
#include "core/calc.h"
#include "game/time.h"
#include "scenario/property.h"

#define MAX_CATS 10

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

static struct {
	int category;
	int workers;
} builtInPriority[MAX_CATS] = {
	{LaborCategory_Engineering, 3},
	{LaborCategory_Water, 1},
	{LaborCategory_Prefectures, 3},
	{LaborCategory_Military, 2},
	{LaborCategory_FoodProduction, 4},
	{LaborCategory_IndustryCommerce, 2},
	{LaborCategory_Entertainment, 1},
	{LaborCategory_HealthEducation, 1},
	{LaborCategory_GovernanceReligion, 1},
};

static void setBuildingWorkerWeight();
static void allocateWorkersToWater();
static void allocateWorkersToBuildings();

static int isIndustryDisabled(int buildingId) {
	if (Data_Buildings[buildingId].type < BUILDING_WHEAT_FARM ||
		Data_Buildings[buildingId].type > BUILDING_POTTERY_WORKSHOP) {
		return 0;
	}
	int resourceId = Data_Buildings[buildingId].outputResourceId;
	if (Data_CityInfo.resourceIndustryMothballed[resourceId]) {
		return 1;
	}
	return 0;
}

static int shouldHaveWorkers(int buildingId, int category, int checkAccess)
{
	if (category < 0) {
		return 0;
	}

	// exceptions: hippodrome 'other' tiles and disabled industries
	if (category == LaborCategory_Entertainment) {
		if (Data_Buildings[buildingId].type == BUILDING_HIPPODROME && Data_Buildings[buildingId].prevPartBuildingId) {
			return 0;
		}
	} else if (category == LaborCategory_FoodProduction || category == LaborCategory_IndustryCommerce) {
		if (isIndustryDisabled(buildingId)) {
			return 0;
		}
	}
	// engineering and water are always covered
	if (category == LaborCategory_Engineering || category == LaborCategory_Water) {
		return 1;
	}
	if (checkAccess) {
		return Data_Buildings[buildingId].housesCovered > 0 ? 1 : 0;
	}
	return 1;
}

void CityInfo_Labor_calculateWorkersNeededPerCategory()
{
	for (int cat = 0; cat < MAX_CATS; cat++) {
		Data_CityInfo.laborCategory[cat].buildings = 0;
		Data_CityInfo.laborCategory[cat].totalHousesCovered = 0;
		Data_CityInfo.laborCategory[cat].workersAllocated = 0;
		Data_CityInfo.laborCategory[cat].workersNeeded = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		int category = buildingTypeToLaborCategory[Data_Buildings[i].type];
		Data_Buildings[i].laborCategory = category;
		if (!shouldHaveWorkers(i, category, 1)) {
			continue;
		}
		Data_CityInfo.laborCategory[category].workersNeeded +=
			model_get_building(Data_Buildings[i].type)->laborers;
		Data_CityInfo.laborCategory[category].totalHousesCovered +=
			Data_Buildings[i].housesCovered;
		Data_CityInfo.laborCategory[category].buildings++;
	}
}

void CityInfo_Labor_checkEmployment()
{
	int origNeeded = Data_CityInfo.workersNeeded;
	CityInfo_Labor_allocateWorkersToCategories();
	// senate unemployment display is delayed when unemployment is rising
	if (Data_CityInfo.unemploymentPercentage < Data_CityInfo.unemploymentPercentageForSenate) {
		Data_CityInfo.unemploymentPercentageForSenate = Data_CityInfo.unemploymentPercentage;
	} else if (Data_CityInfo.unemploymentPercentage < Data_CityInfo.unemploymentPercentageForSenate + 5) {
		Data_CityInfo.unemploymentPercentageForSenate = Data_CityInfo.unemploymentPercentage;
	} else {
		Data_CityInfo.unemploymentPercentageForSenate += 5;
	}
	if (Data_CityInfo.unemploymentPercentageForSenate > 100) {
		Data_CityInfo.unemploymentPercentageForSenate = 100;
	}

	// workers needed message
	if (!origNeeded && Data_CityInfo.workersNeeded > 0) {
		if (game_time_year() >= scenario_property_start_year()) {
			game.messages.post_with_message_delay(MESSAGE_CAT_WORKERS_NEEDED, 0, MESSAGE_WORKERS_NEEDED, 6);
		}
	}
}

void CityInfo_Labor_allocateWorkersToCategories()
{
	int workersNeeded = 0;
	for (int i = 0; i < MAX_CATS; i++) {
		Data_CityInfo.laborCategory[i].workersAllocated = 0;
		workersNeeded += Data_CityInfo.laborCategory[i].workersNeeded;
	}
	Data_CityInfo.workersNeeded = 0;
	if (workersNeeded <= Data_CityInfo.workersAvailable) {
		for (int i = 0; i < MAX_CATS; i++) {
			Data_CityInfo.laborCategory[i].workersAllocated = Data_CityInfo.laborCategory[i].workersNeeded;
		}
		Data_CityInfo.workersEmployed = workersNeeded;
	} else {
		// not enough workers
		int available = Data_CityInfo.workersAvailable;
		// distribute by user-defined priority
		for (int p = 1; p <= 9 && available > 0; p++) {
			for (int c = 0; c < 9; c++) {
				if (p == Data_CityInfo.laborCategory[c].priority) {
					int toAllocate = Data_CityInfo.laborCategory[c].workersNeeded;
					if (toAllocate > available) {
						toAllocate = available;
					}
					Data_CityInfo.laborCategory[c].workersAllocated = toAllocate;
					available -= toAllocate;
					break;
				}
			}
		}
		// (sort of) round-robin distribution over unprioritized categories:
		int guard = 0;
		do {
			guard++;
			if (guard >= Data_CityInfo.workersAvailable) {
				break;
			}
			for (int p = 0; p < 9; p++) {
				int cat = builtInPriority[p].category;
				if (!Data_CityInfo.laborCategory[cat].priority) {
					int needed = Data_CityInfo.laborCategory[cat].workersNeeded - Data_CityInfo.laborCategory[cat].workersAllocated;
					if (needed > 0) {
						int toAllocate = builtInPriority[p].workers;
						if (toAllocate > available) {
							toAllocate = available;
						}
						if (toAllocate > needed) {
							toAllocate = needed;
						}
						Data_CityInfo.laborCategory[cat].workersAllocated += toAllocate;
						available -= toAllocate;
						if (available <= 0) {
							break;
						}
					}
				}
			}
		} while (available > 0);

		Data_CityInfo.workersEmployed = Data_CityInfo.workersAvailable;
		for (int i = 0; i < 9; i++) {
			Data_CityInfo.workersNeeded +=
				Data_CityInfo.laborCategory[i].workersNeeded - Data_CityInfo.laborCategory[i].workersAllocated;
		}
	}
	Data_CityInfo.workersUnemployed = Data_CityInfo.workersAvailable - Data_CityInfo.workersEmployed;
	Data_CityInfo.unemploymentPercentage =
		calc_percentage(Data_CityInfo.workersUnemployed, Data_CityInfo.workersAvailable);
}

void CityInfo_Labor_allocateWorkersToBuildings()
{
	setBuildingWorkerWeight();
	allocateWorkersToWater();
	allocateWorkersToBuildings();
}

static void setBuildingWorkerWeight()
{
	int waterPer10kPerBuilding = calc_percentage(100, Data_CityInfo.laborCategory[LaborCategory_Water].buildings);
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		int cat = buildingTypeToLaborCategory[Data_Buildings[i].type];
		if (cat == LaborCategory_Water) {
			Data_Buildings[i].percentageHousesCovered = waterPer10kPerBuilding;
		} else if (cat >= 0) {
			Data_Buildings[i].percentageHousesCovered = 0;
			if (Data_Buildings[i].housesCovered) {
				Data_Buildings[i].percentageHousesCovered =
					calc_percentage(100 * Data_Buildings[i].housesCovered,
						Data_CityInfo.laborCategory[cat].totalHousesCovered);
			}
		}
	}
}

static void allocateWorkersToBuildings()
{
	int categoryWorkersNeeded[MAX_CATS];
	int categoryWorkersAllocated[MAX_CATS];
	for (int i = 0; i < MAX_CATS; i++) {
		categoryWorkersAllocated[i] = 0;
		categoryWorkersNeeded[i] =
			Data_CityInfo.laborCategory[i].workersAllocated < Data_CityInfo.laborCategory[i].workersNeeded
			? 1 : 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = &Data_Buildings[i];
		int cat = buildingTypeToLaborCategory[b->type];
		if (cat == LaborCategory_Water || cat < 0) {
			// water is handled by allocateWorkersToWater()
			continue;
		}
		b->numWorkers = 0;
		if (!shouldHaveWorkers(i, cat, 0)) {
			continue;
		}
		if (b->percentageHousesCovered > 0) {
            int requiredWorkers = model_get_building(b->type)->laborers;
			if (categoryWorkersNeeded[cat]) {
				int numWorkers = calc_adjust_with_percentage(
					Data_CityInfo.laborCategory[cat].workersAllocated,
					b->percentageHousesCovered) / 100;
				if (numWorkers > requiredWorkers) {
					numWorkers = requiredWorkers;
				}
				b->numWorkers = numWorkers;
				categoryWorkersAllocated[cat] += numWorkers;
			} else {
				b->numWorkers = requiredWorkers;
			}
		}
	}
	for (int i = 0; i < MAX_CATS; i++) {
		if (categoryWorkersNeeded[i]) {
			// watch out: categoryWorkersNeeded is now reset to 'unallocated workers available'
			if (categoryWorkersAllocated[i] >= Data_CityInfo.laborCategory[i].workersAllocated) {
				categoryWorkersNeeded[i] = 0;
				categoryWorkersAllocated[i] = 0;
			} else {
				categoryWorkersNeeded[i] = Data_CityInfo.laborCategory[i].workersAllocated - categoryWorkersAllocated[i];
			}
		}
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (!BuildingIsInUse(i)) {
			continue;
		}
		struct Data_Building *b = &Data_Buildings[i];
		int cat = buildingTypeToLaborCategory[b->type];
		if (cat < 0 || cat == LaborCategory_Water || cat == LaborCategory_Military) {
			continue;
		}
		if (!shouldHaveWorkers(i, cat, 0)) {
			continue;
		}
		if (b->percentageHousesCovered > 0 && categoryWorkersNeeded[cat]) {
            int requiredWorkers = model_get_building(b->type)->laborers;
			if (b->numWorkers < requiredWorkers) {
				int needed = requiredWorkers - b->numWorkers;
				if (needed > categoryWorkersNeeded[cat]) {
					b->numWorkers += categoryWorkersNeeded[cat];
					categoryWorkersNeeded[cat] = 0;
				} else {
					b->numWorkers += needed;
					categoryWorkersNeeded[cat] -= needed;
				}
			}
		}
	}
}

static void allocateWorkersToWater()
{
	static int startBuildingId = 1;

	int percentageNotFilled = 100 - calc_percentage(
		Data_CityInfo.laborCategory[LaborCategory_Water].workersAllocated,
		Data_CityInfo.laborCategory[LaborCategory_Water].workersNeeded);

	int buildingsToSkip = calc_adjust_with_percentage(
		Data_CityInfo.laborCategory[LaborCategory_Water].buildings, percentageNotFilled);

	int workersPerBuilding;
	if (buildingsToSkip == Data_CityInfo.laborCategory[LaborCategory_Water].buildings) {
		workersPerBuilding = 1;
	} else {
		workersPerBuilding = Data_CityInfo.laborCategory[LaborCategory_Water].workersAllocated /
			(Data_CityInfo.laborCategory[LaborCategory_Water].buildings - buildingsToSkip);
	}
	int buildingId = startBuildingId;
	startBuildingId = 0;
	for (int guard = 1; guard < MAX_BUILDINGS; guard++, buildingId++) {
		if (buildingId >= 2000) {
			buildingId = 1;
		}
		if (!BuildingIsInUse(buildingId) ||
			buildingTypeToLaborCategory[Data_Buildings[buildingId].type] != LaborCategory_Water) {
			continue;
		}
		Data_Buildings[buildingId].numWorkers = 0;
		if (Data_Buildings[buildingId].percentageHousesCovered > 0) {
			if (percentageNotFilled > 0) {
				if (buildingsToSkip) {
					--buildingsToSkip;
				} else if (startBuildingId) {
					Data_Buildings[buildingId].numWorkers = workersPerBuilding;
				} else {
					startBuildingId = buildingId;
					Data_Buildings[buildingId].numWorkers = workersPerBuilding;
				}
			} else {
				Data_Buildings[buildingId].numWorkers =
					model_get_building(Data_Buildings[buildingId].type)->laborers;
			}
		}
	}
	if (!startBuildingId) {
		// no buildings assigned or full employment
		startBuildingId = 1;
	}
}

void CityInfo_Labor_update()
{
	CityInfo_Labor_calculateWorkersNeededPerCategory();
	CityInfo_Labor_checkEmployment();
	CityInfo_Labor_allocateWorkersToBuildings();
}
