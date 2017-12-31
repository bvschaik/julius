#include "data.h"

#include "city/constants.h"
#include "city/gods.h"

#include "Data/CityInfo.h"

#include <string.h>

void city_data_init()
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
    Data_CityInfo.festivalSize = FESTIVAL_SMALL;
    Data_CityInfo.giftCost_modest = 0;
    Data_CityInfo.giftCost_generous = 0;
    Data_CityInfo.giftCost_lavish = 0;

    city_gods_reset();
}
