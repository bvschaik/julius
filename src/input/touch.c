#include "input/touch.h"

#include "game/system.h"
#include "input/mouse.h"

#include <stdlib.h>

#define CLICK_TIME 300
#define NOT_MOVING_RANGE 5
#define SCROLL_FINGER_RADIUS 25
#define MILLIS_TOLERANCE_BETWEEN_LAST_MOVE_AND_TOUCH_END 60

typedef enum {
    EMULATED_MOUSE_CLICK_NONE = 0,
    EMULATED_MOUSE_CLICK_LEFT = 1,
    EMULATED_MOUSE_CLICK_RIGHT = 2
} emulated_mouse_click;

static struct {
    touch finger[MAX_ACTIVE_TOUCHES + 1];
    touch old_touch;
    int last_scroll_position;
    touch_mode mode;
    emulated_mouse_click touchpad_mode_click_type;
} data;

static int start_delayed(const touch *t)
{
    return (t->has_started && !t->has_moved && !t->has_ended && ((time_get_millis() - t->start_time) < (time_millis) (CLICK_TIME / 2)));
}

const touch *get_earliest_touch(void)
{
    time_millis timestamp = -1;
    int touch_index = MAX_ACTIVE_TOUCHES;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (data.finger[i].in_use && !start_delayed(&data.finger[i]) && data.finger[i].start_time < timestamp) {
            timestamp = data.finger[i].start_time;
            touch_index = i;
        }
    }
    return &data.finger[touch_index];
}

const touch *get_latest_touch(void)
{
    int active_touches = 0;
    time_millis timestamp = 0;
    int touch_index = MAX_ACTIVE_TOUCHES;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (data.finger[i].in_use && !start_delayed(&data.finger[i])) {
            ++active_touches;
            if (data.finger[i].start_time > timestamp) {
                timestamp = data.finger[i].start_time;
                touch_index = i;
            }
        }
    }
    return (active_touches > 1) ? &data.finger[touch_index] : &data.finger[MAX_ACTIVE_TOUCHES];
}

int get_total_active_touches(void)
{
    int active_touches = 0;
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (data.finger[i].in_use) {
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
    return (touch_was_click(t) && touch_was_click(&data.old_touch) &&
        (t->start_time > data.old_touch.last_change_time) &&
        (t->start_time - data.old_touch.last_change_time) < CLICK_TIME);
}

int touch_is_scroll(void)
{
    int num_touches = get_total_active_touches();
    if (num_touches > 2) {
        return 0;
    }
    const touch *first = get_earliest_touch();
    if (num_touches == 2) {
        const touch *last = get_latest_touch();
        return ((last->start_time - first->start_time) < CLICK_TIME) && !touch_was_click(last);
    }
    return first->in_use && start_delayed(first);
}

int touch_get_scroll(void)
{
    const touch *first = get_earliest_touch();
    const touch *last = get_latest_touch();
    if (!touch_is_scroll() || !first->has_moved || !last->has_moved) {
        return SCROLL_NONE;
    }
    if (!data.last_scroll_position) {
        data.last_scroll_position = first->start_point.y;
    }
    int delta_x = abs((first->current_point.x - first->start_point.x) - (last->current_point.x - last->start_point.x));
    int delta_y = abs((first->current_point.y - first->start_point.y) - (last->current_point.y - last->start_point.y));
    if (delta_x > SCROLL_FINGER_RADIUS || delta_y > SCROLL_FINGER_RADIUS) {
        return SCROLL_NONE;
    }
    int delta = first->current_point.y - data.last_scroll_position;
    if (abs(delta) < NOT_MOVING_RANGE * 2) {
        return SCROLL_NONE;
    }
    data.last_scroll_position = first->current_point.y;
    return (delta > 0) ? SCROLL_UP : SCROLL_DOWN;
}

static int get_unused_touch_index(void)
{
    int i = 0;
    while (i < MAX_ACTIVE_TOUCHES) {
        if (!data.finger[i].in_use) {
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
        touch *t = &data.finger[index];
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
    return index >= 0 && index < MAX_ACTIVE_TOUCHES && data.finger[index].in_use;
}

void touch_move(int index, touch_coords current_coords, time_millis current_time)
{
    if (index < 0 || index >= MAX_ACTIVE_TOUCHES || !data.finger[index].in_use) {
        return;
    }
    touch *t = &data.finger[index];
    t->last_change_time = current_time;
    t->current_point = current_coords;
    if ((abs(current_coords.x - t->start_point.x) > NOT_MOVING_RANGE) || (abs(current_coords.y - t->start_point.y) > NOT_MOVING_RANGE)) {
        t->has_moved = 1;
    }
}

void touch_end(int index, time_millis current_time)
{
    if (index < 0 || index >= MAX_ACTIVE_TOUCHES || !data.finger[index].in_use) {
        return;
    }
    touch *t = &data.finger[index];
    t->has_ended = 1;
    // This is needed because sometimes SDL waits for up to three frames to register the touch end
    if (current_time - t->last_change_time < MILLIS_TOLERANCE_BETWEEN_LAST_MOVE_AND_TOUCH_END) {
        t->frame_movement = t->last_movement;
    }
}

void reset_touches(int reset_old_touch)
{
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        touch *t = &data.finger[i];
        if (!t->in_use) {
            continue;
        }
        if (t->has_ended) {
            t->in_use = 0;
            if (!reset_old_touch) {
                data.old_touch = *t;
            } else {
                data.old_touch.last_change_time = 0;
            }
            t->has_started = 0;
            t->has_moved = 0;
            t->has_ended = 0;
            data.last_scroll_position = 0;
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

static int any_touch_went_up(void)
{
    for (int i = 0; i < MAX_ACTIVE_TOUCHES; ++i) {
        if (data.finger[i].has_ended) {
            return 1;
        }
    }
    return 0;
}

static int handle_emulated_mouse_clicks(void)
{
    mouse_reset_scroll();
    switch (data.touchpad_mode_click_type) {
        case EMULATED_MOUSE_CLICK_LEFT:
            mouse_set_left_down(0);
            break;
        case EMULATED_MOUSE_CLICK_RIGHT:
            mouse_set_right_down(0);
            break;
        default:
            mouse_reset_button_state();
            return 0;
    }
    mouse_determine_button_state();
    data.touchpad_mode_click_type = EMULATED_MOUSE_CLICK_NONE;
    return 1;
}

static void handle_mouse_touchpad(void)
{
    if (handle_emulated_mouse_clicks()) {
        return;
    }

    int num_fingers = get_total_active_touches();

    if (!num_fingers) {
        return;
    }

    if (any_touch_went_up()) {
        if (num_fingers == 1 && touch_was_click(get_earliest_touch())) {
            mouse_set_left_down(1);
            mouse_determine_button_state();
            data.touchpad_mode_click_type = EMULATED_MOUSE_CLICK_LEFT;
        } else if (num_fingers == 2 &&
            (touch_was_click(get_earliest_touch()) || touch_was_click(get_latest_touch()))) {
            mouse_set_right_down(1);
            mouse_determine_button_state();
            data.touchpad_mode_click_type = EMULATED_MOUSE_CLICK_RIGHT;
        }
    } else {
        const touch *t = get_earliest_touch();
        if (!t->has_moved) {
            return;
        }
        system_move_mouse_cursor(t->frame_movement.x, t->frame_movement.y);
    }
}

static void handle_mouse_direct(void)
{
    mouse_reset_scroll();
    mouse_reset_button_state();

    const touch *first = get_earliest_touch();
    int x = first->current_point.x;
    int y = first->current_point.y;
    system_set_mouse_position(&x, &y);
    mouse_set_position(x, y);
}

int touch_to_mouse(void)
{
    const touch *first = get_earliest_touch();
    if (!first->in_use) {
        if (mouse_get()->is_touch) {
            mouse_reset_scroll();
            mouse_reset_button_state();
            return 1;
        } else if (data.touchpad_mode_click_type != EMULATED_MOUSE_CLICK_NONE) {
            return handle_emulated_mouse_clicks();
        }
        return 0;
    }
    switch (data.mode) {
        case TOUCH_MODE_TOUCHPAD:
            handle_mouse_touchpad();
            break;
        case TOUCH_MODE_DIRECT:
            handle_mouse_direct();
            break;
        default:
            mouse_set_from_touch(first, get_latest_touch());
    }
    return 1;
}

void touch_set_mode(touch_mode mode)
{
    data.mode = mode;
}

void touch_cycle_mode(void)
{
    data.mode = (data.mode + 1) % TOUCH_MODE_MAX;
}
