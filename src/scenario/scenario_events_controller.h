#ifndef SCENARIO_EVENTS_CONTOLLER_H
#define SCENARIO_EVENTS_CONTOLLER_H

#include "building/building.h"
#include "core/buffer.h"
#include "scenario/scenario_event_data.h"

typedef enum {
    SCENARIO_EVENTS_VERSION = 2,

    SCENARIO_EVENTS_VERSION_NONE = 0,
    SCENARIO_EVENTS_VERSION_INITIAL = 1,
    SCENARIO_EVENTS_VERSION_TRIGGERS = 2,
} scenario_events_version;

typedef struct {
    event_trigger cause_of_context;
    building_type related_building_type;
} scenario_event_context_t;

void scenario_events_init(void);
void scenario_events_clear(void);
scenario_event_t *scenario_event_get(int event_id);
scenario_event_t *scenario_event_create(int repeat_min, int repeat_max, int max_repeats, int trigger_type);
void scenario_event_delete(scenario_event_t *event);
int scenario_events_get_count(void);

void scenario_events_save_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions);
void scenario_events_load_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions);

void scenario_events_set_context(event_trigger cause, building_type b_type);
void scenario_events_process_by_trigger_type(event_trigger type_to_process);
void scenario_events_full_process(event_trigger cause, int progress_count, building_type b_type);
scenario_event_t *scenario_events_get_using_custom_variable(int custom_variable_id);

scenario_event_context_t *scenario_events_get_context(void);

#endif // SCENARIO_EVENTS_CONTOLLER_H
