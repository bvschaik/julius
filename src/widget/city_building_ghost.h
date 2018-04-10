#ifndef WIDGET_CITY_BUILDING_GHOST_H
#define WIDGET_CITY_BUILDING_GHOST_H

#include "map/point.h"

void city_building_ghost_record_view_position(int view_x, int view_y, int grid_offset);

void city_building_ghost_draw(const map_tile *tile);

#endif // WIDGET_CITY_BUILDING_GHOST_H
