#ifndef SCENARIO_EVENT_H
#define SCENARIO_EVENT_H

#include "core/buffer.h"
#include "scenario/scenario_event_data.h"

void scenario_event_init(scenario_event_t *event);

void scenario_event_save_state(buffer *buf, scenario_event_t *event);
void scenario_event_load_state(buffer *buf, scenario_event_t *event);
scenario_condition_t *scenario_event_get_condition(scenario_event_t *event, int id);
scenario_condition_t *scenario_event_condition_create(scenario_event_t *event, int type);
void scenario_event_link_condition(scenario_event_t *event, scenario_condition_t *condition);
scenario_action_t *scenario_event_get_action(scenario_event_t *event, int id);
scenario_action_t *scenario_event_action_create(scenario_event_t *event, int type);
void scenario_event_link_action(scenario_event_t *event, scenario_action_t *action);
void scenario_event_initialize_new(scenario_event_t *event, int position);
int scenario_event_is_valid(const scenario_event_t *event);


int scenario_event_can_repeat(scenario_event_t *event);
int scenario_event_all_conditions_met(scenario_event_t *event);

int scenario_event_decrease_pause_time(scenario_event_t *event, int months_passed);
int scenario_event_conditional_execute(scenario_event_t *event);
int scenario_event_execute(scenario_event_t *event);

#endif // SCENARIO_EVENT_H
