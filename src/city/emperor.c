#include "emperor.h"

#include "city/data_private.h"
#include "city/finance.h"
#include "city/message.h"
#include "city/ratings.h"
#include "core/calc.h"
#include "figure/formation.h"
#include "game/difficulty.h"
#include "game/time.h"
#include "scenario/property.h"
#include "scenario/invasion.h"

const int SALARY_FOR_RANK[11] = {0, 2, 5, 8, 12, 20, 30, 40, 60, 80, 100};

void city_emperor_init_scenario(int rank)
{
    city_data.ratings.favor = scenario_starting_favor();
    city_data.emperor.personal_savings = scenario_starting_personal_savings();
    city_data.emperor.player_rank = rank;
    int salary_rank = rank;
    if (scenario_is_custom()) {
        city_data.emperor.personal_savings = 0;
        city_data.emperor.player_rank = scenario_property_player_rank();
        salary_rank = scenario_property_player_rank();
    }
    if (salary_rank > 10) {
        salary_rank = 10;
    }
    city_emperor_set_salary_rank(salary_rank);
}

static void update_debt_state(void)
{
    if (city_data.finance.treasury >= 0) {
        city_data.emperor.months_in_debt = -1;
        return;
    }
    if (city_data.emperor.debt_state == 0) {
        // provide bailout
        int rescue_loan = difficulty_adjust_money(scenario_rescue_loan());
        city_finance_process_donation(rescue_loan);
        city_finance_calculate_totals();

        city_data.emperor.debt_state = 1;
        city_data.emperor.months_in_debt = 0;
        city_message_post(1, MESSAGE_CITY_IN_DEBT, 0, 0);
        city_ratings_reduce_prosperity_after_bailout();
    } else if (city_data.emperor.debt_state == 1) {
        city_data.emperor.debt_state = 2;
        city_data.emperor.months_in_debt = 0;
        city_message_post(1, MESSAGE_CITY_IN_DEBT_AGAIN, 0, 0);
        city_ratings_change_favor(-5);
    } else if (city_data.emperor.debt_state == 2) {
        if (city_data.emperor.months_in_debt == -1) {
            city_message_post(1, MESSAGE_CITY_IN_DEBT_AGAIN, 0, 0);
            city_data.emperor.months_in_debt = 0;
        }
        if (game_time_day() == 0) {
            city_data.emperor.months_in_debt++;
        }
        if (city_data.emperor.months_in_debt >= 12) {
            city_data.emperor.debt_state = 3;
            city_data.emperor.months_in_debt = 0;
            if (!city_data.figure.imperial_soldiers) {
                city_message_post(1, MESSAGE_CITY_STILL_IN_DEBT, 0, 0);
                city_ratings_change_favor(-10);
            }
        }
    } else if (city_data.emperor.debt_state == 3) {
        if (city_data.emperor.months_in_debt == -1) {
            city_message_post(1, MESSAGE_CITY_STILL_IN_DEBT, 0, 0);
            city_data.emperor.months_in_debt = 0;
        }
        if (game_time_day() == 0) {
            city_data.emperor.months_in_debt++;
        }
        if (city_data.emperor.months_in_debt >= 12) {
            city_data.emperor.debt_state = 4;
            city_data.emperor.months_in_debt = 0;
            if (!city_data.figure.imperial_soldiers) {
                city_ratings_limit_favor(10);
            }
        }
    }
}

static void process_caesar_invasion(void)
{
    if (city_data.figure.imperial_soldiers) {
        // caesar invasion in progress
        city_data.emperor.invasion.duration_day_countdown--;
        if (city_data.ratings.favor >= 35 && city_data.emperor.invasion.duration_day_countdown < 176) {
            formation_caesar_pause();
        } else if (city_data.ratings.favor >= 22) {
            if (city_data.emperor.invasion.duration_day_countdown > 0) {
                formation_caesar_retreat();
                if (!city_data.emperor.invasion.retreat_message_shown) {
                    city_data.emperor.invasion.retreat_message_shown = 1;
                    city_message_post(1, MESSAGE_CAESAR_ARMY_RETREAT, 0, 0);
                }
            } else if (city_data.emperor.invasion.duration_day_countdown == 0) {
                city_message_post(1, MESSAGE_CAESAR_ARMY_CONTINUE, 0, 0); // a year has passed (11 months), siege goes on
            }
        }
    } else if (city_data.emperor.invasion.soldiers_killed && city_data.emperor.invasion.soldiers_killed >= city_data.emperor.invasion.size) {
        // player defeated caesar army
        city_data.emperor.invasion.size = 0;
        city_data.emperor.invasion.soldiers_killed = 0;
        if (city_data.ratings.favor < 35) {
            city_ratings_change_favor(10);
            if (city_data.emperor.invasion.count < 2) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_1, 0, 0);
            } else if (city_data.emperor.invasion.count < 3) {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_2, 0, 0);
            } else {
                city_message_post(1, MESSAGE_CAESAR_RESPECT_3, 0, 0);
            }
        }
    } else if (city_data.emperor.invasion.days_until_invasion <= 0) {
        if (city_data.ratings.favor <= 10) {
            // warn player that caesar is angry and will invade in a year
            city_data.emperor.invasion.warnings_given++;
            city_data.emperor.invasion.days_until_invasion = 192;
            if (city_data.emperor.invasion.warnings_given <= 1) {
                city_message_post(1, MESSAGE_CAESAR_WRATH, 0, 0);
            }
        }
    } else {
        city_data.emperor.invasion.days_until_invasion--;
        if (city_data.emperor.invasion.days_until_invasion == 0) {
            // invade!
            int size;
            if (city_data.emperor.invasion.count == 0) {
                size = 32;
            } else if (city_data.emperor.invasion.count == 1) {
                size = 64;
            } else if (city_data.emperor.invasion.count == 2) {
                size = 96;
            } else {
                size = 144;
            }
            if (scenario_invasion_start_from_caesar(size)) {
                city_data.emperor.invasion.count++;
                city_data.emperor.invasion.duration_day_countdown = 192;
                city_data.emperor.invasion.retreat_message_shown = 0;
                city_data.emperor.invasion.size = size;
                city_data.emperor.invasion.soldiers_killed = 0;
            }
        }
    }
}

void city_emperor_update(void)
{
    update_debt_state();
    process_caesar_invasion();
}

void city_emperor_init_selected_gift(void)
{
    if (city_data.emperor.selected_gift_size == GIFT_LAVISH && !city_emperor_can_send_gift(GIFT_LAVISH)) {
        city_data.emperor.selected_gift_size = GIFT_GENEROUS;
    }
    if (city_data.emperor.selected_gift_size == GIFT_GENEROUS && !city_emperor_can_send_gift(GIFT_GENEROUS)) {
        city_data.emperor.selected_gift_size = GIFT_MODEST;
    }
}

int city_emperor_set_gift_size(int size)
{
    if (city_data.emperor.gifts[size].cost <= city_data.emperor.personal_savings) {
        city_data.emperor.selected_gift_size = size;
        return 1;
    } else {
        return 0;
    }
}

int city_emperor_selected_gift_size(void)
{
    return city_data.emperor.selected_gift_size;
}

const emperor_gift *city_emperor_get_gift(int size)
{
    return &city_data.emperor.gifts[size];
}

int city_emperor_can_send_gift(int size)
{
    return city_data.emperor.gifts[size].cost <= city_data.emperor.personal_savings;
}

void city_emperor_calculate_gift_costs(void)
{
    int savings = city_data.emperor.personal_savings;
    city_data.emperor.gifts[GIFT_MODEST].cost = savings / 8 + 20;
    city_data.emperor.gifts[GIFT_GENEROUS].cost = savings / 4 + 50;
    city_data.emperor.gifts[GIFT_LAVISH].cost = savings / 2 + 100;
}

void city_emperor_send_gift(void)
{
    int size = city_data.emperor.selected_gift_size;
    if (size < GIFT_MODEST || size > GIFT_LAVISH) {
        return;
    }
    int cost = city_data.emperor.gifts[size].cost;

    if (cost > city_data.emperor.personal_savings) {
        return;
    }

    if (city_data.emperor.gift_overdose_penalty <= 0) {
        city_data.emperor.gift_overdose_penalty = 1;
        if (size == GIFT_MODEST) {
            city_ratings_change_favor(3);
        } else if (size == GIFT_GENEROUS) {
            city_ratings_change_favor(5);
        } else if (size == GIFT_LAVISH) {
            city_ratings_change_favor(10);
        }
    } else if (city_data.emperor.gift_overdose_penalty == 1) {
        city_data.emperor.gift_overdose_penalty = 2;
        if (size == GIFT_MODEST) {
            city_ratings_change_favor(1);
        } else if (size == GIFT_GENEROUS) {
            city_ratings_change_favor(3);
        } else if (size == GIFT_LAVISH) {
            city_ratings_change_favor(5);
        }
    } else if (city_data.emperor.gift_overdose_penalty == 2) {
        city_data.emperor.gift_overdose_penalty = 3;
        if (size == GIFT_MODEST) {
            city_ratings_change_favor(0);
        } else if (size == GIFT_GENEROUS) {
            city_ratings_change_favor(1);
        } else if (size == GIFT_LAVISH) {
            city_ratings_change_favor(3);
        }
    } else if (city_data.emperor.gift_overdose_penalty == 3) {
        city_data.emperor.gift_overdose_penalty = 4;
        if (size == GIFT_MODEST) {
            city_ratings_change_favor(0);
        } else if (size == GIFT_GENEROUS) {
            city_ratings_change_favor(0);
        } else if (size == GIFT_LAVISH) {
            city_ratings_change_favor(1);
        }
    }

    city_data.emperor.months_since_gift = 0;
    // rotate gift type
    city_data.emperor.gifts[size].id++;
    if (city_data.emperor.gifts[size].id >= 4) {
        city_data.emperor.gifts[size].id = 0;
    }

    city_data.emperor.personal_savings -= cost;
}

int city_emperor_months_since_gift(void)
{
    return city_data.emperor.months_since_gift;
}

int city_emperor_salary_for_rank(int rank)
{
    return SALARY_FOR_RANK[rank];
}

void city_emperor_set_salary_rank(int rank)
{
    city_data.emperor.salary_rank = rank;
    city_data.emperor.salary_amount = SALARY_FOR_RANK[rank];
}

int city_emperor_salary_rank(void)
{
    return city_data.emperor.salary_rank;
}

int city_emperor_salary_amount(void)
{
    return city_data.emperor.salary_amount;
}

int city_emperor_personal_savings(void)
{
    return city_data.emperor.personal_savings;
}

int city_emperor_rank(void)
{
    return city_data.emperor.player_rank;
}

void city_emperor_init_donation_amount(void)
{
    if (city_data.emperor.donate_amount > city_data.emperor.personal_savings) {
        city_data.emperor.donate_amount = city_data.emperor.personal_savings;
    }
}

void city_emperor_set_donation_amount(int amount)
{
    city_data.emperor.donate_amount = calc_bound(amount, 0, city_data.emperor.personal_savings);
}

void city_emperor_change_donation_amount(int change)
{
    city_emperor_set_donation_amount(city_data.emperor.donate_amount + change);
}

void city_emperor_donate_savings_to_city(void)
{
    city_finance_process_donation(city_data.emperor.donate_amount);
    city_data.emperor.personal_savings -= city_data.emperor.donate_amount;
    city_finance_calculate_totals();
}

int city_emperor_donate_amount(void)
{
    return city_data.emperor.donate_amount;
}

void city_emperor_mark_soldier_killed(void)
{
    city_data.emperor.invasion.soldiers_killed++;
}
