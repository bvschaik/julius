#ifndef BUILDING_BARRACKS_H
#define BUILDING_BARRACKS_H

#include "building/building.h"

int building_get_barracks_for_weapon(int resource, int road_network_id, int *x_dst, int *y_dst);

void building_barracks_add_weapon(building *barracks);

int building_barracks_create_soldier(building *barracks, int x, int y);

int building_barracks_create_tower_sentry(building *barracks, int x, int y);

#endif // BUILDING_BARRACKS_H
