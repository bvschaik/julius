#ifndef CITY_CULTURE_H
#define CITY_CULTURE_H

#include "city/constants.h"
#include "core/buffer.h"

#define THEATER_COVERAGE 750
#define AMPHITHEATER_COVERAGE 1200
#define TAVERN_COVERAGE 1200

void city_culture_update_coverage(void);

int city_culture_coverage_tavern(void);
int city_culture_coverage_theater(void);
int city_culture_coverage_amphitheater(void);
int city_culture_coverage_colosseum(void);
int city_culture_coverage_hippodrome(void);
int city_culture_coverage_average_entertainment(void);

int city_culture_coverage_religion(god_type god);

int city_culture_coverage_school(void);
int city_culture_coverage_library(void);
int city_culture_coverage_academy(void);

int city_culture_coverage_hospital(void);

int city_culture_average_education(void);
int city_culture_average_entertainment(void);
int city_culture_average_health(void);

void city_culture_calculate(void);

void city_culture_save_state(buffer *buf);

void city_culture_load_state(buffer *buf);

#endif // CITY_CULTURE_H
