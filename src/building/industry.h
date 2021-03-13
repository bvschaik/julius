#ifndef BUILDING_INDUSTRY_H
#define BUILDING_INDUSTRY_H

#include "building/building.h"
#include "map/point.h"

int building_is_farm(building_type type);
int building_is_raw_resource_producer(building_type type);
int building_is_workshop(building_type type);

void building_industry_update_production(void);
void building_industry_update_wheat_production(void);

int building_industry_has_produced_resource(building *b);
void building_industry_start_new_production(building *b);

void building_bless_farms(void);
void building_curse_farms(int big_curse);
void building_bless_industry(void);


void building_workshop_add_raw_material(building *b);

int building_get_workshop_for_raw_material(
    int x, int y, int resource, int distance_from_entry, int road_network_id, map_point *dst);
int building_get_workshop_for_raw_material_with_room(
    int x, int y, int resource, int distance_from_entry, int road_network_id, map_point *dst);

void building_industry_start_strikes(void);

#endif // BUILDING_INDUSTRY_H
