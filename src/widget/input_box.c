#include "input_box.h"

#include "game/system.h"
#include "graphics/panel.h"

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
    if (selected) {
        system_keyboard_show();
    } else {
        system_keyboard_hide();
    }
    return selected;
}
