#include "scroll.h"

#include "core/direction.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/screen.h"

#include <math.h>

static const int SCROLL_BORDER = 5;
static const int SCROLL_DECAY_MULTIPLIER = 250;
static const time_millis SCROLL_DECAY_BASE_TIME = 325;
static const time_millis SCROLL_DECAY_MAX_TIME = 975; // SCROLL_DECAY_BASE_TIME * 3

static struct {
    int is_scrolling;
    time_millis last_scroll_time;
    struct {
        int up;
        int down;
        int left;
        int right;
    } arrow_key;
    struct {
        view_tile original;
        view_tile current;
    } position;
    struct {
        int x;
        int y;
        time_millis last_time;
        view_tile last_position;
        int decaying;
    } speed;
    struct {
        int active;
        int x;
        int y;
        int width;
        int height;
        int margin;
        int multiplier;
    } limits;
    touch_coords start_touch;
} data;

int scroll_in_progress(void)
{
    return data.is_scrolling;
}

static int should_scroll(int speed_modifier)
{
    return 1;
    time_millis current_time = time_get_millis();
    time_millis diff = current_time - data.last_scroll_time;
    unsigned int scroll_delay = (100 - setting_scroll_speed() + speed_modifier) / 10;
    if (scroll_delay < 10) { // 0% = 10 = no scroll at all
        if (diff >= 12 * scroll_delay + 2) {
            data.last_scroll_time = current_time;
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
    } else if (left && bottom) {
        return DIR_5_BOTTOM_LEFT;
    } else if (right && top) {
        return DIR_1_TOP_RIGHT;
    } else if (right && bottom) {
        return DIR_3_BOTTOM_RIGHT;
    }
    // one side
    if (left) {
        return DIR_6_LEFT;
    } else if (right) {
        return DIR_2_RIGHT;
    } else if (top) {
        return DIR_0_TOP;
    } else if (bottom) {
        return DIR_4_BOTTOM;
    }
    // none of them
    return DIR_8_NONE;
}

static void clear_scroll_decay(const view_tile *position)
{
    data.speed.x = 0;
    data.speed.y = 0;
    data.speed.last_time = time_get_millis();
    data.speed.last_position = position ? *position : (view_tile) { 0, 0 };
    data.speed.decaying = 0;
}

static void find_scroll_speed(const view_tile *position)
{
    int current_time = time_get_millis();
    int time_delta = current_time - data.speed.last_time;

    if (time_delta < 50) {
        return;
    }

    int position_delta = (position->x - data.speed.last_position.x) * SCROLL_DECAY_MULTIPLIER;
    data.speed.x = position_delta / time_delta;
    position_delta = (position->y - data.speed.last_position.y) * SCROLL_DECAY_MULTIPLIER;
    data.speed.y = position_delta / time_delta;

    data.speed.last_time = current_time;
    data.speed.last_position = *position;
}

void scroll_set_custom_margins(int x, int y, int width, int height)
{
    data.limits.active = 1;
    data.limits.x = x;
    data.limits.y = y;
    data.limits.width = width;
    data.limits.height = height;
    data.limits.margin = setting_scroll_speed();
    data.limits.multiplier = (width < 900 || height < 500) ? 2 : 3;
}

void scroll_restore_margins(void)
{
    data.limits.active = 0;
}

touch_coords scroll_get_original_touch_position(void)
{
    return data.start_touch;
}

void scroll_start_touch_drag(const view_tile *position, touch_coords coords)
{
    data.position.original = *position;
    data.position.current = *position;
    data.start_touch = coords;
    data.is_scrolling = 1;
    clear_scroll_decay(position);
}

int scroll_move_touch_drag(int original_x, int original_y, int current_x, int current_y, view_tile *position)
{
    data.is_scrolling = 1;

    position->x = data.position.original.x - (current_x - original_x);
    position->y = data.position.original.y - (current_y - original_y);

    find_scroll_speed(position);

    if (position->x != data.position.current.x || position->y != data.position.current.y) {
        data.position.current.x = position->x;
        data.position.current.y = position->y;
        return 1;
    }
    return 0;
}

void scroll_end_touch_drag(void) {
    data.speed.last_position.x = data.position.current.x + data.speed.x;
    data.speed.last_position.y = data.position.current.y + data.speed.y;
    data.speed.last_time = time_get_millis();
    data.speed.decaying = 1;
}

int scroll_decay(view_tile *position)
{
    if (!data.speed.decaying) {
        return 0;
    }
    time_millis elapsed = time_get_millis() - data.speed.last_time;
    if (elapsed > SCROLL_DECAY_MAX_TIME) {
        data.is_scrolling = 0;
        clear_scroll_decay(position);
        return 0;
    }
    double exponent = exp(-((int)elapsed) / (double)SCROLL_DECAY_BASE_TIME);
    position->x = data.speed.last_position.x - ((int)(data.speed.x * exponent));
    position->y = data.speed.last_position.y - ((int)(data.speed.y * exponent));
    if (position->x == data.speed.last_position.x && position->y == data.speed.last_position.y) {
        data.is_scrolling = 0;
        clear_scroll_decay(position);
    } else {
        data.is_scrolling = 1;
    }
    return 1;
}

int scroll_get_direction(const mouse *m)
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
    if (data.speed.decaying && m->left.went_down) {
        data.is_scrolling = 0;
        clear_scroll_decay(0);
    }
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    int speed_modifier = 0;
    int border = SCROLL_BORDER;
    int x = m->x;
    int y = m->y;
    if (data.limits.active) {
        border = data.limits.margin * data.limits.multiplier;
        speed_modifier = border;
        width = data.limits.width;
        height = data.limits.height;
        x -= data.limits.x;
        y -= data.limits.y;
    }
    // mouse near map edge
    if ((!m->is_touch || data.limits.active) && (x >= 0 && x < width && y >= 0 && y < height)) {
        if (x < border) {
            left = 1;
            speed_modifier = (x < speed_modifier) ? x : speed_modifier;
        }
        if (x >= width - border) {
            right = 1;
            speed_modifier = ((width - x) < speed_modifier) ? (width - x) : speed_modifier;
        }
        if (y < border) {
            top = 1;
            speed_modifier = (y < speed_modifier) ? y : speed_modifier;
        }
        if (y >= height - border) {
            bottom = 1;
            speed_modifier = ((height - y) < speed_modifier) ? (height - y) : speed_modifier;
        }
    }

    if (!data.limits.active) {
        // keyboard arrow keys
        if (data.arrow_key.left) {
            left = 1;
        } else if (data.arrow_key.right) {
            right = 1;
        }
        if (data.arrow_key.up) {
            top = 1;
        } else if (data.arrow_key.down) {
            bottom = 1;
        }
        data.is_scrolling = (top | left | bottom | right);
    } else {
        speed_modifier /= data.limits.multiplier;
    }

    if (should_scroll(speed_modifier)) {
        int direction = direction_from_sides(top, left, bottom, right);
        if (direction != DIR_8_NONE) {
            clear_scroll_decay(0);
        }
        return direction;
    } else {
        return DIR_8_NONE;
    }
}

void scroll_arrow_left(int is_down)
{
    data.arrow_key.left = is_down;
}

void scroll_arrow_right(int is_down)
{
    data.arrow_key.right = is_down;
}

void scroll_arrow_up(int is_down)
{
    data.arrow_key.up = is_down;
}

void scroll_arrow_down(int is_down)
{
    data.arrow_key.down = is_down;
}
