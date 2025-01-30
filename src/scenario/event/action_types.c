#include "action_types.h"

#include "building/destruction.h"
#include "building/granary.h"
#include "building/menu.h"
#include "building/warehouse.h"
#include "city/data_private.h"
#include "city/emperor.h"
#include "city/finance.h"
#include "city/gods.h"
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
#include "game/resource.h"
#include "map/building.h"
#include "map/grid.h"
#include "scenario/allowed_building.h"
#include "scenario/custom_variable.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/custom_messages.h"
#include "scenario/invasion.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "scenario/scenario.h"

#include <stdlib.h>

int scenario_action_type_change_allowed_buildings_execute(scenario_action_t *action)
{
    int building_id = action->parameter1;
    int allowed = action->parameter2;

    scenario_allowed_building_set(building_id, allowed);
    building_menu_update();
    
    return 1;
}

int scenario_action_type_change_city_rating_execute(scenario_action_t *action)
{
    selected_rating rating = action->parameter1;
    int value = action->parameter2;
    int is_hard_set = action->parameter3;

    switch (rating) {
        case SELECTED_RATING_PROSPERITY:
            if (is_hard_set) {
                city_ratings_set_prosperity(value);
            } else {
                city_ratings_change_prosperity(value);
            }
            break;
        case SELECTED_RATING_PEACE:
            if (is_hard_set) {
                city_ratings_set_peace(value);
            } else {
                city_ratings_change_peace(value);
            }
            break;
        default:
            break;
    }

    return 1;
}

int scenario_action_type_change_custom_variable_execute(scenario_action_t *action)
{
    int variable_id = action->parameter1;
    int value = action->parameter2;
    int is_hard_set = action->parameter3;

    if (!is_hard_set) {
        value = scenario_custom_variable_get_value(variable_id) + value;
    }

    scenario_custom_variable_set_value(variable_id, value);
    
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

int scenario_action_type_change_resource_stockpiles_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int amount = action->parameter2;
    storage_types storage_type = action->parameter3;
    int respect_settings = action->parameter4;
    
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }
    if (amount == 0) {
        return 1;
    }

    int remaining = abs(amount);
    int to_remove = (amount < 0);
    switch(storage_type) {
        case STORAGE_TYPE_ALL:
            if (to_remove) {
                remaining = building_warehouses_remove_resource(resource, remaining);
                remaining = building_granaries_remove_resource(resource, remaining * RESOURCE_ONE_LOAD);
            } else {
                remaining = building_warehouses_add_resource(resource, remaining, respect_settings);
                remaining = building_granaries_add_resource(resource, remaining * RESOURCE_ONE_LOAD, respect_settings);
            }
            break;
        case STORAGE_TYPE_GRANARIES:
            if (to_remove) {
                remaining = building_granaries_remove_resource(resource, remaining * RESOURCE_ONE_LOAD);
            } else {
                remaining = building_granaries_add_resource(resource, remaining * RESOURCE_ONE_LOAD, respect_settings);
            }
            break;
        case STORAGE_TYPE_WAREHOUSES:
            if (to_remove) {
                remaining = building_warehouses_remove_resource(resource, remaining);
            } else {
                remaining = building_warehouses_add_resource(resource, remaining, respect_settings);
            }
            break;
        default:
            break;
    }

    return 1;
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
        building_menu_update();

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

int scenario_action_type_invasion_immediate_execute(scenario_action_t *action)
{
    int attack_type = action->parameter1;
    int size = action->parameter2;
    int invasion_point = action->parameter3;
    int target_type = action->parameter4;
    int enemy_id = action->parameter5;

    // Invasion points are internally 0-indexed, but the map editor displays them as 1-indexed, convert to adjust for this
    invasion_point -= 1;

    if (invasion_point < 0 || invasion_point > MAX_INVASION_POINTS) {
        return 0;
    }
    if (enemy_id < ENEMY_UNDEFINED || enemy_id >= ENEMY_MAX) {
        enemy_id = ENEMY_UNDEFINED;
    }

    scenario_invasion_start_from_action(attack_type, size, invasion_point, target_type, enemy_id);

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
    if (request_id < 0 || request_id >= scenario_request_count_total()) {
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

int scenario_action_type_building_force_collapse_execute(scenario_action_t *action)
{
    int grid_offset = action->parameter1;
    int block_radius = action->parameter2;
    building_type type = action->parameter3;
    int destroy_all = action->parameter4;

    if (!map_grid_is_valid_offset(grid_offset)) {
        return 0;
    }

    for (int y = -block_radius; y <= block_radius; y++) {
        for (int x = -block_radius; x <= block_radius; x++) {
            int current_grid_offset = map_grid_add_delta(grid_offset, x, y);
            if (!map_grid_is_valid_offset(current_grid_offset)) {
                continue;
            }
            int building_id = map_building_at(current_grid_offset);
            if (!building_id) {
                continue;
            }
            building *b = building_main(building_get(building_id));
            if (b->type == BUILDING_BURNING_RUIN) {
                continue;
            }
            if ((b->state != BUILDING_STATE_IN_USE && b->state != BUILDING_STATE_MOTHBALLED) || b->is_deleted) {
                continue;
            }
            if (destroy_all || b->type == type) {
                building_destroy_by_collapse(b);
            }
        }
    }
    
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
    } else {
        adjustment = amount - current_price;
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

int scenario_action_type_trade_set_buy_price_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int amount = action->parameter2;
    
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    return trade_price_set_buy(resource, amount);
}

int scenario_action_type_trade_set_sell_price_execute(scenario_action_t *action)
{
    int resource = action->parameter1;
    int amount = action->parameter2;
    
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    return trade_price_set_sell(resource, amount);
}

int scenario_action_type_trade_add_new_resource_execute(scenario_action_t *action)
{
    int route_id = action->parameter1;
    int resource = action->parameter2;
    int amount = action->parameter3;
    int add_as_buying = action->parameter4;
    int show_message = action->parameter5;

    if (amount <= 0) {
        return 0;
    }
    if (!trade_route_is_valid(route_id)) {
        return 0;
    }
    if (resource < RESOURCE_MIN || resource > RESOURCE_MAX) {
        return 0;
    }

    int city_id = empire_city_get_for_trade_route(route_id);
    if (show_message && empire_city_is_trade_route_open(route_id)) {
        if (city_id < 0) {
            city_id = 0;
        }
        city_message_post(1, MESSAGE_INCREASED_TRADING, city_id, resource);
    }

    empire_city *empire_city = empire_city_get(city_id);
    if (add_as_buying) {
        empire_city_change_buying_of_resource(empire_city, resource, amount);
    } else {
        empire_city_change_selling_of_resource(empire_city, resource, amount);
        building_menu_update();
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

int scenario_action_type_trade_route_adjust_open_price_execute(scenario_action_t *action)
{
    int route_id = action->parameter1;
    int amount = action->parameter2;
    int is_hard_set = action->parameter3;
    int show_message = action->parameter4;

    if (!trade_route_is_valid(route_id)) {
        return 0;
    }

    int old_cost = empire_city_get_trade_route_cost(route_id);
    if (!is_hard_set) {
        amount = old_cost + amount;
    }

    empire_city_set_trade_route_cost(route_id, amount);

    if (show_message && !empire_city_is_trade_route_open(route_id)) {
        int city_id = empire_city_get_for_trade_route(route_id);
        if (city_id < 0) {
            city_id = 0;
        }
        city_message_post(1, MESSAGE_ROUTE_PRICE_CHANGE, city_id, amount - old_cost);
    }
    return 1;
}

int scenario_action_type_trade_route_open_execute(scenario_action_t *action)
{
    int route_id = action->parameter1;
    int apply_cost = action->parameter2;

    if (!trade_route_is_valid(route_id)) {
        return 0;
    }
    int city_id = empire_city_get_for_trade_route(route_id);

    empire_city_open_trade(city_id, apply_cost);
    building_menu_update();

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
    trade_route_set_limit(route_id, resource, amount);
    building_menu_update();

    return 1;
}

int scenario_action_type_show_custom_message_execute(scenario_action_t *action)
{
    int custom_message_id = action->parameter1;

    int custom_message_count = custom_messages_count();
    if (custom_message_id < 1 || custom_message_id > custom_message_count) {
        return 0;
    }

    custom_message_t *message = custom_messages_get(custom_message_id);

    if (message && message->in_use) {
        city_message_post(1, MESSAGE_CUSTOM_MESSAGE, message->id, 0);
    }
    return 1;
}

int scenario_action_type_tax_rate_set_execute(scenario_action_t *action)
{
    int new_rate = action->parameter1;

    city_finance_set_tax_percentage(new_rate);

    return 1;
}

int scenario_action_type_blessing_execute(scenario_action_t *action)
{
    int god = action->parameter1;

    city_god_blessing(god);

    return 1;
}

int scenario_action_type_minor_curse_execute(scenario_action_t *action)
{
    int god = action->parameter1;

    city_god_curse(god,0);

    return 1;
}

int scenario_action_type_major_curse_execute(scenario_action_t *action)
{
    int god = action->parameter1;

    city_god_curse(god, 1);

    return 1;
}

int scenario_action_type_change_climate_execute(scenario_action_t *action)
{
    int climate = action->parameter1;

    scenario_change_climate(climate);

    return 1;
}
