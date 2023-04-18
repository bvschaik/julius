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
#include "empire/trade_prices.h"
#include "game/settings.h"
#include "game/time.h"
#include "scenario/request.h"
#include "scenario/condition_types/comparison_helper.h"

int scenario_condition_type_building_count_active_met(const scenario_condition_t *condition)
{
    int comparison = condition->parameter1;
    int value = condition->parameter2;
    building_type type = condition->parameter3;

    int total_active_count = 0;
    switch(type) {
        case BUILDING_MENU_FARMS:
            total_active_count += building_count_active(BUILDING_WHEAT_FARM);
            total_active_count += building_count_active(BUILDING_VEGETABLE_FARM);
            total_active_count += building_count_active(BUILDING_FRUIT_FARM);
            total_active_count += building_count_active(BUILDING_OLIVE_FARM);
            total_active_count += building_count_active(BUILDING_VINES_FARM);
            total_active_count += building_count_active(BUILDING_PIG_FARM);
            break;
        case BUILDING_MENU_RAW_MATERIALS:
            total_active_count += building_count_active(BUILDING_MARBLE_QUARRY);
            total_active_count += building_count_active(BUILDING_IRON_MINE);
            total_active_count += building_count_active(BUILDING_TIMBER_YARD);
            total_active_count += building_count_active(BUILDING_CLAY_PIT);
            total_active_count += building_count_active(BUILDING_GOLD_MINE);
            break;
        case BUILDING_MENU_WORKSHOPS:
            total_active_count += building_count_active(BUILDING_WINE_WORKSHOP);
            total_active_count += building_count_active(BUILDING_OIL_WORKSHOP);
            total_active_count += building_count_active(BUILDING_WEAPONS_WORKSHOP);
            total_active_count += building_count_active(BUILDING_FURNITURE_WORKSHOP);
            total_active_count += building_count_active(BUILDING_POTTERY_WORKSHOP);
            total_active_count += building_count_active(BUILDING_CITY_MINT);
            break;
        case BUILDING_MENU_SMALL_TEMPLES:
            total_active_count += building_count_active(BUILDING_SMALL_TEMPLE_CERES);
            total_active_count += building_count_active(BUILDING_SMALL_TEMPLE_NEPTUNE);
            total_active_count += building_count_active(BUILDING_SMALL_TEMPLE_MERCURY);
            total_active_count += building_count_active(BUILDING_SMALL_TEMPLE_MARS);
            total_active_count += building_count_active(BUILDING_SMALL_TEMPLE_VENUS);
            break;
        case BUILDING_MENU_LARGE_TEMPLES:
            total_active_count += building_count_active(BUILDING_LARGE_TEMPLE_CERES);
            total_active_count += building_count_active(BUILDING_LARGE_TEMPLE_NEPTUNE);
            total_active_count += building_count_active(BUILDING_LARGE_TEMPLE_MERCURY);
            total_active_count += building_count_active(BUILDING_LARGE_TEMPLE_MARS);
            total_active_count += building_count_active(BUILDING_LARGE_TEMPLE_VENUS);
            break;
        case BUILDING_MENU_GRAND_TEMPLES:
            total_active_count += building_count_active(BUILDING_GRAND_TEMPLE_CERES);
            total_active_count += building_count_active(BUILDING_GRAND_TEMPLE_NEPTUNE);
            total_active_count += building_count_active(BUILDING_GRAND_TEMPLE_MERCURY);
            total_active_count += building_count_active(BUILDING_GRAND_TEMPLE_MARS);
            total_active_count += building_count_active(BUILDING_GRAND_TEMPLE_VENUS);
            break;
        case BUILDING_MENU_TREES:
            total_active_count += building_count_active(BUILDING_PINE_TREE);
            total_active_count += building_count_active(BUILDING_FIR_TREE);
            total_active_count += building_count_active(BUILDING_OAK_TREE);
            total_active_count += building_count_active(BUILDING_ELM_TREE);
            total_active_count += building_count_active(BUILDING_FIG_TREE);
            total_active_count += building_count_active(BUILDING_PLUM_TREE);
            total_active_count += building_count_active(BUILDING_PALM_TREE);
            total_active_count += building_count_active(BUILDING_DATE_TREE);
            break;
        case BUILDING_MENU_PATHS:
            total_active_count += building_count_active(BUILDING_PINE_PATH);
            total_active_count += building_count_active(BUILDING_FIR_PATH);
            total_active_count += building_count_active(BUILDING_OAK_PATH);
            total_active_count += building_count_active(BUILDING_ELM_PATH);
            total_active_count += building_count_active(BUILDING_FIG_PATH);
            total_active_count += building_count_active(BUILDING_PLUM_PATH);
            total_active_count += building_count_active(BUILDING_PALM_PATH);
            total_active_count += building_count_active(BUILDING_DATE_PATH);
            total_active_count += building_count_active(BUILDING_GARDEN_PATH);
            break;
        case BUILDING_MENU_PARKS:
            total_active_count += building_count_active(BUILDING_GARDENS);
            total_active_count += building_count_active(BUILDING_GRAND_GARDEN);
            total_active_count += building_count_active(BUILDING_SMALL_STATUE);
            total_active_count += building_count_active(BUILDING_MEDIUM_STATUE);
            total_active_count += building_count_active(BUILDING_LARGE_STATUE);
            total_active_count += building_count_active(BUILDING_SMALL_STATUE_ALT);
            total_active_count += building_count_active(BUILDING_SMALL_STATUE_ALT_B);
            total_active_count += building_count_active(BUILDING_LEGION_STATUE);
            total_active_count += building_count_active(BUILDING_GLADIATOR_STATUE);
            total_active_count += building_count_active(BUILDING_SMALL_POND);
            total_active_count += building_count_active(BUILDING_LARGE_POND);
            total_active_count += building_count_active(BUILDING_DOLPHIN_FOUNTAIN);
            break;
        default:
            total_active_count += building_count_active(type);
            break;
    }
    

    return comparison_helper_compare_values(comparison, total_active_count, value);
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
