#ifndef CITY_CULTURE_H
#define CITY_CULTURE_H

#include "core/buffer.h"

typedef enum {
    GOD_CERES = 0,
    GOD_NEPTUNE = 1,
    GOD_MERCURY = 2,
    GOD_MARS = 3,
    GOD_VENUS = 4
} god_t;


void city_culture_update_coverage();

int city_culture_coverage_theater();
int city_culture_coverage_amphitheater();
int city_culture_coverage_colosseum();
int city_culture_coverage_hippodrome();
int city_culture_coverage_average_entertainment();

int city_culture_coverage_religion(god_t god);

int city_culture_coverage_school();
int city_culture_coverage_library();
int city_culture_coverage_academy();

int city_culture_coverage_hospital();

void city_culture_save_state(buffer *buf);

void city_culture_load_state(buffer *buf);

#endif // CITY_CULTURE_H
