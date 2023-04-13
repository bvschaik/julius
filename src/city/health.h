#ifndef CITY_HEALTH_H
#define CITY_HEALTH_H

#include "building/building.h"
#include "figure/figure.h"

#define SICKNESS_LEVEL_LOW 0
#define SICKNESS_LEVEL_MEDIUM 1
#define SICKNESS_LEVEL_HIGH 2
#define SICKNESS_LEVEL_PLAGUE 3
#define LOW_SICKNESS_LEVEL 30
#define MEDIUM_SICKNESS_LEVEL 60
#define HIGH_SICKNESS_LEVEL 90
#define MAX_SICKNESS_LEVEL 100

int city_health(void);

void city_health_change(int amount);

void city_health_set(int new_value);

void city_health_update(void);

void city_health_update_sickness_level_in_building(int building_id);

void city_health_dispatch_sickness(figure *f);

int city_health_get_global_sickness_level(void);

int city_health_get_population_with_clinic_access(void);
int city_health_get_population_with_barber_access(void);
int city_health_get_population_with_baths_access(void);

#endif // CITY_HEALTH_H
