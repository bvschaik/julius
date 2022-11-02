#ifndef SCENARIO_DATA_H
#define SCENARIO_DATA_H

#include "map/point.h"
#include "scenario/types.h"

#include <stdint.h>

#define MAX_REQUESTS 20
#define MAX_INVASIONS 20
#define MAX_DEMAND_CHANGES 20
#define MAX_PRICE_CHANGES 20

#define MAX_HERD_POINTS 4
#define MAX_FISH_POINTS 8
#define MAX_INVASION_POINTS 8

#define MAX_ALLOWED_BUILDINGS 50

#define MAX_PLAYER_NAME 32
#define MAX_SCENARIO_NAME 65
#define MAX_BRIEF_DESCRIPTION 64
#define MAX_BRIEFING 522

enum {
    EVENT_NOT_STARTED = 0,
    EVENT_IN_PROGRESS = 1,
    EVENT_FINISHED = 2
};

enum {
    ALLOWED_BUILDING_NONE = 0,
    ALLOWED_BUILDING_FARMS = 1,
    ALLOWED_BUILDING_RAW_MATERIALS = 2,
    ALLOWED_BUILDING_WORKSHOPS = 3,
    ALLOWED_BUILDING_ROAD = 4,
    ALLOWED_BUILDING_WALL = 5,
    ALLOWED_BUILDING_AQUEDUCT = 6,
    ALLOWED_BUILDING_HOUSING = 7,
    ALLOWED_BUILDING_AMPHITHEATER = 8,
    ALLOWED_BUILDING_THEATER = 9,
    ALLOWED_BUILDING_HIPPODROME = 10,
    ALLOWED_BUILDING_COLOSSEUM = 11,
    ALLOWED_BUILDING_GLADIATOR_SCHOOL = 12,
    ALLOWED_BUILDING_LION_HOUSE = 13,
    ALLOWED_BUILDING_ACTOR_COLONY = 14,
    ALLOWED_BUILDING_CHARIOT_MAKER = 15,
    ALLOWED_BUILDING_GARDENS = 16,
    ALLOWED_BUILDING_PLAZA = 17,
    ALLOWED_BUILDING_STATUES = 18,
    ALLOWED_BUILDING_DOCTOR = 19,
    ALLOWED_BUILDING_HOSPITAL = 20,
    ALLOWED_BUILDING_BATHHOUSE = 21,
    ALLOWED_BUILDING_BARBER = 22,
    ALLOWED_BUILDING_SCHOOL = 23,
    ALLOWED_BUILDING_ACADEMY = 24,
    ALLOWED_BUILDING_LIBRARY = 25,
    ALLOWED_BUILDING_PREFECTURE = 26,
    ALLOWED_BUILDING_FORT = 27,
    ALLOWED_BUILDING_GATEHOUSE = 28,
    ALLOWED_BUILDING_TOWER = 29,
    ALLOWED_BUILDING_SMALL_TEMPLES = 30,
    ALLOWED_BUILDING_LARGE_TEMPLES = 31,
    ALLOWED_BUILDING_MARKET = 32,
    ALLOWED_BUILDING_GRANARY = 33,
    ALLOWED_BUILDING_WAREHOUSE = 34,
    ALLOWED_BUILDING_TRIUMPHAL_ARCH = 35,
    ALLOWED_BUILDING_DOCK = 36,
    ALLOWED_BUILDING_WHARF = 37,
    ALLOWED_BUILDING_GOVERNOR_HOME = 38,
    ALLOWED_BUILDING_ENGINEERS_POST = 39,
    ALLOWED_BUILDING_SENATE = 40,
    ALLOWED_BUILDING_FORUM = 41,
    ALLOWED_BUILDING_WELL = 42,
    ALLOWED_BUILDING_ORACLE = 43,
    ALLOWED_BUILDING_MISSION_POST = 44,
    ALLOWED_BUILDING_BRIDGE = 45,
    ALLOWED_BUILDING_BARRACKS = 46,
    ALLOWED_BUILDING_MILITARY_ACADEMY = 47,
    ALLOWED_BUILDING_MONUMENTS = 48,
};

struct win_criteria_t {
    int enabled;
    int goal;
};

typedef struct {
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
} request_t;

typedef struct {
    int year;
    int type;
    int amount;
    int from;
    int attack_type;
    int month;
} invasion_t;

typedef struct {
    int year;
    int month;
    int resource;
    int amount;
    int is_rise;
} price_change_t;

typedef struct {
    int year;
    int month;
    int resource;
    int route_id;
    int amount;
} demand_change_t;

#define DEMAND_CHANGE_LEGACY_IS_RISE 9999
#define DEMAND_CHANGE_LEGACY_IS_FALL -9999

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
    int climate;
    int player_rank;

    int initial_funds;
    int rescue_loan;

    int rome_supplies_wheat;
    int image_id;
    uint8_t brief_description[MAX_BRIEF_DESCRIPTION];
    uint8_t briefing[MAX_BRIEFING];
    int enemy_id;
    int is_open_play;
    int open_play_scenario_id;

    scenario_win_criteria win_criteria;

    struct {
        int id;
        int is_expanded;
        int expansion_year;
        int distant_battle_roman_travel_months;
        int distant_battle_enemy_travel_months;
        uint8_t custom_name[50];
    } empire;

    request_t requests[MAX_REQUESTS];

    demand_change_t demand_changes[MAX_DEMAND_CHANGES];

    price_change_t price_changes[MAX_DEMAND_CHANGES];

    invasion_t invasions[MAX_INVASIONS];

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

    short allowed_buildings[MAX_ALLOWED_BUILDINGS];

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
        uint8_t player_name[MAX_PLAYER_NAME];
        /** Temp storage for carrying over player name to next campaign mission */
        uint8_t campaign_player_name[MAX_PLAYER_NAME];
    } settings;

    int is_saved;
} scenario;

#endif // SCENARIO_DATA_H
