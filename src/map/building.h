#ifndef MAP_BUILDING_H
#define MAP_BUILDING_H

#include "core/buffer.h"

/**
 * Returns the building at the given offset
 * @param grid_offset Map offset
 * @return Building ID of building at offset, 0 means no building
 */
int map_building_at(int grid_offset);

void map_building_set(int grid_offset, int building_id);

/**
 * Clears the map
 */
void map_building_clear();

void map_building_save_state(buffer *buf);

void map_building_load_state(buffer *buf);

#endif // MAP_BUILDING_H
