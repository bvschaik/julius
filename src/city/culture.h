#ifndef CITY_CULTURE_H
#define CITY_CULTURE_H

#include "city/constants.h"
#include "core/buffer.h"

#define THEATER_COVERAGE 500
#define THEATER_UPGRADE_BONUS_COVERAGE 700
#define AMPHITHEATER_COVERAGE 1200
#define TAVERN_COVERAGE 1200
#define ARENA_COVERAGE 2500
#define SCHOOL_COVERAGE 75
#define SCHOOL_UPGRADE_BONUS_COVERAGE 150
#define LIBRARY_COVERAGE 800
#define LIBRARY_UPGRADE_BONUS_COVERAGE 900
#define ACADEMY_COVERAGE 100
#define ACADEMY_UPGRADE_BONUS_COVERAGE 50
#define HOSPITAL_COVERAGE 1500

void city_culture_update_coverage(void);

int city_culture_coverage_tavern(void);
int city_culture_coverage_theater(void);
int city_culture_coverage_amphitheater(void);
int city_culture_coverage_arena(void);
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

int city_culture_get_theatre_person_coverage(void);
int city_culture_get_school_person_coverage(void);
int city_culture_get_library_person_coverage(void);
int city_culture_get_academy_person_coverage(void);
int city_culture_get_tavern_person_coverage(void);
int city_culture_get_ampitheatre_person_coverage(void);
int city_culture_get_arena_person_coverage(void);



void city_culture_save_state(buffer *buf);

void city_culture_load_state(buffer *buf);

#endif // CITY_CULTURE_H
