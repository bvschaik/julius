#ifndef ACTION_TYPES_H
#define ACTION_TYPES_H

#include "scenario/event/data.h"

int scenario_action_type_change_allowed_buildings_execute(scenario_action_t *action);

int scenario_action_type_change_city_rating_execute(scenario_action_t *action);

int scenario_action_type_change_custom_variable_execute(scenario_action_t *action);

int scenario_action_type_change_resource_produced_execute(scenario_action_t *action);

int scenario_action_type_change_resource_stockpiles_execute(scenario_action_t *action);

int scenario_action_type_empire_map_convert_future_trade_city_execute(scenario_action_t *action);

int scenario_action_type_favor_add_execute(scenario_action_t *action);

int scenario_action_type_gladiator_revolt_execute(scenario_action_t *action);

int scenario_action_type_invasion_immediate_execute(scenario_action_t *action);

void scenario_action_type_money_add_init(scenario_action_t *action);
int scenario_action_type_money_add_execute(scenario_action_t *action);

int scenario_action_type_request_immediately_start_execute(scenario_action_t *action);

void scenario_action_type_rome_wages_init(scenario_action_t *action);
int scenario_action_type_rome_wages_execute(scenario_action_t *action);

void scenario_action_type_savings_add_init(scenario_action_t *action);
int scenario_action_type_savings_add_execute(scenario_action_t *action);

int scenario_action_type_building_force_collapse_execute(scenario_action_t *action);

int scenario_action_type_send_standard_message_execute(scenario_action_t *action);

int scenario_action_type_trade_price_set_execute(scenario_action_t *action);

int scenario_action_type_trade_set_buy_price_execute(scenario_action_t *action);
int scenario_action_type_trade_set_sell_price_execute(scenario_action_t *action);

int scenario_action_type_trade_add_new_resource_execute(scenario_action_t *action);

int scenario_action_type_trade_price_adjust_execute(scenario_action_t *action);

int scenario_action_type_trade_problems_land_execute(scenario_action_t *action);

int scenario_action_type_trade_problems_sea_execute(scenario_action_t *action);

int scenario_action_type_trade_route_adjust_open_price_execute(scenario_action_t *action);

int scenario_action_type_trade_route_open_execute(scenario_action_t *action);

int scenario_action_type_trade_route_amount_execute(scenario_action_t *action);

void scenario_action_type_city_health_init(scenario_action_t *action);
int scenario_action_type_city_health_execute(scenario_action_t *action);

int scenario_action_type_show_custom_message_execute(scenario_action_t *action);

int scenario_action_type_tax_rate_set_execute(scenario_action_t *action);

int scenario_action_type_blessing_execute(scenario_action_t *action);
int scenario_action_type_minor_curse_execute(scenario_action_t *action);
int scenario_action_type_major_curse_execute(scenario_action_t *action);

int scenario_action_type_change_climate_execute(scenario_action_t *action);


#endif // ACTION_TYPES_H
