#ifndef CITY_GODS_H
#define CITY_GODS_H

#define MAX_GODS 5

void city_gods_reset();

void city_gods_calculate_moods(int update_moods);

int city_gods_calculate_least_happy();

void city_gods_schedule_festival();

void city_gods_check_festival();

#endif // CITY_GODS_H
