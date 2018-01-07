#include "input/mouse.h"

#include "core/debug.h"

#include "Data/Screen.h"

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
    data.left.new_is_down = down;
}

void mouse_set_right_down(int down)
{
    data.right.new_is_down = down;
}

void mouse_set_inside_window(int inside)
{
    data.is_inside_window = inside;
}

static void update_button_state(mouse_button *button)
{
    int was_down = button->is_down;
    button->went_down = 0;
    button->went_up = 0;
    
    button->is_down = button->new_is_down;
    if (button->is_down != was_down) {
        if (button->is_down) {
            button->went_down = 1;
        } else {
            button->went_up = 1;
        }
    }
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

    dialog.x = m->x - Data_Screen.offset640x480.x;
    dialog.y = m->y - Data_Screen.offset640x480.y;
    return &dialog;
}
