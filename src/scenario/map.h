#ifndef SCENARIO_MAP_H
#define SCENARIO_MAP_H

#include "map/point.h"

void scenario_map_init(void);

int scenario_map_size(void);

void scenario_map_init_entry_exit(void);

map_point scenario_map_entry(void);

map_point scenario_map_exit(void);

int scenario_map_has_river_entry(void);

map_point scenario_map_river_entry(void);

int scenario_map_has_river_exit(void);

map_point scenario_map_river_exit(void);

void scenario_map_foreach_herd_point(void (*callback)(int x, int y));

void scenario_map_foreach_fishing_point(void (*callback)(int x, int y));

int scenario_map_closest_fishing_point(int x, int y, map_point *fish);

int scenario_map_has_flotsam(void);

#endif // SCENARIO_MAP_H
