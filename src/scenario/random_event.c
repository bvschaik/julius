#include "random_event.h"

#include "building/destruction.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/population.h"
#include "city/trade.h"
#include "core/random.h"
#include "scenario/data.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

enum {
    EVENT_ROME_RAISES_WAGES = 1,
    EVENT_ROME_LOWERS_WAGES = 2,
    EVENT_LAND_TRADE_DISRUPTED = 3,
    EVENT_LAND_SEA_DISRUPTED = 4,
    EVENT_CONTAMINATED_WATER = 5,
    EVENT_IRON_MINE_COLLAPSED = 6,
    EVENT_CLAY_PIT_FLOODED = 7
};

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

static void raise_wages()
{
    if (scenario.random_events.raise_wages) {
        if (city_labor_raise_wages_rome()) {
            city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
        }
    }
}

static void lower_wages()
{
    if (scenario.random_events.lower_wages) {
        if (city_labor_lower_wages_rome()) {
            city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
        }
    }
}

static void disrupt_land_trade()
{
    if (scenario.random_events.land_trade_problem) {
        if (city_trade_has_land_trade_route()) {
            city_trade_start_land_trade_problems(48);
            if (scenario_property_climate() == CLIMATE_DESERT) {
                city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS, 0, 0);
            } else {
                city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
            }
        }
    }
}

static void disrupt_sea_trade()
{
    if (scenario.random_events.sea_trade_problem) {
        if (city_trade_has_sea_trade_route()) {
            city_trade_start_sea_trade_problems(48);
            city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
        }
    }
}

static void contaminate_water()
{
    if (scenario.random_events.contaminated_water) {
        if (city_population() > 200) {
            int change;
            int health_rate = city_health();
            if (health_rate > 80) {
                change = -50;
            } else if (health_rate > 60) {
                change = -40;
            } else {
                change = -25;
            }
            city_health_change(change);
            city_message_post(1, MESSAGE_CONTAMINATED_WATER, 0, 0);
        }
    }
}

static void destroy_iron_mine()
{
    if (scenario.random_events.iron_mine_collapse) {
        int grid_offset = building_destroy_first_of_type(BUILDING_IRON_MINE);
        if (grid_offset) {
            city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, grid_offset);
        }
    }
}

static void destroy_clay_pit()
{
    if (scenario.random_events.clay_pit_flooded) {
        int grid_offset = building_destroy_first_of_type(BUILDING_CLAY_PIT);
        if (grid_offset) {
            city_message_post(1, MESSAGE_CLAY_PIT_FLOODED, 0, grid_offset);
        }
    }
}

void scenario_random_event_process()
{
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
            contaminate_water();
            break;
        case EVENT_IRON_MINE_COLLAPSED:
            destroy_iron_mine();
            break;
        case EVENT_CLAY_PIT_FLOODED:
            destroy_clay_pit();
            break;
    }
}

