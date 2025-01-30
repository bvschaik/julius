#ifndef SCENARIO_EVENT_H
#define SCENARIO_EVENT_H

#include "core/buffer.h"
#include "scenario/event/data.h"

void scenario_event_new(scenario_event_t *event, unsigned int position);
int scenario_event_is_active(const scenario_event_t *event);

void scenario_event_init(scenario_event_t *event);

void scenario_event_save_state(buffer *buf, scenario_event_t *event);
void scenario_event_load_state(buffer *buf, scenario_event_t *event, int is_new_version);
scenario_condition_t *scenario_event_condition_create(scenario_condition_group_t *group, int type);
void scenario_event_link_condition_group(scenario_event_t *event, scenario_condition_group_t *group);
scenario_action_t *scenario_event_action_create(scenario_event_t *event, int type);
void scenario_event_link_action(scenario_event_t *event, scenario_action_t *action);

int scenario_event_count_conditions(const scenario_event_t *event);

int scenario_event_can_repeat(scenario_event_t *event);

int scenario_event_decrease_pause_time(scenario_event_t *event, int months_passed);
int scenario_event_conditional_execute(scenario_event_t *event);
int scenario_event_execute(scenario_event_t *event);
int scenario_event_uses_custom_variable(const scenario_event_t *event, int custom_variable_id);

#endif // SCENARIO_EVENT_H
