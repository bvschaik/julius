#include "trade.h"

#include "building/count.h"
#include "building/monument.h"
#include "city/constants.h"
#include "core/config.h"
#include "city/data_private.h"
#include "empire/city.h"

void city_trade_update(void)
{
    city_data.trade.num_sea_routes = 0;
    city_data.trade.num_land_routes = 0;
    // Wine types
    city_data.resource.wine_types_available = building_count_industry_total(RESOURCE_WINE) > 0 ? 1 : 0;
    if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
        city_data.resource.wine_types_available += 1;
    }
    if (city_data.resource.trade_status[RESOURCE_WINE] == TRADE_STATUS_IMPORT || config_get(CONFIG_GP_CH_WINE_COUNTS_IF_OPEN_TRADE_ROUTE)) {
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

void city_trade_add_land_trade_route(void)
{
    city_data.trade.num_land_routes++;
}

void city_trade_add_sea_trade_route(void)
{
    city_data.trade.num_sea_routes++;
}

int city_trade_has_land_trade_route(void)
{
    return city_data.trade.num_land_routes > 0;
}

int city_trade_has_sea_trade_route(void)
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

int city_trade_has_land_trade_problems(void)
{
    return city_data.trade.land_trade_problem_duration > 0;
}

int city_trade_has_sea_trade_problems(void)
{
    return city_data.trade.sea_trade_problem_duration > 0;
}

int city_trade_current_caravan_import_resource(void)
{
    return city_data.trade.caravan_import_resource;
}

int city_trade_next_caravan_import_resource(void)
{
    city_data.trade.caravan_import_resource++;
    if (city_data.trade.caravan_import_resource >= RESOURCE_MAX) {
        city_data.trade.caravan_import_resource = RESOURCE_MIN;
    }
    return city_data.trade.caravan_import_resource;
}

int city_trade_next_caravan_backup_import_resource(void)
{
    city_data.trade.caravan_backup_import_resource++;
    if (city_data.trade.caravan_backup_import_resource >= RESOURCE_MAX) {
        city_data.trade.caravan_backup_import_resource = RESOURCE_MIN;
    }
    return city_data.trade.caravan_backup_import_resource;
}

int city_trade_next_docker_import_resource(void)
{
    city_data.trade.docker_import_resource++;
    if (city_data.trade.docker_import_resource >= RESOURCE_MAX) {
        city_data.trade.docker_import_resource = RESOURCE_MIN;
    }
    return city_data.trade.docker_import_resource;
}

int city_trade_next_docker_export_resource(void)
{
    city_data.trade.docker_export_resource++;
    if (city_data.trade.docker_export_resource >= RESOURCE_MAX) {
        city_data.trade.docker_export_resource = RESOURCE_MIN;
    }
    return city_data.trade.docker_export_resource;
}
