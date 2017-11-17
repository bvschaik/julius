#include "demand_change.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "scenario/data.h"

#include "Data/Scenario.h"

void scenario_demand_change_init()
{
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        random_generate_next();
        if (Data_Scenario.demandChanges.year[i]) {
            Data_Scenario.demandChanges.month[i] = (random_byte() & 7) + 2;
        }
    }
}

void scenario_demand_change_process()
{
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        if (!Data_Scenario.demandChanges.year[i]) {
            continue;
        }
        if (game_time_year() != Data_Scenario.demandChanges.year[i] + Data_Scenario.startYear ||
            game_time_month() != Data_Scenario.demandChanges.month[i]) {
            continue;
        }
        int route = Data_Scenario.demandChanges.routeId[i];
        int resource = Data_Scenario.demandChanges.resourceId[i];
        int city_id = empire_city_get_for_trade_route(route);
        if (Data_Scenario.demandChanges.isRise[i]) {
            if (trade_route_increase_limit(route, resource) && empire_city_is_trade_route_open(route)) {
                city_message_post(1, MESSAGE_INCREASED_TRADING, city_id, resource);
            }
        } else {
            if (trade_route_decrease_limit(route, resource) && empire_city_is_trade_route_open(route)) {
                if (trade_route_limit(route, resource) > 0) {
                    city_message_post(1, MESSAGE_DECREASED_TRADING, city_id, resource);
                } else {
                    city_message_post(1, MESSAGE_TRADE_STOPPED, city_id, resource);
                }
            }
        }
    }
}
