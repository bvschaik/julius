#ifndef SCENARIO_EVENTS_CONTOLLER_H
#define SCENARIO_EVENTS_CONTOLLER_H

#include "core/buffer.h"
#include "scenario/event/data.h"

typedef enum {
    SCENARIO_EVENTS_VERSION = 1,

    SCENARIO_EVENTS_VERSION_NONE = 0,
    SCENARIO_EVENTS_VERSION_INITIAL = 1,
} scenario_events_version;

void scenario_events_init(void);
void scenario_events_clear(void);
scenario_event_t *scenario_event_get(int event_id);
scenario_event_t *scenario_event_create(int repeat_min, int repeat_max, int max_repeats);
void scenario_event_delete(scenario_event_t *event);
int scenario_events_get_count(void);

void scenario_events_save_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions);
void scenario_events_load_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions, int is_new_version);

void scenario_events_process_all(void);
void scenario_events_progress_paused(int months_passed);
scenario_event_t *scenario_events_get_using_custom_variable(int custom_variable_id);

#endif // SCENARIO_EVENTS_CONTOLLER_H
