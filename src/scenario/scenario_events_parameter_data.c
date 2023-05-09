#include "scenario_events_parameter_data.h"
#include "game/resource.h"

#define UNLIMITED 1000000000
#define NEGATIVE_UNLIMITED -1000000000

static scenario_condition_data_t scenario_condition_data[CONDITION_TYPE_MAX] = {
    [CONDITION_TYPE_TIME_PASSED]     = { .type = CONDITION_TYPE_TIME_PASSED,
                                        .xml_attr =     { .name = "time",           .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_TIME_PASSED },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,                     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "min",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MIN },
                                        .xml_parm3 =    { .name = "max",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MAX }, },
    [CONDITION_TYPE_DIFFICULTY]     = { .type = CONDITION_TYPE_DIFFICULTY,
                                        .xml_attr =     { .name = "difficulty",     .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_DIFFICULTY },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_DIFFICULTY,       .min_limit = 0,         .max_limit = 4,     .key = TR_PARAMETER_TYPE_DIFFICULTY }, },
    [CONDITION_TYPE_MONEY]     = { .type = CONDITION_TYPE_MONEY,
                                        .xml_attr =     { .name = "money",          .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_MONEY },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = -10000,    .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_SAVINGS]     = { .type = CONDITION_TYPE_SAVINGS,
                                        .xml_attr =     { .name = "savings",        .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_SAVINGS },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,   .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_STATS_FAVOR]     = { .type = CONDITION_TYPE_STATS_FAVOR,
                                        .xml_attr =     { .name = "stats_favor",    .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_STATS_FAVOR },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 100,   .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_STATS_PROSPERITY]     = { .type = CONDITION_TYPE_STATS_PROSPERITY,
                                        .xml_attr =     { .name = "stats_prosperity",     .type = PARAMETER_TYPE_TEXT,       .key = TR_CONDITION_TYPE_STATS_PROSPERITY },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 100,   .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_STATS_CULTURE]     = { .type = CONDITION_TYPE_STATS_CULTURE,
                                        .xml_attr =     { .name = "stats_culture",        .type = PARAMETER_TYPE_TEXT,       .key = TR_CONDITION_TYPE_STATS_CULTURE },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 100,   .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_STATS_PEACE]     = { .type = CONDITION_TYPE_STATS_PEACE,
                                        .xml_attr =     { .name = "stats_peace",          .type = PARAMETER_TYPE_TEXT,       .key = TR_CONDITION_TYPE_STATS_PEACE },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,     .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 100,   .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_TRADE_SELL_PRICE]     = { .type = CONDITION_TYPE_TRADE_SELL_PRICE,
                                        .xml_attr =     { .name = "trade_sell_price",     .type = PARAMETER_TYPE_TEXT,       .key = TR_CONDITION_TYPE_TRADE_SELL_PRICE },
                                        .xml_parm1 =    { .name = "resource",       .type = PARAMETER_TYPE_RESOURCE,         .key = TR_PARAMETER_TYPE_RESOURCE },
                                        .xml_parm2 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm3 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_POPS_UNEMPLOYMENT]     = { .type = CONDITION_TYPE_POPS_UNEMPLOYMENT,
                                        .xml_attr =     { .name = "population_unemployed",     .type = PARAMETER_TYPE_TEXT,  .key = TR_CONDITION_TYPE_POPS_UNEMPLOYMENT },
                                        .xml_parm1 =    { .name = "percentage",     .type = PARAMETER_TYPE_BOOLEAN,          .key = TR_PARAMETER_USE_PERCENTAGE },
                                        .xml_parm2 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm3 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_ROME_WAGES]     = { .type = CONDITION_TYPE_ROME_WAGES,
                                        .xml_attr =     { .name = "rome_wages",     .type = PARAMETER_TYPE_TEXT,             .key = TR_CONDITION_TYPE_ROME_WAGES },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,         .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 10000,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_CITY_POPULATION]     = { .type = CONDITION_TYPE_CITY_POPULATION,
                                        .xml_attr =     { .name = "city_population",     .type = PARAMETER_TYPE_TEXT,        .key = TR_CONDITION_TYPE_CITY_POPULATION },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm3 =    { .name = "class",          .type = PARAMETER_TYPE_POP_CLASS,        .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_POP_CLASS }, },
    [CONDITION_TYPE_BUILDING_COUNT_ACTIVE]     = { .type = CONDITION_TYPE_BUILDING_COUNT_ACTIVE,
                                        .xml_attr =     { .name = "building_count_active",     .type = PARAMETER_TYPE_TEXT,  .key = TR_CONDITION_TYPE_BUILDING_COUNT_ACTIVE },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm3 =    { .name = "building",       .type = PARAMETER_TYPE_BUILDING_COUNTING,                        .key = TR_PARAMETER_TYPE_BUILDING_COUNTING }, },
    [CONDITION_TYPE_STATS_CITY_HEALTH]     = { .type = CONDITION_TYPE_STATS_CITY_HEALTH,
                                        .xml_attr =     { .name = "stats_health",          .type = PARAMETER_TYPE_TEXT,      .key = TR_CONDITION_TYPE_STATS_CITY_HEALTH },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,       .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 100,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [CONDITION_TYPE_COUNT_OWN_TROOPS]     = { .type = CONDITION_TYPE_COUNT_OWN_TROOPS,
                                        .xml_attr =     { .name = "count_own_troops",     .type = PARAMETER_TYPE_TEXT,       .key = TR_CONDITION_TYPE_COUNT_OWN_TROOPS },
                                        .xml_parm1 =    { .name = "check",          .type = PARAMETER_TYPE_CHECK,            .min_limit = 1,         .max_limit = 3,             .key = TR_PARAMETER_TYPE_CHECK },
                                        .xml_parm2 =    { .name = "value",          .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm3 =    { .name = "in_city_only",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,         .max_limit = 1,             .key = TR_PARAMETER_IN_CITY_ONLY }, },
    [CONDITION_TYPE_REQUEST_IS_ONGOING]     = { .type = CONDITION_TYPE_REQUEST_IS_ONGOING,
                                        .xml_attr =     { .name = "request_is_ongoing",     .type = PARAMETER_TYPE_TEXT,     .key = TR_CONDITION_TYPE_REQUEST_IS_ONGOING },
                                        .xml_parm1 =    { .name = "request_id",     .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,         .max_limit = 19,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm2 =    { .name = "check_for_ongoing",   .type = PARAMETER_TYPE_BOOLEAN,     .min_limit = 0,         .max_limit = 1,      .key = TR_PARAMETER_CHECK_FOR_ONGOING }, },
};

scenario_condition_data_t *scenario_events_parameter_data_get_conditions_xml_attributes(condition_types type)
{
    return &scenario_condition_data[type];
}

static scenario_action_data_t scenario_action_data[ACTION_TYPE_MAX] = {
    [ACTION_TYPE_ADJUST_FAVOR]     = { .type = ACTION_TYPE_ADJUST_FAVOR,
                                        .xml_attr =     { .name = "favor_add",          .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_ADJUST_FAVOR },
                                        .xml_parm1 =    { .name = "amount",         .type = PARAMETER_TYPE_NUMBER,           .min_limit = -100,      .max_limit = 100,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [ACTION_TYPE_ADJUST_MONEY]     = { .type = ACTION_TYPE_ADJUST_MONEY,
                                        .xml_attr =     { .name = "money_add",          .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_ADJUST_MONEY },
                                        .xml_parm1 =    { .name = "min",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = NEGATIVE_UNLIMITED,      .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MIN  },
                                        .xml_parm2 =    { .name = "max",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = NEGATIVE_UNLIMITED,      .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MAX  }, },
    [ACTION_TYPE_ADJUST_SAVINGS]     = { .type = ACTION_TYPE_ADJUST_SAVINGS,
                                        .xml_attr =     { .name = "savings_add",        .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_ADJUST_SAVINGS },
                                        .xml_parm1 =    { .name = "min",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = NEGATIVE_UNLIMITED,      .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MIN  },
                                        .xml_parm2 =    { .name = "max",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = NEGATIVE_UNLIMITED,      .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MAX }, },
    [ACTION_TYPE_TRADE_ADJUST_PRICE]     = { .type = ACTION_TYPE_TRADE_ADJUST_PRICE,
                                        .xml_attr =     { .name = "trade_price_adjust",    .type = PARAMETER_TYPE_TEXT,      .key = TR_ACTION_TYPE_TRADE_ADJUST_PRICE },
                                        .xml_parm1 =    { .name = "resource",       .type = PARAMETER_TYPE_RESOURCE,         .key = TR_PARAMETER_TYPE_RESOURCE },
                                        .xml_parm2 =    { .name = "amount",         .type = PARAMETER_TYPE_NUMBER,           .min_limit = -10000,      .max_limit = 10000,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm3 =    { .name = "show_message",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,         .key = TR_PARAMETER_SHOW_MESSAGE }, },
    [ACTION_TYPE_TRADE_PROBLEM_LAND]     = { .type = ACTION_TYPE_TRADE_PROBLEM_LAND,
                                        .xml_attr =     { .name = "trade_problems_land",    .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_TRADE_PROBLEM_LAND },
                                        .xml_parm1 =    { .name = "duration",       .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,      .max_limit = 10000,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [ACTION_TYPE_TRADE_PROBLEM_SEA]     = { .type = ACTION_TYPE_TRADE_PROBLEM_SEA,
                                        .xml_attr =     { .name = "trade_problems_sea",    .type = PARAMETER_TYPE_TEXT,      .key = TR_ACTION_TYPE_TRADE_PROBLEM_SEA },
                                        .xml_parm1 =    { .name = "duration",       .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,      .max_limit = 10000,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [ACTION_TYPE_TRADE_ADJUST_ROUTE_AMOUNT]     = { .type = ACTION_TYPE_TRADE_ADJUST_ROUTE_AMOUNT,
                                        .xml_attr =     { .name = "trade_route_amount",    .type = PARAMETER_TYPE_TEXT,      .key = TR_ACTION_TYPE_TRADE_ADJUST_ROUTE_AMOUNT },
                                        .xml_parm1 =    { .name = "target_city",    .type = PARAMETER_TYPE_ROUTE,            .key = TR_PARAMETER_TYPE_ROUTE },
                                        .xml_parm2 =    { .name = "resource",       .type = PARAMETER_TYPE_RESOURCE,         .key = TR_PARAMETER_TYPE_RESOURCE },
                                        .xml_parm3 =    { .name = "amount",         .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,      .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm4 =    { .name = "show_message",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,      .max_limit = 1,          .key = TR_PARAMETER_SHOW_MESSAGE }, },
    [ACTION_TYPE_ADJUST_ROME_WAGES]     = { .type = ACTION_TYPE_ADJUST_ROME_WAGES,
                                        .xml_attr =     { .name = "change_rome_wages",     .type = PARAMETER_TYPE_TEXT,      .key = TR_ACTION_TYPE_ADJUST_ROME_WAGES },
                                        .xml_parm1 =    { .name = "min",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = -10000,      .max_limit = 10000,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MIN },
                                        .xml_parm2 =    { .name = "max",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = -10000,      .max_limit = 10000,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MAX },
                                        .xml_parm3 =    { .name = "set_to_value",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,      .max_limit = 1,      .key = TR_PARAMETER_SET_TO_VALUE }, },
    [ACTION_TYPE_GLADIATOR_REVOLT]     = { .type = ACTION_TYPE_GLADIATOR_REVOLT,
                                        .xml_attr =     { .name = "gladiator_revolt",     .type = PARAMETER_TYPE_TEXT,       .key = TR_ACTION_TYPE_GLADIATOR_REVOLT }, },
    [ACTION_TYPE_CHANGE_RESOURCE_PRODUCED]     = { .type = ACTION_TYPE_CHANGE_RESOURCE_PRODUCED,
                                        .xml_attr =     { .name = "change_resource_produced",    .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_CHANGE_RESOURCE_PRODUCED },
                                        .xml_parm1 =    { .name = "resource",       .type = PARAMETER_TYPE_RESOURCE,         .key = TR_PARAMETER_TYPE_RESOURCE },
                                        .xml_parm2 =    { .name = "produced",       .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,      .key = TR_PARAMETER_PRODUCED }, },
    [ACTION_TYPE_CHANGE_ALLOWED_BUILDINGS]     = { .type = ACTION_TYPE_CHANGE_ALLOWED_BUILDINGS,
                                        .xml_attr =     { .name = "change_allowed_buildings",    .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_CHANGE_ALLOWED_BUILDINGS },
                                        .xml_parm1 =    { .name = "building",       .type = PARAMETER_TYPE_ALLOWED_BUILDING,      .key = TR_PARAMETER_TYPE_ALLOWED_BUILDING },
                                        .xml_parm2 =    { .name = "allowed",        .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,      .key = TR_PARAMETER_ALLOWED }, },
    [ACTION_TYPE_SEND_STANDARD_MESSAGE]     = { .type = ACTION_TYPE_SEND_STANDARD_MESSAGE,
                                        .xml_attr =     { .name = "send_standard_message",    .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_SEND_STANDARD_MESSAGE },
                                        .xml_parm1 =    { .name = "text_id",        .type = PARAMETER_TYPE_STANDARD_MESSAGE,   .key = TR_PARAMETER_TYPE_STANDARD_MESSAGE }, },
    [ACTION_TYPE_ADJUST_CITY_HEALTH]     = { .type = ACTION_TYPE_ADJUST_CITY_HEALTH,
                                        .xml_attr =     { .name = "city_health",     .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_ADJUST_CITY_HEALTH },
                                        .xml_parm1 =    { .name = "min",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = -100,      .max_limit = 100,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MIN },
                                        .xml_parm2 =    { .name = "max",            .type = PARAMETER_TYPE_MIN_MAX_NUMBER,           .min_limit = -100,      .max_limit = 100,     .key = TR_PARAMETER_TYPE_MIN_MAX_NUMBER_MAX },
                                        .xml_parm3 =    { .name = "set_to_value",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,      .max_limit = 1,      .key = TR_PARAMETER_SET_TO_VALUE }, },
    [ACTION_TYPE_TRADE_SET_PRICE]     = { .type = ACTION_TYPE_TRADE_SET_PRICE,
                                        .xml_attr =     { .name = "trade_price_set",    .type = PARAMETER_TYPE_TEXT,         .key = TR_ACTION_TYPE_TRADE_SET_PRICE },
                                        .xml_parm1 =    { .name = "resource",       .type = PARAMETER_TYPE_RESOURCE,         .key = TR_PARAMETER_TYPE_RESOURCE },
                                        .xml_parm2 =    { .name = "amount",         .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,           .max_limit = UNLIMITED,     .key = TR_PARAMETER_TYPE_NUMBER },
                                        .xml_parm3 =    { .name = "set_buy_price",  .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,      .key = TR_PARAMETER_SET_BUY_PRICE },
                                        .xml_parm4 =    { .name = "show_message",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,      .key = TR_PARAMETER_SHOW_MESSAGE }, },
    [ACTION_TYPE_EMPIRE_MAP_CONVERT_FUTURE_TRADE_CITY]     = { .type = ACTION_TYPE_EMPIRE_MAP_CONVERT_FUTURE_TRADE_CITY,
                                        .xml_attr =     { .name = "empire_map_convert_future_trade_city",     .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_EMPIRE_MAP_CONVERT_FUTURE_TRADE_CITY },
                                        .xml_parm1 =    { .name = "target_city",    .type = PARAMETER_TYPE_FUTURE_CITY,      .key = TR_PARAMETER_TYPE_FUTURE_CITY },
                                        .xml_parm2 =    { .name = "show_message",   .type = PARAMETER_TYPE_BOOLEAN,          .min_limit = 0,           .max_limit = 1,      .key = TR_PARAMETER_SHOW_MESSAGE }, },
    [ACTION_TYPE_REQUEST_IMMEDIATELY_START]     = { .type = ACTION_TYPE_REQUEST_IMMEDIATELY_START,
                                        .xml_attr =     { .name = "request_immediately_start",     .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_REQUEST_IMMEDIATELY_START },
                                        .xml_parm1 =    { .name = "request_id",     .type = PARAMETER_TYPE_NUMBER,           .min_limit = 0,           .max_limit = 19,     .key = TR_PARAMETER_TYPE_NUMBER }, },
    [ACTION_TYPE_SHOW_CUSTOM_MESSAGE]     = { .type = ACTION_TYPE_SHOW_CUSTOM_MESSAGE,
                                        .xml_attr =     { .name = "show_custom_message",     .type = PARAMETER_TYPE_TEXT,     .key = TR_ACTION_TYPE_SHOW_CUSTOM_MESSAGE },
                                        .xml_parm1 =    { .name = "message_uid",    .type = PARAMETER_TYPE_CUSTOM_MESSAGE,    .key = TR_PARAMETER_TYPE_NUMBER }, },
};

scenario_action_data_t *scenario_events_parameter_data_get_actions_xml_attributes(action_types type)
{
    return &scenario_action_data[type];
}

static special_attribute_mapping_t special_attribute_mappings_check[] = {
    { .type = PARAMETER_TYPE_CHECK,                .text = "eq",                            .value = COMPARISON_TYPE_EQUAL,               .key = TR_PARAMETER_VALUE_COMPARISON_TYPE_EQUAL },
    { .type = PARAMETER_TYPE_CHECK,                .text = "lte",                           .value = COMPARISON_TYPE_EQUAL_OR_LESS,       .key = TR_PARAMETER_VALUE_COMPARISON_TYPE_EQUAL_OR_LESS },
    { .type = PARAMETER_TYPE_CHECK,                .text = "gte",                           .value = COMPARISON_TYPE_EQUAL_OR_MORE,       .key = TR_PARAMETER_VALUE_COMPARISON_TYPE_EQUAL_OR_MORE },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_CHECK_SIZE (sizeof(special_attribute_mappings_check) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_difficulty[] = {
    { .type = PARAMETER_TYPE_DIFFICULTY,           .text = "very_easy",                     .value = DIFFICULTY_VERY_EASY,               .key = TR_PARAMETER_VALUE_DIFFICULTY_VERY_EASY },
    { .type = PARAMETER_TYPE_DIFFICULTY,           .text = "easy",                          .value = DIFFICULTY_EASY,                    .key = TR_PARAMETER_VALUE_DIFFICULTY_EASY },
    { .type = PARAMETER_TYPE_DIFFICULTY,           .text = "normal",                        .value = DIFFICULTY_NORMAL,                  .key = TR_PARAMETER_VALUE_DIFFICULTY_NORMAL },
    { .type = PARAMETER_TYPE_DIFFICULTY,           .text = "hard",                          .value = DIFFICULTY_HARD,                    .key = TR_PARAMETER_VALUE_DIFFICULTY_HARD },
    { .type = PARAMETER_TYPE_DIFFICULTY,           .text = "very_hard",                     .value = DIFFICULTY_VERY_HARD,               .key = TR_PARAMETER_VALUE_DIFFICULTY_VERY_HARD },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_CHECK_DIFFICULTY (sizeof(special_attribute_mappings_difficulty) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_boolean[] = {
    { .type = PARAMETER_TYPE_BOOLEAN,              .text = "false",                         .value = 0,                                  .key = TR_PARAMETER_VALUE_BOOLEAN_FALSE },
    { .type = PARAMETER_TYPE_BOOLEAN,              .text = "true",                          .value = 1,                                  .key = TR_PARAMETER_VALUE_BOOLEAN_TRUE },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_BOOLEAN_SIZE (sizeof(special_attribute_mappings_boolean) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_pop_class[] = {
    { .type = PARAMETER_TYPE_POP_CLASS,            .text = "all",                           .value = POP_CLASS_ALL,                      .key = TR_PARAMETER_VALUE_POP_CLASS_ALL },
    { .type = PARAMETER_TYPE_POP_CLASS,            .text = "patrician",                     .value = POP_CLASS_PATRICIAN,                .key = TR_PARAMETER_VALUE_POP_CLASS_PATRICIAN },
    { .type = PARAMETER_TYPE_POP_CLASS,            .text = "plebeian",                      .value = POP_CLASS_PLEBEIAN,                 .key = TR_PARAMETER_VALUE_POP_CLASS_PLEBEIAN },
    { .type = PARAMETER_TYPE_POP_CLASS,            .text = "slums",                         .value = POP_CLASS_SLUMS,                    .key = TR_PARAMETER_VALUE_POP_CLASS_SLUMS },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_POP_CLASS_SIZE (sizeof(special_attribute_mappings_pop_class) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_buildings[] = {
    { .type = PARAMETER_TYPE_BUILDING,            .text = "none",                          .value = BUILDING_NONE,                    .key = TR_PARAMETER_VALUE_BUILDING_NONE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "vacant_lot",                    .value = BUILDING_HOUSE_VACANT_LOT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_farms",                     .value = BUILDING_MENU_FARMS,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_FARMS },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_raw_materials",             .value = BUILDING_MENU_RAW_MATERIALS,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_RAW_MATERIALS },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_workshops",                 .value = BUILDING_MENU_WORKSHOPS,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_WORKSHOPS },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "road",                          .value = BUILDING_ROAD,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "wall",                          .value = BUILDING_WALL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "draggable_reservoir",           .value = BUILDING_DRAGGABLE_RESERVOIR,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "aqueduct",                      .value = BUILDING_AQUEDUCT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "clear_land",                    .value = BUILDING_CLEAR_LAND,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_tent",              .value = BUILDING_HOUSE_SMALL_TENT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_tent",              .value = BUILDING_HOUSE_LARGE_TENT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_shack",             .value = BUILDING_HOUSE_SMALL_SHACK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_shack",             .value = BUILDING_HOUSE_LARGE_SHACK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_hovel",             .value = BUILDING_HOUSE_SMALL_HOVEL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_hovel",             .value = BUILDING_HOUSE_LARGE_HOVEL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_casa",              .value = BUILDING_HOUSE_SMALL_CASA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_casa",              .value = BUILDING_HOUSE_LARGE_CASA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_insula",            .value = BUILDING_HOUSE_SMALL_INSULA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_medium_insula",           .value = BUILDING_HOUSE_MEDIUM_INSULA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_insula",            .value = BUILDING_HOUSE_LARGE_INSULA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_grand_insula",            .value = BUILDING_HOUSE_GRAND_INSULA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_villa",             .value = BUILDING_HOUSE_SMALL_VILLA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_medium_villa",            .value = BUILDING_HOUSE_MEDIUM_VILLA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_villa",             .value = BUILDING_HOUSE_LARGE_VILLA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_grand_villa",             .value = BUILDING_HOUSE_GRAND_VILLA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_small_palace",            .value = BUILDING_HOUSE_SMALL_PALACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_medium_palace",           .value = BUILDING_HOUSE_MEDIUM_PALACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_large_palace",            .value = BUILDING_HOUSE_LARGE_PALACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "house_luxury_palace",           .value = BUILDING_HOUSE_LUXURY_PALACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "amphitheater",                  .value = BUILDING_AMPHITHEATER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "theater",                       .value = BUILDING_THEATER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hippodrome",                    .value = BUILDING_HIPPODROME,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "colosseum",                     .value = BUILDING_COLOSSEUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "gladiator_school",              .value = BUILDING_GLADIATOR_SCHOOL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "lion_house",                    .value = BUILDING_LION_HOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "actor_colony",                  .value = BUILDING_ACTOR_COLONY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "chariot_maker",                 .value = BUILDING_CHARIOT_MAKER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "plaza",                         .value = BUILDING_PLAZA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "gardens",                       .value = BUILDING_GARDENS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fort_legionaries",              .value = BUILDING_FORT_LEGIONARIES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_statue",                  .value = BUILDING_SMALL_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "medium_statue",                 .value = BUILDING_MEDIUM_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_statue",                  .value = BUILDING_LARGE_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fort_javelin",                  .value = BUILDING_FORT_JAVELIN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fort_mounted",                  .value = BUILDING_FORT_MOUNTED,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "doctor",                        .value = BUILDING_DOCTOR,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hospital",                      .value = BUILDING_HOSPITAL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "bathhouse",                     .value = BUILDING_BATHHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "barber",                        .value = BUILDING_BARBER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "distribution_center",           .value = BUILDING_DISTRIBUTION_CENTER_UNUSED,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "school",                        .value = BUILDING_SCHOOL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "academy",                       .value = BUILDING_ACADEMY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "library",                       .value = BUILDING_LIBRARY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fort_ground",                   .value = BUILDING_FORT_GROUND,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "prefecture",                    .value = BUILDING_PREFECTURE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "triumphal_arch",                .value = BUILDING_TRIUMPHAL_ARCH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fort",                          .value = BUILDING_FORT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "gatehouse",                     .value = BUILDING_GATEHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "tower",                         .value = BUILDING_TOWER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_temple_ceres",            .value = BUILDING_SMALL_TEMPLE_CERES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_temple_neptune",          .value = BUILDING_SMALL_TEMPLE_NEPTUNE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_temple_mercury",          .value = BUILDING_SMALL_TEMPLE_MERCURY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_temple_mars",             .value = BUILDING_SMALL_TEMPLE_MARS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_temple_venus",            .value = BUILDING_SMALL_TEMPLE_VENUS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_temple_ceres",            .value = BUILDING_LARGE_TEMPLE_CERES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_temple_neptune",          .value = BUILDING_LARGE_TEMPLE_NEPTUNE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_temple_mercury",          .value = BUILDING_LARGE_TEMPLE_MERCURY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_temple_mars",             .value = BUILDING_LARGE_TEMPLE_MARS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_temple_venus",            .value = BUILDING_LARGE_TEMPLE_VENUS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "market",                        .value = BUILDING_MARKET,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "granary",                       .value = BUILDING_GRANARY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "warehouse",                     .value = BUILDING_WAREHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "warehouse_space",               .value = BUILDING_WAREHOUSE_SPACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "shipyard",                      .value = BUILDING_SHIPYARD,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "dock",                          .value = BUILDING_DOCK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "wharf",                         .value = BUILDING_WHARF,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "governors_house",               .value = BUILDING_GOVERNORS_HOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "governors_villa",               .value = BUILDING_GOVERNORS_VILLA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "governors_palace",              .value = BUILDING_GOVERNORS_PALACE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "mission_post",                  .value = BUILDING_MISSION_POST,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "engineers_post",                .value = BUILDING_ENGINEERS_POST,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "low_bridge",                    .value = BUILDING_LOW_BRIDGE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "ship_bridge",                   .value = BUILDING_SHIP_BRIDGE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "senate",                        .value = BUILDING_SENATE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "senate_upgraded",               .value = BUILDING_SENATE_UPGRADED,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "forum",                         .value = BUILDING_FORUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "forum_upgraded",                .value = BUILDING_FORUM_UPGRADED,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "native_hut",                    .value = BUILDING_NATIVE_HUT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "native_meeting",                .value = BUILDING_NATIVE_MEETING,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "reservoir",                     .value = BUILDING_RESERVOIR,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fountain",                      .value = BUILDING_FOUNTAIN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "well",                          .value = BUILDING_WELL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "native_crops",                  .value = BUILDING_NATIVE_CROPS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "military_academy",              .value = BUILDING_MILITARY_ACADEMY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "barracks",                      .value = BUILDING_BARRACKS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_small_temples",             .value = BUILDING_MENU_SMALL_TEMPLES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_SMALL_TEMPLES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_large_temples",             .value = BUILDING_MENU_LARGE_TEMPLES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_LARGE_TEMPLES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "oracle",                        .value = BUILDING_ORACLE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "burning_ruin",                  .value = BUILDING_BURNING_RUIN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "wheat_farm",                    .value = BUILDING_WHEAT_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "vegetable_farm",                .value = BUILDING_VEGETABLE_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fruit_farm",                    .value = BUILDING_FRUIT_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "olive_farm",                    .value = BUILDING_OLIVE_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "vines_farm",                    .value = BUILDING_VINES_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pig_farm",                      .value = BUILDING_PIG_FARM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "marble_quarry",                 .value = BUILDING_MARBLE_QUARRY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "iron_mine",                     .value = BUILDING_IRON_MINE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "timber_yard",                   .value = BUILDING_TIMBER_YARD,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "clay_pit",                      .value = BUILDING_CLAY_PIT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "wine_workshop",                 .value = BUILDING_WINE_WORKSHOP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "oil_workshop",                  .value = BUILDING_OIL_WORKSHOP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "weapons_workshop",              .value = BUILDING_WEAPONS_WORKSHOP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "furniture_workshop",            .value = BUILDING_FURNITURE_WORKSHOP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pottery_workshop",              .value = BUILDING_POTTERY_WORKSHOP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "roadblock",                     .value = BUILDING_ROADBLOCK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "workcamp",                      .value = BUILDING_WORKCAMP,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_temple_ceres",            .value = BUILDING_GRAND_TEMPLE_CERES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_temple_neptune",          .value = BUILDING_GRAND_TEMPLE_NEPTUNE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_temple_mercury",          .value = BUILDING_GRAND_TEMPLE_MERCURY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_temple_mars",             .value = BUILDING_GRAND_TEMPLE_MARS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_temple_venus",            .value = BUILDING_GRAND_TEMPLE_VENUS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_grand_temples",             .value = BUILDING_MENU_GRAND_TEMPLES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_GRAND_TEMPLES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_trees",                     .value = BUILDING_MENU_TREES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_TREES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_paths",                     .value = BUILDING_MENU_PATHS,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_PATHS },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_parks",                     .value = BUILDING_MENU_PARKS,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_PARKS },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_pond",                    .value = BUILDING_SMALL_POND,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_pond",                    .value = BUILDING_LARGE_POND,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pine_tree",                     .value = BUILDING_PINE_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fir_tree",                      .value = BUILDING_FIR_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "oak_tree",                      .value = BUILDING_OAK_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "elm_tree",                      .value = BUILDING_ELM_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fig_tree",                      .value = BUILDING_FIG_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "plum_tree",                     .value = BUILDING_PLUM_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "palm_tree",                     .value = BUILDING_PALM_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "date_tree",                     .value = BUILDING_DATE_TREE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pine_tree",                     .value = BUILDING_PINE_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fir_path",                      .value = BUILDING_FIR_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "oak_path",                      .value = BUILDING_OAK_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "elm_path",                      .value = BUILDING_ELM_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "fig_path",                      .value = BUILDING_FIG_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "plum_path",                     .value = BUILDING_PLUM_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "palm_path",                     .value = BUILDING_PALM_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "date_path",                     .value = BUILDING_DATE_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pavilion_blue",                 .value = BUILDING_PAVILION_BLUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pavilion_red",                  .value = BUILDING_PAVILION_RED,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pavilion_orange",               .value = BUILDING_PAVILION_ORANGE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pavilion_yellow",               .value = BUILDING_PAVILION_YELLOW,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pavilion_green",                .value = BUILDING_PAVILION_GREEN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_statue_alt",              .value = BUILDING_SMALL_STATUE_ALT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_statue_alt_b",            .value = BUILDING_SMALL_STATUE_ALT_B,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "obelisk",                       .value = BUILDING_OBELISK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "pantheon",                      .value = BUILDING_PANTHEON,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "architect_guild",               .value = BUILDING_ARCHITECT_GUILD,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "mess_hall",                     .value = BUILDING_MESS_HALL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "lighthouse",                    .value = BUILDING_LIGHTHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_statues",                   .value = BUILDING_MENU_STATUES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_STATUES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "all_governors_houses",          .value = BUILDING_MENU_GOV_RES,                    .key = TR_PARAMETER_VALUE_BUILDING_MENU_GOV_RES },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "tavern",                        .value = BUILDING_TAVERN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "grand_garden",                  .value = BUILDING_GRAND_GARDEN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "arena",                         .value = BUILDING_ARENA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "horse_statue",                  .value = BUILDING_HORSE_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "dolphin_fountain",              .value = BUILDING_DOLPHIN_FOUNTAIN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hedge_dark",                    .value = BUILDING_HEDGE_DARK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hedge_light",                   .value = BUILDING_HEDGE_LIGHT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "garden_wall",                   .value = BUILDING_GARDEN_WALL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "legion_statue",                 .value = BUILDING_LEGION_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "decorative_column",             .value = BUILDING_DECORATIVE_COLUMN,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "colonnade",                     .value = BUILDING_COLONNADE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "lararium",                      .value = BUILDING_LARARIUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "nymphaeum",                     .value = BUILDING_NYMPHAEUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "small_mausoleum",               .value = BUILDING_SMALL_MAUSOLEUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "large_mausoleum",               .value = BUILDING_LARGE_MAUSOLEUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "watchtower",                    .value = BUILDING_WATCHTOWER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "palisade",                      .value = BUILDING_PALISADE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "garden_path",                   .value = BUILDING_GARDEN_PATH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "caravanserai",                  .value = BUILDING_CARAVANSERAI,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "roofed_garden_wall",            .value = BUILDING_ROOFED_GARDEN_WALL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "garden_wall_gate",              .value = BUILDING_GARDEN_WALL_GATE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hedge_gate_dark",               .value = BUILDING_HEDGE_GATE_DARK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "hedge_gate_light",              .value = BUILDING_HEDGE_GATE_LIGHT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "palisade_gate",                 .value = BUILDING_PALISADE_GATE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "gladiator_statue",              .value = BUILDING_GLADIATOR_STATUE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "highway",                       .value = BUILDING_HIGHWAY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "gold_mine",                     .value = BUILDING_GOLD_MINE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_BUILDING,            .text = "city_mint",                     .value = BUILDING_CITY_MINT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
};

static special_attribute_mapping_t special_attribute_mappings_allowed_buildings[] = {
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "none",                  .value = ALLOWED_BUILDING_NONE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "farms",                 .value = ALLOWED_BUILDING_FARMS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "raw_materials",         .value = ALLOWED_BUILDING_RAW_MATERIALS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "workshops",             .value = ALLOWED_BUILDING_WORKSHOPS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "road",                  .value = ALLOWED_BUILDING_ROAD,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "wall",                  .value = ALLOWED_BUILDING_WALL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "aqueduct",              .value = ALLOWED_BUILDING_AQUEDUCT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "housing",               .value = ALLOWED_BUILDING_HOUSING,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "amphitheater",          .value = ALLOWED_BUILDING_AMPHITHEATER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "theater",               .value = ALLOWED_BUILDING_THEATER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "hippodrome",            .value = ALLOWED_BUILDING_HIPPODROME,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "colosseum",             .value = ALLOWED_BUILDING_COLOSSEUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "gladiator_school",      .value = ALLOWED_BUILDING_GLADIATOR_SCHOOL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "lion_house",            .value = ALLOWED_BUILDING_LION_HOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "actor_colony",          .value = ALLOWED_BUILDING_ACTOR_COLONY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "chariot_maker",         .value = ALLOWED_BUILDING_CHARIOT_MAKER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "gardens",               .value = ALLOWED_BUILDING_GARDENS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "plaza",                 .value = ALLOWED_BUILDING_PLAZA,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "statues",               .value = ALLOWED_BUILDING_STATUES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "doctor",                .value = ALLOWED_BUILDING_DOCTOR,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "hospital",              .value = ALLOWED_BUILDING_HOSPITAL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "bathhouse",             .value = ALLOWED_BUILDING_BATHHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "barber",                .value = ALLOWED_BUILDING_BARBER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "school",                .value = ALLOWED_BUILDING_SCHOOL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "academy",               .value = ALLOWED_BUILDING_ACADEMY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "library",               .value = ALLOWED_BUILDING_LIBRARY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "prefecture",            .value = ALLOWED_BUILDING_PREFECTURE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "fort",                  .value = ALLOWED_BUILDING_FORT,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "gatehouse",             .value = ALLOWED_BUILDING_GATEHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "tower",                 .value = ALLOWED_BUILDING_TOWER,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "small_temples",         .value = ALLOWED_BUILDING_SMALL_TEMPLES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "large_temples",         .value = ALLOWED_BUILDING_LARGE_TEMPLES,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "market",                .value = ALLOWED_BUILDING_MARKET,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "granary",               .value = ALLOWED_BUILDING_GRANARY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "warehouse",             .value = ALLOWED_BUILDING_WAREHOUSE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "triumphal_arch",        .value = ALLOWED_BUILDING_TRIUMPHAL_ARCH,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "dock",                  .value = ALLOWED_BUILDING_DOCK,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "wharf",                 .value = ALLOWED_BUILDING_WHARF,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "governor_home",         .value = ALLOWED_BUILDING_GOVERNOR_HOME,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "engineers_post",        .value = ALLOWED_BUILDING_ENGINEERS_POST,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "senate",                .value = ALLOWED_BUILDING_SENATE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "forum",                 .value = ALLOWED_BUILDING_FORUM,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "well",                  .value = ALLOWED_BUILDING_WELL,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "oracle",                .value = ALLOWED_BUILDING_ORACLE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "mission_post",          .value = ALLOWED_BUILDING_MISSION_POST,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "bridge",                .value = ALLOWED_BUILDING_BRIDGE,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "barracks",              .value = ALLOWED_BUILDING_BARRACKS,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "military_academy",      .value = ALLOWED_BUILDING_MILITARY_ACADEMY,                    .key = TR_PARAMETER_VALUE_DYNAMIC_RESOLVE },
    { .type = PARAMETER_TYPE_ALLOWED_BUILDING,            .text = "monuments",             .value = ALLOWED_BUILDING_MONUMENTS,                    .key = TR_EDITOR_ALLOWED_BUILDINGS_MONUMENTS },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_ALLOWED_BUILDINGS_SIZE (sizeof(special_attribute_mappings_allowed_buildings) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_standard_message[] = {
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "none",                      .value = 0,                    .key = TR_PARAMETER_VALUE_NONE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_debt",               .value = MESSAGE_CITY_IN_DEBT,                    .key = TR_PARAMETER_VALUE_MESSAGE_CITY_IN_DEBT },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_debt_again",         .value = MESSAGE_CITY_IN_DEBT_AGAIN,                    .key = TR_PARAMETER_VALUE_MESSAGE_CITY_IN_DEBT_AGAIN },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_ebt_still",          .value = MESSAGE_CITY_STILL_IN_DEBT,                    .key = TR_PARAMETER_VALUE_MESSAGE_CITY_STILL_IN_DEBT },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_wrath",              .value = MESSAGE_CAESAR_WRATH,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_WRATH },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_army_continue",      .value = MESSAGE_CAESAR_ARMY_CONTINUE,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_ARMY_CONTINUE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_army_retreat",       .value = MESSAGE_CAESAR_ARMY_RETREAT,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_ARMY_RETREAT },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "local_uprising",            .value = MESSAGE_DISTANT_BATTLE,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "local_uprising",            .value = MESSAGE_ENEMIES_CLOSING,                    .key = TR_PARAMETER_VALUE_MESSAGE_ENEMIES_CLOSING },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "local_uprising",            .value = MESSAGE_ENEMIES_AT_THE_DOOR,                    .key = TR_PARAMETER_VALUE_MESSAGE_ENEMIES_AT_THE_DOOR },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "small_festival",            .value = MESSAGE_SMALL_FESTIVAL,                    .key = TR_PARAMETER_VALUE_MESSAGE_SMALL_FESTIVAL },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "large_festival",            .value = MESSAGE_LARGE_FESTIVAL,                    .key = TR_PARAMETER_VALUE_MESSAGE_LARGE_FESTIVAL },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "grand_festival",            .value = MESSAGE_GRAND_FESTIVAL,                    .key = TR_PARAMETER_VALUE_MESSAGE_GRAND_FESTIVAL },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "gods_unhappy",              .value = MESSAGE_GODS_UNHAPPY,                    .key = TR_PARAMETER_VALUE_MESSAGE_GODS_UNHAPPY },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "gladiator_revolt",          .value = MESSAGE_GLADIATOR_REVOLT,                    .key = TR_PARAMETER_VALUE_MESSAGE_GLADIATOR_REVOLT },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "gladiator_revolt_over",     .value = MESSAGE_GLADIATOR_REVOLT_FINISHED,                    .key = TR_PARAMETER_VALUE_MESSAGE_GLADIATOR_REVOLT_FINISHED },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "emperor_change",            .value = MESSAGE_EMPEROR_CHANGE,                    .key = TR_PARAMETER_VALUE_MESSAGE_EMPEROR_CHANGE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "land_trade_sandstorms",     .value = MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS,                    .key = TR_PARAMETER_VALUE_MESSAGE_LAND_TRADE_DISRUPTED_SANDSTORMS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "land_trade_landslides",     .value = MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES,                    .key = TR_PARAMETER_VALUE_MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "land_trade_storms",         .value = MESSAGE_SEA_TRADE_DISRUPTED,                    .key = TR_PARAMETER_VALUE_MESSAGE_SEA_TRADE_DISRUPTED },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "rome_raises_wages",         .value = MESSAGE_ROME_RAISES_WAGES,                    .key = TR_PARAMETER_VALUE_MESSAGE_ROME_RAISES_WAGES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "rome_lowers_wages",         .value = MESSAGE_ROME_LOWERS_WAGES,                    .key = TR_PARAMETER_VALUE_MESSAGE_ROME_LOWERS_WAGES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "contaminated_water",        .value = MESSAGE_CONTAMINATED_WATER,                    .key = TR_PARAMETER_VALUE_MESSAGE_CONTAMINATED_WATER },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "empire_expanded",           .value = MESSAGE_EMPIRE_HAS_EXPANDED,                    .key = TR_PARAMETER_VALUE_MESSAGE_EMPIRE_HAS_EXPANDED },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_ceres",            .value = MESSAGE_WRATH_OF_CERES,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_CERES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_neptune_2",        .value = MESSAGE_WRATH_OF_NEPTUNE_NO_SEA_TRADE,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_NEPTUNE_NO_SEA_TRADE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_mercury",          .value = MESSAGE_WRATH_OF_MERCURY,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_MERCURY },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_mars_2",           .value = MESSAGE_WRATH_OF_MARS_NO_MILITARY,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_MARS_NO_MILITARY },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_venus",            .value = MESSAGE_WRATH_OF_VENUS,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_VENUS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_neptune",          .value = MESSAGE_WRATH_OF_NEPTUNE,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_NEPTUNE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_mars",             .value = MESSAGE_WRATH_OF_MARS,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_MARS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "ceres_upset",               .value = MESSAGE_CERES_IS_UPSET,                    .key = TR_PARAMETER_VALUE_MESSAGE_CERES_IS_UPSET },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "neptune_upset",             .value = MESSAGE_NEPTUNE_IS_UPSET,                    .key = TR_PARAMETER_VALUE_MESSAGE_NEPTUNE_IS_UPSET },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "mercury_upset",             .value = MESSAGE_MERCURY_IS_UPSET,                    .key = TR_PARAMETER_VALUE_MESSAGE_MERCURY_IS_UPSET },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "mars_upset",                .value = MESSAGE_MARS_IS_UPSET,                    .key = TR_PARAMETER_VALUE_MESSAGE_MARS_IS_UPSET },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "venus_upset",               .value = MESSAGE_VENUS_IS_UPSET,                    .key = TR_PARAMETER_VALUE_MESSAGE_VENUS_IS_UPSET },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_ceres",            .value = MESSAGE_BLESSING_FROM_CERES,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_CERES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_neptune",          .value = MESSAGE_BLESSING_FROM_NEPTUNE,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_NEPTUNE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_mercury",          .value = MESSAGE_BLESSING_FROM_MERCURY,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_MERCURY },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_mars",             .value = MESSAGE_BLESSING_FROM_MARS,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_MARS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_venus",            .value = MESSAGE_BLESSING_FROM_VENUS,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_VENUS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_mercury_2",        .value = MESSAGE_BLESSING_FROM_MERCURY_ALTERNATE,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_MERCURY_ALTERNATE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_neptune_2",        .value = MESSAGE_BLESSING_FROM_NEPTUNE_ALTERNATE,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_NEPTUNE_ALTERNATE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "blessing_venus_2",          .value = MESSAGE_BLESSING_FROM_VENUS_ALTERNATE,                    .key = TR_PARAMETER_VALUE_MESSAGE_BLESSING_FROM_VENUS_ALTERNATE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "wrath_of_mars_3",           .value = MESSAGE_WRATH_OF_MARS_NO_NATIVES,                    .key = TR_PARAMETER_VALUE_MESSAGE_WRATH_OF_MARS_NO_NATIVES },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "gods_wrathful",             .value = MESSAGE_GODS_WRATHFUL,                    .key = TR_PARAMETER_VALUE_MESSAGE_GODS_WRATHFUL },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "distant_battle_lost_no_troops",   .value = MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE_LOST_NO_TROOPS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "distant_battle_lost_too_late",    .value = MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE_LOST_TOO_LATE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "distant_battle_lost_too_weak",    .value = MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE_LOST_TOO_WEAK },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "distant_battle_won",              .value = MESSAGE_DISTANT_BATTLE_WON,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE_WON },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "distant_battle_city_retaken",     .value = MESSAGE_DISTANT_BATTLE_CITY_RETAKEN,                    .key = TR_PARAMETER_VALUE_MESSAGE_DISTANT_BATTLE_CITY_RETAKEN },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "health_illness",            .value = MESSAGE_HEALTH_ILLNESS,                    .key = TR_PARAMETER_VALUE_MESSAGE_HEALTH_ILLNESS },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "health_disease",            .value = MESSAGE_HEALTH_DISEASE,                    .key = TR_PARAMETER_VALUE_MESSAGE_HEALTH_DISEASE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "health_pestilence",         .value = MESSAGE_HEALTH_PESTILENCE,                    .key = TR_PARAMETER_VALUE_MESSAGE_HEALTH_PESTILENCE },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_respect_1",          .value = MESSAGE_CAESAR_RESPECT_1,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_RESPECT_1 },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_respect_2",          .value = MESSAGE_CAESAR_RESPECT_2,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_RESPECT_2 },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_respect_3",          .value = MESSAGE_CAESAR_RESPECT_3,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_RESPECT_3 },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "emigration",                .value = MESSAGE_EMIGRATION,                    .key = TR_PARAMETER_VALUE_MESSAGE_EMIGRATION },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "fired",                     .value = MESSAGE_FIRED,                    .key = TR_PARAMETER_VALUE_MESSAGE_FIRED },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "soldiers_starving",         .value = MESSAGE_SOLDIERS_STARVING,                    .key = TR_PARAMETER_VALUE_MESSAGE_SOLDIERS_STARVING },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "caesar_angry",              .value = MESSAGE_CAESAR_ANGER,                    .key = TR_PARAMETER_VALUE_MESSAGE_CAESAR_ANGER },
    { .type = PARAMETER_TYPE_STANDARD_MESSAGE,            .text = "enemies_leaving",           .value = MESSAGE_ENEMIES_LEAVING,                    .key = TR_PARAMETER_VALUE_MESSAGE_ENEMIES_LEAVING },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_STANDARD_MESSAGE_SIZE (sizeof(special_attribute_mappings_standard_message) / sizeof(special_attribute_mapping_t))

static special_attribute_mapping_t special_attribute_mappings_media_type[] = {
    { .type = PARAMETER_TYPE_MEDIA_TYPE,                  .text = "sound",                     .value = 1,          .key = TR_PARAMETER_VALUE_MEDIA_TYPE_SOUND },
    { .type = PARAMETER_TYPE_MEDIA_TYPE,                  .text = "video",                     .value = 2,          .key = TR_PARAMETER_VALUE_MEDIA_TYPE_VIDEO },
};

#define SPECIAL_ATTRIBUTE_MAPPINGS_MEDIA_TYPE_SIZE (sizeof(special_attribute_mappings_media_type) / sizeof(special_attribute_mapping_t))

special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping(parameter_type type, int index)
{
    switch (type) {
        case PARAMETER_TYPE_CHECK:
            return &special_attribute_mappings_check[index];
        case PARAMETER_TYPE_DIFFICULTY:
            return &special_attribute_mappings_difficulty[index];
        case PARAMETER_TYPE_BOOLEAN:
            return &special_attribute_mappings_boolean[index];
        case PARAMETER_TYPE_POP_CLASS:
            return &special_attribute_mappings_pop_class[index];
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_BUILDING_COUNTING:
            return &special_attribute_mappings_buildings[index];
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            return &special_attribute_mappings_allowed_buildings[index];
        case PARAMETER_TYPE_STANDARD_MESSAGE:
            return &special_attribute_mappings_standard_message[index];
        case PARAMETER_TYPE_MEDIA_TYPE:
            return &special_attribute_mappings_media_type[index];
        default:
            return 0;
    }
}

int scenario_events_parameter_data_get_mappings_size(parameter_type type)
{
    switch (type) {
        case PARAMETER_TYPE_CHECK:
            return SPECIAL_ATTRIBUTE_MAPPINGS_CHECK_SIZE;
        case PARAMETER_TYPE_DIFFICULTY:
            return SPECIAL_ATTRIBUTE_MAPPINGS_CHECK_DIFFICULTY;
        case PARAMETER_TYPE_BOOLEAN:
            return SPECIAL_ATTRIBUTE_MAPPINGS_BOOLEAN_SIZE;
        case PARAMETER_TYPE_POP_CLASS:
            return SPECIAL_ATTRIBUTE_MAPPINGS_POP_CLASS_SIZE;
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_BUILDING_COUNTING:
            return BUILDING_TYPE_MAX;
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            return SPECIAL_ATTRIBUTE_MAPPINGS_ALLOWED_BUILDINGS_SIZE;
        case PARAMETER_TYPE_STANDARD_MESSAGE:
            return SPECIAL_ATTRIBUTE_MAPPINGS_STANDARD_MESSAGE_SIZE;
        case PARAMETER_TYPE_MEDIA_TYPE:
            return SPECIAL_ATTRIBUTE_MAPPINGS_MEDIA_TYPE_SIZE;
        default:
            return 0;
    }
}

special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping_by_value(parameter_type type, int target)
{
    int array_size = scenario_events_parameter_data_get_mappings_size(type);
    for (int i = 0; i < array_size; i++) {
        special_attribute_mapping_t *current = scenario_events_parameter_data_get_attribute_mapping(type, i);
        if (target == current->value) {
            return current;
        }
    }
    return 0;
}

special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping_by_text(parameter_type type, const char *value)
{
    if (!value) {
        return 0;
    }

    int array_size = scenario_events_parameter_data_get_mappings_size(type);
    for (int i = 0; i < array_size; i++) {
        special_attribute_mapping_t *current = scenario_events_parameter_data_get_attribute_mapping(type, i);
        if (strcmp(value, current->text) == 0) {
            return current;
        }
    }
    return 0;
}

int scenario_events_parameter_data_get_default_value_for_parameter(xml_data_attribute_t *attribute_data)
{
    switch (attribute_data->type) {
        case PARAMETER_TYPE_NUMBER:
            if (attribute_data->min_limit > 0) {
                return attribute_data->min_limit;
            } else {
                if (attribute_data->max_limit < 0) {
                    return attribute_data->max_limit;
                }
                return 0;
            }
        case PARAMETER_TYPE_CHECK:
            return COMPARISON_TYPE_EQUAL;
        case PARAMETER_TYPE_DIFFICULTY:
            return DIFFICULTY_NORMAL;
        case PARAMETER_TYPE_RESOURCE:
            return RESOURCE_WHEAT;
        case PARAMETER_TYPE_POP_CLASS:
            return POP_CLASS_ALL;
        case PARAMETER_TYPE_BUILDING:
            return BUILDING_WELL;
        case PARAMETER_TYPE_BUILDING_COUNTING:
            return BUILDING_WELL;
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            return ALLOWED_BUILDING_FARMS;
        case PARAMETER_TYPE_STANDARD_MESSAGE:
            return MESSAGE_CAESAR_WRATH;
        default:
            return 0;
    }
}
