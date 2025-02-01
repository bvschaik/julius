#include "event.h"

#include "core/encoding.h"
#include "core/log.h"
#include "core/random.h"
#include "scenario/event/action_handler.h"
#include "scenario/event/condition_handler.h"

#define SCENARIO_ACTIONS_ARRAY_SIZE_STEP 20
#define SCENARIO_CONDITIONS_ARRAY_SIZE_STEP 20
#define SCENARIO_CONDITION_GROUPS_ARRAY_SIZE_STEP 2

static int action_in_use(const scenario_action_t *action)
{
    return action->type != ACTION_TYPE_UNDEFINED;
}

void scenario_event_new(scenario_event_t *event, unsigned int position)
{
    event->id = position;
    if (!array_init(event->actions, SCENARIO_ACTIONS_ARRAY_SIZE_STEP, 0, action_in_use) ||
        !array_init(event->condition_groups, SCENARIO_CONDITION_GROUPS_ARRAY_SIZE_STEP,
            scenario_condition_group_new, scenario_condition_group_in_use)) {
        log_error("Unable to allocate enough memory for the scenario event. The game will now crash.", 0, 0);
    }
}

int scenario_event_is_active(const scenario_event_t *event)
{
    return event->state != EVENT_STATE_UNDEFINED;
}

void scenario_event_init(scenario_event_t *event)
{
    event->state = EVENT_STATE_ACTIVE;
    scenario_condition_group_t *group;
    scenario_condition_t *condition;
    array_foreach(event->condition_groups, group) {
        array_foreach(group->conditions, condition) {
            scenario_condition_type_init(condition);
        }
    }
    scenario_action_t *action;
    array_foreach(event->actions, action) {
       scenario_action_type_init(action);
    }
}

void scenario_event_save_state(buffer *buf, scenario_event_t *event)
{
    if (event->state == EVENT_STATE_UNDEFINED) {
        event->state = EVENT_STATE_DELETED; // We need a different state than undefined to avoid array overrides on load which breaks the linking by id.
    }

    buffer_write_i32(buf, event->id);
    buffer_write_i16(buf, event->state);
    buffer_write_i32(buf, event->repeat_months_min);
    buffer_write_i32(buf, event->repeat_months_max);
    buffer_write_i32(buf, event->months_until_active);
    buffer_write_i32(buf, event->max_number_of_repeats);
    buffer_write_i32(buf, event->execution_count);
    buffer_write_u16(buf, event->actions.size);
    buffer_write_u16(buf, event->condition_groups.size);
    char name_utf8[EVENT_NAME_LENGTH * 2] = { 0 };
    encoding_to_utf8(event->name, name_utf8, EVENT_NAME_LENGTH * 2, 0);
    buffer_write_raw(buf, name_utf8, EVENT_NAME_LENGTH * 2);
}

void scenario_event_load_state(buffer *buf, scenario_event_t *event, int is_new_version)
{
    int saved_id = buffer_read_i32(buf);
    event->state = buffer_read_i16(buf);
    event->repeat_months_min = buffer_read_i32(buf);
    event->repeat_months_max = buffer_read_i32(buf);
    event->months_until_active = buffer_read_i32(buf);
    event->max_number_of_repeats = buffer_read_i32(buf);
    event->execution_count = buffer_read_i32(buf);
    if (!is_new_version) {
       buffer_skip(buf, 2);
    }
    unsigned int actions_count = buffer_read_u16(buf);
    unsigned int condition_groups_count = 1;
    if (is_new_version) {
        condition_groups_count = buffer_read_u16(buf);
        char name_utf8[EVENT_NAME_LENGTH * 2];
        buffer_read_raw(buf, name_utf8, EVENT_NAME_LENGTH * 2);
        encoding_from_utf8(name_utf8, event->name, EVENT_NAME_LENGTH);
    }

    if (!array_init(event->actions, SCENARIO_ACTIONS_ARRAY_SIZE_STEP, 0, action_in_use) ||
        !array_expand(event->actions, actions_count)) {
        log_error("Unable to create actions array. The game will now crash.", 0, 0);
    }
    if (!array_init(event->condition_groups, SCENARIO_CONDITION_GROUPS_ARRAY_SIZE_STEP,
        scenario_condition_group_new, scenario_condition_group_in_use) ||
        !array_expand(event->condition_groups, condition_groups_count)) {
        log_error("Unable to create condition groups array. The game will now crash.", 0, 0);
    }
    // Add the condition group
    if (!is_new_version) {
        array_advance(event->condition_groups);
    }
    if (event->id != saved_id) {
        log_error("Loaded event id does not match what it was saved with. The game will likely crash. event->id: ",
            0, event->id);
    }
}

scenario_condition_t *scenario_event_condition_create(scenario_condition_group_t *group, int type)
{
    scenario_condition_t *condition;
    array_new_item(group->conditions, condition);
    if (!condition) {
        return 0;
    }
    condition->type = type;

    return condition;
}

void scenario_event_link_condition_group(scenario_event_t *event, scenario_condition_group_t *group)
{
    scenario_condition_group_t *new_group = 0;
    array_new_item(event->condition_groups, new_group);
    new_group->conditions = group->conditions;
}

scenario_action_t *scenario_event_action_create(scenario_event_t *event, int type)
{
    scenario_action_t *action = 0;
    array_new_item(event->actions, action);
    if (!action) {
        return 0;
    }
    action->type = type;

    return action;
}

void scenario_event_link_action(scenario_event_t *event, scenario_action_t *action)
{
    scenario_action_t *new_action = 0;
    array_new_item(event->actions, new_action);

    new_action->type = action->type;
    new_action->parameter1 = action->parameter1;
    new_action->parameter2 = action->parameter2;
    new_action->parameter3 = action->parameter3;
    new_action->parameter4 = action->parameter4;
    new_action->parameter5 = action->parameter5;
}

int scenario_event_can_repeat(scenario_event_t *event)
{
    return (event->repeat_months_min > 0) && (event->repeat_months_max >= event->repeat_months_min) &&
        ((event->execution_count < event->max_number_of_repeats) || (event->max_number_of_repeats <= 0));
}

static int conditions_fulfilled(scenario_event_t *event)
{
    if (event->state != EVENT_STATE_ACTIVE) {
        return 0;
    }
    if (event->actions.size == 0) {
        return 0;
    }
    
    scenario_condition_group_t *group;
    array_foreach(event->condition_groups, group) {
        int group_fulfilled = 0;
        for (unsigned int i = 0; i < group->conditions.size; i++) {
            scenario_condition_t *condition = array_item(group->conditions, i);
            if (group->type == FULFILLMENT_TYPE_ALL && !scenario_condition_type_is_met(condition)) {
                return 0;
            }
            if (group->type == FULFILLMENT_TYPE_ANY && scenario_condition_type_is_met(condition)) {
                group_fulfilled = 1;
                break;
            }
        }
        if (group->type == FULFILLMENT_TYPE_ANY && group->conditions.size > 0 && !group_fulfilled) {
            return 0;
        }
    }

    return 1;
}

int scenario_event_decrease_pause_time(scenario_event_t *event, int months_passed)
{
    if (event->state != EVENT_STATE_PAUSED) {
        return 0;
    }

    if (event->months_until_active > 0) {
        event->months_until_active -= months_passed;
    }
    if (event->months_until_active < 0) {
        event->months_until_active = 0;
    }
    if (event->months_until_active == 0) {
        event->state = EVENT_STATE_ACTIVE;
    }
    return 1;
}

int scenario_event_count_conditions(const scenario_event_t *event)
{
    int total_conditions = 0;
    const scenario_condition_group_t *group;
    array_foreach(event->condition_groups, group) {
        total_conditions += group->conditions.size;
    }
    return total_conditions;
}

int scenario_event_conditional_execute(scenario_event_t *event)
{
    if (conditions_fulfilled(event)) {
        int result = scenario_event_execute(event);
        event->execution_count++;
        if (scenario_event_can_repeat(event)) {
            scenario_event_init(event);
            event->state = EVENT_STATE_PAUSED;
            event->months_until_active = random_between_from_stdlib(event->repeat_months_min, event->repeat_months_max);
        } else {
            event->state = EVENT_STATE_DISABLED;
        }
        return result;
    }
    return 0;
}

int scenario_event_execute(scenario_event_t *event)
{
    int actioned = 1;

    scenario_action_t *current;
    array_foreach(event->actions, current) {
        int action_result = scenario_action_type_execute(current);
        actioned &= action_result;
    }

    return actioned;
}

int scenario_event_uses_custom_variable(const scenario_event_t *event, int custom_variable_id)
{
    scenario_condition_group_t *group;
    scenario_condition_t *condition;
    array_foreach(event->condition_groups, group) {
        array_foreach(group->conditions, condition) {
            if (scenario_condition_uses_custom_variable(condition, custom_variable_id)) {
                return 1;
            }
        }
    }

    scenario_action_t *action;
    array_foreach(event->actions, action) {
        if (scenario_action_uses_custom_variable(action, custom_variable_id)) {
            return 1;
        }
    }

    return 0;
}
