#ifndef CITY_EMPEROR_H
#define CITY_EMPEROR_H

void city_emperor_init_scenario(int rank);

void city_emperor_update();

void city_emperor_calculate_gift_costs();

void city_emperor_send_gift();

int city_emperor_salary_for_rank(int rank);

void city_emperor_set_salary_rank(int rank);

#endif // CITY_EMPEROR_H
