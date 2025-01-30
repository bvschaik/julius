#ifndef CONDITION_TYPES_H
#define CONDITION_TYPES_H

#include "scenario/event/data.h"

int scenario_condition_type_building_count_active_met(const scenario_condition_t *condition);

int scenario_condition_type_building_count_any_met(const scenario_condition_t *condition);

int scenario_condition_type_building_count_area_met(const scenario_condition_t *condition);

int scenario_condition_type_city_population_met(const scenario_condition_t *condition);

int scenario_condition_type_count_own_troops_met(const scenario_condition_t *condition);

int scenario_condition_type_custom_variable_check_met(const scenario_condition_t *condition);

int scenario_condition_type_difficulty_met(const scenario_condition_t *condition);

int scenario_condition_type_money_met(const scenario_condition_t *condition);

int scenario_condition_type_population_unemployed_met(const scenario_condition_t *condition);

int scenario_condition_type_request_is_ongoing_met(const scenario_condition_t *condition);

int scenario_condition_type_resource_storage_available_met(const scenario_condition_t *condition);

int scenario_condition_type_resource_stored_count_met(const scenario_condition_t *condition);

int scenario_condition_type_rome_wages_met(const scenario_condition_t *condition);

int scenario_condition_type_savings_met(const scenario_condition_t *condition);

int scenario_condition_type_stats_city_health_met(const scenario_condition_t *condition);

int scenario_condition_type_stats_culture_met(const scenario_condition_t *condition);

int scenario_condition_type_stats_favor_met(const scenario_condition_t *condition);

int scenario_condition_type_stats_peace_met(const scenario_condition_t *condition);

int scenario_condition_type_stats_prosperity_met(const scenario_condition_t *condition);

void scenario_condition_type_time_init(scenario_condition_t *condition);
int scenario_condition_type_time_met(const scenario_condition_t *condition);

int scenario_condition_type_trade_route_open_met(const scenario_condition_t *condition);

int scenario_condition_type_trade_route_price_met(const scenario_condition_t *condition);

int scenario_condition_type_trade_sell_price_met(const scenario_condition_t *condition);

int scenario_condition_type_tax_rate_met(const scenario_condition_t *condition);

#endif // CONDITION_TYPES_H
