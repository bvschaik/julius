#include "input_box.h"

#include "core/image_group.h"
#include "core/string.h"
#include "game/system.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "input/keyboard.h"

#include <stdlib.h>

#define CLEAR_BUTTON_WIDTH 43

static void button_clear_text(int param1, int param2);

static uint8_t *text_to_clear;

static image_button clear_text_button = {
    0, 0, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_clear_text, button_none, 0, 0, 1
};

void input_box_start(input_box *box)
{
    int text_width = (box->width_blocks - 2) * BLOCK_SIZE - 35;
    keyboard_start_capture(box->text, box->text_length, box->allow_punctuation, text_width, box->font);
    clear_text_button.x_offset = box->x + box->width_blocks * 16;
    if (!box->put_clear_button_outside_box) {
        clear_text_button.x_offset -= CLEAR_BUTTON_WIDTH;
    }
    clear_text_button.y_offset = box->y + 3;
    system_keyboard_set_input_rect(box->x, box->y,
        box->width_blocks * BLOCK_SIZE - 35, box->height_blocks * BLOCK_SIZE);
    if (box->on_change) {
        box->old_text = malloc(sizeof(uint8_t) * box->text_length);
        string_copy(box->text, box->old_text, box->text_length);
    }
}

void input_box_pause(void)
{
    keyboard_pause_capture();
}

void input_box_resume(void)
{
    keyboard_resume_capture();
}

void input_box_stop(input_box *box)
{
    free(box->old_text);
    box->old_text = 0;
    keyboard_stop_capture();
    system_keyboard_set_input_rect(0, 0, 0, 0);
}

void input_box_refresh_text(input_box *box)
{
    if (box->old_text) {
        string_copy(box->text, box->old_text, box->text_length);
    }
    keyboard_refresh();
}

int input_box_is_accepted(void)
{
    return keyboard_input_is_accepted();
}

static int is_mouse_inside_input(const mouse *m, const input_box *box)
{
    return m->x >= box->x && m->x < box->x + box->width_blocks * BLOCK_SIZE &&
        m->y >= box->y && m->y < box->y + box->height_blocks * BLOCK_SIZE;
}

void input_box_draw(const input_box *box)
{
    inner_panel_draw(box->x, box->y, box->width_blocks, box->height_blocks);
    text_capture_cursor(keyboard_cursor_position(), keyboard_offset_start(), keyboard_offset_end());
    int text_x = box->x + 16;
    int text_y = box->y + 10;
    text_draw(box->text, text_x, text_y, box->font, 0);
    text_draw_cursor(text_x, text_y + 1, keyboard_is_insert());

    if (!*box->text && box->placeholder) {
        text_draw(box->placeholder, text_x + 1, text_y + 1, FONT_NORMAL_PLAIN, COLOR_BLACK);
        text_draw(box->placeholder, text_x, text_y, FONT_NORMAL_PLAIN, COLOR_FONT_LIGHT_GRAY);
    }

    image_buttons_draw(0, 0, &clear_text_button, 1);
}

int input_box_handle_mouse(const mouse *m, const input_box *box)
{
    text_to_clear = box->text;
    if (image_buttons_handle_mouse(m, 0, 0, &clear_text_button, 1, 0)) {
        return 1;
    }
    if (box->old_text && box->on_change && !string_equals(box->old_text, box->text)) {
        box->on_change(string_equals_until(box->old_text, box->text, string_length(box->old_text)));
        string_copy(box->text, box->old_text, box->text_length);
    }
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

static void button_clear_text(int param1, int param2)
{
    if (text_to_clear) {
        *text_to_clear = 0;
        keyboard_refresh();
    }
}
