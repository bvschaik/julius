#include "input_box.h"

#include "graphics/panel.h"
#include "platform/virtual_keyboard.h"

static int is_mouse_inside_input(const mouse *m, const input_box *box)
{
    return m->x >= box->x && m->x < box->x + box->width_blocks  * INPUT_BOX_BLOCK_SIZE &&
           m->y >= box->y && m->y < box->y + box->height_blocks * INPUT_BOX_BLOCK_SIZE;
}

void input_box_draw(const input_box *box)
{
    inner_panel_draw(box->x, box->y, box->width_blocks, box->height_blocks);
}

int input_box_handle_mouse(const mouse *m, const input_box *box)
{
    if (!m->left.went_up) {
        return 0;
    }
    int selected = is_mouse_inside_input(m, box);
    if (!platform_virtual_keyboard_showing() && selected) {
        platform_virtual_keyboard_show();
    } else if (platform_virtual_keyboard_showing() && !selected) {
        platform_virtual_keyboard_hide();
    }
    return selected;
}
