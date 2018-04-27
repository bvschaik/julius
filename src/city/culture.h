#ifndef CITY_CULTURE_H
#define CITY_CULTURE_H

#include "city/constants.h"
#include "core/buffer.h"

void city_culture_update_coverage();

int city_culture_coverage_theater();
int city_culture_coverage_amphitheater();
int city_culture_coverage_colosseum();
int city_culture_coverage_hippodrome();
int city_culture_coverage_average_entertainment();

int city_culture_coverage_religion(god_type god);

int city_culture_coverage_school();
int city_culture_coverage_library();
int city_culture_coverage_academy();

int city_culture_coverage_hospital();

int city_culture_average_education();
int city_culture_average_entertainment();
int city_culture_average_health();

void city_culture_calculate();

void city_culture_save_state(buffer *buf);

void city_culture_load_state(buffer *buf);

#endif // CITY_CULTURE_H
