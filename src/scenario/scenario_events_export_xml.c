#include "scenario_events_export_xml.h"

#include "core/buffer.h"
#include "core/io.h"
#include "core/log.h"
#include "core/string.h"
#include "core/xml_exporter.h"
#include "empire/city.h"
#include "scenario/custom_messages.h"
#include "scenario/scenario_events_controller.h"
#include "scenario/scenario_events_parameter_data.h"
#include "window/plain_message_dialog.h"

#include <string.h>

#define XML_EXPORT_MAX_SIZE 5000000

static struct {
    int success;
    char error_message[200];
} data;

static void log_exporting_error(const char *msg)
{
    data.success = 0;
    strcpy(data.error_message, msg);
    log_error("Error while exporting scenario events to XML. ", data.error_message, 0);

    window_plain_message_dialog_show_with_extra(
        TR_EDITOR_UNABLE_TO_SAVE_EVENTS_TITLE, TR_EDITOR_CHECK_LOG_MESSAGE,
        string_from_ascii(data.error_message),
        0);
}

static int export_attribute_by_type(xml_data_attribute_t *attr, parameter_type type, int target)
{
    special_attribute_mapping_t *found = scenario_events_parameter_data_get_attribute_mapping_by_value(type, target);
    if (found != 0) {
        xml_exporter_add_attribute_text(attr->name, string_from_ascii(found->text));
        return 1;
    }
    return 0;
}

static int export_attribute_future_city(xml_data_attribute_t *attr, int target)
{
    empire_city *city = empire_city_get(target);
    if (city) {
        const uint8_t *city_name = empire_city_get_name(city);
        xml_exporter_add_attribute_text(attr->name, city_name);
        return 1;
    }
    return 0;
}

static int export_attribute_number(xml_data_attribute_t *attr, int target)
{
    xml_exporter_add_attribute_int(attr->name, target);
    return 1;
}

static int export_attribute_route(xml_data_attribute_t *attr, int target)
{
    int city_id = empire_city_get_for_trade_route(target);
    if (city_id) {
        empire_city *city = empire_city_get(city_id);
        const uint8_t *city_name = empire_city_get_name(city);
        xml_exporter_add_attribute_text(attr->name, city_name);
        return 1;
    }
    return 0;
}

static int export_attribute_resource(xml_data_attribute_t *attr, int target)
{
    const char *resource_name = resource_get_data(target)->xml_attr_name;
    char resource_name_to_use[50] = " ";

    const char *next = strchr(resource_name, '|');
    size_t length = next ? (next - resource_name) : strlen(resource_name);
    if (length > 48) {
        length = 48;
    }
    strncpy(resource_name_to_use, resource_name, length);

    xml_exporter_add_attribute_text(attr->name, string_from_ascii(resource_name_to_use));
    return 1;
}

static int export_attribute_custom_message(xml_data_attribute_t *attr, int target)
{
    custom_message_t *message = custom_messages_get(target);
    if (message) {
        const uint8_t *message_uid = message->linked_uid->text;
        xml_exporter_add_attribute_text(attr->name, message_uid);
        return 1;
    }
    return 0;
}

static int export_parse_attribute(xml_data_attribute_t *attr, int target)
{
    switch (attr->type) {
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            return export_attribute_by_type(attr, PARAMETER_TYPE_ALLOWED_BUILDING, target);
        case PARAMETER_TYPE_BOOLEAN:
            return export_attribute_by_type(attr, PARAMETER_TYPE_BOOLEAN, target);
        case PARAMETER_TYPE_BUILDING:
            return export_attribute_by_type(attr, PARAMETER_TYPE_BUILDING, target);
        case PARAMETER_TYPE_BUILDING_COUNTING:
            return export_attribute_by_type(attr, PARAMETER_TYPE_BUILDING, target);
        case PARAMETER_TYPE_CHECK:
            return export_attribute_by_type(attr, PARAMETER_TYPE_CHECK, target);
        case PARAMETER_TYPE_DIFFICULTY:
            return export_attribute_by_type(attr, PARAMETER_TYPE_DIFFICULTY, target);
        case PARAMETER_TYPE_FUTURE_CITY:
            return export_attribute_future_city(attr, target);
        case PARAMETER_TYPE_MIN_MAX_NUMBER:
            return export_attribute_number(attr, target);
        case PARAMETER_TYPE_NUMBER:
            return export_attribute_number(attr, target);
        case PARAMETER_TYPE_POP_CLASS:
            return export_attribute_by_type(attr, PARAMETER_TYPE_POP_CLASS, target);
        case PARAMETER_TYPE_RESOURCE:
            return export_attribute_resource(attr, target);
        case PARAMETER_TYPE_ROUTE:
            return export_attribute_route(attr, target);
        case PARAMETER_TYPE_STANDARD_MESSAGE:
            return export_attribute_by_type(attr, PARAMETER_TYPE_STANDARD_MESSAGE, target);
        case PARAMETER_TYPE_CUSTOM_MESSAGE:
            return export_attribute_custom_message(attr, target);
        case PARAMETER_TYPE_UNDEFINED:
            return 1;
        default:
            log_exporting_error("Something is very wrong. Failed to find attribute type.");
            return 0;
    }
}

static void export_event_condition(scenario_condition_t *condition)
{
    if (condition->type == CONDITION_TYPE_UNDEFINED) {
        return;
    }

    scenario_condition_data_t *data = scenario_events_parameter_data_get_conditions_xml_attributes(condition->type);
    if (data->type == CONDITION_TYPE_UNDEFINED) {
        return;
    }

    if (data->xml_attr.name) {
        xml_exporter_new_element(data->xml_attr.name, 1);
    } else {
        log_exporting_error("Error while exporting condition.");
        return;
    }

    export_parse_attribute(&data->xml_parm1, condition->parameter1);
    export_parse_attribute(&data->xml_parm2, condition->parameter2);
    export_parse_attribute(&data->xml_parm3, condition->parameter3);
    export_parse_attribute(&data->xml_parm4, condition->parameter4);
    export_parse_attribute(&data->xml_parm5, condition->parameter5);

    xml_exporter_close_element(0);
}

static void export_event_action(scenario_action_t *action)
{
    if (action->type == ACTION_TYPE_UNDEFINED) {
        return;
    }

    scenario_action_data_t *data = scenario_events_parameter_data_get_actions_xml_attributes(action->type);
    if (data->type == ACTION_TYPE_UNDEFINED) {
        return;
    }

    if (data->xml_attr.name) {
        xml_exporter_new_element(data->xml_attr.name, 1);
    } else {
        log_exporting_error("Error while exporting action.");
        return;
    }

    export_parse_attribute(&data->xml_parm1, action->parameter1);
    export_parse_attribute(&data->xml_parm2, action->parameter2);
    export_parse_attribute(&data->xml_parm3, action->parameter3);
    export_parse_attribute(&data->xml_parm4, action->parameter4);
    export_parse_attribute(&data->xml_parm5, action->parameter5);

    xml_exporter_close_element(0);
}

static int export_event(scenario_event_t *event)
{
    if (event->state == EVENT_STATE_UNDEFINED) {
        return 1;
    }

    xml_exporter_new_element("event", 1);

    if (event->repeat_months_min > 0) {
        xml_exporter_add_attribute_int("repeat_months_min", event->repeat_months_min);
    }
    if (event->repeat_months_max > 0) {
        xml_exporter_add_attribute_int("repeat_months_max", event->repeat_months_min);
    }
    if (event->max_number_of_repeats > 0) {
        xml_exporter_add_attribute_int("max_number_of_repeats", event->repeat_months_min);
    }

    xml_exporter_new_element("conditions", 1);
    for (int i = 0; i < event->conditions.size; i++) {
        scenario_condition_t *condition = array_item(event->conditions, i);
        export_event_condition(condition);
    }
    xml_exporter_close_element(0);

    xml_exporter_new_element("actions", 1);
    for (int i = 0; i < event->actions.size; i++) {
        scenario_action_t *action = array_item(event->actions, i);
        export_event_action(action);
    }
    xml_exporter_close_element(0);

    xml_exporter_close_element(0);
    
    return 1;
}

static void export_scenario_events(buffer *buf)
{
    xml_exporter_init(buf, "events");
    xml_exporter_new_element("events", 0);
    xml_exporter_add_attribute_int("version", SCENARIO_EVENTS_XML_VERSION);

    int event_count = scenario_events_get_count();
    for (int i = 0; i < event_count; i++) {
        scenario_event_t *event = scenario_event_get(i);
        export_event(event);
    }
    xml_exporter_close_element(0);
    xml_exporter_newline();
}

int scenario_events_export_to_xml(const char *filename)
{
    buffer buf;
    int buf_size = XML_EXPORT_MAX_SIZE;
    uint8_t *buf_data = malloc(buf_size);
    if (!buf_data) {
        log_error("Unable to allocate buffer to export scenario events XML", 0, 0);
        free(buf_data);
        return 0;
    }
    buffer_init(&buf, buf_data, buf_size);
    export_scenario_events(&buf);
    io_write_buffer_to_file(filename, buf.data, buf.index);
    free(buf_data);
    return 1;
}
