#include "CityInfo.h"
#include "Empire.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"

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
