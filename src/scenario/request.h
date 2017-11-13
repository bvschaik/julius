#ifndef SCENARIO_REQUEST_H
#define SCENARIO_REQUEST_H

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

void scenario_request_init();

void scenario_request_process();

void scenario_request_dispatch();

const scenario_request *scenario_request_get(int id);

int scenario_request_foreach_visible(int start_index, void (*callback)(int index, const scenario_request *request));

const scenario_request *scenario_request_get_visible(int index);

#endif // SCENARIO_REQUEST_H
