#ifndef GAME_TUTORIAL_H
#define GAME_TUTORIAL_H

#include "core/buffer.h"

typedef enum {
    AVAILABLE,
    NOT_AVAILABLE,
    NOT_AVAILABLE_YET,
} tutorial_availability;

typedef enum {
    TUT_BUILD_NORMAL,
    TUT1_BUILD_START,
    TUT1_BUILD_AFTER_FIRE,
    TUT1_BUILD_AFTER_COLLAPSE,
    TUT2_BUILD_START,
    TUT2_BUILD_UP_TO_250,
    TUT2_BUILD_UP_TO_450,
    TUT2_BUILD_AFTER_450
} tutorial_build_buttons;

void tutorial_init(void);

tutorial_availability tutorial_advisor_empire_availability(void);

tutorial_build_buttons tutorial_get_build_buttons(void);

int tutorial_get_population_cap(int current_cap);

int tutorial_get_immediate_goal_text(void);

int tutorial_adjust_request_year(int *year);

int tutorial_extra_fire_risk(void);

int tutorial_extra_damage_risk(void);

int tutorial_handle_fire(void);

int tutorial_handle_collapse(void);

void tutorial_on_crime(void);

void tutorial_on_disease(void);

void tutorial_on_filled_granary(void);

void tutorial_on_add_to_warehouse(void);

void tutorial_on_day_tick(void);

void tutorial_on_month_tick(void);

void tutorial_save_state(buffer *buf1, buffer *buf2, buffer *buf3);

void tutorial_load_state(buffer *buf1, buffer *buf2, buffer *buf3);

#endif // GAME_TUTORIAL_H
