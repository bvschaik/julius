#include "trade.h"

#include "building/count.h"
#include "city/constants.h"
#include "city/data_private.h"
#include "empire/city.h"

void city_trade_update()
{
    city_data.trade.num_sea_routes = 0;
    city_data.trade.num_land_routes = 0;
    // Wine types
    city_data.resource.wine_types_available = building_count_industry_total(RESOURCE_WINE) > 0 ? 1 : 0;
    if (city_data.resource.trade_status[RESOURCE_WINE] == TRADE_STATUS_IMPORT) {
        city_data.resource.wine_types_available += empire_city_count_wine_sources();
    }
    // Update trade problems
    if (city_data.trade.land_trade_problem_duration > 0) {
        city_data.trade.land_trade_problem_duration--;
    } else {
        city_data.trade.land_trade_problem_duration = 0;
    }
    if (city_data.trade.sea_trade_problem_duration > 0) {
        city_data.trade.sea_trade_problem_duration--;
    } else {
        city_data.trade.sea_trade_problem_duration = 0;
    }
    empire_city_generate_trader();
}

void city_trade_add_land_trade_route()
{
    city_data.trade.num_land_routes++;
}

void city_trade_add_sea_trade_route()
{
    city_data.trade.num_sea_routes++;
}

int city_trade_has_land_trade_route()
{
    return city_data.trade.num_land_routes > 0;
}

int city_trade_has_sea_trade_route()
{
    return city_data.trade.num_sea_routes > 0;
}

void city_trade_start_land_trade_problems(int duration)
{
    city_data.trade.land_trade_problem_duration = duration;
}

void city_trade_start_sea_trade_problems(int duration)
{
    city_data.trade.sea_trade_problem_duration = duration;
}

int city_trade_has_land_trade_problems()
{
    return city_data.trade.land_trade_problem_duration > 0;
}

int city_trade_has_sea_trade_problems()
{
    return city_data.trade.sea_trade_problem_duration > 0;
}
