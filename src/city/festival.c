#include "festival.h"

#include "building/warehouse.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/sentiment.h"

#include "Data/CityInfo.h"

void city_festival_schedule()
{
    Data_CityInfo.plannedFestivalGod = Data_CityInfo.festivalGod;
    Data_CityInfo.plannedFestivalSize = Data_CityInfo.festivalSize;
    int cost;
    if (Data_CityInfo.festivalSize == FESTIVAL_SMALL) {
        Data_CityInfo.plannedFestivalMonthsToGo = 2;
        cost = Data_CityInfo.festivalCostSmall;
    } else if (Data_CityInfo.festivalSize == FESTIVAL_LARGE) {
        Data_CityInfo.plannedFestivalMonthsToGo = 3;
        cost = Data_CityInfo.festivalCostLarge;
    } else {
        Data_CityInfo.plannedFestivalMonthsToGo = 4;
        cost = Data_CityInfo.festivalCostGrand;
    }

    city_finance_process_sundry(cost);

    if (Data_CityInfo.festivalSize == FESTIVAL_GRAND) {
        building_warehouses_remove_resource(RESOURCE_WINE, Data_CityInfo.festivalWineGrand);
    }
}

void city_festival_update()
{
    Data_CityInfo.monthsSinceFestival++;
    if (Data_CityInfo.festivalEffectMonthsDelayFirst) {
        --Data_CityInfo.festivalEffectMonthsDelayFirst;
    }
    if (Data_CityInfo.festivalEffectMonthsDelaySecond) {
        --Data_CityInfo.festivalEffectMonthsDelaySecond;
    }
    if (Data_CityInfo.plannedFestivalSize <= FESTIVAL_NONE) {
        return;
    }
    Data_CityInfo.plannedFestivalMonthsToGo--;
    if (Data_CityInfo.plannedFestivalMonthsToGo > 0) {
        return;
    }
    // throw a party!
    if (Data_CityInfo.festivalEffectMonthsDelayFirst <= 0) {
        Data_CityInfo.festivalEffectMonthsDelayFirst = 12;
        switch (Data_CityInfo.plannedFestivalSize) {
            case FESTIVAL_SMALL: city_sentiment_change_happiness(7); break;
            case FESTIVAL_LARGE: city_sentiment_change_happiness(9); break;
            case FESTIVAL_GRAND: city_sentiment_change_happiness(12); break;
        }
    } else if (Data_CityInfo.festivalEffectMonthsDelaySecond <= 0) {
        Data_CityInfo.festivalEffectMonthsDelaySecond = 12;
        switch (Data_CityInfo.plannedFestivalSize) {
            case FESTIVAL_SMALL: city_sentiment_change_happiness(2); break;
            case FESTIVAL_LARGE: city_sentiment_change_happiness(3); break;
            case FESTIVAL_GRAND: city_sentiment_change_happiness(5); break;
        }
    }
    Data_CityInfo.monthsSinceFestival = 1;
    city_data.religion.gods[Data_CityInfo.plannedFestivalGod].months_since_festival = 0;
    switch (Data_CityInfo.plannedFestivalSize) {
        case FESTIVAL_SMALL: city_message_post(1, MESSAGE_SMALL_FESTIVAL, 0, 0); break;
        case FESTIVAL_LARGE: city_message_post(1, MESSAGE_LARGE_FESTIVAL, 0, 0); break;
        case FESTIVAL_GRAND: city_message_post(1, MESSAGE_GRAND_FESTIVAL, 0, 0); break;
    }
    Data_CityInfo.plannedFestivalSize = FESTIVAL_NONE;
    Data_CityInfo.plannedFestivalMonthsToGo = 0;
}
