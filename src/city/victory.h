#ifndef CITY_VICTORY_H
#define CITY_VICTORY_H

enum {
    VICTORY_STATE_LOST = -1,
    VICTORY_STATE_NONE = 0,
    VICTORY_STATE_WON = 1
};

void city_victory_reset();

void city_victory_force_win();

int city_victory_state();

void city_victory_check();

void city_victory_update_months_to_govern();

void city_victory_continue_governing(int months);

void city_victory_stop_governing();

int city_victory_has_won();

#endif // CITY_VICTORY_H
