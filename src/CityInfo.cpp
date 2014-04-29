#include "CityInfo.h"
#include "Data/CityInfo.h"

void CityInfo_init()
{
	// TODO
}

void CityInfo_initGameTime()
{
	// TODO
}

void CityInfo_Imperial_calculateGiftCosts()
{
	int savings = Data_CityInfo.personalSavings;
	Data_CityInfo.giftCost_modest = savings / 8 + 20;
	Data_CityInfo.giftCost_generous = savings / 4 + 50;
	Data_CityInfo.giftCost_lavish = savings / 2 + 100;
}

void CityInfo_Entertainment_calculate()
{
	// TODO implement
}

