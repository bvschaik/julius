#ifndef GAME_STATE_H
#define GAME_STATE_H

enum {
    OVERLAY_NONE = 0,
    OVERLAY_WATER = 2,
    OVERLAY_RELIGION = 4,
    OVERLAY_FIRE = 8,
    OVERLAY_DAMAGE = 9,
    OVERLAY_CRIME = 10,
    OVERLAY_ENTERTAINMENT = 11,
    OVERLAY_THEATER = 12,
    OVERLAY_AMPHITHEATER = 13,
    OVERLAY_COLOSSEUM = 14,
    OVERLAY_HIPPODROME = 15,
    OVERLAY_EDUCATION = 16,
    OVERLAY_SCHOOL = 17,
    OVERLAY_LIBRARY = 18,
    OVERLAY_ACADEMY = 19,
    OVERLAY_BARBER = 20,
    OVERLAY_BATHHOUSE = 21,
    OVERLAY_CLINIC = 22,
    OVERLAY_HOSPITAL = 23,
    OVERLAY_TAX_INCOME = 24,
    OVERLAY_FOOD_STOCKS = 25,
    OVERLAY_DESIRABILITY = 26,
    OVERLAY_WORKERS_UNUSED = 27,
    OVERLAY_NATIVE = 28,
    OVERLAY_PROBLEMS = 29,
    OVERLAY_ROADS = 30,
    OVERLAY_LEVY = 31,
    OVERLAY_TAVERN = 32,
    OVERLAY_ARENA = 33,
    OVERLAY_SENTIMENT = 34,
    OVERLAY_MOTHBALL = 35,
    OVERLAY_ENEMY = 36,
    OVERLAY_WAREHOUSE = 37,
    OVERLAY_SICKNESS = 38
};

void game_state_init(void);

int game_state_is_paused(void);

void game_state_toggle_paused(void);

void game_state_pause(void);

void game_state_unpause(void);

int game_state_overlay(void);

void game_state_reset_overlay(void);

void game_state_toggle_overlay(void);

void game_state_set_overlay(int overlay);

#endif // GAME_STATE_H
