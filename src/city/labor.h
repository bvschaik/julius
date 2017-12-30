#ifndef CITY_LABOR_H
#define CITY_LABOR_H

void city_labor_allocate_workers();

void city_labor_update();

void city_labor_set_priority(int category, int new_priority);

int city_labor_max_selectable_priority(int category);

#endif // CITY_LABOR_H
