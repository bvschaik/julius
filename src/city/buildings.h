#ifndef CITY_BUILDINGS_H
#define CITY_BUILDINGS_H

#include "building/building.h"

int city_buildings_has_senate(void);
int city_buildings_has_governor_house(void);

int city_buildings_has_barracks(void);
int city_buildings_get_barracks(void);

int city_buildings_has_mess_hall(void);
int city_buildings_get_mess_hall(void);

int city_buildings_has_hippodrome(void);

int city_buildings_has_lighthouse(void);

int city_buildings_has_caravanserai(void);
int city_buildings_get_caravanserai(void);

int city_buildings_has_city_mint(void);

int city_buildings_triumphal_arch_available(void);
void city_buildings_earn_triumphal_arch(void);
void city_buildings_build_triumphal_arch(void);
void city_buildings_remove_triumphal_arch(void);

int city_buildings_has_working_dock(void);

void city_buildings_main_native_meeting_center(int *x, int *y);

int city_buildings_get_closest_plague(int x, int y, int *distance);
void city_buildings_update_plague(void);

#endif // CITY_BUILDINGS_H
