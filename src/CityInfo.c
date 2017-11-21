#include "CityInfo.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"

#include "game/time.h"

#include <string.h>

//another realization for namespace alternate
void CityInfo_init();

struct _CityInfo CityInfo =
{
    CityInfo_init
};

void CityInfo_init()
{
	memset(&Data_CityInfo, 0, sizeof(struct _Data_CityInfo));

	Data_CityInfo_Extra.unknownBytes[0] = 0;
	Data_CityInfo_Extra.unknownBytes[1] = 0;

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
	Data_CityInfo.festivalSize = Festival_Small;
	Data_CityInfo.giftCost_modest = 0;
	Data_CityInfo.giftCost_generous = 0;
	Data_CityInfo.giftCost_lavish = 0;

	CityInfo_Gods_reset();
}

void CityInfo_initGameTime()
{
    game_time_init(2098);
	Data_Buildings_Extra.incorrectHousePositions = 0;
	Data_Buildings_Extra.unfixableHousePositions = 0;
	Data_Buildings_Extra.highestBuildingIdEver = 0;
}

void CityInfo_Imperial_calculateGiftCosts()
{
	int savings = Data_CityInfo.personalSavings;
	Data_CityInfo.giftCost_modest = savings / 8 + 20;
	Data_CityInfo.giftCost_generous = savings / 4 + 50;
	Data_CityInfo.giftCost_lavish = savings / 2 + 100;
}

