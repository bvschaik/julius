#ifndef CITY_EMPEROR_H
#define CITY_EMPEROR_H

enum {
    GIFT_MODEST = 0,
    GIFT_GENEROUS = 1,
    GIFT_LAVISH = 2
};

typedef struct {
    int id;
    int cost;
} emperor_gift;

void city_emperor_init_scenario(int rank);

void city_emperor_update();

void city_emperor_init_selected_gift();

void city_emperor_calculate_gift_costs();

int city_emperor_set_gift_size(int size);

int city_emperor_selected_gift_size();

int city_emperor_can_send_gift(int size);

const emperor_gift *city_emperor_get_gift(int size);

void city_emperor_send_gift();

int city_emperor_months_since_gift();

int city_emperor_salary_for_rank(int rank);

void city_emperor_set_salary_rank(int rank);

void city_emperor_mark_soldier_killed();

#endif // CITY_EMPEROR_H
