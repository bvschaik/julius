#ifndef SCENARIO_ALLOWED_BUILDING_H
#define SCENARIO_ALLOWED_BUILDING_H

#include "building/type.h"
#include "core/buffer.h"

#define MAX_ORIGINAL_ALLOWED_BUILDINGS 50

int scenario_allowed_building(building_type type);
void scenario_allowed_building_set(building_type type, int allowed);

const building_type *scenario_allowed_building_get_buildings_from_original_id(unsigned int original);

void scenario_allowed_building_enable_all(void);
void scenario_allowed_building_disable_all(void);

void scenario_allowed_building_load_state(buffer *buf);
void scenario_allowed_building_load_state_old_version(buffer *buf);
void scenario_allowed_building_save_state(buffer *buf);

#endif // SCENARIO_ALLOWED_BUILDING_H
