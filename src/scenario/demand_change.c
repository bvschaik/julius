#include "demand_change.h"

#include "city/message.h"
#include "core/random.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "scenario/data.h"

void scenario_demand_change_init(void)
{
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        random_generate_next();
        if (scenario.demand_changes[i].year) {
            scenario.demand_changes[i].month = (random_byte() & 7) + 2;
        }
    }
}

void scenario_demand_change_process(void)
{
    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        if (!scenario.demand_changes[i].year) {
            continue;
        }
        if (game_time_year() != scenario.demand_changes[i].year + scenario.start_year ||
            game_time_month() != scenario.demand_changes[i].month) {
            continue;
        }
        int route = scenario.demand_changes[i].route_id;
        int resource = scenario.demand_changes[i].resource;
        int city_id = empire_city_get_for_trade_route(route);
        if (city_id < 0) {
            city_id = 0;
        }
        if (scenario.demand_changes[i].is_rise) {
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
