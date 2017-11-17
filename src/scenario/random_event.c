#include "random_event.h"

#include "building/type.h"
#include "city/message.h"
#include "core/random.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"
#include "Data/Scenario.h"
#include "../Building.h"
#include "CityInfo.h"

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
    if (Data_Scenario.raiseWagesEnabled) {
        if (Data_CityInfo.wagesRome < 45) {
            Data_CityInfo.wagesRome += 1 + (random_byte_alt() & 3);
            if (Data_CityInfo.wagesRome > 45) {
                Data_CityInfo.wagesRome = 45;
            }
            city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
        }
    }
}

static void lower_wages()
{
    if (Data_Scenario.lowerWagesEnabled) {
        if (Data_CityInfo.wagesRome > 5) {
            Data_CityInfo.wagesRome -= 1 + (random_byte_alt() & 3);
            city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
        }
    }
}

static void disrupt_land_trade()
{
    if (Data_Scenario.landTradeProblemEnabled) {
        if (Data_CityInfo.tradeNumOpenLandRoutes > 0) {
            Data_CityInfo.tradeLandProblemDuration = 48;
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
    if (Data_Scenario.seaTradeProblemEnabled) {
        if (Data_CityInfo.tradeNumOpenSeaRoutes > 0) {
            Data_CityInfo.tradeSeaProblemDuration = 48;
            city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
        }
    }
}

static void contaminate_water()
{
    if (Data_Scenario.contaminatedWaterEnabled) {
        if (Data_CityInfo.population > 200) {
            int change;
            if (Data_CityInfo.healthRate > 80) {
                change = -50;
            } else if (Data_CityInfo.healthRate > 60) {
                change = -40;
            } else {
                change = -25;
            }
            CityInfo_Population_changeHealthRate(change);
            city_message_post(1, MESSAGE_CONTAMINATED_WATER, 0, 0);
        }
    }
}

static void destroy_iron_mine()
{
    if (Data_Scenario.ironMineCollapseEnabled) {
        int grid_offset = Building_collapseFirstOfType(BUILDING_IRON_MINE);
        if (grid_offset) {
            city_message_post(1, MESSAGE_IRON_MINE_COLLAPED, 0, grid_offset);
        }
    }
}

static void destroy_clay_pit()
{
    if (Data_Scenario.clayPitFloodEnabled) {
        int grid_offset = Building_collapseFirstOfType(BUILDING_CLAY_PIT);
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

