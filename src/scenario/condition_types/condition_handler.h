#ifndef CONDITION_HANDLER_H
#define CONDITION_HANDLER_H

#include "core/buffer.h"
#include "scenario/scenario_event_data.h"

void scenario_condition_type_init(scenario_condition_t *condition);
int scenario_condition_type_is_met(scenario_condition_t *condition);

void scenario_condition_type_delete(scenario_condition_t *condition);
void scenario_condition_type_save_state(buffer *buf, scenario_condition_t *condition, int link_type, int32_t link_id);
void scenario_condition_type_load_state(buffer *buf, scenario_condition_t *condition, int *link_type, int32_t *link_id);

#endif // CONDITION_HANDLER_H
