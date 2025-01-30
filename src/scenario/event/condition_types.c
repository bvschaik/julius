#include "condition_types.h"

#include "building/count.h"
#include "building/type.h"
#include "city/data_private.h"
#include "city/emperor.h"
#include "city/finance.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/military.h"
#include "city/ratings.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "game/settings.h"
#include "game/time.h"
#include "map/grid.h"
#include "scenario/custom_variable.h"
#include "scenario/event/condition_comparison_helper.h"
#include "scenario/request.h"
#include "scenario/scenario.h"

int scenario_condition_type_building_count_active_met(const scenario_condition_t *condition)
{
    int comparison = condition->parameter1;
    int value = condition->parameter2;
    building_type type = condition->parameter3;

    int total_active_count = 0;
    switch(type) {
        case BUILDING_MENU_FARMS:
            total_active_count = building_set_count_farms(1);
            break;
        case BUILDING_MENU_RAW_MATERIALS:
            total_active_count = building_set_count_raw_materials(1);
            break;
        case BUILDING_MENU_WORKSHOPS:
            total_active_count = building_set_count_workshops(1);
            break;
        case BUILDING_MENU_SMALL_TEMPLES:
            total_active_count = building_set_count_small_temples(1);
            break;
        case BUILDING_MENU_LARGE_TEMPLES:
            total_active_count = building_set_count_large_temples(1);
            break;
        case BUILDING_MENU_GRAND_TEMPLES:
            total_active_count = building_count_grand_temples_active();
            break;
        case BUILDING_MENU_TREES:
            total_active_count = building_set_count_deco_trees();
            break;
        case BUILDING_MENU_PATHS:
            total_active_count = building_set_count_deco_paths();
            break;
        case BUILDING_MENU_PARKS:
            total_active_count = building_set_count_deco_statues();
            break;
        case BUILDING_ANY:
            total_active_count = building_count_any_total(1);
            break;
        case BUILDING_FORT_LEGIONARIES:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_LEGIONARY);
            break;
        case BUILDING_FORT_JAVELIN:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_JAVELIN);
            break;
        case BUILDING_FORT_MOUNTED:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_MOUNTED);
            break;
        default:
            total_active_count = building_count_active(type);
            break;
    }

    return comparison_helper_compare_values(comparison, total_active_count, value);
}

int scenario_condition_type_building_count_any_met(const scenario_condition_t *condition)
{
    int comparison = condition->parameter1;
    int value = condition->parameter2;
    building_type type = condition->parameter3;

    int total_active_count = 0;
    switch(type) {
        case BUILDING_MENU_FARMS:
            total_active_count = building_set_count_farms(0);
            break;
        case BUILDING_MENU_RAW_MATERIALS:
            total_active_count = building_set_count_raw_materials(0);
            break;
        case BUILDING_MENU_WORKSHOPS:
            total_active_count = building_set_count_workshops(0);
            break;
        case BUILDING_MENU_SMALL_TEMPLES:
            total_active_count = building_set_count_small_temples(0);
            break;
        case BUILDING_MENU_LARGE_TEMPLES:
            total_active_count = building_set_count_large_temples(0);
            break;
        case BUILDING_MENU_GRAND_TEMPLES:
            total_active_count = building_count_grand_temples();
            break;
        case BUILDING_MENU_TREES:
            total_active_count = building_set_count_deco_trees();
            break;
        case BUILDING_MENU_PATHS:
            total_active_count = building_set_count_deco_paths();
            break;
        case BUILDING_MENU_PARKS:
            total_active_count = building_set_count_deco_statues();
            break;
        case BUILDING_ANY:
            total_active_count = building_count_any_total(0);
            break;
        case BUILDING_FORT_LEGIONARIES:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_LEGIONARY);
            break;
        case BUILDING_FORT_JAVELIN:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_JAVELIN);
            break;
        case BUILDING_FORT_MOUNTED:
            total_active_count += building_count_fort_type_total(FIGURE_FORT_MOUNTED);
            break;
        default:
            total_active_count = building_count_total(type);
            break;
    }

    return comparison_helper_compare_values(comparison, total_active_count, value);
}

int scenario_condition_type_building_count_area_met(const scenario_condition_t *condition)
{
    int grid_offset = condition->parameter1;
    int block_radius = condition->parameter2;
    building_type type = condition->parameter3;
    int comparison = condition->parameter4;
    int value = condition->parameter5;

    if (!map_grid_is_valid_offset(grid_offset)) {
        return 0;
    }

    int minx = map_grid_offset_to_x(grid_offset) - block_radius;
    int miny = map_grid_offset_to_y(grid_offset) - block_radius;
    int maxx = map_grid_offset_to_x(grid_offset) + block_radius;
    int maxy = map_grid_offset_to_y(grid_offset) + block_radius;
    int buildings_in_area = 0;
    switch(type) {
        case BUILDING_MENU_FARMS:
            buildings_in_area = building_set_area_count_farms(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_RAW_MATERIALS:
            buildings_in_area = building_set_area_count_raw_materials(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_WORKSHOPS:
            buildings_in_area = building_set_area_count_workshops(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_SMALL_TEMPLES:
            buildings_in_area = building_set_area_count_small_temples(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_LARGE_TEMPLES:
            buildings_in_area = building_set_area_count_large_temples(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_GRAND_TEMPLES:
            buildings_in_area = building_set_area_count_grand_temples(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_TREES:
            buildings_in_area = building_set_area_count_deco_trees(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_PATHS:
            buildings_in_area = building_set_area_count_deco_paths(minx, miny, maxx, maxy);
            break;
        case BUILDING_MENU_PARKS:
            buildings_in_area = building_set_area_count_deco_statues(minx, miny, maxx, maxy);
            break;
        case BUILDING_FORT_LEGIONARIES:
            buildings_in_area = building_count_fort_type_in_area(minx, miny, maxx, maxy, FIGURE_FORT_LEGIONARY);
            break;
        case BUILDING_FORT_JAVELIN:
            buildings_in_area = building_count_fort_type_in_area(minx, miny, maxx, maxy, FIGURE_FORT_JAVELIN);
            break;
        case BUILDING_FORT_MOUNTED:
            buildings_in_area = building_count_fort_type_in_area(minx, miny, maxx, maxy, FIGURE_FORT_MOUNTED);
            break;
        default:
            buildings_in_area = building_count_in_area(type, minx, miny, maxx, maxy);
            break;
    }

    return comparison_helper_compare_values(comparison, buildings_in_area, value);
}

int scenario_condition_type_city_population_met(const scenario_condition_t *condition)
{
    int comparison = condition->parameter1;
    int value = condition->parameter2;
    int class = condition->parameter3;

    int population_value_to_use = city_data.population.population;
    if (class == POP_CLASS_PATRICIAN) {
        population_value_to_use = city_data.population.people_in_villas_palaces;
    } else if (class == POP_CLASS_PLEBEIAN) {
        population_value_to_use = city_data.population.population - city_data.population.people_in_villas_palaces;
    } else if (class == POP_CLASS_SLUMS) {
        population_value_to_use = city_data.population.people_in_tents_shacks;
    }

    return comparison_helper_compare_values(comparison, population_value_to_use, value);
}

int scenario_condition_type_count_own_troops_met(const scenario_condition_t *condition)
{
    int comparison = condition->parameter1;
    int value = condition->parameter2;
    int in_city_only = condition->parameter3;

    int soldier_count = in_city_only ? city_military_total_soldiers_in_city() : city_military_total_soldiers();

    return comparison_helper_compare_values(comparison, soldier_count, value);
}

int scenario_condition_type_custom_variable_check_met(const scenario_condition_t *condition)
{
    int target_variable = scenario_custom_variable_get_value(condition->parameter1);
    int comparison = condition->parameter2;
    int value = condition->parameter3;

    return comparison_helper_compare_values(comparison, target_variable, value);
}

int scenario_condition_type_difficulty_met(const scenario_condition_t *condition)
{
    int difficulty = setting_difficulty();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, difficulty, value);
}

int scenario_condition_type_money_met(const scenario_condition_t *condition)
{
    int funds = city_finance_treasury();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, funds, value);
}

int scenario_condition_type_population_unemployed_met(const scenario_condition_t *condition)
{
    int use_percentage = condition->parameter1;
    int comparison = condition->parameter2;
    int value = condition->parameter3;

    int unemployed_total = use_percentage ? city_labor_unemployment_percentage() : city_labor_workers_unemployed();

    return comparison_helper_compare_values(comparison, unemployed_total, value);
}

int scenario_condition_type_request_is_ongoing_met(const scenario_condition_t *condition)
{
    int request_id = condition->parameter1;
    int check_for_ongoing = condition->parameter2;
    int is_ongoing = scenario_request_is_ongoing(request_id);

    return check_for_ongoing ? is_ongoing : !is_ongoing;
}

int scenario_condition_type_resource_storage_available_met(const scenario_condition_t *condition)
{
    int resource = condition->parameter1;
    int comparison = condition->parameter2;
    int value = condition->parameter3;
    storage_types storage_type = condition->parameter4;
    int respect_settings = condition->parameter5;

    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    int storage_available = 0;
    switch(storage_type) {
        case STORAGE_TYPE_ALL:
            storage_available += city_resource_get_available_empty_space_warehouses(resource, respect_settings);
            storage_available += city_resource_get_available_empty_space_granaries(resource, respect_settings) / RESOURCE_ONE_LOAD;
            break;
        case STORAGE_TYPE_GRANARIES:
            storage_available += city_resource_get_available_empty_space_granaries(resource, respect_settings) / RESOURCE_ONE_LOAD;
            break;
        case STORAGE_TYPE_WAREHOUSES:
            storage_available += city_resource_get_available_empty_space_warehouses(resource, respect_settings);
            break;
        default:
            break;
    }

    return comparison_helper_compare_values(comparison, storage_available, value);
}

int scenario_condition_type_resource_stored_count_met(const scenario_condition_t *condition)
{
    int resource = condition->parameter1;
    int comparison = condition->parameter2;
    int value = condition->parameter3;
    storage_types storage_type = condition->parameter4;

    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    int amount_stored = 0;
    switch(storage_type) {
        case STORAGE_TYPE_ALL:
            amount_stored += city_resource_count(resource);
            if (resource_is_food(resource)) {
                amount_stored += city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
            }
            break;
        case STORAGE_TYPE_GRANARIES:
            if (resource_is_food(resource)) {
                amount_stored += city_resource_count_food_on_granaries(resource) / RESOURCE_ONE_LOAD;
            }
            break;
        case STORAGE_TYPE_WAREHOUSES:
            amount_stored += city_resource_count(resource);
            break;
        default:
            break;
    }

    return comparison_helper_compare_values(comparison, amount_stored, value);
}

int scenario_condition_type_rome_wages_met(const scenario_condition_t *condition)
{
    int wages = city_labor_wages_rome();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, wages, value);
}

int scenario_condition_type_savings_met(const scenario_condition_t *condition)
{
    int funds = city_emperor_personal_savings();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, funds, value);
}

int scenario_condition_type_stats_city_health_met(const scenario_condition_t *condition)
{
    int stat_value = city_health();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, stat_value, value);
}

int scenario_condition_type_stats_culture_met(const scenario_condition_t *condition)
{
    int stat_value = city_rating_culture();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, stat_value, value);
}

int scenario_condition_type_stats_favor_met(const scenario_condition_t *condition)
{
    int stat_value = city_rating_favor();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, stat_value, value);
}

int scenario_condition_type_stats_peace_met(const scenario_condition_t *condition)
{
    int stat_value = city_rating_peace();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, stat_value, value);
}

int scenario_condition_type_stats_prosperity_met(const scenario_condition_t *condition)
{
    int stat_value = city_rating_prosperity();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, stat_value, value);
}

void scenario_condition_type_time_init(scenario_condition_t *condition)
{
    int min_months = condition->parameter2;
    int max_months = condition->parameter3;

    if (max_months < min_months) {
        max_months = min_months;
        condition->parameter3 = min_months;
    }

    condition->parameter4 = random_between_from_stdlib(min_months, max_months);
}

int scenario_condition_type_time_met(const scenario_condition_t *condition)
{
    int total_months = game_time_total_months();
    int comparison = condition->parameter1;
    int target_months = condition->parameter4;

    return comparison_helper_compare_values(comparison, total_months, target_months);
}

int scenario_condition_type_trade_route_open_met(const scenario_condition_t *condition)
{
    int route_id = condition->parameter1;
    int check_for_open = condition->parameter2;

    if (!trade_route_is_valid(route_id)) {
        return 0;
    }

    int route_is_open = empire_city_is_trade_route_open(route_id);
    return route_is_open == check_for_open;
}

int scenario_condition_type_trade_route_price_met(const scenario_condition_t *condition)
{
    int route_id = condition->parameter1;
    int comparison = condition->parameter2;
    int value = condition->parameter3;

    if (!trade_route_is_valid(route_id)) {
        return 0;
    }

    int route_is_open = empire_city_is_trade_route_open(route_id);
    int route_price = route_is_open ? 0 : empire_city_get_trade_route_cost(route_id);
    return comparison_helper_compare_values(comparison, route_price, value);
}

int scenario_condition_type_trade_sell_price_met(const scenario_condition_t *condition)
{
    int resource = condition->parameter1;
    int comparison = condition->parameter2;
    int value = condition->parameter3;

    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }
    
    int trade_sell_price = trade_price_base_sell(resource);
    return comparison_helper_compare_values(comparison, trade_sell_price, value);
}

int scenario_condition_type_tax_rate_met(const scenario_condition_t *condition)
{
    int tax_rate = city_finance_tax_percentage();
    int comparison = condition->parameter1;
    int value = condition->parameter2;

    return comparison_helper_compare_values(comparison, tax_rate, value);
}
