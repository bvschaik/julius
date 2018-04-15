#include "trade.h"

#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "empire/city.h"

#include "Data/CityInfo.h"

void city_trade_update()
{
    Data_CityInfo.tradeNumOpenSeaRoutes = 0;
    Data_CityInfo.tradeNumOpenLandRoutes = 0;
    // Wine types
    city_data.resource.wine_types_available = building_count_industry_total(RESOURCE_WINE) > 0 ? 1 : 0;
    if (city_data.resource.trade_status[RESOURCE_WINE] == TRADE_STATUS_IMPORT) {
        city_data.resource.wine_types_available += empire_city_count_wine_sources();
    }
    // Update trade problems
    if (Data_CityInfo.tradeLandProblemDuration > 0) {
        Data_CityInfo.tradeLandProblemDuration--;
    } else {
        Data_CityInfo.tradeLandProblemDuration = 0;
    }
    if (Data_CityInfo.tradeSeaProblemDuration > 0) {
        Data_CityInfo.tradeSeaProblemDuration--;
    } else {
        Data_CityInfo.tradeSeaProblemDuration = 0;
    }
    empire_city_generate_trader();
}
