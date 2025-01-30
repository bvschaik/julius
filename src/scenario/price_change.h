#ifndef SCENARIO_PRICE_CHANGE_H
#define SCENARIO_PRICE_CHANGE_H

#include "core/buffer.h"

#define MAX_ORIGINAL_PRICE_CHANGES 20

typedef struct {
    unsigned int id;
    int year;
    int month;
    int resource;
    int amount;
    int is_rise;
} price_change_t;

void scenario_price_change_init(void);

void scenario_price_change_clear_all(void);

int scenario_price_change_new(void);

void scenario_price_change_process(void);

const price_change_t *scenario_price_change_get(int id);

void scenario_price_change_update(const price_change_t *price_change);

void scenario_price_change_delete(int id);

void scenario_price_change_remap_resource(void);

int scenario_price_change_count_total(void);

void scenario_price_change_process(void);

void scenario_price_change_save_state(buffer *buf);

void scenario_price_change_load_state(buffer *buf);

void scenario_price_change_load_state_old_version(buffer *buf);

#endif // SCENARIO_PRICE_CHANGE_H
