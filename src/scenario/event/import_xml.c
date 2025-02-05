#include "import_xml.h"

#include "core/encoding.h"
#include "core/file.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_parser.h"
#include "empire/city.h"
#include "empire/type.h"
#include "scenario/custom_messages.h"
#include "scenario/custom_variable.h"
#include "scenario/event/controller.h"
#include "scenario/event/data.h"
#include "scenario/event/event.h"
#include "scenario/event/parameter_data.h"
#include "window/plain_message_dialog.h"

#include <math.h>
#include <stdio.h>

#define XML_TOTAL_ELEMENTS 63
#define ERROR_MESSAGE_LENGTH 200

static struct {
    int success;
    char error_message[ERROR_MESSAGE_LENGTH];
    int error_line_number;
    uint8_t error_line_number_text[50];
    int version;
    scenario_event_t *current_event;
    scenario_condition_group_t *current_group;
    int variables_count;
} data;

static int xml_import_start_scenario_events(void);
static int xml_import_start_event(void);
static void xml_import_end_conditions(void);
static int xml_import_start_group(void);
static void xml_import_end_group(void);
static int xml_import_create_condition(void);
static int xml_import_create_action(void);
static int xml_import_start_custom_variables(void);
static int xml_import_create_custom_variable(void);
static void xml_import_log_error(const char *msg);

static int xml_import_special_parse_attribute(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_building_counting(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_future_city(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_limited_number(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_min_max_number(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_resource(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_route(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_custom_message(xml_data_attribute_t *attr, int *target);
static int xml_import_special_parse_custom_variable(xml_data_attribute_t *attr, int *target);

static condition_types get_condition_type_from_element_name(const char *name);
static action_types get_action_type_from_element_name(const char *name);

static int condition_populate_parameters(scenario_condition_t *condition);
static int action_populate_parameters(scenario_action_t *action);

static const xml_parser_element xml_elements[XML_TOTAL_ELEMENTS] = {
    { "events", xml_import_start_scenario_events },
    { "event", xml_import_start_event, 0, "events" },
    { "conditions", 0, xml_import_end_conditions, "event" },
    { "group", xml_import_start_group, xml_import_end_group, "conditions" },
    { "actions", 0, 0, "event" },
    { "time", xml_import_create_condition, 0, "conditions|group" },
    { "difficulty", xml_import_create_condition, 0, "conditions|group" },
    { "money", xml_import_create_condition, 0, "conditions|group" },
    { "savings", xml_import_create_condition, 0, "conditions|group" },
    { "stats_favor", xml_import_create_condition, 0, "conditions|group" },
    { "stats_prosperity", xml_import_create_condition, 0, "conditions|group" }, //10
    { "stats_culture", xml_import_create_condition, 0, "conditions|group" },
    { "stats_peace", xml_import_create_condition, 0, "conditions|group" },
    { "trade_sell_price", xml_import_create_condition, 0, "conditions|group" },
    { "population_unemployed", xml_import_create_condition, 0, "conditions|group" },
    { "rome_wages", xml_import_create_condition, 0, "conditions|group" },
    { "city_population", xml_import_create_condition, 0, "conditions|group" },
    { "building_count_active", xml_import_create_condition, 0, "conditions|group" },
    { "stats_health", xml_import_create_condition, 0, "conditions|group" },
    { "count_own_troops", xml_import_create_condition, 0, "conditions|group" },
    { "request_is_ongoing", xml_import_create_condition, 0, "conditions|group" }, //20
    { "tax_rate", xml_import_create_condition, 0, "conditions|group" },
    { "favor_add", xml_import_create_action, 0, "actions" },
    { "money_add", xml_import_create_action, 0, "actions" },
    { "savings_add", xml_import_create_action, 0, "actions" },
    { "trade_price_adjust", xml_import_create_action, 0, "actions" },
    { "trade_problems_land", xml_import_create_action, 0, "actions" },
    { "trade_problems_sea", xml_import_create_action, 0, "actions" },
    { "trade_route_amount", xml_import_create_action, 0, "actions" },
    { "change_rome_wages", xml_import_create_action, 0, "actions" },
    { "gladiator_revolt", xml_import_create_action, 0, "actions" }, //30
    { "change_resource_produced", xml_import_create_action, 0, "actions" },
    { "change_allowed_buildings", xml_import_create_action, 0, "actions" },
    { "send_standard_message", xml_import_create_action, 0, "actions" },
    { "city_health", xml_import_create_action, 0, "actions" },
    { "trade_price_set", xml_import_create_action, 0, "actions" },
    { "empire_map_convert_future_trade_city", xml_import_create_action, 0, "actions" },
    { "request_immediately_start", xml_import_create_action, 0, "actions" },
    { "show_custom_message", xml_import_create_action, 0, "actions" },
    { "tax_rate_set", xml_import_create_action, 0, "actions" },
    { "building_count_any", xml_import_create_condition, 0, "conditions|group" }, //40
    { "variable_check", xml_import_create_condition, 0, "conditions|group" },
    { "trade_route_open", xml_import_create_condition, 0, "conditions|group" },
    { "trade_route_price", xml_import_create_condition, 0, "conditions|group" },
    { "change_variable", xml_import_create_action, 0, "actions" },
    { "change_trade_route_open_price", xml_import_create_action, 0, "actions" },
    { "variables", xml_import_start_custom_variables, 0, "events" },
    { "variable", xml_import_create_custom_variable, 0, "variables" },
    { "change_city_rating", xml_import_create_action, 0, "actions" },
    { "change_resource_stockpiles", xml_import_create_action, 0, "actions" },
    { "resource_stored_count", xml_import_create_condition, 0, "conditions|group" }, //50
    { "resource_storage_available", xml_import_create_condition, 0, "conditions|group" },
    { "trade_route_set_open", xml_import_create_action, 0, "actions" },
    { "trade_route_add_new_resource", xml_import_create_action, 0, "actions" },
    { "trade_set_buy_price_only", xml_import_create_action, 0, "actions" },
    { "trade_set_sell_price_only", xml_import_create_action, 0, "actions" },
    { "building_force_collapse", xml_import_create_action, 0, "actions" },
    { "invasion_start_immediate", xml_import_create_action, 0, "actions" },
    { "building_count_area", xml_import_create_condition, 0, "conditions|group" },
    { "cause_blessing", xml_import_create_action, 0, "actions" },
    { "cause_minor_curse", xml_import_create_action, 0, "actions" }, // 60
    { "cause_major_curse", xml_import_create_action, 0, "actions" },
    { "change_climate", xml_import_create_action, 0, "actions"}
};

static int xml_import_start_scenario_events(void)
{
    if (!data.success) {
        return 0;
    }

    data.version = xml_parser_get_attribute_int("version");
    if (!data.version) {
        data.success = 0;
        log_error("No version set", 0, 0);
        return 0;
    }
    return 1;
}

static int xml_import_start_custom_variables(void)
{
    if (!data.success) {
        return 0;
    }

    scenario_custom_variable_delete_all();
    data.variables_count = 0;
    return 1;
}

static int xml_import_create_custom_variable(void)
{
    if (!data.success) {
        return 0;
    }

    if (!xml_parser_has_attribute("uid") && !xml_parser_has_attribute("name")) {
        xml_import_log_error("Variable has no unique identifier (uid)");
        return 0;
    }

    int value = 0;
    if (xml_parser_has_attribute("initial_value")) {
        value = xml_parser_get_attribute_int("initial_value");
    } else if (xml_parser_has_attribute("value")) {
        value = xml_parser_get_attribute_int("value");
    }

    const char *name = xml_parser_get_attribute_string("uid");
    if (!name) {
        name = xml_parser_get_attribute_string("name");
    }

    uint8_t encoded_name[300];
    encoding_from_utf8(name, encoded_name, 300);
    unsigned int id = scenario_custom_variable_get_id_by_name(encoded_name);
    if(id) {
        xml_import_log_error("Variable unique identifier is not unique");
        return 0;
    }

    id = scenario_custom_variable_create(encoded_name, value);
    if (!id) {
        xml_import_log_error("Could not import the variable!");
        return 0;
    }

    return 1;
}

static int xml_import_start_event(void)
{
    if (!data.success) {
        return 0;
    }

    int min = xml_parser_get_attribute_int("repeat_months_min");
    if (!min) {
        min = 0;
    }

    int max = xml_parser_get_attribute_int("repeat_months_max");
    if (!max) {
        max = min;
    }

    int max_repeats = xml_parser_get_attribute_int("max_number_of_repeats");
    if (!max_repeats) {
        max_repeats = 0;
    }

    data.current_event = scenario_event_create(min, max, max_repeats);

    if (!data.current_event) {
        data.success = 0;
        log_error("Could not create the event - out of memory", 0, 0);
        return 0;
    }
    if (xml_parser_has_attribute("name")) {
        encoding_from_utf8(xml_parser_get_attribute_string("name"), data.current_event->name, EVENT_NAME_LENGTH);
    }
    return 1;
}

static void xml_import_end_conditions(void)
{
    if (data.current_event->condition_groups.size == 0) {
        array_advance(data.current_event->condition_groups);
    }
}

static scenario_condition_group_t *get_first_group(void)
{
    if (data.current_event->condition_groups.size == 0) {
        return array_advance(data.current_event->condition_groups);
    }
    return array_item(data.current_event->condition_groups, 0);
}

static int xml_import_start_group(void)
{
    if (!data.success) {
        return 0;
    }

    fulfillment_type type = FULFILLMENT_TYPE_ANY;
    if (xml_parser_has_attribute("fulfillment_type")) {
        const char *values[2] = { "all", "any" };
        int result = xml_parser_get_attribute_enum("fulfillment_type", values, 2, 0);
        if (result != -1) {
            type = result;
        }
    }
    // Only group 0 has fulfillment type all
    if (type == FULFILLMENT_TYPE_ALL) {
        data.current_group = get_first_group();
    } else {
        array_new_item_after_index(data.current_event->condition_groups, 1, data.current_group);
    }
    return data.current_group != 0;
}

static void xml_import_end_group(void)
{
    data.current_group = 0;
}

static condition_types get_condition_type_from_element_name(const char *name)
{
    for (condition_types i = CONDITION_TYPE_MIN; i < CONDITION_TYPE_MAX; i++) {
        const char *condition_name = scenario_events_parameter_data_get_conditions_xml_attributes(i)->xml_attr.name;
        if (condition_name && xml_parser_compare_multiple(condition_name, name)) {
            return i;
        }
    }
    return CONDITION_TYPE_UNDEFINED;
}

static int condition_populate_parameters(scenario_condition_t *condition)
{
    scenario_condition_data_t *condition_data = scenario_events_parameter_data_get_conditions_xml_attributes(condition->type);
    int success = 1;
    success &= xml_import_special_parse_attribute(&condition_data->xml_parm1, &condition->parameter1);
    success &= xml_import_special_parse_attribute(&condition_data->xml_parm2, &condition->parameter2);
    success &= xml_import_special_parse_attribute(&condition_data->xml_parm3, &condition->parameter3);
    success &= xml_import_special_parse_attribute(&condition_data->xml_parm4, &condition->parameter4);
    success &= xml_import_special_parse_attribute(&condition_data->xml_parm5, &condition->parameter5);

    return success;
}

static int xml_import_create_condition(void)
{
    if (!data.success) {
        return 0;
    }

    const char *name = xml_parser_get_current_element_name();
    condition_types type = get_condition_type_from_element_name(name);
    if (type == CONDITION_TYPE_UNDEFINED) {
        log_info("Invalid condition type specified", 0, 0);
        return 0;
    }
    scenario_condition_group_t *group = data.current_group ? data.current_group : get_first_group();
    scenario_condition_t *condition = scenario_event_condition_create(group, type);
    return condition_populate_parameters(condition);
}

static action_types get_action_type_from_element_name(const char *name)
{
    for (action_types i = ACTION_TYPE_MIN; i < ACTION_TYPE_MAX; i++) {
        const char *action_name = scenario_events_parameter_data_get_actions_xml_attributes(i)->xml_attr.name;
        if (xml_parser_compare_multiple(action_name, name)) {
            return i;
        }
    }
    return ACTION_TYPE_UNDEFINED;
}

static int action_populate_parameters(scenario_action_t *action)
{
    scenario_action_data_t *action_data = scenario_events_parameter_data_get_actions_xml_attributes(action->type);
    int success = 1;
    success &= xml_import_special_parse_attribute(&action_data->xml_parm1, &action->parameter1);
    success &= xml_import_special_parse_attribute(&action_data->xml_parm2, &action->parameter2);
    success &= xml_import_special_parse_attribute(&action_data->xml_parm3, &action->parameter3);
    success &= xml_import_special_parse_attribute(&action_data->xml_parm4, &action->parameter4);
    success &= xml_import_special_parse_attribute(&action_data->xml_parm5, &action->parameter5);

    return success;
}

static int xml_import_create_action(void)
{
    if (!data.success) {
        return 0;
    }

    const char *name = xml_parser_get_current_element_name();
    action_types type = get_action_type_from_element_name(name);
    if (type == ACTION_TYPE_UNDEFINED) {
        xml_import_log_error("Invalid action type specified");
        return 0;
    }

    scenario_action_t *action = scenario_event_action_create(data.current_event, type);
    return action_populate_parameters(action);
}

static void xml_import_log_error(const char *msg)
{
    data.success = 0;
    data.error_line_number = xml_parser_get_current_line_number();
    snprintf(data.error_message, ERROR_MESSAGE_LENGTH, "%s", msg);
    log_error("Error while import scenario events from XML. ", data.error_message, 0);
    log_error("Line:", 0, data.error_line_number);

    string_copy(translation_for(TR_EDITOR_IMPORT_LINE), data.error_line_number_text, 50);
    int length = string_length(data.error_line_number_text);

    uint8_t number_as_text[15];
    string_from_int(number_as_text, data.error_line_number, 0);
    string_copy(number_as_text, data.error_line_number_text + length, 50);

    window_plain_message_dialog_show_with_extra(
        TR_EDITOR_UNABLE_TO_LOAD_EVENTS_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE,
        string_from_ascii(data.error_message),
        data.error_line_number_text);
}

static int xml_import_special_parse_type(xml_data_attribute_t *attr, parameter_type type, int *target)
{
    if (!attr->name) {
        return 1;
    }

    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    special_attribute_mapping_t *found = scenario_events_parameter_data_get_attribute_mapping_by_text(type, value);
    if (found == 0) {
        xml_import_log_error("Could not resolve the given value.");
        return 0;
    }

    *target = found->value;
    return 1;
}

static int xml_import_special_parse_attribute(xml_data_attribute_t *attr, int *target)
{
    switch (attr->type) {
        case PARAMETER_TYPE_INVASION_TYPE:
        case PARAMETER_TYPE_BOOLEAN:
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_ALLOWED_BUILDING:
        case PARAMETER_TYPE_CHECK:
        case PARAMETER_TYPE_DIFFICULTY:
        case PARAMETER_TYPE_ENEMY_TYPE:
        case PARAMETER_TYPE_POP_CLASS:
        case PARAMETER_TYPE_RATING_TYPE:
        case PARAMETER_TYPE_STANDARD_MESSAGE:
        case PARAMETER_TYPE_STORAGE_TYPE:
        case PARAMETER_TYPE_TARGET_TYPE:
        case PARAMETER_TYPE_GOD:
        case PARAMETER_TYPE_CLIMATE:
            return xml_import_special_parse_type(attr, attr->type, target);
        case PARAMETER_TYPE_BUILDING_COUNTING:
            return xml_import_special_parse_building_counting(attr, target);
        case PARAMETER_TYPE_FUTURE_CITY:
            return xml_import_special_parse_future_city(attr, target);
        case PARAMETER_TYPE_REQUEST:
        case PARAMETER_TYPE_NUMBER:
            return xml_import_special_parse_limited_number(attr, target);
        case PARAMETER_TYPE_MIN_MAX_NUMBER:
            return xml_import_special_parse_min_max_number(attr, target);
        case PARAMETER_TYPE_RESOURCE:
            return xml_import_special_parse_resource(attr, target);
        case PARAMETER_TYPE_ROUTE:
            return xml_import_special_parse_route(attr, target);
        case PARAMETER_TYPE_CUSTOM_MESSAGE:
            return xml_import_special_parse_custom_message(attr, target);
        case PARAMETER_TYPE_CUSTOM_VARIABLE:
            return xml_import_special_parse_custom_variable(attr, target);
        case PARAMETER_TYPE_UNDEFINED:
            return 1;
        default:
            xml_import_log_error("Something is very wrong. Failed to find attribute type.");
            return 0;
    }
}

static int xml_import_special_parse_building_counting(xml_data_attribute_t *attr, int *target)
{
    if (!attr->name) {
        return 0;
    }

    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    special_attribute_mapping_t *found = scenario_events_parameter_data_get_attribute_mapping_by_text(PARAMETER_TYPE_BUILDING, value);
    if (found == 0) {
        xml_import_log_error("Could not find the given building type for counting.");
        return 0;
    }

    switch(found->value) {
        case BUILDING_CLEAR_LAND:
        case BUILDING_DISTRIBUTION_CENTER_UNUSED:
        case BUILDING_BURNING_RUIN:
            xml_import_log_error("I cannot count that.");
            return 0;
        default:
            break;
    }
    
    *target = found->value;
    return 1;
}

static int xml_import_special_parse_future_city(xml_data_attribute_t *attr, int *target)
{
    if (!attr->name) {
        return 0;
    }

    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    const uint8_t *converted_name = string_from_ascii(value);
    int city_id = empire_city_get_id_by_name(converted_name);
    empire_city *city = empire_city_get(city_id);
    if (city) {
        if (city->type == EMPIRE_CITY_FUTURE_TRADE) {
            *target = city_id;
            return 1;
        } else {
            xml_import_log_error("Can only target cities with the future_trade type");
            return 0;
        }
    } else {
        xml_import_log_error("Could not find city");
        return 0;
    }
}

static int xml_import_special_parse_resource(xml_data_attribute_t *attr, int *target)
{
    if (!attr->name) {
        return 0;
    }

    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }
    
    const char *value = xml_parser_get_attribute_string(attr->name);
    for (resource_type i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        const char *resource_name = resource_get_data(i)->xml_attr_name;
        if (xml_parser_compare_multiple(resource_name, value)) {
            *target = (int)i;
            return 1;
        }
    }

    xml_import_log_error("Could not find a matching resource.");
    return 0;
}

static int xml_import_special_parse_route(xml_data_attribute_t *attr, int *target)
{
    if (!attr->name) {
        return 0;
    }

    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    const uint8_t *converted_name = string_from_ascii(value);
    int city_id = empire_city_get_id_by_name(converted_name);
    empire_city *city = empire_city_get(city_id);
    if (city) {
        *target = city->route_id;
        return 1;
    } else {
        xml_import_log_error("Could not find city");
        return 0;
    }
}

static int xml_import_special_parse_limited_number(xml_data_attribute_t *attr, int *target)
{
    if (!attr->name) {
        return 0;
    }
    
    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    int param_value = xml_parser_get_attribute_int(attr->name);

    if (param_value < attr->min_limit) {
        xml_import_log_error("Number too small.");
        return 0;
    }

    if (param_value > attr->max_limit) {
        xml_import_log_error("Number too big.");
        return 0;
    }

    *target = param_value;
    return 1;
}

static int xml_import_special_parse_min_max_number(xml_data_attribute_t *attr, int *target)
{
    int has_attr = xml_parser_has_attribute("amount");
    if (has_attr) {
        int param_value = xml_parser_get_attribute_int("amount");
        if (param_value >= attr->min_limit && param_value <= attr->max_limit) {
            *target = param_value;
            return 1;
        }
    }

    return xml_import_special_parse_limited_number(attr, target);
}

static int xml_import_special_parse_custom_message(xml_data_attribute_t *attr, int *target)
{
    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    const uint8_t *converted_name = string_from_ascii(value);
    int message_id = custom_messages_get_id_by_uid(converted_name);

    if (message_id) {
        *target = message_id;
        return 1;
    } else {
        xml_import_log_error("Could not find custom message. Setup custom messages first.");
        return 0;
    }
}

static int xml_import_special_parse_custom_variable(xml_data_attribute_t *attr, int *target)
{
    int has_attr = xml_parser_has_attribute(attr->name);
    if (!has_attr) {
        xml_import_log_error("Missing attribute.");
        return 0;
    }

    const char *value = xml_parser_get_attribute_string(attr->name);
    const uint8_t *converted_name = string_from_ascii(value);
    int variable_id = scenario_custom_variable_get_id_by_name(converted_name);

    if (variable_id) {
        *target = variable_id;
        return 1;
    } else {
        xml_import_log_error("Could not find custom variable. Setup custom variables first via import or editor.");
        return 0;
    }
}

static void reset_data(void)
{
    data.success = 1;
    data.version = -1;
    data.error_line_number = -1;
}

static int parse_xml(char *buf, int buffer_length)
{
    reset_data();
    scenario_events_clear();
    data.success = 1;
    if (!xml_parser_init(xml_elements, XML_TOTAL_ELEMENTS, 1)) {
        data.success = 0;
    }
    if (data.success) {
        if (!xml_parser_parse(buf, buffer_length, 1)) {
            data.success = 0;
            scenario_events_clear();
        }
    }
    xml_parser_free();

    return data.success;
}

static char *file_to_buffer(const char *filename, int *output_length)
{
    FILE *file = file_open(filename, "r");
    if (!file) {
        log_error("Error opening empire file", filename, 0);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *buf = malloc(size);
    if (!buf) {
        log_error("Error opening empire file", filename, 0);
        file_close(file);
        return 0;
    }
    memset(buf, 0, size);
    if (!buf) {
        log_error("Unable to allocate buffer to read XML file", filename, 0);
        free(buf);
        file_close(file);
        return 0;
    }
    *output_length = (int) fread(buf, 1, size, file);
    if (*output_length > size) {
        log_error("Unable to read file into buffer", filename, 0);
        free(buf);
        file_close(file);
        *output_length = 0;
        return 0;
    }
    file_close(file);
    return buf;
}

int scenario_events_xml_parse_file(const char *filename)
{
    int output_length = 0;
    char *xml_contents = file_to_buffer(filename, &output_length);
    if (!xml_contents) {
        return 0;
    }
    int success = parse_xml(xml_contents, output_length);
    free(xml_contents);
    if (!success) {
        log_error("Error parsing file", filename, 0);
        scenario_events_clear();
    }
    return success;
}
