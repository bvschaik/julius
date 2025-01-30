#ifndef SCENARIO_DEMAND_CHANGE_H
#define SCENARIO_DEMAND_CHANGE_H

#include "core/buffer.h"

#define DEMAND_CHANGE_LEGACY_IS_RISE 9999
#define DEMAND_CHANGE_LEGACY_IS_FALL -9999

#define MAX_ORIGINAL_DEMAND_CHANGES 20

typedef struct {
    unsigned int id;
    int year;
    int month;
    int resource;
    int route_id;
    int amount;
} demand_change_t;

void scenario_demand_change_init(void);

void scenario_demand_change_clear_all(void);

int scenario_demand_change_new(void);

void scenario_demand_change_process(void);

const demand_change_t *scenario_demand_change_get(int id);

void scenario_demand_change_update(const demand_change_t *demand_change);

void scenario_demand_change_delete(int id);

void scenario_demand_change_remap_resource(void);

int scenario_demand_change_count_total(void);

void scenario_demand_change_save_state(buffer *buf);

void scenario_demand_change_load_state(buffer *buf);

void scenario_demand_change_load_state_old_version(buffer *buf, int is_legacy_change);

#endif // SCENARIO_DEMAND_CHANGE_H
