#include "scroll.h"

#include "core/config.h"
#include "core/direction.h"
#include "core/time.h"
#include "game/settings.h"
#include "graphics/screen.h"

#include <math.h>

static const int MOUSE_BORDER = 5;
static const int TOUCH_BORDER = 100;
static const int DECAY_MULTIPLIER = 300;
static const time_millis DECAY_BASE_TIME = 350;
static const time_millis DECAY_MAX_TIME = 1050; // DECAY_BASE_TIME * 3
static const time_millis MAX_SPEED_TIME_WEIGHT = 200;

static const int DIRECTION_X[] = {  0,  1,  1,  1,  0, -1, -1, -1,  0 };
static const int DIRECTION_Y[] = { -1, -1,  0,  1,  1,  1,  0, -1,  0 };

static struct {
    int is_scrolling;
    int is_touch;
    struct {
        int up;
        int down;
        int left;
        int right;
    } arrow_key;
    struct {
        pixel_offset original;
        pixel_offset current;
    } position;
    struct {
        int x;
        int y;
        time_millis start_time;
        time_millis last_time;
        pixel_offset last_position;
        int decaying;
        float modifier;
    } speed;
    struct {
        int active;
        int x;
        int y;
        int width;
        int height;
    } limits;
    touch_coords start_touch;
} data;

int scroll_in_progress(void)
{
    return data.is_scrolling;
}

static int should_scroll(void)
{
    time_millis current_time = time_get_millis();
    time_millis diff = current_time - data.speed.last_time;
    unsigned int scroll_delay = (100 - setting_scroll_speed()) / 10;
    if (scroll_delay < 10) { // 0% = 10 = no scroll at all
        if (diff >= 12 * scroll_delay + 2) {
            data.speed.last_time = current_time;
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

static void clear_scroll_decay(const pixel_offset *position)
{
    data.speed.x = 0;
    data.speed.y = 0;
    data.speed.start_time = time_get_millis();
    data.speed.last_time = time_get_millis();
    data.speed.last_position = position ? *position : (pixel_offset) { 0, 0 };
    data.speed.decaying = 0;
}

static void find_scroll_speed(const pixel_offset *position)
{
    time_millis current_time = time_get_millis();
    time_millis delta_time = current_time - data.speed.last_time;
    time_millis total_time = current_time - data.speed.start_time;

    if (!delta_time) {
        return;
    }
    if (total_time > MAX_SPEED_TIME_WEIGHT) {
        total_time = MAX_SPEED_TIME_WEIGHT;
    }
    if (delta_time > MAX_SPEED_TIME_WEIGHT) {
        delta_time = MAX_SPEED_TIME_WEIGHT;
    }

    time_millis weighted_time = total_time - delta_time;
    int delta_position = (position->x - data.speed.last_position.x) * DECAY_MULTIPLIER;
    data.speed.x = (int) (data.speed.x * weighted_time + delta_position) / (int) total_time;
    delta_position = (position->y - data.speed.last_position.y) * DECAY_MULTIPLIER;
    data.speed.y = (int) (data.speed.y * weighted_time + delta_position) / (int) total_time;

    data.speed.last_time = current_time;
    data.speed.last_position = *position;
}

void scroll_set_custom_margins(int x, int y, int width, int height)
{
    data.is_touch = 1;
    data.limits.active = 1;
    data.limits.x = x;
    data.limits.y = y;
    data.limits.width = width;
    data.limits.height = height;
}

void scroll_restore_margins(void)
{
    data.is_touch = 0;
    data.limits.active = 0;
}

touch_coords scroll_get_original_touch_position(void)
{
    return data.start_touch;
}

void scroll_start_touch_drag(const pixel_offset *position, touch_coords coords)
{
    data.position.original = *position;
    data.position.current = *position;
    data.start_touch = coords;
    data.is_scrolling = 1;
    data.is_touch = 1;
    clear_scroll_decay(position);
}

int scroll_move_touch_drag(int original_x, int original_y, int current_x, int current_y, pixel_offset *position)
{
    data.is_scrolling = 1;
    data.is_touch = 1;

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

void scroll_end_touch_drag(void)
{
    data.is_touch = 0;
    data.speed.last_position.x = data.position.current.x + data.speed.x;
    data.speed.last_position.y = data.position.current.y + data.speed.y;
    data.speed.last_time = time_get_millis();
    data.speed.decaying = 1;
}

int scroll_decay(pixel_offset *position)
{
    if (!data.speed.decaying) {
        return 0;
    }
    time_millis elapsed = time_get_millis() - data.speed.last_time;
    if (elapsed > DECAY_MAX_TIME) {
        data.is_scrolling = 0;
        clear_scroll_decay(0);
        return 0;
    }
    double exponent = exp(-((int)elapsed) / (double)DECAY_BASE_TIME);
    position->x = data.speed.last_position.x - ((int)(data.speed.x * exponent));
    position->y = data.speed.last_position.y - ((int)(data.speed.y * exponent));
    if (position->x == data.speed.last_position.x && position->y == data.speed.last_position.y) {
        data.is_scrolling = 0;
        clear_scroll_decay(0);
    } else {
        data.is_scrolling = 1;
    }
    return 1;
}

static int absolute_decrement(int value, int step)
{
    if (value >= 0) {
        return (step > value) ? 0 : value - step;
    }
    return (step > -value) ? 0 : value + step;
}

void scroll_get_delta(const mouse *m, pixel_offset *delta)
{
    int use_smooth_scrolling = config_get(CONFIG_UI_ENABLE_SMOOTH_SCROLLING);
    int direction = scroll_get_direction(m);

    if (direction == DIR_8_NONE) {
        if (!data.is_touch && !data.speed.decaying && (data.speed.x || data.speed.y) && use_smooth_scrolling) {
            data.is_scrolling = 1;
            data.speed.x = absolute_decrement(data.speed.x, 2);
            data.speed.y = absolute_decrement(data.speed.y, 1);
            delta->x = data.speed.x;
            delta->y = data.speed.y;
        } else {
            delta->x = 0;
            delta->y = 0;
        }
        return;
    }
    if (data.speed.decaying) {
        clear_scroll_decay(0);
    }
    int dir_x = DIRECTION_X[direction];
    int dir_y = DIRECTION_Y[direction];

    if (!use_smooth_scrolling && !data.is_touch) {
        int do_scroll = should_scroll();
        delta->x = 60 * dir_x * do_scroll;
        delta->y = 30 * dir_y * do_scroll;
        return;
    }

    int max_speed = (30 * setting_scroll_speed() / 100) & ~1;
    int max_speed_x = max_speed * dir_x;
    int max_speed_y = max_speed * dir_y / 2;

    if (!data.limits.active) {
        if (!dir_x) {
            data.speed.x = absolute_decrement(data.speed.x, 2);
        } else if (data.speed.x * dir_x < 0) {
            data.speed.x = -data.speed.x;
        } else if (data.speed.x != max_speed_x) {
            data.speed.x += dir_x;
        }
        if (!dir_y) {
            data.speed.y = absolute_decrement(data.speed.y, 1);
        } else if (data.speed.y * dir_y < 0) {
            data.speed.y = -data.speed.y;
        } else if (data.speed.y != max_speed_y) {
            data.speed.y += dir_y;
        }
    } else {
        data.speed.x = (int) (max_speed_x * data.speed.modifier);
        data.speed.y = (int) (max_speed_y * data.speed.modifier);
    }

    // Adjust delta for time (allows scrolling at the same rate at lower FPS)
    // Expects the game to run at least at 5FPS to work
    time_millis current_time = time_get_millis();
    time_millis time_delta = current_time - data.speed.last_time;
    data.speed.last_time = current_time;
    if(time_delta > 17 && time_delta < MAX_SPEED_TIME_WEIGHT) {
        delta->x = (int) ((float) (data.speed.x / 17.0f) * time_delta);
        delta->y = (int) ((float) (data.speed.y / 17.0f) * time_delta);
    } else {
        delta->x = data.speed.x;
        delta->y = data.speed.y;
    }
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
    int border = MOUSE_BORDER;
    int x = m->x;
    int y = m->y;
    if (data.limits.active) {
        border = TOUCH_BORDER;
        width = data.limits.width;
        height = data.limits.height;
        x -= data.limits.x;
        y -= data.limits.y;
        data.speed.modifier = 0;
    }
    // mouse near map edge
    if ((!m->is_touch || data.limits.active) && (x >= 0 && x < width && y >= 0 && y < height)) {
        if (x < border) {
            left = 1;
            data.speed.modifier = 1 - x / (float) border;
        } else if (x >= width - border) {
            right = 1;
            data.speed.modifier = 1 - (width - x) / (float) border;
        }
        if (y < border) {
            top = 1;
            data.speed.modifier = (float) fmax(data.speed.modifier, 1 - y / (float) border);
        } else if (y >= height - border) {
            bottom = 1;
            data.speed.modifier = (float) fmax(data.speed.modifier, 1 - (height - y) / (float) border);
        }
    }

    if (!data.is_touch) {
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
    }
    return direction_from_sides(top, left, bottom, right);
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
