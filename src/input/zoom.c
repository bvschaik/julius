#include "zoom.h"

#include "core/calc.h"
#include "core/config.h"
#include "core/speed.h"
#include "graphics/menu.h"
#include "input/hotkey.h"

#include <math.h>

#define ZOOM_STEP 2
#define ZOOM_DELTA 25

static struct {
    int delta;
    int restore;
    pixel_offset input_offset;
    speed_type step;
    struct {
        int active;
        int start_zoom;
        int current_zoom;
    } touch;
} data;

static void start_touch(const touch *first, const touch *last, int scale)
{
    data.restore = 0;
    data.touch.active = 1;
    data.input_offset.x = first->current_point.x;
    data.input_offset.y = first->current_point.y;
    data.touch.start_zoom = scale;
    data.touch.current_zoom = scale;
}

void zoom_update_touch(const touch *first, const touch *last, int scale)
{
    if (!data.touch.active) {
        start_touch(first, last, scale);
        return;
    }
    int original_distance, current_distance;
    pixel_offset temp;
    temp.x = first->start_point.x - last->start_point.x;
    temp.y = first->start_point.y - last->start_point.y;
    original_distance = (int) sqrt(temp.x * temp.x + temp.y * temp.y);
    temp.x = first->current_point.x - last->current_point.x;
    temp.y = first->current_point.y - last->current_point.y;
    current_distance = (int) sqrt(temp.x * temp.x + temp.y * temp.y);

    if (!original_distance || !current_distance) {
        data.touch.active = 0;
        return;
    }

    int finger_distance_percentage = calc_percentage(current_distance, original_distance);
    data.touch.current_zoom = calc_percentage(data.touch.start_zoom, finger_distance_percentage);
}

void zoom_end_touch(void)
{
    data.touch.active = 0;
}

void zoom_map(const mouse *m, int current_zoom)
{
    if (data.touch.active || m->is_touch) {
        return;
    }
    if (m->middle.went_up) {
        data.restore = 1;
        speed_clear(&data.step);
        data.input_offset.x = m->x;
        data.input_offset.y = m->y - TOP_MENU_HEIGHT;
    }
    if (m->scrolled != SCROLL_NONE) {
        data.restore = 0;
        int zoom_offset;
        int zoom_delta;
        if (m->scrolled == SCROLL_DOWN) {
            zoom_offset = 0;
            zoom_delta = hotkey_shift_pressed() ? 1 : ZOOM_DELTA;
        } else {
            zoom_offset = -1;
            zoom_delta = hotkey_shift_pressed() ? -1 : -ZOOM_DELTA;
        }
        int multiplier = (current_zoom + zoom_offset) / 100 + 1;
        data.delta = zoom_delta;
        if (!hotkey_shift_pressed()) {
            data.delta *= multiplier;
        }
        if (config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
            speed_clear(&data.step);
            speed_set_target(&data.step, ZOOM_STEP, SPEED_CHANGE_IMMEDIATE, 1);
        }
        data.input_offset.x = m->x;
        data.input_offset.y = m->y - TOP_MENU_HEIGHT;
    }
}

int zoom_update_value(int *zoom, int max, pixel_offset *camera_position)
{
    int step;
    if (!data.touch.active) {
        if (data.restore) {
            data.delta = 100 - *zoom;
            if (config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
                speed_set_target(&data.step, ZOOM_STEP, SPEED_CHANGE_IMMEDIATE, 1);
            }
            data.restore = 0;
        }
        if (data.delta == 0) {
            return 0;
        }
        if (config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
            step = speed_get_delta(&data.step);
            step *= (*zoom / 100) + 1;
            if (!step) {
                return 1;
            }
        } else {
            step = data.delta;
        }

        data.delta = calc_absolute_decrement(data.delta, &step);

        if (data.delta == 0) {
            speed_clear(&data.step);
        }
    } else {
        speed_clear(&data.step);
        data.restore = 0;
        int current_zoom = data.touch.current_zoom;
        if (current_zoom > 90 && current_zoom < 110) {
            current_zoom = 100;
        }
        step = current_zoom - *zoom;
    }

    int result = calc_bound(*zoom + step, 50, max);
    if (*zoom == result) {
        speed_clear(&data.step);
        data.delta = 0;
        return 0;
    }
    pixel_offset old_offset, new_offset;
    old_offset.x = calc_adjust_with_percentage(data.input_offset.x, *zoom);
    old_offset.y = calc_adjust_with_percentage(data.input_offset.y, *zoom);

    new_offset.x = calc_adjust_with_percentage(data.input_offset.x, result);
    new_offset.y = calc_adjust_with_percentage(data.input_offset.y, result);

    camera_position->x -= new_offset.x - old_offset.x;
    camera_position->y -= new_offset.y - old_offset.y;

    if (!config_get(CONFIG_UI_SMOOTH_SCROLLING) && !data.touch.active) {
        int remaining_x = camera_position->x & 60;
        int remaining_y = camera_position->y & 15;
        if (remaining_x >= 30) {
            remaining_x -= 60;
        }
        if (remaining_y >= 8) {
            remaining_y -= 15;
        }
        camera_position->x -= remaining_x;
        camera_position->y -= remaining_y;
    }
    *zoom = result;
    return 1;
}
