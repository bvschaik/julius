#ifndef CITY_LABOR_H
#define CITY_LABOR_H

typedef struct {
    int workers_needed;
    int workers_allocated;
    int buildings;
    int priority;
    int total_houses_covered;
} labor_category_data;

int city_labor_unemployment_percentage(void);
int city_labor_unemployment_percentage_for_senate(void);

int city_labor_workers_needed(void);
int city_labor_workers_employed(void);
int city_labor_workers_unemployed(void);

int city_labor_wages(void);
void city_labor_change_wages(int amount);

int city_labor_wages_rome(void);
int city_labor_raise_wages_rome(void);
int city_labor_lower_wages_rome(void);

const labor_category_data *city_labor_category(int category);

void city_labor_calculate_workers(int num_plebs, int num_patricians);

void city_labor_allocate_workers(void);

void city_labor_update(void);

void city_labor_set_priority(int category, int new_priority);

int city_labor_max_selectable_priority(int category);

#endif // CITY_LABOR_H
