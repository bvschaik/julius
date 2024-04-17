#ifndef BUILDING_BARRACKS_H
#define BUILDING_BARRACKS_H

#include "building/building.h"
#include "map/point.h"

#define MAX_WEAPONS_BARRACKS 4

int building_get_barracks_for_weapon(int x, int y, int resource, int road_network_id, map_point *dst);

void building_barracks_add_weapon(building *barracks);

int building_barracks_create_soldier(building *barracks, int x, int y);

building *building_barracks_get_unmanned_tower(building *barracks, map_point *road);

int building_barracks_create_tower_sentry(building *barracks, int x, int y);

void building_barracks_set_priority(building* barracks, int priority);

void building_barracks_toggle_delivery(building* barracks);

int building_barracks_get_priority(building* barracks);


typedef enum {
	PRIORITY_FORT = 0,
	PRIORITY_FORT_JAVELIN = 1,
	PRIORITY_FORT_MOUNTED = 2,
	PRIORITY_FORT_AUXILIA_INFANTRY = 3,	
	PRIORITY_FORT_AUXILIA_ARCHERY = 4,
	PRIORITY_TOWER = 5,
	PRIORITY_WATCHTOWER = 6,
} barracks_priority;

#endif // BUILDING_BARRACKS_H
