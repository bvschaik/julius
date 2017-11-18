#ifndef SCENARIO_MAP_H
#define SCENARIO_MAP_H

typedef struct {
    int x;
    int y;
} map_point;

int scenario_map_has_river_entry();

map_point scenario_map_river_entry();

int scenario_map_has_river_exit();

map_point scenario_map_river_exit();

#endif // SCENARIO_MAP_H
