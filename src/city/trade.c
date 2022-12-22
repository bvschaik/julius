#include "trade.h"

#include "building/caravanserai.h"
#include "building/count.h"
#include "building/lighthouse.h"
#include "building/monument.h"
#include "city/constants.h"
#include "core/config.h"
#include "city/data_private.h"
#include "empire/city.h"
#include "figure/figure.h"

void city_trade_update(void)
{
    city_data.trade.num_sea_routes = 0;
    city_data.trade.num_land_routes = 0;
    // Wine types
    city_data.resource.wine_types_available = building_count_total(resource_get_data(RESOURCE_WINE)->industry) > 0 ? 1 : 0;
    if (building_monument_gt_module_is_active(VENUS_MODULE_1_DISTRIBUTE_WINE)) {
        city_data.resource.wine_types_available += 1;
    }

    city_data.resource.wine_types_available += empire_city_count_wine_sources();
    
    // Update trade problems
    if (city_data.trade.land_trade_problem_duration > 0) {
        city_data.trade.land_trade_problem_duration--;
        if (building_caravanserai_is_fully_functional()) {
            city_data.trade.land_trade_problem_duration--;
        }
    } else {
        city_data.trade.land_trade_problem_duration = 0;
    }
    if (city_data.trade.sea_trade_problem_duration > 0) {
        city_data.trade.sea_trade_problem_duration--;
        if (building_lighthouse_is_fully_functional()) {
            city_data.trade.sea_trade_problem_duration--;
        }
    } 
    if (city_data.trade.sea_trade_problem_duration <= 0) {
        city_data.trade.sea_trade_problem_duration = 0;
    }
    if (city_data.trade.land_trade_problem_duration <= 0) {
        city_data.trade.land_trade_problem_duration = 0;
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

int trade_caravan_count(void)
{
    int count = 0;
    for (int i = 1; i < figure_count(); i++) {
        figure *f = figure_get(i);
        if (f->type == FIGURE_TRADE_CARAVAN || f->type == FIGURE_TRADE_CARAVAN_DONKEY || f->type == FIGURE_NATIVE_TRADER) {
            count++;
        }
    }
    return count;
}
