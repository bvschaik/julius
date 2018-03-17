#ifndef CITY_BUILDINGS_H
#define CITY_BUILDINGS_H

#include "building/building.h"

int city_buildings_has_senate();
void city_buildings_add_senate(building *senate);
void city_buildings_remove_senate(building *senate);

void city_buildings_add_barracks(building *barracks);
void city_buildings_remove_barracks(building *barracks);
int city_buildings_get_barracks();
void city_buildings_set_barracks(int building_id);

int city_buildings_has_distribution_center();
void city_buildings_add_distribution_center(building *center);
void city_buildings_remove_distribution_center(building *center);

int city_buildings_get_trade_center();
void city_buildings_set_trade_center(int building_id);

int city_buildings_has_hippodrome();
void city_buildings_add_hippodrome();
void city_buildings_remove_hippodrome();

#endif // CITY_BUILDINGS_H
