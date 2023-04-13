#include "scenario_events_controller.h"

#include "core/log.h"
#include "game/save_version.h"
#include "scenario/action_types/action_handler.h"
#include "scenario/condition_types/condition_handler.h"
#include "scenario/scenario.h"
#include "scenario/scenario_event.h"

#define SCENARIO_EVENTS_SIZE_STEP 50

static array(scenario_event_t) scenario_events;

static int event_in_use(const scenario_event_t *event)
{
    return event->state != EVENT_STATE_UNDEFINED;
}

void scenario_events_init(void)
{
    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        scenario_event_init(current);
    }
}

void new_scenario_event(scenario_event_t *obj, int position)
{
    obj->id = position;
}

void scenario_events_clear(void)
{
    scenario_events.size = 0;
    if (!array_init(scenario_events, SCENARIO_EVENTS_SIZE_STEP, new_scenario_event, event_in_use)) {
        log_error("Unable to allocate enough memory for the scenario events array. The game will now crash.", 0, 0);
    }
}

scenario_event_t *scenario_event_get(int event_id)
{
    return array_item(scenario_events, event_id);
}

scenario_event_t *scenario_event_create(int repeat_min, int repeat_max, int max_repeats)
{
    if (repeat_min < 0) {
        log_error("Event minimum repeat is less than 0.", 0, 0);
        return 0;
    }
    if (repeat_max < 0) {
        log_error("Event maximum repeat is less than 0.", 0, 0);
        return 0;
    }

    if (repeat_max < repeat_min) {
        log_info("Event maximum repeat is less than its minimum. Swapping the two values.", 0, 0);
        int temp = repeat_min;
        repeat_min = repeat_max;
        repeat_max = temp;
    }

    scenario_event_t *event = 0;
    array_new_item(scenario_events, 0, event);
    if (!event) {
        return 0;
    }
    event->state = EVENT_STATE_ACTIVE;
    event->repeat_months_min = repeat_min;
    event->repeat_months_max = repeat_max;
    event->max_number_of_repeats = max_repeats;

    return event;
}

void scenario_event_delete(scenario_event_t *event)
{
    memset(event, 0, sizeof(scenario_event_t));
    event->state = EVENT_STATE_UNDEFINED;
    array_trim(scenario_events);
}

int scenario_events_get_count(void)
{
    return scenario_events.size;
}

static void info_save_state(buffer *buf)
{
    int32_t array_size = scenario_events.size;
    int32_t struct_size = (6 * sizeof(int32_t)) + (3 * sizeof(int16_t));
    buffer_init_dynamic_piece(buf,
        SCENARIO_EVENTS_VERSION,
        array_size,
        struct_size);

    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        scenario_event_save_state(buf, current);
    }
}

static void conditions_save_state(buffer *buf)
{
    int32_t array_size = 0;
    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        array_size += current->conditions.size;
    }
    // If in future conditions can be linked to more things, then also take them into account here.

    int32_t struct_size = (2 * sizeof(int16_t)) + (6 * sizeof(int32_t));
    buffer_init_dynamic_piece(buf,
        SCENARIO_EVENTS_VERSION,
        array_size,
        struct_size);

    for (int i = 0; i < scenario_events.size; i++) {
        scenario_event_t *current_event = array_item(scenario_events, i);

        for (int j = 0; j < current_event->conditions.size; j++) {
            scenario_condition_t *current = array_item(current_event->conditions, j);
            scenario_condition_type_save_state(buf, current, LINK_TYPE_SCENARIO_EVENT, current_event->id);
        }
    }
}

static void actions_save_state(buffer *buf)
{
    int32_t array_size = 0;
    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        array_size += current->actions.size;
    }
    // If in future actions can be linked to more things, then also take them into account here.

    int32_t struct_size = (2 * sizeof(int16_t)) + (6 * sizeof(int32_t));
    buffer_init_dynamic_piece(buf,
        SCENARIO_EVENTS_VERSION,
        array_size,
        struct_size);

    for (int i = 0; i < scenario_events.size; i++) {
        scenario_event_t *current_event = array_item(scenario_events, i);

        for (int j = 0; j < current_event->actions.size; j++) {
            scenario_action_t *current = array_item(current_event->actions, j);
            scenario_action_type_save_state(buf, current, LINK_TYPE_SCENARIO_EVENT, current_event->id);
        }
    }
}

void scenario_events_save_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions)
{
    info_save_state(buf_events);
    conditions_save_state(buf_conditions);
    actions_save_state(buf_actions);
}

static void load_link_condition(scenario_condition_t *condition, int link_type, int32_t link_id)
{
    switch (link_type) {
        case LINK_TYPE_SCENARIO_EVENT:
            {
                scenario_event_t *event = scenario_event_get(link_id);
                scenario_event_link_condition(event, condition);
            }
            break;
        default:
            log_error("Unhandled condition link type. The game will probably crash.", 0, 0);
            break;
    }
}

static void info_load_state(buffer *buf)
{
    int buffer_size, version, array_size, struct_size;
    buffer_load_dynamic_piece_header_data(buf,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    for (int i = 0; i < array_size; i++) {
        scenario_event_t *event = scenario_event_create(0, 0, 0);
        scenario_event_load_state(buf, event);
    }
}

static void conditions_load_state(buffer *buf)
{
    int buffer_size, version, array_size, struct_size;
    buffer_load_dynamic_piece_header_data(buf,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    int link_type = 0;
    int32_t link_id = 0;
    for (int i = 0; i < array_size; i++) {
        scenario_condition_t condition;
        scenario_condition_type_load_state(buf, &condition, &link_type, &link_id);
        load_link_condition(&condition, link_type, link_id);
    }
}

static void load_link_action(scenario_action_t *action, int link_type, int32_t link_id)
{
    switch (link_type) {
        case LINK_TYPE_SCENARIO_EVENT:
            {
                scenario_event_t *event = scenario_event_get(link_id);
                scenario_event_link_action(event, action);
            }
            break;
        default:
            log_error("Unhandled action link type. The game will probably crash.", 0, 0);
            break;
    }
}

static void actions_load_state(buffer *buf)
{
    int buffer_size, version, array_size, struct_size;
    buffer_load_dynamic_piece_header_data(buf,
        &buffer_size,
        &version,
        &array_size,
        &struct_size);

    int link_type = 0;
    int32_t link_id = 0;
    for (int i = 0; i < array_size; i++) {
        scenario_action_t action;
        scenario_action_type_load_state(buf, &action, &link_type, &link_id);
        load_link_action(&action, link_type, link_id);
    }
}

void scenario_events_load_state(buffer *buf_events, buffer *buf_conditions, buffer *buf_actions)
{
    scenario_events_clear();
    info_load_state(buf_events);
    conditions_load_state(buf_conditions);
    actions_load_state(buf_actions);
}

void scenario_events_process_all(void)
{
    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        scenario_event_conditional_execute(current);
    }
}

void scenario_events_progress_paused(int months_passed)
{
    scenario_event_t *current;
    array_foreach(scenario_events, current) {
        scenario_event_decrease_pause_time(current, months_passed);
    }
}
