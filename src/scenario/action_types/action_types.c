#include "action_types.h"

#include "building/menu.h"
#include "city/data_private.h"
#include "city/emperor.h"
#include "city/finance.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/ratings.h"
#include "core/random.h"
#include "city/trade.h"
#include "empire/city.h"
#include "empire/object.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "game/time.h"
#include "scenario/data.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/property.h"
#include "scenario/request.h"

int scenario_action_type_change_allowed_buildings_execute(scenario_action_t *action)
{
    int building = action->parameter1;
    int allowed = action->parameter2;

    scenario.allowed_buildings[building] = allowed;
    building_menu_update();
    
    return 1;
}

int scenario_action_type_change_resource_produced_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int new_state = action->parameter2;

    int successfully_changed = empire_city_change_own_resource_availability(resource, new_state);
    building_menu_update();
    
    return successfully_changed;
}

void scenario_action_type_city_health_init(scenario_action_t *action)
{
    action->parameter4 = random_between_from_stdlib(action->parameter1, action->parameter2);
}

int scenario_action_type_city_health_execute(scenario_action_t *action)
{
    int is_hard_set = action->parameter3;
    int adjustment = action->parameter4;
    
    if (is_hard_set) {
        city_health_set(adjustment);
    } else {
        city_health_change(adjustment);
    }

    return 1;
}

int scenario_action_type_empire_map_convert_future_trade_city_execute(scenario_action_t *action)
{
    int target_city_id = action->parameter1;
    int show_message = action->parameter2;

    empire_city *city = empire_city_get(target_city_id);
    if (city->type == EMPIRE_CITY_FUTURE_TRADE) {
        city->type = EMPIRE_CITY_TRADE;
        empire_object_set_expanded(city->empire_object_id, city->type);

        if (show_message) {
            city_message_post(1, MESSAGE_EMPIRE_HAS_EXPANDED, 0, 0);
        }
        return 1;
    }

    return 0;
}

int scenario_action_type_favor_add_execute(scenario_action_t *action)
{
    int adjustment = action->parameter1;
    city_ratings_change_favor(adjustment);

    return 1;
}

int scenario_action_type_gladiator_revolt_execute(scenario_action_t *action)
{
    scenario_gladiator_revolt_start_new();

    return 1;
}

void scenario_action_type_money_add_init(scenario_action_t *action)
{
    action->parameter3 = random_between_from_stdlib(action->parameter1, action->parameter2);
}

int scenario_action_type_money_add_execute(scenario_action_t *action)
{
    int adjustment = action->parameter3;
    city_finance_treasury_add_miscellaneous(adjustment);

    return 1;
}

int scenario_action_type_request_immediately_start_execute(scenario_action_t *action)
{
    int total_months = game_time_total_months();
    if (total_months < 1) {
        return 0; // Firing this (request immediately) event off at the start of the scenario breaks it. So prevent that.
    }

    int request_id = action->parameter1;
    if (request_id < 0 || request_id >= MAX_REQUESTS) {
        return 0;
    }

    return scenario_request_force_start(request_id);
}

void scenario_action_type_rome_wages_init(scenario_action_t *action)
{
    action->parameter4 = random_between_from_stdlib(action->parameter1, action->parameter2);
}

int scenario_action_type_rome_wages_execute(scenario_action_t *action)
{
    int is_hard_set = action->parameter3;
    int adjustment = action->parameter4;

    city_data.labor.months_since_last_wage_change = 0;

    int current_wages = city_labor_wages_rome();
    if (is_hard_set && adjustment == current_wages) {
        return 1;
    }

    if (is_hard_set) {
        if (adjustment < 1) {
            adjustment = 1;
        }

        city_data.labor.wages_rome = adjustment;
        if (adjustment > current_wages) {
            city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
        } else {
            city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
        }
    } else {
        if (adjustment == 0) {
            return 1;
        }
    
        city_data.labor.wages_rome += adjustment;
        if (adjustment > 0) {
            city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
        } else {
            city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
        }
    }

    if (city_data.labor.wages_rome < 1) {
        city_data.labor.wages_rome = 1;
    }

    return 1;
}

void scenario_action_type_savings_add_init(scenario_action_t *action)
{
    action->parameter3 = random_between_from_stdlib(action->parameter1, action->parameter2);
}

int scenario_action_type_savings_add_execute(scenario_action_t *action)
{
    int adjustment = action->parameter3;
    city_emperor_add_personal_savings(adjustment);

    return 1;
}

int scenario_action_type_send_standard_message_execute(scenario_action_t *action)
{
    int text_id = action->parameter1;
    if (text_id < MESSAGE_POPULATION_500 || text_id > MESSAGE_ROAD_TO_ROME_WARNING) {
        return 1;
    }

    city_message_post(1, text_id, 0, 0);

    return 1;
}

int scenario_action_type_trade_price_set_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int amount = action->parameter2;
    int set_buy_price = action->parameter3;
    int show_message = action->parameter4;
    
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    int current_price = 0;
    if (set_buy_price) {
        current_price = trade_price_base_buy(resource);
    } else {
        current_price = trade_price_base_sell(resource);
    }

    int adjustment = 0;
    if (current_price == amount) {
        return 1;
    } else if (current_price < amount) {
        adjustment = amount - current_price;
    } else {
        adjustment = current_price - amount;
    }

    int successfully_changed = trade_price_change(resource, adjustment);
    
    if (successfully_changed && show_message) {
        if (adjustment >= 0) {
            city_message_post(1, MESSAGE_PRICE_INCREASED, adjustment, resource);
        } else {
            city_message_post(1, MESSAGE_PRICE_DECREASED, -adjustment, resource);
        }
    }

    return 1;
}

int scenario_action_type_trade_price_adjust_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int adjustment = action->parameter2;
    int show_message = action->parameter3;

    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    if (adjustment == 0) {
        return 1;
    }

    int successfully_changed = trade_price_change(resource, adjustment);
    
    if (successfully_changed && show_message) {
        if (adjustment >= 0) {
            city_message_post(1, MESSAGE_PRICE_INCREASED, adjustment, resource);
        } else {
            city_message_post(1, MESSAGE_PRICE_DECREASED, -adjustment, resource);
        }
    }

    return 1;
}

int scenario_action_type_trade_problems_land_execute(scenario_action_t *action)
{
    int duration = action->parameter1;

    city_data.trade.months_since_last_land_trade_problem = 0;
    city_trade_start_land_trade_problems(duration);
    if (scenario_property_climate() == CLIMATE_DESERT) {
        city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS, 0, 0);
    } else {
        city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
    }

    return 1;
}

int scenario_action_type_trade_problems_sea_execute(scenario_action_t *action)
{
    int duration = action->parameter1;

    city_data.trade.months_since_last_sea_trade_problem = 0;
    city_trade_start_sea_trade_problems(duration);
    city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);

    return 1;
}

int scenario_action_type_trade_route_amount_execute(scenario_action_t *action)
{
    int route_id = action->parameter1;
    int resource = action->parameter2;
    int amount = action->parameter3;
    int show_message = action->parameter4;

    if (!trade_route_is_valid(route_id)) {
        return 0;
    }
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    trade_route_set_limit(route_id, resource, amount);
    if (show_message && empire_city_is_trade_route_open(route_id)) {
        int city_id = empire_city_get_for_trade_route(route_id);
        if (city_id < 0) {
            city_id = 0;
        }
        int last_amount = trade_route_limit(route_id, resource);

        int change = amount - last_amount;
        if (amount > 0 && change > 0) {
            city_message_post(1, MESSAGE_INCREASED_TRADING, city_id, resource);
        } else if (amount > 0 && change < 0) {
            city_message_post(1, MESSAGE_DECREASED_TRADING, city_id, resource);
        } else if (amount <= 0) {
            city_message_post(1, MESSAGE_TRADE_STOPPED, city_id, resource);
        }
    }

    return 1;
}
