#include "input/touch.h"

#include "input/mouse.h"

#include <stdlib.h>

#define CLICK_TIME 300
#define NOT_MOVING_RANGE 5
#define SCROLL_FINGER_RADIUS 25
#define MILLIS_TOLERANCE_BETWEEN_LAST_MOVE_AND_TOUCH_END 60

static touch touch_data[MAX_ACTIVE_TOUCHES + 1];
static touch old_touch;
static int last_scroll_position;

static int start_delayed(const touch *t)
{
    return (t->has_started && !t->has_moved && !t->has_ended && ((time_get_millis() - t->start_time) < (time_millis) (CLICK_TIME / 2)));
}

const touch *get_earliest_touch(void)
{
    time_millis timestamp = -1;
    int touch_index = MAX_ACTIVE_TOUCHES;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (touch_data[i].in_use && !start_delayed(&touch_data[i]) && touch_data[i].start_time < timestamp) {
            timestamp = touch_data[i].start_time;
            touch_index = i;
        }
    }
    return &touch_data[touch_index];
}

const touch *get_latest_touch(void)
{
    int active_touches = 0;
    time_millis timestamp = 0;
    int touch_index = MAX_ACTIVE_TOUCHES;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (touch_data[i].in_use && !start_delayed(&touch_data[i])) {
            ++active_touches;
            if (touch_data[i].start_time > timestamp) {
                timestamp = touch_data[i].start_time;
                touch_index = i;
            }
        }
    }
    return (active_touches > 1) ? &touch_data[touch_index] : &touch_data[MAX_ACTIVE_TOUCHES];
}

int get_total_active_touches(void)
{
    int active_touches = 0;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (touch_data[i].in_use) {
            ++active_touches;
        }
    }
    return active_touches;
}

int touch_not_click(const touch *t)
{
    return (t->has_moved || (!t->has_ended && (time_get_millis() - t->start_time) >= CLICK_TIME) ||
        (t->has_ended && (t->last_change_time - t->start_time) >= CLICK_TIME));
}

int touch_was_click(const touch *t)
{
    return (t->has_ended && !t->has_moved && (t->last_change_time - t->start_time) < CLICK_TIME);
}

int touch_was_double_click(const touch *t)
{
    return (touch_was_click(t) && touch_was_click(&old_touch) &&
        (t->start_time > old_touch.last_change_time) &&
        (t->start_time - old_touch.last_change_time) < CLICK_TIME);
}

int touch_is_scroll(void)
{
    if (touch_data[0].in_use && touch_data[1].in_use) {
        int first = 0;
        int last = 1;
        if (touch_data[0].start_time > touch_data[1].start_time) {
            first = 1;
            last = 0;
        }
        return ((touch_data[last].start_time - touch_data[first].start_time) < CLICK_TIME) && !touch_was_click(get_latest_touch());
    }
    const touch *result = get_earliest_touch();
    return (result->in_use && start_delayed(result));
}

int touch_get_scroll(void)
{
    if (!touch_is_scroll() || !touch_data[0].has_moved || !touch_data[1].has_moved) {
        return SCROLL_NONE;
    }
    if (!last_scroll_position) {
        last_scroll_position = touch_data[0].start_point.y;
    }
    int delta_x = abs((touch_data[0].current_point.x - touch_data[0].start_point.x) - (touch_data[1].current_point.x - touch_data[1].start_point.x));
    int delta_y = abs((touch_data[0].current_point.y - touch_data[0].start_point.y) - (touch_data[1].current_point.y - touch_data[1].start_point.y));
    if (delta_x > SCROLL_FINGER_RADIUS || delta_y > SCROLL_FINGER_RADIUS) {
        return SCROLL_NONE;
    }
    int delta = touch_data[0].current_point.y - last_scroll_position;
    if (abs(delta) < NOT_MOVING_RANGE * 2) {
        return SCROLL_NONE;
    }
    last_scroll_position = touch_data[0].current_point.y;
    return (delta > 0) ? SCROLL_UP : SCROLL_DOWN;
}

static int get_unused_touch_index(void)
{
    int i = 0;
    while (i < MAX_ACTIVE_TOUCHES) {
        if (!touch_data[i].in_use) {
            break;
        }
        ++i;
    }
    return i;
}

int touch_create(touch_coords start_coords, time_millis start_time)
{
    int index = get_unused_touch_index();
    if (index != MAX_ACTIVE_TOUCHES) {
        touch *t = &touch_data[index];
        t->in_use = 1;
        t->has_started = 1;
        t->has_ended = 0;
        t->start_point = start_coords;
        t->current_point = start_coords;
        t->previous_frame_point = start_coords;
        t->frame_movement.x = 0;
        t->frame_movement.y = 0;
        t->last_movement = t->frame_movement;
        t->start_time = start_time;
        t->last_change_time = start_time;
    }
    return index;
}

int touch_in_use(int index)
{
    return index >= 0 && index < MAX_ACTIVE_TOUCHES && touch_data[index].in_use;
}

void touch_move(int index, touch_coords current_coords, time_millis current_time)
{
    if (index < 0 || index >= MAX_ACTIVE_TOUCHES || !touch_data[index].in_use) {
        return;
    }
    touch *t = &touch_data[index];
    t->last_change_time = current_time;
    t->current_point = current_coords;
    if ((abs(current_coords.x - t->start_point.x) > NOT_MOVING_RANGE) || (abs(current_coords.y - t->start_point.y) > NOT_MOVING_RANGE)) {
        t->has_moved = 1;
    }
}

void touch_end(int index, time_millis current_time)
{
    if (index < 0 || index >= MAX_ACTIVE_TOUCHES || !touch_data[index].in_use) {
        return;
    }
    touch *t = &touch_data[index];
    t->has_ended = 1;
    // This is needed because sometimes SDL waits for up to three frames to register the touch end
    if (current_time - t->last_change_time < MILLIS_TOLERANCE_BETWEEN_LAST_MOVE_AND_TOUCH_END) {
        t->frame_movement = t->last_movement;
    }
}

void reset_touches(int reset_old_touch)
{
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        touch *t = &touch_data[i];
        if (!t->in_use) {
            continue;
        }
        if (t->has_ended) {
            t->in_use = 0;
            if (!reset_old_touch) {
                old_touch = *t;
            } else {
                old_touch.last_change_time = 0;
            }
            t->has_started = 0;
            t->has_moved = 0;
            t->has_ended = 0;
            last_scroll_position = 0;
        } else {
            t->frame_movement.x = t->current_point.x - t->previous_frame_point.x;
            t->frame_movement.y = t->current_point.y - t->previous_frame_point.y;
            if (t->frame_movement.x != 0 || t->frame_movement.y != 0) {
                t->last_movement = t->frame_movement;
            }
            t->previous_frame_point = t->current_point;
            t->has_started = start_delayed(t);
        }
    }
}

int touch_to_mouse(void)
{
    const touch *first = get_earliest_touch();
    if (!first->in_use) {
        if (mouse_get()->is_touch) {
            mouse_reset_scroll();
            mouse_reset_button_state();
            return 1;
        }
        return 0;
    }
    mouse_set_from_touch(first, get_latest_touch());
    return 1;
}
