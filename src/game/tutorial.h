#ifndef GAME_TUTORIAL_H
#define GAME_TUTORIAL_H

#include "core/buffer.h"

typedef enum
{
    AVAILABLE,
    NOT_AVAILABLE,
    NOT_AVAILABLE_YET,
} tutorial_availability;

typedef enum
{
    TUT_BUILD_NORMAL,
    TUT1_BUILD_START,
    TUT1_BUILD_AFTER_FIRE,
    TUT1_BUILD_AFTER_COLLAPSE,
    TUT2_BUILD_START,
    TUT2_BUILD_UP_TO_250,
    TUT2_BUILD_UP_TO_450,
    TUT2_BUILD_AFTER_450
} tutorial_build_buttons;

struct Tutorial
{
    static void init();

    static tutorial_availability advisor_empire_availability();

    static tutorial_build_buttons get_build_buttons();

    static int get_population_cap(int current_cap);

    static int get_immediate_goal_text();

    static int adjust_request_year(int *year);

    static int extra_fire_risk();

    static int extra_damage_risk();

    static int handle_fire();

    static int handle_collapse();

    static void on_crime();

    static void on_disease();

    static void on_filled_granary();

    static void on_add_to_warehouse();

    static void on_day_tick();

    static void on_month_tick();

    static void save_state(buffer *buf1, buffer *buf2, buffer *buf3);

    static void load_state(buffer *buf1, buffer *buf2, buffer *buf3);
};

#endif
