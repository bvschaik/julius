#ifndef CITY_LABOR_H
#define CITY_LABOR_H

typedef struct {
    int workers_needed;
    int workers_allocated;
    int buildings;
    int priority;
    int total_houses_covered;
} labor_category_data;

int city_labor_unemployment_percentage();
int city_labor_unemployment_percentage_for_senate();

int city_labor_workers_needed();
int city_labor_workers_employed();
int city_labor_workers_unemployed();

int city_labor_wages();
void city_labor_change_wages(int amount);

int city_labor_wages_rome();
int city_labor_raise_wages_rome();
int city_labor_lower_wages_rome();

const labor_category_data *city_labor_category(int category);

void city_labor_allocate_workers();

void city_labor_update();

void city_labor_set_priority(int category, int new_priority);

int city_labor_max_selectable_priority(int category);

#endif // CITY_LABOR_H
