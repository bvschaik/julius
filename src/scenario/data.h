#ifndef SCENARIO_DATA_H
#define SCENARIO_DATA_H

#include "map/point.h"

#include <stdint.h>

#define MAX_REQUESTS 20
#define MAX_INVASIONS 20
#define MAX_DEMAND_CHANGES 20
#define MAX_PRICE_CHANGES 20

#define MAX_HERD_POINTS 4
#define MAX_FISH_POINTS 8
#define MAX_INVASION_POINTS 8

#define MAX_PLAYER_NAME 32

enum {
    EVENT_NOT_STARTED = 0,
    EVENT_IN_PROGRESS = 1,
    EVENT_FINISHED = 2
};

enum {
    INVASION_TYPE_LOCAL_UPRISING = 1,
    INVASION_TYPE_ENEMY_ARMY = 2,
    INVASION_TYPE_CAESAR = 3,
    INVASION_TYPE_DISTANT_BATTLE = 4,
};

struct win_criteria_t {
    int enabled;
    int goal;
};

extern struct scenario_t {
    int start_year;
    int climate;
    int player_rank;

    int initial_funds;
    int rescue_loan;

    int rome_supplies_wheat;
    int image_id;
    uint8_t brief_description[64];
    int enemy_id;
    int is_open_play;
    int open_play_scenario_id;

    struct {
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
    } win_criteria;

    struct {
        int id;
        int is_expanded;
        int expansion_year;
        int distant_battle_roman_travel_months;
        int distant_battle_enemy_travel_months;
    } empire;

    struct {
        int year;
        int resource;
        int amount;
        int deadline_years;
        int can_comply_dialog_shown;
        int favor;
        int month;
        int state;
        int visible;
        int months_to_comply;
    } requests[MAX_REQUESTS];

    struct {
        int year;
        int month;
        int resource;
        int route_id;
        int is_rise;
    } demand_changes[MAX_DEMAND_CHANGES];

    struct {
        int year;
        int month;
        int resource;
        int amount;
        int is_rise;
    } price_changes[MAX_DEMAND_CHANGES];

    struct {
        int year;
        int type;
        int amount;
        int from;
        int attack_type;
        int month;
    } invasions[MAX_INVASIONS];

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
        int lower_wages;
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
        short farms;
        short raw_materials;
        short workshops;
        short road;
        short wall;
        short aqueduct;
        short amphitheater;
        short theater;
        short hippodrome;
        short colosseum;
        short gladiator_school;
        short lion_house;
        short actor_colony;
        short chariot_maker;
        short gardens;
        short plaza;
        short statues;
        short doctor;
        short hospital;
        short bathhouse;
        short barber;
        short school;
        short academy;
        short library;
        short prefecture;
        short fort;
        short gatehouse;
        short tower;
        short small_temples;
        short large_temples;
        short market;
        short granary;
        short warehouse;
        short dock;
        short wharf;
        short governor_home;
        short engineers_post;
        short senate;
        short forum;
        short well;
        short oracle;
        short mission_post;
        short bridge;
        short barracks;
        short military_academy;
        short distribution_center;
    } allowed_buildings;

    struct {
        int hut;
        int meeting;
        int crops;
    } native_images;

    struct { // used to be stored in the settings file
        int campaign_rank;
        int campaign_mission;
        int is_custom;
        int starting_favor;
        int starting_personal_savings;
        uint8_t player_name[32];
    } settings;
} scenario;

#endif // SCENARIO_DATA_H
