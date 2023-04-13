#ifndef ACTION_TYPES_H
#define ACTION_TYPES_H

#include "scenario/scenario_event_data.h"

int scenario_action_type_change_allowed_buildings_execute(scenario_action_t *action);

int scenario_action_type_change_resource_produced_execute(scenario_action_t *action);

int scenario_action_type_empire_map_convert_future_trade_city_execute(scenario_action_t *action);

int scenario_action_type_favor_add_execute(scenario_action_t *action);

int scenario_action_type_gladiator_revolt_execute(scenario_action_t *action);

void scenario_action_type_money_add_init(scenario_action_t *action);
int scenario_action_type_money_add_execute(scenario_action_t *action);

int scenario_action_type_request_immediately_start_execute(scenario_action_t *action);

void scenario_action_type_rome_wages_init(scenario_action_t *action);
int scenario_action_type_rome_wages_execute(scenario_action_t *action);

void scenario_action_type_savings_add_init(scenario_action_t *action);
int scenario_action_type_savings_add_execute(scenario_action_t *action);

int scenario_action_type_send_standard_message_execute(scenario_action_t *action);

int scenario_action_type_trade_price_set_execute(scenario_action_t *action);

int scenario_action_type_trade_price_adjust_execute(scenario_action_t *action);

int scenario_action_type_trade_problems_land_execute(scenario_action_t *action);

int scenario_action_type_trade_problems_sea_execute(scenario_action_t *action);

int scenario_action_type_trade_route_amount_execute(scenario_action_t *action);

void scenario_action_type_city_health_init(scenario_action_t *action);
int scenario_action_type_city_health_execute(scenario_action_t *action);

#endif // ACTION_TYPES_H
