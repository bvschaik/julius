#include "scroll.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/direction.h"
#include "core/speed.h"
#include "core/time.h"
#include "game/settings.h"
#include "game/system.h"
#include "graphics/screen.h"
#include "input/touch.h"

#include <math.h>
#include <stdlib.h>

#define MOUSE_BORDER 5
#define TOUCH_BORDER 100
#define SCROLL_DRAG_MIN_DELTA 4
#define SCROLL_DRAG_DECAY_TIME 350
#define SCROLL_REGULAR_DECAY_TIME 75
#define KEY_WAIT_TIME_AFTER_HOLD 500
#define SCROLL_KEY_PRESSED 1
#define SCROLL_KEY_MAX_VALUE 30000.0f
#define TILE_X_PIXELS 60
#define TILE_Y_PIXELS 30

static const int DIRECTION_X[] = { 0,  1,  1,  1,  0, -1, -1, -1,  0 };
static const int DIRECTION_Y[] = { -1, -1,  0,  1,  1,  1,  0, -1,  0 };
static const int SCROLL_STEP[SCROLL_TYPE_MAX][11] = {
    { 60, 44, 30, 20, 16, 12, 10, 8, 6, 4, 2 },
    { 20, 15, 10,  7,  5,  4,  3, 3, 2, 2, 1 }
};

typedef enum {
    KEY_STATE_UNPRESSED = 0,
    KEY_STATE_PRESSED = 1,
    KEY_STATE_HELD = 2,
    KEY_STATE_AXIS = 3
} key_state;

typedef struct {
    key_state state;
    int value;
    time_millis last_change;
} key;

static struct {
    int is_scrolling;
    int constant_input;
    struct {
        key up;
        key down;
        key left;
        key right;
    } arrow_key;
    struct {
        int active;
        int is_touch;
        int has_started;
        pixel_offset delta;
    } drag;
    struct {
        speed_type x;
        speed_type y;
        int decaying;
        float modifier_x;
        float modifier_y;
    } speed;
    speed_direction x_align_direction;
    speed_direction y_align_direction;
    time_millis last_time;
    struct {
        int active;
        int x;
        int y;
        int width;
        int height;
    } limits;
} data;

static void clear_scroll_speed(void)
{
    speed_clear(&data.speed.x);
    speed_clear(&data.speed.y);
    data.speed.decaying = 0;
    data.x_align_direction = SPEED_DIRECTION_STOPPED;
    data.y_align_direction = SPEED_DIRECTION_STOPPED;
}

static int get_arrow_key_value(key* arrow)
{
    if (arrow->state == KEY_STATE_AXIS) {
        return arrow->value;
    }
    if (config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
        return arrow->state != KEY_STATE_UNPRESSED;
    }
    if (arrow->state == KEY_STATE_PRESSED) {
        arrow->state = KEY_STATE_HELD;
        return 1;
    }
    if (arrow->state == KEY_STATE_HELD && time_get_millis() - arrow->last_change >= KEY_WAIT_TIME_AFTER_HOLD) {
        return 1;
    }
    return 0;
}

float get_normalized_arrow_key_value(key* arrow)
{
    int value = get_arrow_key_value(arrow);
    if (value == SCROLL_KEY_PRESSED) {
        return 1.0f;
    }
    else {
        return fminf(arrow->value / SCROLL_KEY_MAX_VALUE, 1.0f);
    }
}

static int is_arrow_active(const key* arrow)
{
    return arrow->value != 0;
}

static void restart_active_arrow(key* arrow, const key* exception)
{
    if (arrow == exception) {
        return;
    }
    if (arrow->state != KEY_STATE_UNPRESSED && arrow->state != KEY_STATE_AXIS) {
        arrow->state = KEY_STATE_PRESSED;
        arrow->last_change = time_get_millis();
    }
}

static void restart_all_active_arrows_except(const key* arrow)
{
    clear_scroll_speed();
    restart_active_arrow(&data.arrow_key.up, arrow);
    restart_active_arrow(&data.arrow_key.down, arrow);
    restart_active_arrow(&data.arrow_key.left, arrow);
    restart_active_arrow(&data.arrow_key.right, arrow);
}

static key_state get_key_state_for_value(int value)
{
    if (!value) {
        return KEY_STATE_UNPRESSED;
    }
    if (value == SCROLL_KEY_PRESSED) {
        return KEY_STATE_PRESSED;
    }
    return KEY_STATE_AXIS;
}

static void set_arrow_key(key* arrow, int value)
{
    key_state state = get_key_state_for_value(value);
    if (state != KEY_STATE_AXIS && state != KEY_STATE_UNPRESSED && arrow->state != KEY_STATE_UNPRESSED) {
        return;
    }
    // Key should retain axis state even if its value is zero
    if (arrow->state != KEY_STATE_AXIS || state != KEY_STATE_UNPRESSED) {
        arrow->state = state;
    }
    arrow->value = value;
    arrow->last_change = time_get_millis();
    if (state != KEY_STATE_AXIS && !config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
        restart_all_active_arrows_except(arrow);
    }
}

int scroll_in_progress(void)
{
    return data.is_scrolling || data.drag.active;
}

static int get_scroll_speed_factor(void)
{
    return calc_bound((100 - setting_scroll_speed()) / 10, 0, 10);
}

int scroll_is_smooth(void)
{
    return config_get(CONFIG_UI_SMOOTH_SCROLLING) || data.drag.active || data.speed.decaying;
}

static int should_scroll(void)
{
    time_millis current_time = time_get_millis();
    time_millis diff = current_time - data.last_time;
    unsigned int scroll_delay = get_scroll_speed_factor();
    int further_delay = data.constant_input ? 20 - (int)(fmaxf(data.speed.modifier_x, data.speed.modifier_y) * 20) : 0;
    if (scroll_delay < 10) { // 0% = 10 = no scroll at all
        if (diff >= 12 * (scroll_delay + further_delay) + 2) {
            data.last_time = current_time;
            return 1;
        }
    }
    return 0;
}

static int direction_from_sides(int top, int left, int bottom, int right)
{
    // two sides
    if (left && top) {
        return DIR_7_TOP_LEFT;
    }
    else if (left && bottom) {
        return DIR_5_BOTTOM_LEFT;
    }
    else if (right && top) {
        return DIR_1_TOP_RIGHT;
    }
    else if (right && bottom) {
        return DIR_3_BOTTOM_RIGHT;
    }
    // one side
    if (left) {
        return DIR_6_LEFT;
    }
    else if (right) {
        return DIR_2_RIGHT;
    }
    else if (top) {
        return DIR_0_TOP;
    }
    else if (bottom) {
        return DIR_4_BOTTOM;
    }
    // none of them
    return DIR_8_NONE;
}

void scroll_set_custom_margins(int x, int y, int width, int height)
{
    data.limits.active = 1;
    data.limits.x = x;
    data.limits.y = y;
    data.limits.width = width;
    data.limits.height = height;
}

void scroll_restore_margins(void)
{
    data.limits.active = 0;
}

void scroll_drag_start(int is_touch)
{
    if (data.drag.active) {
        return;
    }
    data.drag.active = 1;
    data.drag.is_touch = is_touch;
    data.drag.delta.x = 0;
    data.drag.delta.y = 0;
    if (!is_touch) {
        system_mouse_get_relative_state(0, 0);
    }
    clear_scroll_speed();
}

static int set_scroll_speed_from_drag(void)
{
    if (!data.drag.active) {
        return 0;
    }

    int delta_x = 0;
    int delta_y = 0;
    if (!data.drag.is_touch) {
        system_mouse_get_relative_state(&delta_x, &delta_y);
    }
    else {
        const touch* t = get_earliest_touch();
        delta_x = -t->frame_movement.x;
        delta_y = -t->frame_movement.y;
    }

    data.drag.delta.x += delta_x;
    data.drag.delta.y += delta_y;
    if ((delta_x != 0 || delta_y != 0)) {
        if (!data.drag.is_touch) {
            system_mouse_set_relative_mode(1);
        }
        // Store tiny movements until we decide that it's enough to move into scroll mode
        if (!data.drag.has_started) {
            data.drag.has_started = abs(data.drag.delta.x) > SCROLL_DRAG_MIN_DELTA || abs(data.drag.delta.y) > SCROLL_DRAG_MIN_DELTA;
        }
    }
    if (data.drag.has_started) {
        speed_set_target(&data.speed.x, data.drag.delta.x, SPEED_CHANGE_IMMEDIATE, 0);
        speed_set_target(&data.speed.y, data.drag.delta.y, SPEED_CHANGE_IMMEDIATE, 0);
        data.drag.delta.x = 0;
        data.drag.delta.y = 0;
    }
    return 1;
}

int scroll_drag_end(void)
{
    if (!data.drag.active) {
        return 0;
    }

    int has_scrolled = data.drag.has_started;

    data.drag.active = 0;
    data.drag.has_started = 0;

    if (!data.drag.is_touch) {
        system_mouse_set_relative_mode(0);
    }
    else if (has_scrolled) {
        const touch* t = get_earliest_touch();
        speed_set_target(&data.speed.x, -t->frame_movement.x, SPEED_CHANGE_IMMEDIATE, 1);
        speed_set_target(&data.speed.y, -t->frame_movement.y, SPEED_CHANGE_IMMEDIATE, 1);
    }
    data.x_align_direction = speed_get_current_direction(&data.speed.x);
    data.y_align_direction = speed_get_current_direction(&data.speed.y);
    speed_set_target(&data.speed.x, 0, SCROLL_DRAG_DECAY_TIME, 1);
    speed_set_target(&data.speed.y, 0, SCROLL_DRAG_DECAY_TIME, 1);

    return has_scrolled;
}

static int set_arrow_input(key* arrow, const key* opposite_arrow, float* modifier)
{
    if (get_arrow_key_value(arrow) && (!opposite_arrow || !is_arrow_active(opposite_arrow))) {
        if (arrow->state == KEY_STATE_AXIS) {
            data.constant_input = 1;
            *modifier = get_normalized_arrow_key_value(arrow);
        }
        return 1;
    }
    return 0;
}

static int get_direction(const mouse* m)
{
    int is_inside_window = m->is_inside_window;
    int width = screen_width();
    int height = screen_height();
    if (setting_fullscreen() && m->x < width && m->y < height) {
        // For Windows 10, in fullscreen mode, on HiDPI screens, this is needed
        // to get scrolling to work
        is_inside_window = 1;
    }
    if (!is_inside_window && !m->is_touch) {
        return DIR_8_NONE;
    }
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    int border = MOUSE_BORDER;
    int x = m->x;
    int y = m->y;
    data.constant_input = 0;
    data.speed.modifier_x = 0.0f;
    data.speed.modifier_y = 0.0f;
    if (data.limits.active) {
        border = TOUCH_BORDER;
        width = data.limits.width;
        height = data.limits.height;
        x -= data.limits.x;
        y -= data.limits.y;
        data.constant_input = 1;
    }
    // mouse near map edge
    // NOTE: using <= width/height (instead of <) to compensate for rounding
    // errors caused by scaling the display. SDL adds a 1px border to either
    // the right or the bottom when the aspect ratio does not match exactly.
    if ((!m->is_touch || data.limits.active) && (x >= 0 && x <= width && y >= 0 && y <= height)) {
        if (x < border) {
            left = 1;
            data.speed.modifier_x = 1 - x / (float)border;
        }
        else if (x >= width - border) {
            right = 1;
            data.speed.modifier_x = 1 - (width - x) / (float)border;
        }
        if (y < border) {
            top = 1;
            data.speed.modifier_y = 1 - y / (float)border;
        }
        else if (y >= height - border) {
            bottom = 1;
            data.speed.modifier_y = 1 - (height - y) / (float)border;
        }
    }
    // keyboard/joystick arrow keys
    left |= set_arrow_input(&data.arrow_key.left, 0, &data.speed.modifier_x);
    right |= set_arrow_input(&data.arrow_key.right, &data.arrow_key.left, &data.speed.modifier_x);
    top |= set_arrow_input(&data.arrow_key.up, 0, &data.speed.modifier_y);
    bottom |= set_arrow_input(&data.arrow_key.down, &data.arrow_key.up, &data.speed.modifier_y);

    if (data.constant_input) {
        if (!data.speed.modifier_x) {
            data.speed.modifier_x = data.speed.modifier_y;
        }
        if (!data.speed.modifier_y) {
            data.speed.modifier_y = data.speed.modifier_x;
        }
    }

    return direction_from_sides(top, left, bottom, right);
}

static int get_alignment_delta(speed_direction direction, int camera_max_offset, int camera_offset)
{
    if (camera_offset == 0) {
        return 0;
    }
    speed_direction calc_direction = SPEED_DIRECTION_STOPPED;
    switch (direction) {
    case SPEED_DIRECTION_STOPPED:
        calc_direction = (camera_offset >= camera_max_offset / 2) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
        direction = SPEED_DIRECTION_POSITIVE;
        break;
    case SPEED_DIRECTION_NEGATIVE:
        calc_direction = (camera_offset >= camera_max_offset * 0.666667) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
        break;
    default:
        calc_direction = (camera_offset >= camera_max_offset / 3) ? SPEED_DIRECTION_POSITIVE : SPEED_DIRECTION_NEGATIVE;
        break;
    }
    return (calc_direction == SPEED_DIRECTION_POSITIVE) ? (camera_max_offset - camera_offset) : (camera_offset * -direction);
}

static int set_scroll_speed_from_input(const mouse* m, scroll_type type)
{
    if (set_scroll_speed_from_drag()) {
        return 1;
    }
    int direction = get_direction(m);
    if (direction == DIR_8_NONE) {
        time_millis time = config_get(CONFIG_UI_SMOOTH_SCROLLING) ? SCROLL_REGULAR_DECAY_TIME : SPEED_CHANGE_IMMEDIATE;
        speed_set_target(&data.speed.x, 0, time, 1);
        speed_set_target(&data.speed.y, 0, time, 1);
        return 0;
    }
    if (data.speed.decaying) {
        clear_scroll_speed();
    }
    int dir_x = DIRECTION_X[direction];
    int dir_y = DIRECTION_Y[direction];
    int y_fraction = type == SCROLL_TYPE_CITY ? 2 : 1;

    if (!config_get(CONFIG_UI_SMOOTH_SCROLLING) && !data.limits.active) {
        int do_scroll = should_scroll();
        int step = SCROLL_STEP[type][0];
        int align_x = 0;
        int align_y = 0;
        if (type == SCROLL_TYPE_CITY) {
            pixel_offset camera_offset;
            city_view_get_pixel_offset(&camera_offset.x, &camera_offset.y);
            align_x = get_alignment_delta(dir_x, TILE_X_PIXELS, camera_offset.x);
            align_y = get_alignment_delta(dir_y, TILE_Y_PIXELS, camera_offset.y);
        }
        speed_set_target(&data.speed.x, (step + align_x) * dir_x * do_scroll, SPEED_CHANGE_IMMEDIATE, 0);
        speed_set_target(&data.speed.y, ((step / y_fraction) + align_y) * dir_y * do_scroll, SPEED_CHANGE_IMMEDIATE, 0);
        return 1;
    }

    int max_speed = SCROLL_STEP[type][get_scroll_speed_factor()];
    int max_speed_x = max_speed * dir_x;
    int max_speed_y = (max_speed / y_fraction) * dir_y;

    if (!data.constant_input) {
        if (speed_get_current_direction(&data.speed.x) * dir_x < 0) {
            speed_invert(&data.speed.x);
        }
        else if (data.speed.x.desired_speed != max_speed_x) {
            speed_set_target(&data.speed.x, max_speed_x, SCROLL_REGULAR_DECAY_TIME, 1);
        }
        if (speed_get_current_direction(&data.speed.y) * dir_y < 0) {
            speed_invert(&data.speed.y);
        }
        else if (data.speed.y.desired_speed != max_speed_y) {
            speed_set_target(&data.speed.y, max_speed_y, SCROLL_REGULAR_DECAY_TIME, 1);
        }
    }
    else {
        speed_set_target(&data.speed.x, (int)(max_speed_x * data.speed.modifier_x), SPEED_CHANGE_IMMEDIATE, 1);
        speed_set_target(&data.speed.y, (int)(max_speed_y * data.speed.modifier_y), SPEED_CHANGE_IMMEDIATE, 1);
    }
    return 1;
}

int scroll_get_delta(const mouse* m, pixel_offset* delta, scroll_type type)
{
    data.is_scrolling = set_scroll_speed_from_input(m, type);
    delta->x = speed_get_delta(&data.speed.x);
    delta->y = speed_get_delta(&data.speed.y);
    if (!data.is_scrolling) {
        data.speed.decaying = speed_is_changing(&data.speed.x) || speed_is_changing(&data.speed.y);
        data.is_scrolling = data.speed.decaying;
    }
    return delta->x != 0 || delta->y != 0;
}

void scroll_stop(void)
{
    clear_scroll_speed();
    system_mouse_set_relative_mode(0);
    data.is_scrolling = 0;
    data.constant_input = 0;
    data.drag.active = 0;
    data.limits.active = 0;
}

void scroll_arrow_left(int value)
{
    set_arrow_key(&data.arrow_key.left, value);
}

void scroll_arrow_right(int value)
{
    set_arrow_key(&data.arrow_key.right, value);
}

void scroll_arrow_up(int value)
{
    set_arrow_key(&data.arrow_key.up, value);
}

void scroll_arrow_down(int value)
{
    set_arrow_key(&data.arrow_key.down, value);
}