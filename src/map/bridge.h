#ifndef MAP_BRIDGE_H
#define MAP_BRIDGE_H

int map_bridge_building_length();

void map_bridge_reset_building_length();

int map_bridge_calculate_length_direction(int x, int y, int is_ship_bridge, int *length, int *direction);

/**
 * Adds a bridge to the terrain
 * @param x Map X
 * @param y Map Y
 * @param is_ship_bridge Whether this is a ship bridge or low bridge
 * @return Length of the bridge
 */
int map_bridge_add(int x, int y, int is_ship_bridge);

void map_bridge_remove(int grid_offset, int mark_deleted);

void map_bridge_update_after_rotate(int counter_clockwise);

int map_bridge_count_figures(int grid_offset);

int map_is_bridge(int grid_offset);

int map_bridge_height(int grid_offset);

#endif // MAP_BRIDGE_H
