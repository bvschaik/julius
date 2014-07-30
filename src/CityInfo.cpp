#include "CityInfo.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Debug.h"

#include <string.h>

void CityInfo_init()
{
	memset(&Data_CityInfo, 0, sizeof(struct Data_CityInfo));

	Data_CityInfo.citySentiment = 60;
	Data_CityInfo.healthRateTarget = 50;
	Data_CityInfo.healthRate = 50;
	Data_CityInfo.__unknown_00c0 = 3;
	Data_CityInfo.wagesRome = 30;
	Data_CityInfo.wages = 30;
	Data_CityInfo.taxPercentage = 7;
	Data_CityInfo.tradeNextImportResourceCaravan = 1;
	Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
	Data_CityInfo.monthlyPopulationNextIndex = 0;
	Data_CityInfo.monthsSinceStart = 0;
	Data_CityInfo.monthsSinceFestival = 1;
	Data_CityInfo.festivalSize = 1;
	Data_CityInfo.giftCost_modest = 0;
	Data_CityInfo.giftCost_generous = 0;
	Data_CityInfo.giftCost_lavish = 0;

	CityInfo_Gods_reset();
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

