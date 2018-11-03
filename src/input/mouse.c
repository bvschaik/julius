#include "input/mouse.h"

#include "graphics/screen.h"

enum {
    SYSTEM_NONE = 0,
    SYSTEM_UP = 1,
    SYSTEM_DOWN = 2
};

static mouse data;
static mouse dialog;

const mouse *mouse_get()
{
    return &data;
}

void mouse_set_position(int x, int y)
{
    data.x = x;
    data.y = y;
}

void mouse_set_left_down(int down)
{
    data.left.system_change |= down ? SYSTEM_DOWN : SYSTEM_UP;
}

void mouse_set_right_down(int down)
{
    data.right.system_change |= down ? SYSTEM_DOWN : SYSTEM_UP;
}

void mouse_set_inside_window(int inside)
{
    data.is_inside_window = inside;
}

static void update_button_state(mouse_button *button)
{
    button->went_down = (button->system_change & SYSTEM_DOWN) == SYSTEM_DOWN;
    button->went_up = (button->system_change & SYSTEM_UP) == SYSTEM_UP;
    button->system_change = SYSTEM_NONE;
}

void mouse_determine_button_state()
{
    update_button_state(&data.left);
    update_button_state(&data.right);
}


void mouse_set_scroll(scroll_state state)
{
    data.scrolled = state;
}

void mouse_reset_up_state()
{
    data.left.went_up = 0;
    data.right.went_up = 0;
}

const mouse *mouse_in_dialog(const mouse *m)
{
    dialog.left = m->left;
    dialog.right = m->right;
    dialog.scrolled = m->scrolled;
    dialog.is_inside_window = m->is_inside_window;

    dialog.x = m->x - screen_dialog_offset_x();
    dialog.y = m->y - screen_dialog_offset_y();
    return &dialog;
}
