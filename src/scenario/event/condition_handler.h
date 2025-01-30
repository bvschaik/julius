#ifndef CONDITION_HANDLER_H
#define CONDITION_HANDLER_H

#include "core/buffer.h"
#include "scenario/event/data.h"

void scenario_condition_group_new(scenario_condition_group_t *group, unsigned int id);
int scenario_condition_group_in_use(const scenario_condition_group_t *group);

void scenario_condition_type_init(scenario_condition_t *condition);
int scenario_condition_type_is_met(scenario_condition_t *condition);

void scenario_condition_type_delete(scenario_condition_t *condition);
void scenario_condition_group_save_state(buffer *buf, const scenario_condition_group_t *condition_group, int link_type,
    int32_t link_id);
void scenario_condition_load_state(buffer *buf, scenario_condition_group_t *group, scenario_condition_t *condition);
void scenario_condition_group_load_state(buffer *buf, scenario_condition_group_t *condition_group,
    int *link_type, int32_t *link_id);
int scenario_condition_uses_custom_variable(const scenario_condition_t *condition, int custom_variable_id);

#endif // CONDITION_HANDLER_H
