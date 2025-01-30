#ifndef SCENARIO_DATA_H
#define SCENARIO_DATA_H

#include "core/file.h"
#include "map/point.h"
#include "scenario/property.h"
#include "scenario/types.h"

#include <stdint.h>

#define MAX_HERD_POINTS 4
#define MAX_FISH_POINTS 8
#define MAX_INVASION_POINTS 8

#define MAX_PLAYER_NAME 32
#define MAX_SCENARIO_NAME 65
#define MAX_BRIEF_DESCRIPTION 64
#define MAX_BRIEFING 522

enum {
    EVENT_NOT_STARTED = 0,
    EVENT_IN_PROGRESS = 1,
    EVENT_FINISHED = 2
};

struct win_criteria_t {
    int enabled;
    int goal;
};

typedef struct {
    struct win_criteria_t population;
    struct win_criteria_t culture;
    struct win_criteria_t prosperity;
    struct win_criteria_t peace;
    struct win_criteria_t favor;
    struct {
        int enabled;
        int years;
    } time_limit;
    struct {
        int enabled;
        int years;
    } survival_time;
    int milestone25_year;
    int milestone50_year;
    int milestone75_year;
} scenario_win_criteria;

extern struct scenario_t {
    uint8_t scenario_name[MAX_SCENARIO_NAME];

    int start_year;
    scenario_climate climate;
    int player_rank;
    uint16_t caesar_salary;

    int initial_funds;
    int rescue_loan;

    int rome_supplies_wheat;
    int image_id;
    uint8_t brief_description[MAX_BRIEF_DESCRIPTION];
    uint8_t briefing[MAX_BRIEFING];
    int enemy_id;
    int is_open_play;
    int open_play_scenario_id;
    int intro_custom_message_id;
    int victory_custom_message_id;

    scenario_win_criteria win_criteria;

    struct {
        int id;
        int is_expanded;
        int expansion_year;
        int distant_battle_roman_travel_months;
        int distant_battle_enemy_travel_months;
        char custom_name[FILE_NAME_MAX];
    } empire;

    struct {
        int severity;
        int year;
    } earthquake;

    struct {
        int year;
        int enabled;
    } emperor_change;

    struct {
        int year;
        int enabled;
    } gladiator_revolt;

    struct {
        int sea_trade_problem;
        int land_trade_problem;
        int raise_wages;
        int max_wages;
        int lower_wages;
        int min_wages;
        int contaminated_water;
        int iron_mine_collapse;
        int clay_pit_flooded;
    } random_events;

    struct {
        int width;
        int height;
        int grid_start;
        int grid_border_size;
    } map;
    int flotsam_enabled;
    map_point entry_point;
    map_point exit_point;
    map_point river_entry_point;
    map_point river_exit_point;
    map_point earthquake_point;
    map_point herd_points[MAX_HERD_POINTS];
    map_point fishing_points[MAX_FISH_POINTS];
    map_point invasion_points[MAX_INVASION_POINTS];

    struct {
        int hut;
        int meeting;
        int crops;
    } native_images;

    struct { // used to be stored in the settings file
        int is_custom;
        int starting_favor;
        int starting_personal_savings;
        uint8_t player_name[MAX_PLAYER_NAME];
    } settings;

    struct {
        int rank;
        int mission;
        /** Temp storage for carrying over player name to next campaign mission */
        uint8_t player_name[MAX_PLAYER_NAME];
    } campaign;
} scenario;

#endif // SCENARIO_DATA_H
