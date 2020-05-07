#include "zoom.h"

#include "core/calc.h"
#include "core/config.h"
#include "graphics/menu.h"

#include <math.h>

#define ZOOM_STEP 2

static struct {
    int delta;
    int restore;
    pixel_offset input_offset;
    struct {
        int active;
        int start_zoom;
        int current_zoom;
    } touch;
} data;

static void start_touch(const touch *first, const touch *last, int scale)
{
    if (!config_get(CONFIG_UI_ZOOM)) {
        return;
    }
    data.restore = 0;
    data.touch.active = 1;
    data.input_offset.x = first->current_point.x;
    data.input_offset.y = first->current_point.y;
    data.touch.start_zoom = scale;
    data.touch.current_zoom = scale;
}

void zoom_update_touch(const touch *first, const touch *last, int scale)
{
    if (!config_get(CONFIG_UI_ZOOM)) {
        return;
    }
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
    if (!config_get(CONFIG_UI_ZOOM)) {
        return;
    }
    data.touch.active = 0;
}

void zoom_map(const mouse *m)
{
    if (!config_get(CONFIG_UI_ZOOM) || data.touch.active || m->is_touch) {
        return;
    }
    if (m->middle.went_up) {
        data.restore = 1;
        data.input_offset.x = m->x;
        data.input_offset.y = m->y - TOP_MENU_HEIGHT;
    }
    if (m->scrolled != SCROLL_NONE) {
        data.restore = 0;
        data.delta = (m->scrolled == SCROLL_DOWN) ? 20 : -20;
        data.input_offset.x = m->x;
        data.input_offset.y = m->y - TOP_MENU_HEIGHT;
    }
}

int zoom_update_value(int *zoom, pixel_offset *camera_position)
{
    int step;
    if (!data.touch.active) {
        if (data.restore) {
            data.delta = 100 - *zoom;
            data.restore = 0;
        }
        if (data.delta == 0) {
            return 0;
        }
        if (config_get(CONFIG_UI_SMOOTH_SCROLLING)) {
            step = (data.delta > 0) ? ZOOM_STEP : -ZOOM_STEP;
            if (*zoom > 100) {
                step *= 2;
            }
        } else {
            step = data.delta;
        }
        data.delta = calc_absolute_decrement(data.delta, step);
    } else {
        data.restore = 0;
        step = data.touch.current_zoom - *zoom;
    }

    int result = calc_bound(*zoom + step, 50, 200);
    if (*zoom == result) {
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
