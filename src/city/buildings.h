#ifndef CITY_BUILDINGS_H
#define CITY_BUILDINGS_H

#include "building/building.h"

int city_buildings_has_senate(void);
void city_buildings_add_senate(building *senate);
void city_buildings_remove_senate(building *senate);

void city_buildings_add_barracks(building *barracks);
void city_buildings_remove_barracks(building *barracks);
int city_buildings_get_barracks(void);
void city_buildings_set_barracks(int building_id);

int city_buildings_has_distribution_center(void);
void city_buildings_add_distribution_center(building *center);
void city_buildings_remove_distribution_center(building *center);

int city_buildings_get_trade_center(void);
void city_buildings_set_trade_center(int building_id);

int city_buildings_has_hippodrome(void);
void city_buildings_add_hippodrome(void);
void city_buildings_remove_hippodrome(void);

int city_buildings_triumphal_arch_available(void);
void city_buildings_earn_triumphal_arch(void);
void city_buildings_build_triumphal_arch(void);

void city_buildings_add_dock(void);
void city_buildings_remove_dock(void);

void city_buildings_reset_dock_wharf_counters(void);
void city_buildings_add_working_wharf(int needs_fishing_boat);
void city_buildings_add_working_dock(int building_id);
int city_buildings_shipyard_boats_requested(void);
int city_buildings_has_working_dock(void);
int city_buildings_get_working_dock(int index);

void city_buildings_main_native_meeting_center(int *x, int *y);
void city_buildings_set_main_native_meeting_center(int x, int y);

int city_buildings_is_mission_post_operational(void);
void city_buildings_set_mission_post_operational(void);

int city_buildings_unknown_value(void);

#endif // CITY_BUILDINGS_H
