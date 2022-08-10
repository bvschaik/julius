#include "random_event.h"

#include "building/destruction.h"
#include "building/monument.h"
#include "city/data_private.h"
#include "city/finance.h"
#include "city/gods.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/population.h"
#include "city/trade.h"
#include "core/config.h"
#include "core/random.h"
#include "game/difficulty.h"
#include "scenario/data.h"
#include "scenario/property.h"

enum {
    EVENT_ROME_RAISES_WAGES = 1,
    EVENT_ROME_LOWERS_WAGES = 2,
    EVENT_LAND_TRADE_DISRUPTED = 3,
    EVENT_LAND_SEA_DISRUPTED = 4,
    EVENT_CONTAMINATED_WATER = 5,
    EVENT_IRON_MINE_COLLAPSED = 6,
    EVENT_CLAY_PIT_FLOODED = 7
};

#define COOLDOWN_MONTHS_ROME_WAGE_CHANGE 12

static const int RANDOM_EVENT_PROBABILITY[128] = {
    0, 0, 1, 0, 0, 0, 4, 0, 0, 0, 0, 3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 6,
    0, 0, 2, 0, 0, 0, 7, 0, 5, 0, 0, 7, 0, 0, 0, 0,
    0, 7, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 6, 0, 0,
    0, 7, 0, 1, 6, 0, 0, 0, 0, 0, 2, 0, 0, 4, 0, 0,
    0, 0, 3, 0, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0
};

static void raise_wages(void)
{
    if (scenario.random_events.raise_wages &&
        city_data.labor.months_since_last_wage_change > COOLDOWN_MONTHS_ROME_WAGE_CHANGE) {
        if (city_labor_raise_wages_rome()) {
            city_data.labor.months_since_last_wage_change = 0;
            city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
        }
    }
}

static void lower_wages(void)
{
    if (scenario.random_events.lower_wages &&
        city_data.labor.months_since_last_wage_change > COOLDOWN_MONTHS_ROME_WAGE_CHANGE) {
        if (city_labor_lower_wages_rome()) {
            city_data.labor.months_since_last_wage_change = 0;
            city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
        }
    }
}

static void disrupt_land_trade(void)
{
    if (scenario.random_events.land_trade_problem) {
        if (city_trade_has_land_trade_route() &&
            city_data.trade.months_since_last_land_trade_problem > difficulty_random_event_cooldown_months()) {
            city_trade_start_land_trade_problems(48);
            city_data.trade.months_since_last_land_trade_problem = 0;
            if (scenario_property_climate() == CLIMATE_DESERT) {
                city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS, 0, 0);
            } else {
                city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
            }
        }
    }
}

static void disrupt_sea_trade(void)
{
    if (scenario.random_events.sea_trade_problem) {
        if (city_trade_has_sea_trade_route() &&
            city_data.trade.months_since_last_sea_trade_problem > difficulty_random_event_cooldown_months()) {
            city_trade_start_sea_trade_problems(48);
            city_data.trade.months_since_last_sea_trade_problem = 0;
            city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
        }
    }
}

static void contaminate_water(void)
{
    if (scenario.random_events.contaminated_water) {
        if (city_population() > 200 &&
            city_data.health.months_since_last_contaminated_water > difficulty_random_event_cooldown_months()) {
            int change;
            int health_rate = city_health();
            if (health_rate > 80) {
                change = -40;
            } else if (health_rate > 60) {
                change = -30;
            } else {
                change = -20;
            }
            city_health_change(change);
            city_data.health.months_since_last_contaminated_water = 0;
            city_message_post(1, MESSAGE_CONTAMINATED_WATER, 0, 0);
        }
    }
}

static void destroy_iron_mine(void)
{
    if (scenario.random_events.iron_mine_collapse &&
        city_data.building.months_since_last_destroyed_iron_mine > difficulty_random_event_cooldown_months()) {
        if(config_get(CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY)) {
            if(building_find(BUILDING_IRON_MINE)) {
                city_finance_process_sundry(250);
                city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, 0);
            }
        } else {
            int grid_offset = building_destroy_first_of_type(BUILDING_IRON_MINE);
            if (grid_offset) {
                city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, grid_offset);
            }
        }
        city_data.building.months_since_last_destroyed_iron_mine = 0;
    }
}

static void destroy_clay_pit(void)
{
    if (scenario.random_events.clay_pit_flooded &&
        city_data.building.months_since_last_flooded_clay_pit > difficulty_random_event_cooldown_months()) {
        if(config_get(CONFIG_GP_CH_RANDOM_COLLAPSES_TAKE_MONEY)) {
            if(building_find(BUILDING_CLAY_PIT)) {
                city_finance_process_sundry(250);
                city_message_post(1, MESSAGE_CLAY_PIT_FLOODED, 0, 0);
            }
        } else {
            int grid_offset = building_destroy_first_of_type(BUILDING_CLAY_PIT);
            if (grid_offset) {
                city_message_post(1, MESSAGE_CLAY_PIT_FLOODED, 0, grid_offset);
            }
        }
        city_data.building.months_since_last_flooded_clay_pit = 0;
    }
}

static void increase_month_since_last_random_event(void)
{
    city_data.labor.months_since_last_wage_change++;
    city_data.trade.months_since_last_land_trade_problem++;
    city_data.trade.months_since_last_sea_trade_problem++;
    city_data.health.months_since_last_contaminated_water++;
    city_data.building.months_since_last_destroyed_iron_mine++;
    city_data.building.months_since_last_flooded_clay_pit++;
}

static int all_gods_happy(void)
{
    for (int god = 0; god < MAX_GODS; god++) {
        if (city_god_happiness(god) < 60) {
            return 0;
        }
    }
    return 1;
}

void scenario_random_event_process(void)
{
    increase_month_since_last_random_event();
    int skip_event = building_monument_working(BUILDING_PANTHEON) && all_gods_happy();
    int event = RANDOM_EVENT_PROBABILITY[random_byte()];
    switch (event) {
        case EVENT_ROME_RAISES_WAGES:
            raise_wages();
            break;
        case EVENT_ROME_LOWERS_WAGES:
            lower_wages();
            break;
        case EVENT_LAND_TRADE_DISRUPTED:
            disrupt_land_trade();
            break;
        case EVENT_LAND_SEA_DISRUPTED:
            disrupt_sea_trade();
            break;
        case EVENT_CONTAMINATED_WATER:
            if (!skip_event) {
                contaminate_water();
            }
            break;
        case EVENT_IRON_MINE_COLLAPSED:
            if (!skip_event) {
                destroy_iron_mine();
            }
            break;
        case EVENT_CLAY_PIT_FLOODED:
            if (!skip_event) {
                destroy_clay_pit();
            }
            break;
    }
}
