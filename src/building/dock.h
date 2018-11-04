#ifndef BUILDING_DOCK_H
#define BUILDING_DOCK_H

#include "building/building.h"

int building_dock_count_idle_dockers(const building *dock);

void building_dock_update_open_water_access(void);

int building_dock_is_connected_to_open_water(int x, int y);

int building_dock_get_free_destination(int ship_id, int *x_tile, int *y_tile);

int building_dock_get_queue_destination(int* x_tile, int* y_tile);

#endif // BUILDING_DOCK_H
