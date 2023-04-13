#include "scenario_event.h"

#include "core/log.h"
#include "core/random.h"
#include "scenario/action_types/action_handler.h"
#include "scenario/condition_types/condition_handler.h"

static int condition_in_use(const scenario_condition_t *condition)
{
    return condition->type != CONDITION_TYPE_UNDEFINED;
}

static int action_in_use(const scenario_action_t *action)
{
    return action->type != ACTION_TYPE_UNDEFINED;
}

void scenario_event_init(scenario_event_t *event)
{
    event->state = EVENT_STATE_ACTIVE;
    scenario_condition_t *condition;
    array_foreach(event->conditions, condition) {
        scenario_condition_type_init(condition);
    }
    scenario_action_t *action;
    array_foreach(event->actions, action) {
       scenario_action_type_init(action);
    }
}

void scenario_event_save_state(buffer *buf, scenario_event_t *event)
{
    buffer_write_i32(buf, event->id);
    buffer_write_i16(buf, event->state);
    buffer_write_i32(buf, event->repeat_months_min);
    buffer_write_i32(buf, event->repeat_months_max);
    buffer_write_i32(buf, event->months_until_active);
    buffer_write_i32(buf, event->max_number_of_repeats);
    buffer_write_i32(buf, event->execution_count);
    buffer_write_i16(buf, event->conditions.size);
    buffer_write_i16(buf, event->actions.size);
}

void scenario_event_load_state(buffer *buf, scenario_event_t *event)
{
    buffer_read_i32(buf);
    event->state = buffer_read_i16(buf);
    event->repeat_months_min = buffer_read_i32(buf);
    event->repeat_months_max = buffer_read_i32(buf);
    event->months_until_active = buffer_read_i32(buf);
    event->max_number_of_repeats = buffer_read_i32(buf);
    event->execution_count = buffer_read_i32(buf);
    int conditions_count = buffer_read_i16(buf);
    int actions_count = buffer_read_i16(buf);

    if (!array_init(event->conditions, SCENARIO_CONDITIONS_ARRAY_SIZE_STEP, 0, condition_in_use) ||
        !array_expand(event->conditions, conditions_count)) {
        log_error("Unable to create conditions array. The game will now crash.", 0, 0);
    }
    if (!array_init(event->actions, SCENARIO_ACTIONS_ARRAY_SIZE_STEP, 0, action_in_use) ||
        !array_expand(event->actions, actions_count)) {
        log_error("Unable to create actions array. The game will now crash.", 0, 0);
    }
}

scenario_condition_t *scenario_event_get_condition(scenario_event_t *event, int id)
{
    return array_item(event->conditions, id);
}

scenario_condition_t *scenario_event_condition_create(scenario_event_t *event, int type)
{
    scenario_condition_t *condition = 0;
    array_new_item(event->conditions, 0, condition);
    if (!condition) {
        return 0;
    }
    condition->type = type;

    return condition;
}

void scenario_event_link_condition(scenario_event_t *event, scenario_condition_t *condition)
{
    scenario_condition_t *new_condition = 0;
    array_new_item(event->conditions, 0, new_condition);
    new_condition->type = condition->type;
    new_condition->parameter1 = condition->parameter1;
    new_condition->parameter2 = condition->parameter2;
    new_condition->parameter3 = condition->parameter3;
    new_condition->parameter4 = condition->parameter4;
    new_condition->parameter5 = condition->parameter5;
}

scenario_action_t *scenario_event_get_action(scenario_event_t *event, int id)
{
    return array_item(event->actions, id);
}

scenario_action_t *scenario_event_action_create(scenario_event_t *event, int type)
{
    scenario_action_t *action = 0;
    array_new_item(event->actions, 0, action);
    if (!action) {
        return 0;
    }
    action->type = type;

    return action;
}

void scenario_event_link_action(scenario_event_t *event, scenario_action_t *action)
{
    scenario_action_t *new_action = 0;
    array_new_item(event->actions, 0, new_action);

    new_action->type = action->type;
    new_action->parameter1 = action->parameter1;
    new_action->parameter2 = action->parameter2;
    new_action->parameter3 = action->parameter3;
    new_action->parameter4 = action->parameter4;
    new_action->parameter5 = action->parameter5;
}

void scenario_event_initialize_new(scenario_event_t *event, int position)
{
    event->id = position;
}

int scenario_event_is_valid(const scenario_event_t *event)
{
    return event->state != EVENT_STATE_UNDEFINED;
}


int scenario_event_can_repeat(scenario_event_t *event)
{
    return (event->repeat_months_min > 0) && (event->repeat_months_max >= event->repeat_months_min) &&
        ((event->execution_count < event->max_number_of_repeats) || (event->max_number_of_repeats <= 0));
}

int scenario_event_all_conditions_met(scenario_event_t *event)
{
    if (event->state != EVENT_STATE_ACTIVE) {
        return 0;
    }
    if (event->actions.size < 1) {
        return 0;
    }
    
    scenario_condition_t *current;
    array_foreach(event->conditions, current) {
        if (scenario_condition_type_is_met(current) == 0) {
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

int scenario_event_conditional_execute(scenario_event_t *event)
{
    if (scenario_event_all_conditions_met(event)) {
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
