#include "CityInfo.h"

#include "Calc.h"
#include "Empire.h"
#include "PlayerMessage.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "UI/AllWindows.h"
#include "UI/Window.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Model.h"
#include "Data/Scenario.h"
#include "Data/Tutorial.h"

void CityInfo_Resource_calculateAvailableResources()
{
	for (int i = 0; i < 16; i++) {
		Data_CityInfo_Resource.availableResources[i] = 0;
		Data_CityInfo_Resource.availableFoods[i] = 0;
	}
	Data_CityInfo_Resource.numAvailableResources = 0;
	Data_CityInfo_Resource.numAvailableFoods = 0;

	for (int i = 0; i < 16; i++) {
		if (Empire_ourCityCanProduceResource(i) || Empire_canImportResource(i) ||
			(i == Resource_Meat && Data_Scenario.allowedBuildings.wharf)) {
			Data_CityInfo_Resource.availableResources[Data_CityInfo_Resource.numAvailableResources++] = i;
		}
	}
	for (int i = 0; i <= Resource_Meat; i++) {
		if (i == Resource_Olives || i == Resource_Vines) {
			continue;
		}
		if (Empire_ourCityCanProduceResource(i) || Empire_canImportResource(i) ||
			(i == Resource_Meat && Data_Scenario.allowedBuildings.wharf)) {
			Data_CityInfo_Resource.availableFoods[Data_CityInfo_Resource.numAvailableFoods++] = i;
		}
	}
}

void CityInfo_Resource_calculateFood()
{
	for (int i = 0; i < 7; i++) {
		Data_CityInfo.resourceGranaryFoodStored[i] = 0;
	}
	Data_CityInfo.foodInfoFoodStoredInGranaries = 0;
	Data_CityInfo.foodInfoFoodTypesAvailable = 0;
	Data_CityInfo.foodInfoFoodSupplyMonths = 0;
	Data_CityInfo.foodInfoGranariesOperating = 0;
	Data_CityInfo.foodInfoGranariesUnderstaffed = 0;
	Data_CityInfo.foodInfoGranariesNotOperating = 0;
	Data_CityInfo.foodInfoGranariesNotOperatingWithFood = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse != 1 || b->type != Building_Granary) {
			continue;
		}
		b->hasRoadAccess = 0;
		if (Terrain_hasRoadAccessGranary(b->x, b->y, 0, 0)) {
			b->hasRoadAccess = 1;
			int pctWorkers = Calc_getPercentage(
				b->numWorkers, Data_Model_Buildings[b->type].laborers);
			if (pctWorkers < 100) {
				Data_CityInfo.foodInfoGranariesUnderstaffed++;
			}
			int amountStored = 0;
			for (int r = 1; r < 7; r++) {
				amountStored += b->data.storage.resourceStored[r];
			}
			if (pctWorkers < 50) {
				Data_CityInfo.foodInfoGranariesNotOperating++;
				if (amountStored > 0) {
					Data_CityInfo.foodInfoGranariesNotOperatingWithFood++;
				}
			} else {
				Data_CityInfo.foodInfoGranariesOperating++;
				for (int r = 0; r < 7; r++) {
					Data_CityInfo.resourceGranaryFoodStored[r] +=
						b->data.storage.resourceStored[r];
				}
				if (amountStored > 400 && !Data_Tutorial.tutorial2.granaryBuilt) {
					Data_Tutorial.tutorial2.granaryBuilt = 1;
					SidebarMenu_enableBuildingMenuItems();
					SidebarMenu_enableBuildingButtons();
					if (UI_Window_getId() == Window_City) {
						UI_City_drawBackground();
					}
					PlayerMessage_post(1, 56, 0, 0);
				}
			}
		}
	}
	for (int i = 1; i < 7; i++) {
		if (Data_CityInfo.resourceGranaryFoodStored[i]) {
			Data_CityInfo.foodInfoFoodStoredInGranaries +=
				Data_CityInfo.resourceGranaryFoodStored[i];
			Data_CityInfo.foodInfoFoodTypesAvailable++;
		}
	}
	Data_CityInfo.foodInfoFoodNeededPerMonth =
		Calc_adjustWithPercentage(Data_CityInfo.population, 50);
	if (Data_CityInfo.foodInfoFoodNeededPerMonth > 0) {
		Data_CityInfo.foodInfoFoodSupplyMonths =
			Data_CityInfo.foodInfoFoodStoredInGranaries / Data_CityInfo.foodInfoFoodNeededPerMonth;
	} else {
		Data_CityInfo.foodInfoFoodSupplyMonths =
			Data_CityInfo.foodInfoFoodStoredInGranaries > 0 ? 1 : 0;
	}
	if (Data_Scenario.romeSuppliesWheat) {
		Data_CityInfo.foodInfoFoodTypesAvailable = 1;
		Data_CityInfo.foodInfoFoodSupplyMonths = 12;
	}
}

void CityInfo_Resource_calculateFoodAndSupplyRomeWheat()
{
	CityInfo_Resource_calculateFood();
	if (Data_Scenario.romeSuppliesWheat) {
		for (int i = 1; i < MAX_BUILDINGS; i++) {
			if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].type == Building_Market) {
				Data_Buildings[i].data.market.food[0] = 200;
			}
		}
	}
}

void CityInfo_Resource_housesConsumeFood()
{
	CityInfo_Resource_calculateFood();
	Data_CityInfo.foodInfoFoodTypesEaten = 0;
	Data_CityInfo.__unknown_00c0 = 0;
	int totalConsumed = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->inUse == 1 && b->houseSize) {
			int numTypes = Data_Model_Houses[b->subtype.houseLevel].foodTypes;
			int amountPerType = Calc_adjustWithPercentage(b->housePopulation, 50);
			if (numTypes > 1) {
				amountPerType /= numTypes;
			}
			b->data.house.numFoods = 0;
			if (Data_Scenario.romeSuppliesWheat) {
				Data_CityInfo.foodInfoFoodTypesEaten = 1;
				Data_CityInfo.foodInfoFoodTypesAvailable = 1;
				b->data.house.inventory.one.wheat = amountPerType;
				b->data.house.numFoods = 1;
			} else if (numTypes > 0) {
				for (int t = 0; t < 4 && b->data.house.numFoods < numTypes; t++) {
					if (b->data.house.inventory.all[t] >= amountPerType) {
						b->data.house.inventory.all[t] -= amountPerType;
						b->data.house.numFoods++;
						totalConsumed += amountPerType;
					} else if (b->data.house.inventory.all[t]) {
						// has food but not enough
						b->data.house.inventory.all[t] = 0;
						b->data.house.numFoods++;
						totalConsumed += amountPerType; // BUG?
					}
					if (b->data.house.numFoods > Data_CityInfo.foodInfoFoodTypesEaten) {
						Data_CityInfo.foodInfoFoodTypesEaten = b->data.house.numFoods;
					}
				}
			}
		}
	}
	Data_CityInfo.foodInfoFoodConsumedLastMonth = totalConsumed;
	Data_CityInfo.foodInfoFoodStoredLastMonth = Data_CityInfo.foodInfoFoodStoredSoFarThisMonth;
	Data_CityInfo.foodInfoFoodStoredSoFarThisMonth = 0;
}

