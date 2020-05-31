#include "input/mouse.h"

#include "core/time.h"
#include "graphics/screen.h"

enum {
    SYSTEM_NONE = 0,
    SYSTEM_UP = 1,
    SYSTEM_DOWN = 2,
    SYSTEM_DOUBLE_CLICK = 4
};

#define DOUBLE_CLICK_TIME 300

static mouse data;
static mouse dialog;
static time_millis last_click;

const mouse *mouse_get(void)
{
    return &data;
}

static void clear_mouse_button(mouse_button *button)
{
    button->is_down = 0;
    button->went_down = 0;
    button->went_up = 0;
    button->double_click = 0;
    button->system_change = SYSTEM_NONE;
}

void mouse_set_from_touch(const touch *first, const touch *last)
{
    data.x = first->current_point.x;
    data.y = first->current_point.y;
    data.scrolled = touch_get_scroll();
    data.is_inside_window = !first->has_ended;
    data.is_touch = 1;

    data.left.system_change = SYSTEM_NONE;
    data.right.system_change = SYSTEM_NONE;

    if (touch_is_scroll()) {
        mouse_reset_button_state();
        return;
    }

    data.left.is_down = (!first->has_ended && first->in_use);
    data.left.went_down = first->has_started;
    data.left.went_up = first->has_ended;
    data.left.double_click = touch_was_double_click(first);

    data.right.is_down = (!last->has_ended && last->in_use);
    data.right.went_down = last->has_started;
    data.right.went_up = last->has_ended;

    clear_mouse_button(&data.middle);
}

void mouse_set_position(int x, int y)
{
    if (x != data.x || y != data.y) {
        last_click = 0;
    }
    data.x = x;
    data.y = y;
    data.is_touch = 0;
    data.is_inside_window = 1;
}

void mouse_set_left_down(int down)
{
    data.left.system_change |= down ? SYSTEM_DOWN : SYSTEM_UP;
    data.is_touch = 0;
    data.is_inside_window = 1;
    if (!down) {
        time_millis now = time_get_millis();
        data.left.system_change |= ((last_click < now) && ((now - last_click) <= DOUBLE_CLICK_TIME)) ? SYSTEM_DOUBLE_CLICK : SYSTEM_NONE;
        last_click = now;
    }
}

void mouse_set_middle_down(int down)
{
    data.middle.system_change |= down ? SYSTEM_DOWN : SYSTEM_UP;
    data.is_touch = 0;
    data.is_inside_window = 1;
    last_click = 0;
}


void mouse_set_right_down(int down)
{
    data.right.system_change |= down ? SYSTEM_DOWN : SYSTEM_UP;
    data.is_touch = 0;
    data.is_inside_window = 1;
    last_click = 0;
}

void mouse_set_inside_window(int inside)
{
    data.is_inside_window = inside;
    data.is_touch = 0;
}

static void update_button_state(mouse_button *button)
{
    button->went_down = (button->system_change & SYSTEM_DOWN) == SYSTEM_DOWN;
    button->went_up = (button->system_change & SYSTEM_UP) == SYSTEM_UP;
    button->double_click = (button->system_change & SYSTEM_DOUBLE_CLICK) == SYSTEM_DOUBLE_CLICK;
    button->system_change = SYSTEM_NONE;
    button->is_down = (button->is_down || button->went_down) && !button->went_up;
}

void mouse_determine_button_state(void)
{
    update_button_state(&data.left);
    update_button_state(&data.middle);
    update_button_state(&data.right);
}

void mouse_set_scroll(scroll_state state)
{
    data.scrolled = state;
    data.is_touch = 0;
    data.is_inside_window = 1;
}

void mouse_reset_scroll(void)
{
    data.scrolled = SCROLL_NONE;
}

void mouse_reset_up_state(void)
{
    data.left.went_up = 0;
    data.middle.went_up = 0;
    data.right.went_up = 0;
}

void mouse_reset_button_state(void)
{
    last_click = 0;
    clear_mouse_button(&data.left);
    clear_mouse_button(&data.middle);
    clear_mouse_button(&data.right);
}

const mouse *mouse_in_dialog(const mouse *m)
{
    dialog.left = m->left;
    dialog.middle = m->middle;
    dialog.right = m->right;
    dialog.scrolled = m->scrolled;
    dialog.is_inside_window = m->is_inside_window;
    dialog.is_touch = m->is_touch;

    dialog.x = m->x - screen_dialog_offset_x();
    dialog.y = m->y - screen_dialog_offset_y();
    return &dialog;
}
