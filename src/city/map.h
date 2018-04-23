#ifndef CITY_MAP_H
#define CITY_MAP_H

#include "map/point.h"

const map_tile *city_map_entry_flag();
const map_tile *city_map_exit_flag();

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

#endif // CITY_MAP_H
