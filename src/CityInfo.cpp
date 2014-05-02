#include "CityInfo.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Debug.h"

void CityInfo_init()
{
	// TODO
}

void CityInfo_initGameTime()
{
	Data_CityInfo_Extra.gameTimeTick = 0;
	Data_CityInfo_Extra.gameTimeTotalDays = 0;
	Data_CityInfo_Extra.gameTimeDay = 0;
	Data_CityInfo_Extra.gameTimeMonth = 0;
	Data_CityInfo_Extra.gameTimeYear = 2098;
	Data_Debug.incorrectHousePositions = 0;
	Data_Debug.unfixableHousePositions = 0;
	Data_Buildings_Extra.highestBuildingIdEver = 0;
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

