#include "CityInfoUpdater.h"
#include "Data/CityInfo.h"

void CityInfoUpdater_Imperial_calculateGiftCosts()
{
	int savings = Data_CityInfo.personalSavings;
	Data_CityInfo.giftCost_modest = savings / 8 + 20;
	Data_CityInfo.giftCost_generous = savings / 4 + 50;
	Data_CityInfo.giftCost_lavish = savings / 2 + 100;
}

void CityInfoUpdater_Entertainment_calculate()
{
	// TODO implement
}

