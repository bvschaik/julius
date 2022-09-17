#ifndef BUILDING_BARRACKS_H
#define BUILDING_BARRACKS_H

#include "building/building.h"
#include "map/point.h"

#define MAX_WEAPONS_BARRACKS 4

int building_get_barracks_for_weapon(int x, int y, int resource, int road_network_id, int distance_from_entry, map_point *dst);

void building_barracks_add_weapon(building *barracks);

int building_barracks_create_soldier(building *barracks, int x, int y);

building *building_barracks_get_unmanned_tower(building *barracks, map_point *road);

int building_barracks_create_tower_sentry(building *barracks, int x, int y);

void building_barracks_toggle_priority(building* barracks);

int building_barracks_get_priority(building* barracks);


typedef enum {
	PRIORITY_TOWER = 0,
	PRIORITY_FORT = 1,
} barracks_priority;

#endif // BUILDING_BARRACKS_H
