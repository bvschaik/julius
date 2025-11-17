#include "observation.h"

#include "building/count.h"
#include "city/culture.h"
#include "city/data.h"
#include "city/finance.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/migration.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "city/sentiment.h"
#include "city/victory.h"
#include "game/time.h"
#include "scenario/criteria.h"

#include <string.h>

void gymnasium_clear_observation(gymnasium_observation_t *obs)
{
    memset(obs, 0, sizeof(gymnasium_observation_t));
}

int gymnasium_get_observation(gymnasium_observation_t *obs)
{
    if (!obs) {
        return -1; // Invalid pointer
    }

    // Clear the observation first
    gymnasium_clear_observation(obs);

    // Extract ratings (all 0-100)
    obs->ratings.culture = city_rating_culture();
    obs->ratings.prosperity = city_rating_prosperity();
    obs->ratings.peace = city_rating_peace();
    obs->ratings.favor = city_rating_favor();

    // Extract finance data
    obs->finance.treasury = city_finance_treasury();
    obs->finance.tax_percentage = city_finance_tax_percentage();
    obs->finance.estimated_tax_income = city_finance_estimated_tax_income();
    obs->finance.estimated_wages = city_finance_estimated_wages();

    // Last year's financial overview
    const finance_overview *last_year = city_finance_overview_last_year();
    if (last_year) {
        obs->finance.last_year_income = last_year->income.total;
        obs->finance.last_year_expenses = last_year->expenses.total;
        obs->finance.last_year_net = last_year->net_in_out;
    }

    // Extract population data
    obs->population.total = city_population();
    obs->population.school_age = city_population_school_age();
    obs->population.academy_age = city_population_academy_age();
    obs->population.working_age = city_population_people_of_working_age();
    obs->population.sentiment = city_sentiment();

    // Extract labor data
    obs->labor.workers_available = city_labor_workers_unemployed(); // Available = unemployed
    obs->labor.workers_employed = city_labor_workers_employed();
    obs->labor.workers_needed = city_labor_workers_needed();
    obs->labor.unemployment_pct = city_labor_unemployment_percentage();
    obs->labor.wages = city_labor_wages();

    // Extract resource data
    obs->resources.food_stocks = city_resource_food_stored();
    obs->resources.food_types_available = city_resource_food_types_available();
    obs->resources.food_supply_months = city_resource_food_supply_months();
    // Note: food consumed/produced last month not exposed via API
    obs->resources.food_consumed_last_month = 0; // Not available
    obs->resources.food_produced_last_month = 0; // Not available

    // Extract building counts
    // Note: We aggregate different building types into categories
    obs->buildings.housing = building_count_total(BUILDING_HOUSE_SMALL_TENT) +
                             building_count_total(BUILDING_HOUSE_LARGE_TENT) +
                             building_count_total(BUILDING_HOUSE_SMALL_SHACK) +
                             building_count_total(BUILDING_HOUSE_LARGE_SHACK) +
                             building_count_total(BUILDING_HOUSE_SMALL_HOVEL) +
                             building_count_total(BUILDING_HOUSE_LARGE_HOVEL) +
                             building_count_total(BUILDING_HOUSE_SMALL_CASA) +
                             building_count_total(BUILDING_HOUSE_LARGE_CASA) +
                             building_count_total(BUILDING_HOUSE_SMALL_INSULA) +
                             building_count_total(BUILDING_HOUSE_MEDIUM_INSULA) +
                             building_count_total(BUILDING_HOUSE_LARGE_INSULA) +
                             building_count_total(BUILDING_HOUSE_GRAND_INSULA) +
                             building_count_total(BUILDING_HOUSE_SMALL_VILLA) +
                             building_count_total(BUILDING_HOUSE_MEDIUM_VILLA) +
                             building_count_total(BUILDING_HOUSE_LARGE_VILLA) +
                             building_count_total(BUILDING_HOUSE_GRAND_VILLA) +
                             building_count_total(BUILDING_HOUSE_SMALL_PALACE) +
                             building_count_total(BUILDING_HOUSE_MEDIUM_PALACE) +
                             building_count_total(BUILDING_HOUSE_LARGE_PALACE) +
                             building_count_total(BUILDING_HOUSE_LUXURY_PALACE);

    // Note: total housing capacity is not exposed via public API
    obs->buildings.housing_capacity = 0; // Not available

    obs->buildings.food_buildings = building_count_total(BUILDING_WHEAT_FARM) +
                                    building_count_total(BUILDING_VEGETABLE_FARM) +
                                    building_count_total(BUILDING_FRUIT_FARM) +
                                    building_count_total(BUILDING_OLIVE_FARM) +
                                    building_count_total(BUILDING_VINES_FARM) +
                                    building_count_total(BUILDING_PIG_FARM) +
                                    building_count_total(BUILDING_GRANARY);

    obs->buildings.industrial_buildings = building_count_total(BUILDING_WINE_WORKSHOP) +
                                          building_count_total(BUILDING_OIL_WORKSHOP) +
                                          building_count_total(BUILDING_WEAPONS_WORKSHOP) +
                                          building_count_total(BUILDING_FURNITURE_WORKSHOP) +
                                          building_count_total(BUILDING_POTTERY_WORKSHOP);

    obs->buildings.entertainment = building_count_total(BUILDING_THEATER) +
                                   building_count_total(BUILDING_AMPHITHEATER) +
                                   building_count_total(BUILDING_COLOSSEUM) +
                                   building_count_total(BUILDING_HIPPODROME) +
                                   building_count_total(BUILDING_GLADIATOR_SCHOOL) +
                                   building_count_total(BUILDING_LION_HOUSE) +
                                   building_count_total(BUILDING_ACTOR_COLONY) +
                                   building_count_total(BUILDING_CHARIOT_MAKER);

    obs->buildings.education = building_count_total(BUILDING_SCHOOL) +
                               building_count_total(BUILDING_ACADEMY) +
                               building_count_total(BUILDING_LIBRARY);

    obs->buildings.health = building_count_total(BUILDING_DOCTOR) +
                            building_count_total(BUILDING_HOSPITAL) +
                            building_count_total(BUILDING_BATHHOUSE) +
                            building_count_total(BUILDING_BARBER);

    obs->buildings.religious = building_count_total(BUILDING_SMALL_TEMPLE_CERES) +
                               building_count_total(BUILDING_SMALL_TEMPLE_NEPTUNE) +
                               building_count_total(BUILDING_SMALL_TEMPLE_MERCURY) +
                               building_count_total(BUILDING_SMALL_TEMPLE_MARS) +
                               building_count_total(BUILDING_SMALL_TEMPLE_VENUS) +
                               building_count_total(BUILDING_LARGE_TEMPLE_CERES) +
                               building_count_total(BUILDING_LARGE_TEMPLE_NEPTUNE) +
                               building_count_total(BUILDING_LARGE_TEMPLE_MERCURY) +
                               building_count_total(BUILDING_LARGE_TEMPLE_MARS) +
                               building_count_total(BUILDING_LARGE_TEMPLE_VENUS) +
                               building_count_total(BUILDING_ORACLE);

    obs->buildings.total_buildings = building_count_total(BUILDING_NONE); // Gets total of all buildings

    // Extract migration data
    // Note: immigration/emigration amounts are internal to city_data and not exposed via API
    obs->migration.immigration_amount = 0; // Not directly accessible
    obs->migration.emigration_amount = 0; // Not directly accessible
    obs->migration.newcomers = city_migration_newcomers();

    // Extract culture/health coverage
    obs->culture.health_value = city_health();
    obs->culture.average_entertainment = city_culture_average_entertainment();
    obs->culture.average_education = city_culture_average_education();
    obs->culture.average_health = city_culture_average_health();
    // Note: No average_religion function exists; religion coverage is per-god
    obs->culture.average_religion = 0; // Not available via API

    // Extract time data
    obs->time.year = game_time_year();
    obs->time.month = game_time_month();
    obs->time.total_months = game_time_year() * 12 + game_time_month();

    // Extract victory/scenario data
    obs->victory.is_active = !city_victory_has_won();
    obs->victory.has_won = city_victory_has_won();
    obs->victory.population_goal = scenario_criteria_population();
    obs->victory.culture_goal = scenario_criteria_culture();
    obs->victory.prosperity_goal = scenario_criteria_prosperity();
    obs->victory.peace_goal = scenario_criteria_peace();
    obs->victory.favor_goal = scenario_criteria_favor();

    return 0; // Success
}
