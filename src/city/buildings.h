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

int city_buildings_triumphal_arch_available();
void city_buildings_earn_triumphal_arch();
void city_buildings_build_triumphal_arch();

void city_buildings_add_dock();
void city_buildings_remove_dock();

void city_buildings_reset_dock_wharf_counters();
void city_buildings_add_working_wharf(int needs_fishing_boat);
void city_buildings_add_working_dock(int building_id);
int city_buildings_shipyard_boats_requested();
int city_buildings_has_working_dock();
int city_buildings_get_working_dock(int index);

void city_buildings_main_native_meeting_center(int *x, int *y);
void city_buildings_set_main_native_meeting_center(int x, int y);

#endif // CITY_BUILDINGS_H
