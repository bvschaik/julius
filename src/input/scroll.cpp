#include "scroll.h"

#include "core/direction.h"
#include "core/time.h"
#include "game/settings.h"

#include "Data/Screen.h"
#include "Data/State.h"

#define SCROLL_BORDER 5

static struct {
    int is_scrolling;
    time_millis last_scroll_time;
    struct {
        int up;
        int down;
        int left;
        int right;
    } arrow_key;
} data = {0, 0, {0, 0, 0, 0}};

int scroll_in_progress()
{
    return data.is_scrolling;
}

static int should_scroll(const mouse *m)
{
    if (!m->is_inside_window) {
        return 0;
    }
    time_millis current_time = time_get_millis();
    int diff = current_time - data.last_scroll_time;
    if (current_time < data.last_scroll_time) {
        diff = 10000;
    }
    int scroll_delay = (100 - setting_scroll_speed()) / 10;
    if (scroll_delay < 10) { // 0% = 10 = no scroll at all
        if (diff >= 12 * scroll_delay + 2) {
            data.last_scroll_time = current_time;
            return 1;
        }
    }
    return 0;
}

int scroll_get_direction(const mouse *m)
{
    if (!should_scroll(m)) {
        return DIR_8_NONE;
    }
    data.is_scrolling = 0;
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
    // mouse near map edge
    if (m->x < SCROLL_BORDER) {
        left = 1;
        data.is_scrolling = 1;
    }
    if (m->x >= Data_Screen.width - SCROLL_BORDER) {
        right = 1;
        data.is_scrolling = 1;
    }
    if (m->y < SCROLL_BORDER) {
        top = 1;
        data.is_scrolling = 1;
    }
    if (m->y >= Data_Screen.height - SCROLL_BORDER) {
        bottom = 1;
        data.is_scrolling = 1;
    }
    // keyboard arrow keys
    if (data.arrow_key.left) {
        left = 1;
        data.is_scrolling = 1;
    }
    if (data.arrow_key.right) {
        right = 1;
        data.is_scrolling = 1;
    }
    if (data.arrow_key.up) {
        top = 1;
        data.is_scrolling = 1;
    }
    if (data.arrow_key.down) {
        bottom = 1;
        data.is_scrolling = 1;
    }
    data.arrow_key.left = 0;
    data.arrow_key.right = 0;
    data.arrow_key.up = 0;
    data.arrow_key.down = 0;

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

void scroll_arrow_left()
{
    data.arrow_key.left = 1;
}

void scroll_arrow_right()
{
    data.arrow_key.right = 1;
}

void scroll_arrow_up()
{
    data.arrow_key.up = 1;
}

void scroll_arrow_down()
{
    data.arrow_key.down = 1;
}
