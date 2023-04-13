#ifndef SCENARIO_REQUEST_H
#define SCENARIO_REQUEST_H

#include "core/buffer.h"

typedef enum {
    REQUEST_STATE_NORMAL= 0,
    REQUEST_STATE_OVERDUE = 1,
    REQUEST_STATE_DISPATCHED = 2,
    REQUEST_STATE_DISPATCHED_LATE = 3,
    REQUEST_STATE_IGNORED = 4,
    REQUEST_STATE_RECEIVED = 5
} scenario_request_state;

typedef struct {
    int id;
    scenario_request_state state;
    int resource;
    int amount;
    int months_to_comply;
} scenario_request;

#define REQUESTS_DEFAULT_DEADLINE_YEARS 5
#define REQUESTS_DEFAULT_FAVOUR 8
#define REQUESTS_DEFAULT_MONTHS_TO_COMPLY 24
#define REQUESTS_DEFAULT_EXTENSION_DISFAVOUR 3
#define REQUESTS_DEFAULT_IGNORED_DISFAVOUR 5

typedef enum {
    REQUESTS_OLD_STATE_SECTIONS_TARGET = 1,
    REQUESTS_OLD_STATE_SECTIONS_CAN_COMPLY = 2,
    REQUESTS_OLD_STATE_SECTIONS_FAVOR_REWARD = 3,
    REQUESTS_OLD_STATE_SECTIONS_ONGOING_INFO = 4
} requests_old_state_sections;

void scenario_request_init(void);

void scenario_request_process(void);

void scenario_request_dispatch(int id);

int scenario_request_count_visible(void);

const scenario_request *scenario_request_get(int id);

int scenario_request_foreach_visible(int start_index, void (*callback)(int index, const scenario_request *request));

const scenario_request *scenario_request_get_visible(int index);

int scenario_request_is_ongoing(int id);

int scenario_request_force_start(int id);

void scenario_request_save_state(buffer *list);
void scenario_request_load_state(buffer *list);

void scenario_request_load_state_old_version(buffer *list, int state_version, requests_old_state_sections section);

#endif // SCENARIO_REQUEST_H
