#ifndef SCENARIO_EVENTS_PARAMETER_DATA_H
#define SCENARIO_EVENTS_PARAMETER_DATA_H

#include "building/type.h"
#include "city/message.h"
#include "game/settings.h"
#include "scenario/data.h"
#include "scenario/event/data.h"
#include "translation/translation.h"

typedef enum {
    PARAMETER_TYPE_UNDEFINED = 0,
    PARAMETER_TYPE_NUMBER,
    PARAMETER_TYPE_REQUEST,
    PARAMETER_TYPE_TEXT,
    PARAMETER_TYPE_CHECK,
    PARAMETER_TYPE_DIFFICULTY,
    PARAMETER_TYPE_RESOURCE,
    PARAMETER_TYPE_BOOLEAN,
    PARAMETER_TYPE_ROUTE,
    PARAMETER_TYPE_POP_CLASS,
    PARAMETER_TYPE_BUILDING,
    PARAMETER_TYPE_BUILDING_COUNTING,
    PARAMETER_TYPE_ALLOWED_BUILDING,
    PARAMETER_TYPE_STANDARD_MESSAGE,
    PARAMETER_TYPE_FUTURE_CITY,
    PARAMETER_TYPE_MIN_MAX_NUMBER,
    PARAMETER_TYPE_MEDIA_TYPE,
    PARAMETER_TYPE_CUSTOM_MESSAGE,
    PARAMETER_TYPE_CUSTOM_VARIABLE,
    PARAMETER_TYPE_RATING_TYPE,
    PARAMETER_TYPE_STORAGE_TYPE,
    PARAMETER_TYPE_INVASION_TYPE,
    PARAMETER_TYPE_TARGET_TYPE,
    PARAMETER_TYPE_ENEMY_TYPE,
    PARAMETER_TYPE_GOD,
    PARAMETER_TYPE_CLIMATE,
} parameter_type;

typedef struct {
    const char *name;
    parameter_type type;
    int min_limit;
    int max_limit;
    translation_key key;
} xml_data_attribute_t;

typedef struct {
    condition_types type;
    xml_data_attribute_t xml_attr;
    xml_data_attribute_t xml_parm1;
    xml_data_attribute_t xml_parm2;
    xml_data_attribute_t xml_parm3;
    xml_data_attribute_t xml_parm4;
    xml_data_attribute_t xml_parm5;
} scenario_condition_data_t;

typedef struct {
    action_types type;
    xml_data_attribute_t xml_attr;
    xml_data_attribute_t xml_parm1;
    xml_data_attribute_t xml_parm2;
    xml_data_attribute_t xml_parm3;
    xml_data_attribute_t xml_parm4;
    xml_data_attribute_t xml_parm5;
} scenario_action_data_t;

typedef struct {
    parameter_type type;
    const char *text;
    int value;
    translation_key key;
} special_attribute_mapping_t;

scenario_condition_data_t *scenario_events_parameter_data_get_conditions_xml_attributes(condition_types type);
scenario_condition_data_t *scenario_events_parameter_data_get_conditions_xml_attributes_alphabetical(int index);
scenario_action_data_t *scenario_events_parameter_data_get_actions_xml_attributes(action_types type);
scenario_action_data_t *scenario_events_parameter_data_get_actions_xml_attributes_alphabetical(int index);
special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping(parameter_type type, int index);
special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping_by_value(parameter_type type, int target);
special_attribute_mapping_t *scenario_events_parameter_data_get_attribute_mapping_by_text(parameter_type type, const char *value);
int scenario_events_parameter_data_get_mappings_size(parameter_type type);

void scenario_events_parameter_data_sort_alphabetically(void);

int scenario_events_parameter_data_get_default_value_for_parameter(xml_data_attribute_t *attribute_data);

const uint8_t *scenario_events_parameter_data_get_display_string(special_attribute_mapping_t *entry);
void scenario_events_parameter_data_get_display_string_for_value(parameter_type type, int value, uint8_t *result_text,
    int maxlength);
void scenario_events_parameter_data_get_display_string_for_action(const scenario_action_t *action, uint8_t *result_text,
    int maxlength);
void scenario_events_parameter_data_get_display_string_for_condition(const scenario_condition_t *condition,
    uint8_t *result_text, int maxlength);

#endif // SCENARIO_EVENTS_PARAMETER_DATA_H
