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

void city_emperor_update(void);

void city_emperor_init_selected_gift(void);

void city_emperor_calculate_gift_costs(void);

int city_emperor_set_gift_size(int size);

int city_emperor_selected_gift_size(void);

int city_emperor_can_send_gift(int size);

const emperor_gift *city_emperor_get_gift(int size);

void city_emperor_send_gift(void);

int city_emperor_months_since_gift(void);

int city_emperor_salary_for_rank(int rank);

void city_emperor_set_salary_rank(int rank);

int city_emperor_rank_for_salary_paid(int salary);

int city_emperor_salary_rank(void);
int city_emperor_salary_amount(void);

int city_emperor_personal_savings(void);
void city_emperor_add_personal_savings(int amount);

void city_emperor_init_donation_amount(void);
void city_emperor_set_donation_amount(int amount);
void city_emperor_change_donation_amount(int change);
void city_emperor_donate_savings_to_city(void);
int city_emperor_donate_amount(void);

int city_emperor_rank(void);

void city_emperor_mark_soldier_killed(void);

void city_emperor_force_attack(int size);

void city_emperor_decrement_personal_savings(int amount);

#endif // CITY_EMPEROR_H
