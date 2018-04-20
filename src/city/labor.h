#ifndef CITY_LABOR_H
#define CITY_LABOR_H

int city_labor_unemployment_percentage();
int city_labor_unemployment_percentage_for_senate();

int city_labor_wages();
void city_labor_change_wages(int amount);

int city_labor_wages_rome();
int city_labor_raise_wages_rome();
int city_labor_lower_wages_rome();

void city_labor_allocate_workers();

void city_labor_update();

void city_labor_set_priority(int category, int new_priority);

int city_labor_max_selectable_priority(int category);

#endif // CITY_LABOR_H
