#ifndef WIDGET_CITY_BUILDING_GHOST_H
#define WIDGET_CITY_BUILDING_GHOST_H

#include "map/point.h"

void city_building_ghost_draw_well_range(int x, int y, int grid_offset);
void city_building_ghost_draw_fountain_range(int x, int y, int grid_offset);
int city_building_ghost_mark_deleting(const map_tile *tile);
void city_building_ghost_draw(const map_tile *tile);

#endif // WIDGET_CITY_BUILDING_GHOST_H
