#ifndef CITY_MAP_H
#define CITY_MAP_H

#include "map/point.h"

const map_tile *city_map_entry_point(void);
const map_tile *city_map_exit_point(void);

const map_tile *city_map_entry_flag(void);
const map_tile *city_map_exit_flag(void);

void city_map_set_entry_point(int x, int y);
void city_map_set_exit_point(int x, int y);

/**
 * Sets entry flag tile
 * @return Grid offset of tile
 */
int city_map_set_entry_flag(int x, int y);

/**
 * Sets exit flag tile
 * @return Grid offset of tile
 */
int city_map_set_exit_flag(int x, int y);

/**
 * Returns the index of the road network recorded in the 10 largest road networks
 * @return index 0-9, or 11 if it's not one of the 10 largest
 */
int city_map_road_network_index(int network_id);

void city_map_clear_largest_road_networks(void);

void city_map_add_to_largest_road_networks(int network_id, int size);

#endif // CITY_MAP_H
