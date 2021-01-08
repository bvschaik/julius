#ifndef WIDGET_CITY_BUILDING_GHOST_H
#define WIDGET_CITY_BUILDING_GHOST_H

#include "map/point.h"

int city_building_ghost_mark_deleting(const map_tile *tile);
void city_building_ghost_draw(const map_tile *tile);
void city_building_ghost_toggle_mergers_display();
void city_building_ghost_draw_house_merge_locations();

#endif // WIDGET_CITY_BUILDING_GHOST_H
