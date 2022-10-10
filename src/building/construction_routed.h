#ifndef BUILDING_CONSTRUCTION_ROUTED_H
#define BUILDING_CONSTRUCTION_ROUTED_H

#include "map/routing.h"

int building_construction_place_road(int measure_only, int x_start, int y_start, int x_end, int y_end);

int building_construction_place_highway(int measure_only, int x_start, int y_start, int x_end, int y_end);

int building_construction_place_wall(int measure_only, int x_start, int y_start, int x_end, int y_end);

int building_construction_place_aqueduct(int x_start, int y_start, int x_end, int y_end, int *cost);

int building_construction_place_aqueduct_for_reservoir(
    int measure_only, int x_start, int y_start, int x_end, int y_end, int *items);

#endif // BUILDING_CONSTRUCTION_ROUTED_H
