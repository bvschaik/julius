#include "numeric_input.h"

#include "core/string.h"
#include "game/system.h"
#include "graphics/color.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "sound/effect.h"

#include <limits.h>

#define WIDTH_BLOCKS 8
#define HEIGHT_BLOCKS 15

static void button_number(const generic_button *button);
static void button_accept(const generic_button *button);
static void button_negative(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_cancel(const generic_button *button);

static void input_number(int number, int minus);
static void input_accept(void);
static void input_delete(void);

static generic_button buttons[] = {
    {21, 51, 25, 25, button_number, 0, 1},
    {51, 51, 25, 25, button_number, 0, 2},
    {81, 51, 25, 25, button_number, 0, 3},
    {21, 81, 25, 25, button_number, 0, 4},
    {51, 81, 25, 25, button_number, 0, 5},
    {81, 81, 25, 25, button_number, 0, 6},
    {21, 111, 25, 25, button_number, 0, 7},
    {51, 111, 25, 25, button_number, 0, 8},
    {81, 111, 25, 25, button_number, 0, 9},
    {21, 141, 25, 25, button_number, 0, 0},
    {51, 141, 25, 25, button_negative},
    {51, 171, 56, 25, button_accept},
    {21, 201, 25, 25, button_delete},
    {51, 201, 56, 25, button_cancel}
};

static struct {
    int x;
    int y;
    unsigned int max_digits;
    int min_value;
    int max_value;
    void (*callback)(int);

    unsigned int num_digits;
    int value;
    int is_negative_value;
    unsigned int focus_button_id;
} data;

static void determine_offsets(int x, int y, const generic_button *button)
{
    x += screen_dialog_offset_x();
    y += screen_dialog_offset_y();

    if (!button) {
        data.x = x;
        data.y = y;
        return;
    }

    data.x = x + button->x;
    data.y = y + button->y;

    if (data.x + WIDTH_BLOCKS * BLOCK_SIZE > screen_width()) {
        data.x -= WIDTH_BLOCKS * BLOCK_SIZE - button->width;
    }

    if (data.y + button->height + HEIGHT_BLOCKS * BLOCK_SIZE > screen_height()) {
        data.y -= HEIGHT_BLOCKS * BLOCK_SIZE;
    } else {
        data.y += button->height;
    }
}

static void init(int x, int y, const generic_button *button, int max_digits,
    int min_value, int max_value, void (*callback)(int))
{
    data.is_negative_value = 0;

    determine_offsets(x, y, button);

    data.max_digits = max_digits;
    if (!min_value && !max_value) {
        data.min_value = INT_MIN;
        data.max_value = INT_MAX;
    } else {
        data.min_value = min_value;
        data.max_value = max_value;
    }
    data.callback = callback;
    data.num_digits = 0;
    data.value = 0;
    data.focus_button_id = 0;
    keyboard_start_capture_numeric(input_number);
    system_keyboard_set_input_rect(data.x + 16, data.y + 16, 96, 30);
}

static void close(void)
{
    keyboard_stop_capture_numeric();
    system_keyboard_set_input_rect(0, 0, 0, 0);
    window_go_back();
}

static void draw_number_button(int x, int y, int number, int is_selected)
{
    color_t color = is_selected ? COLOR_FONT_BLUE : COLOR_BLACK;
    graphics_draw_rect(x, y - 1, 26, 26, color);
    uint8_t number_string[2];
    number_string[0] = '0' + number;
    number_string[1] = 0;
    text_draw_centered(number_string, x, y, 27, FONT_LARGE_PLAIN, color);
}

static void draw_foreground(void)
{
    outer_panel_draw(data.x, data.y, WIDTH_BLOCKS, HEIGHT_BLOCKS);

    graphics_fill_rect(data.x + 16, data.y + 16, 96, 30, COLOR_BLACK);
    if (data.num_digits > 0) {
        text_draw_number_centered_colored(data.value, data.x + 19, data.y + 19, 92, FONT_LARGE_PLAIN, COLOR_FONT_RED);
    }

    draw_number_button(data.x + 21, data.y + 51, 1, data.focus_button_id == 1);
    draw_number_button(data.x + 51, data.y + 51, 2, data.focus_button_id == 2);
    draw_number_button(data.x + 81, data.y + 51, 3, data.focus_button_id == 3);
    draw_number_button(data.x + 21, data.y + 81, 4, data.focus_button_id == 4);
    draw_number_button(data.x + 51, data.y + 81, 5, data.focus_button_id == 5);
    draw_number_button(data.x + 81, data.y + 81, 6, data.focus_button_id == 6);
    draw_number_button(data.x + 21, data.y + 111, 7, data.focus_button_id == 7);
    draw_number_button(data.x + 51, data.y + 111, 8, data.focus_button_id == 8);
    draw_number_button(data.x + 81, data.y + 111, 9, data.focus_button_id == 9);
    draw_number_button(data.x + 21, data.y + 141, 0, data.focus_button_id == 10);

    if (data.min_value < 0) {
        graphics_draw_rect(data.x + 51, data.y + 141, 25, 25,
            data.focus_button_id == 11 ? COLOR_FONT_BLUE : COLOR_BLACK);
        text_draw_centered(string_from_ascii("-"), data.x + 51, data.y + 147, 25, FONT_NORMAL_PLAIN,
            data.focus_button_id == 11 ? COLOR_FONT_BLUE : COLOR_BLACK);
    }

    graphics_draw_rect(data.x + 51, data.y + 171, 56, 25, data.focus_button_id == 12 ? COLOR_FONT_BLUE : COLOR_BLACK);
    lang_text_draw_centered_colored(44, 16, data.x + 52, data.y + 177, 55, FONT_NORMAL_PLAIN,
            data.focus_button_id == 12 ? COLOR_FONT_BLUE : COLOR_BLACK);

    if (data.focus_button_id == 13) {
        graphics_fill_rect(data.x + 21, data.y + 201, 25, 25, COLOR_RED);
    } else {
        graphics_draw_rect(data.x + 21, data.y + 201, 25, 25, COLOR_BLACK);
    }

    text_draw_centered(string_from_ascii("X"), data.x + 22, data.y + 207, 25, FONT_NORMAL_PLAIN,
        data.focus_button_id == 13 ? COLOR_WHITE : COLOR_RED);
    graphics_draw_rect(data.x + 51, data.y + 201, 56, 25, data.focus_button_id == 14 ? COLOR_FONT_BLUE : COLOR_BLACK);
    lang_text_draw_centered_colored(44, 17, data.x + 52, data.y + 207, 55, FONT_NORMAL_PLAIN,
            data.focus_button_id == 14 ? COLOR_FONT_BLUE : COLOR_BLACK);
}

static int click_outside_window(const mouse *m)
{
    int width = WIDTH_BLOCKS * BLOCK_SIZE;
    int height = HEIGHT_BLOCKS * BLOCK_SIZE;

    return m->left.went_up && (m->x < data.x || m->x >= data.x + width || m->y < data.y || m->y >= data.y + height);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(m, data.x, data.y, buttons, 14, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h) || click_outside_window(m)) {
        close();
    }
    if (h->enter_pressed) {
        input_accept();
    }
    if (h->delete_pressed || h->backspace_pressed) {
        input_delete();
    }
}

static void button_number(const generic_button *button)
{
    int number = button->parameter1;
    input_number(number, 0);
}

static void button_accept(const generic_button *button)
{
    input_accept();
}

static void input_negative(void)
{
    if (data.value == 0) {
        data.is_negative_value = !data.is_negative_value;
    } else {
        data.value *= -1;
        data.is_negative_value = data.value < 0;
    }
}

static void button_negative(const generic_button *button)
{
    input_negative();
}

static void button_delete(const generic_button *button)
{
    input_delete();
}

static void button_cancel(const generic_button *button)
{
    close();
}

static void input_number(int number, int minus)
{
    if (minus) {
        input_negative();
        return;
    }
    if (data.is_negative_value) {
        number *= -1;
    }
    if (data.num_digits < data.max_digits) {
        data.value = data.value * 10 + number;
        data.num_digits++;
        sound_effect_play(SOUND_EFFECT_BUILD);
    }
    if (data.is_negative_value && data.value > 0) {
        data.value *= -1;
    }
}

static void input_accept(void)
{
    close();
    if (data.value > data.max_value) {
        data.value = data.max_value;
    }
    if (data.value < data.min_value) {
        data.value = data.min_value;
    }
    data.callback(data.value);
}

static void input_delete(void)
{
    if (data.num_digits > 0) {
        data.value = data.value / 10;
        data.num_digits--;
        sound_effect_play(SOUND_EFFECT_BUILD);
    }
}

void window_numeric_input_show(int x, int y, const generic_button *button,
    int max_digits, int max_value, void (*callback)(int))
{
    window_numeric_input_bound_show(x, y, button, max_digits, 0, max_value, callback);
}

void window_numeric_input_bound_show(int x, int y, const generic_button *button, int max_digits,
    int min_value, int max_value, void (*callback)(int))
{
    window_type window = {
        WINDOW_NUMERIC_INPUT,
        window_draw_underlying_window,
        draw_foreground,
        handle_input,
    };
    init(x, y, button, max_digits, min_value, max_value, callback);
    window_show(&window);
}
