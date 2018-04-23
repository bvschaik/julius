#ifndef CITY_FESTIVAL_H
#define CITY_FESTIVAL_H

int city_festival_is_planned();
int city_festival_months_since_last();

int city_festival_small_cost();
int city_festival_large_cost();
int city_festival_grand_cost();
int city_festival_grand_wine();

int city_festival_out_of_wine();

int city_festival_selected_god();
void city_festival_select_god(int god_id);

int city_festival_selected_size();
int city_festival_select_size(int size);

void city_festival_schedule();

void city_festival_update();
void city_festival_calculate_costs();

#endif // CITY_FESTIVAL_H
