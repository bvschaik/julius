#ifndef SCENARIO_EVENT_DATA_H
#define SCENARIO_EVENT_DATA_H

#include "core/array.h"

#include <stdint.h>

#define SCENARIO_EVENTS_ARRAY_SIZE_STEP 100
#define SCENARIO_ACTIONS_ARRAY_SIZE_STEP 20
#define SCENARIO_CONDITIONS_ARRAY_SIZE_STEP 20

typedef enum {
    EVENT_STATE_UNDEFINED = 0,
    EVENT_STATE_DISABLED = 1,
    EVENT_STATE_ACTIVE = 2,
    EVENT_STATE_PAUSED = 3
} event_state;

typedef enum {
    CONDITION_TYPE_UNDEFINED = 0,
    CONDITION_TYPE_TIME_PASSED = 1,
    CONDITION_TYPE_DIFFICULTY = 2,
    CONDITION_TYPE_MONEY = 3,
    CONDITION_TYPE_SAVINGS = 4,
    CONDITION_TYPE_STATS_FAVOR = 5,
    CONDITION_TYPE_STATS_PROSPERITY = 6,
    CONDITION_TYPE_STATS_CULTURE = 7,
    CONDITION_TYPE_STATS_PEACE = 8,
    CONDITION_TYPE_TRADE_SELL_PRICE = 9,
    CONDITION_TYPE_POPS_UNEMPLOYMENT = 10,
    CONDITION_TYPE_ROME_WAGES = 11,
    CONDITION_TYPE_CITY_POPULATION = 12,
    CONDITION_TYPE_BUILDING_COUNT_ACTIVE = 13,
    CONDITION_TYPE_STATS_CITY_HEALTH = 14,
    CONDITION_TYPE_COUNT_OWN_TROOPS = 15,
    CONDITION_TYPE_REQUEST_IS_ONGOING = 16,
    CONDITION_TYPE_MAX,
    // helper constants
    CONDITION_TYPE_MIN = CONDITION_TYPE_TIME_PASSED,
} condition_types;

typedef enum {
    ACTION_TYPE_UNDEFINED = 0,
    ACTION_TYPE_ADJUST_FAVOR = 1,
    ACTION_TYPE_ADJUST_MONEY = 2,
    ACTION_TYPE_ADJUST_SAVINGS = 3,
    ACTION_TYPE_TRADE_ADJUST_PRICE = 4,
    ACTION_TYPE_TRADE_PROBLEM_LAND = 5,
    ACTION_TYPE_TRADE_PROBLEM_SEA = 6,
    ACTION_TYPE_TRADE_ADJUST_ROUTE_AMOUNT = 7,
    ACTION_TYPE_ADJUST_ROME_WAGES = 8,
    ACTION_TYPE_GLADIATOR_REVOLT = 9,
    ACTION_TYPE_CHANGE_RESOURCE_PRODUCED = 10,
    ACTION_TYPE_CHANGE_ALLOWED_BUILDINGS = 11,
    ACTION_TYPE_SEND_STANDARD_MESSAGE = 12,
    ACTION_TYPE_ADJUST_CITY_HEALTH = 13,
    ACTION_TYPE_TRADE_SET_PRICE = 14,
    ACTION_TYPE_EMPIRE_MAP_CONVERT_FUTURE_TRADE_CITY = 15,
    ACTION_TYPE_REQUEST_IMMEDIATELY_START = 16,
    ACTION_TYPE_SHOW_CUSTOM_MESSAGE = 17,
    ACTION_TYPE_MAX,
    // helper constants
    ACTION_TYPE_MIN = ACTION_TYPE_ADJUST_FAVOR,
} action_types;

typedef enum {
    LINK_TYPE_UNDEFINED = -1,
    LINK_TYPE_SCENARIO_EVENT = 0
} link_type;

enum {
    COMPARISON_TYPE_UNDEFINED = 0,
    COMPARISON_TYPE_EQUAL = 1,
    COMPARISON_TYPE_EQUAL_OR_LESS = 2,
    COMPARISON_TYPE_EQUAL_OR_MORE = 3
};

enum {
    POP_CLASS_UNDEFINED = 0,
    POP_CLASS_ALL = 1,
    POP_CLASS_PATRICIAN = 2,
    POP_CLASS_PLEBEIAN = 3,
    POP_CLASS_SLUMS = 4
};

typedef struct {
    condition_types type;
    int parameter1;
    int parameter2;
    int parameter3;
    int parameter4;
    int parameter5;
} scenario_condition_t;

typedef struct {
    action_types type;
    int parameter1;
    int parameter2;
    int parameter3;
    int parameter4;
    int parameter5;
} scenario_action_t;

typedef struct {
    int id;
    event_state state;
    int repeat_months_min;
    int repeat_months_max;
    int max_number_of_repeats;
    int execution_count;
    int months_until_active;
    array(scenario_condition_t) conditions;
    array(scenario_action_t) actions;
} scenario_event_t;

#endif // SCENARIO_EVENT_DATA_H
