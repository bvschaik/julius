#ifndef SCENARIO_REQUEST_H
#define SCENARIO_REQUEST_H

#include "core/buffer.h"

#define REQUESTS_DEFAULT_DEADLINE_YEARS 5
#define REQUESTS_DEFAULT_FAVOUR 8
#define REQUESTS_DEFAULT_MONTHS_TO_COMPLY 24
#define REQUESTS_DEFAULT_EXTENSION_DISFAVOUR 3
#define REQUESTS_DEFAULT_IGNORED_DISFAVOUR 5

#define MAX_ORIGINAL_REQUESTS 20
#define REQUESTS_REPEAT_INFINITE -1

typedef enum {
    REQUEST_STATE_NORMAL= 0,
    REQUEST_STATE_OVERDUE = 1,
    REQUEST_STATE_DISPATCHED = 2,
    REQUEST_STATE_DISPATCHED_LATE = 3,
    REQUEST_STATE_IGNORED = 4,
    REQUEST_STATE_RECEIVED = 5
} scenario_request_state;

typedef struct {
    unsigned int id;
    int year;
    int resource;
    struct {
        unsigned int min;
        unsigned int max;
        unsigned int requested;
    } amount;
    int deadline_years;
    int can_comply_dialog_shown;
    int favor;
    int month;
    scenario_request_state state;
    int visible;
    int months_to_comply;
    int extension_months_to_comply;
    int extension_disfavor;
    int ignored_disfavor;
    struct {
        int times;
        struct {
            unsigned int min;
            unsigned int max;
        } interval; // in years
    } repeat;
} scenario_request;

typedef enum {
    REQUESTS_OLD_STATE_SECTIONS_TARGET = 1,
    REQUESTS_OLD_STATE_SECTIONS_CAN_COMPLY = 2,
    REQUESTS_OLD_STATE_SECTIONS_FAVOR_REWARD = 3,
    REQUESTS_OLD_STATE_SECTIONS_ONGOING_INFO = 4
} requests_old_state_sections;

void scenario_request_clear_all(void);

void scenario_request_init(void);

int scenario_request_new(void);

void scenario_request_process(void);

void scenario_request_dispatch(int id);

int scenario_request_count_total(void);

int scenario_request_count_active(void);

int scenario_request_count_visible(void);

const scenario_request *scenario_request_get(int id);

void scenario_request_update(const scenario_request *request);

void scenario_request_delete(int id);

void scenario_request_remap_resource(void);

int scenario_request_foreach_visible(int start_index, void (*callback)(int index, const scenario_request *request));

const scenario_request *scenario_request_get_visible(int index);

int scenario_request_is_ongoing(int id);

int scenario_request_force_start(int id);

void scenario_request_save_state(buffer *list);
void scenario_request_load_state(buffer *list, int version);

void scenario_request_load_state_old_version(buffer *list, requests_old_state_sections section);

#endif // SCENARIO_REQUEST_H
