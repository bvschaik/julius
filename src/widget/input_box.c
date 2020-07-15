#include "input_box.h"

#include "game/system.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "input/keyboard.h"

void input_box_start(input_box *box, uint8_t *text, int length, int allow_punctuation)
{
    box->text = text;
    box->max_length = length;
    int text_width = (box->width_blocks - 2) * INPUT_BOX_BLOCK_SIZE;
    keyboard_start_capture(text, length, allow_punctuation, text_width, box->font);
    system_keyboard_set_input_rect(box->x, box->y,
            box->width_blocks * INPUT_BOX_BLOCK_SIZE,
            box->height_blocks * INPUT_BOX_BLOCK_SIZE);
}

void input_box_pause(input_box *box)
{
    keyboard_pause_capture();
}

void input_box_resume(input_box *box)
{
    keyboard_resume_capture();
}

void input_box_stop(input_box *box)
{
    box->text = 0;
    keyboard_stop_capture();
    system_keyboard_set_input_rect(0, 0, 0, 0);
}

void input_box_refresh_text(input_box *box)
{
    keyboard_refresh();
}

int input_box_is_accepted(input_box *box)
{
    return keyboard_input_is_accepted();
}

static int is_mouse_inside_input(const mouse *m, const input_box *box)
{
    return m->x >= box->x && m->x < box->x + box->width_blocks  * INPUT_BOX_BLOCK_SIZE &&
           m->y >= box->y && m->y < box->y + box->height_blocks * INPUT_BOX_BLOCK_SIZE;
}

void input_box_draw(const input_box *box)
{
    inner_panel_draw(box->x, box->y, box->width_blocks, box->height_blocks);
    text_capture_cursor(keyboard_cursor_position(), keyboard_offset_start(), keyboard_offset_end());
    int text_x = box->x + 16;
    int text_y = box->y + 10;
    text_draw(box->text, text_x, text_y, box->font, 0);
    text_draw_cursor(text_x, text_y + 1, keyboard_is_insert());
}

int input_box_handle_mouse(const mouse *m, const input_box *box)
{
    if (!m->left.went_up) {
        return 0;
    }
    int selected = is_mouse_inside_input(m, box);
    if (selected) {
        system_keyboard_show(box->text, box->max_length);
    } else {
        system_keyboard_hide();
    }
    return selected;
}
