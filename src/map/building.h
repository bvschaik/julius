#ifndef MAP_BUILDING_H
#define MAP_BUILDING_H

#include "building/type.h"
#include "core/buffer.h"

/**
 * Returns the building at the given offset
 * @param grid_offset Map offset
 * @return Building ID of building at offset, 0 means no building
 */
int map_building_at(int grid_offset);

int map_building_from_buffer(buffer *buildings, int grid_offset);

void map_building_set(int grid_offset, int building_id);

/**
 * Increases building damage by 1
 * @param grid_offset Map offset
 * @return New damage amount
 */
int map_building_damage_increase(int grid_offset);

void map_building_damage_clear(int grid_offset);

int map_rubble_building_type(int grid_offset);

void map_set_rubble_building_type(int grid_offset, building_type type);

/**
 * Clears the maps related to buildings
 */
void map_building_clear(void);

void map_building_save_state(buffer *buildings, buffer *damage);

void map_building_load_state(buffer *buildings, buffer *damage);

int map_building_is_reservoir(int x, int y);

#endif // MAP_BUILDING_H
