#ifndef BUILDING_BARRACKS_H
#define BUILDING_BARRACKS_H

#include "building/building.h"
#include "map/point.h"

int building_get_barracks_for_weapon(int resource, int road_network_id, map_point *dst);

void building_barracks_add_weapon(building *barracks);

int building_barracks_create_soldier(building *barracks, int x, int y);

int building_barracks_create_tower_sentry(building *barracks, int x, int y);

void building_barracks_request_tower_sentry(void);

void building_barracks_decay_tower_sentry_request(void);

int building_barracks_has_tower_sentry_request(void);

void building_barracks_save_state(buffer *buf);

void building_barracks_load_state(buffer *buf);

#endif // BUILDING_BARRACKS_H
