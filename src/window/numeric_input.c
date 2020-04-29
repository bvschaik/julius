#include "numeric_input.h"

#include "graphics/color.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "sound/effect.h"

static void button_number(int number, int param2);
static void button_accept(int param1, int param2);
static void button_cancel(int param1, int param2);

static void input_number(int number);
static void input_accept(void);

static generic_button buttons[] = {
    {21, 51, 25, 25, button_number, button_none, 1, 0},
    {51, 51, 25, 25, button_number, button_none, 2, 0},
    {81, 51, 25, 25, button_number, button_none, 3, 0},
    {21, 81, 25, 25, button_number, button_none, 4, 0},
    {51, 81, 25, 25, button_number, button_none, 5, 0},
    {81, 81, 25, 25, button_number, button_none, 6, 0},
    {21, 111, 25, 25, button_number, button_none, 7, 0},
    {51, 111, 25, 25, button_number, button_none, 8, 0},
    {81, 111, 25, 25, button_number, button_none, 9, 0},
    {21, 141, 25, 25, button_number, button_none, 0, 0},
    {51, 141, 55, 25, button_accept, button_none, 1, 0},
    {21, 171, 85, 25, button_cancel, button_none, 1, 0}
};

static struct {
    int x;
    int y;
    int max_digits;
    int max_value;
    void (*callback)(int);

    int num_digits;
    int value;
    int focus_button_id;
} data;

static void init(int x, int y, int max_digits, int max_value, void (*callback)(int))
{
    data.x = x;
    data.y = y;
    data.max_digits = max_digits;
    data.max_value = max_value;
    data.callback = callback;
    data.num_digits = 0;
    data.value = 0;
    data.focus_button_id = 0;
    keyboard_start_capture_numeric(input_number);
}

static void close(void)
{
    keyboard_stop_capture_numeric();
    window_go_back();
}

static void draw_number_button(int x, int y, int number, int is_selected)
{
    color_t color = is_selected ? COLOR_FONT_BLUE : COLOR_BLACK;
    graphics_draw_rect(x, y, 25, 25, color);
    uint8_t number_string[2];
    number_string[0] = '0' + number;
    number_string[1] = 0;
    text_draw_centered(number_string, x, y, 25, FONT_LARGE_PLAIN, color);
}

static void draw_foreground(void)
{
    outer_panel_draw(data.x, data.y, 8, 14);

    graphics_fill_rect(data.x + 16, data.y + 16, 96, 30, COLOR_BLACK);
    if (data.num_digits > 0) {
        text_draw_number_centered_colored(data.value, data.x + 16, data.y + 19, 92, FONT_LARGE_PLAIN, COLOR_FONT_RED);
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

    graphics_draw_rect(data.x + 51, data.y + 141, 55, 25, data.focus_button_id == 11 ? COLOR_FONT_BLUE : COLOR_BLACK);
    lang_text_draw_centered_colored(44, 16, data.x + 51, data.y + 147, 55, FONT_NORMAL_PLAIN,
            data.focus_button_id == 11 ? COLOR_FONT_BLUE : COLOR_BLACK);

    graphics_draw_rect(data.x + 21, data.y + 171, 85, 25, data.focus_button_id == 12 ? COLOR_FONT_BLUE : COLOR_BLACK);
    lang_text_draw_centered_colored(44, 17, data.x + 21, data.y + 177, 85, FONT_NORMAL_PLAIN,
            data.focus_button_id == 12 ? COLOR_FONT_BLUE : COLOR_BLACK);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(m, data.x, data.y, buttons, 12, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        close();
    }
    if (h->enter_pressed) {
        input_accept();
    }
}

static void button_number(int number, int param2)
{
    input_number(number);
}

static void button_accept(int param1, int param2)
{
    input_accept();
}

static void button_cancel(int param1, int param2)
{
    close();
}

static void input_number(int number)
{
    if (data.num_digits < data.max_digits) {
        data.value = data.value * 10 + number;
        data.num_digits++;
        sound_effect_play(SOUND_EFFECT_BUILD);
    }
}

static void input_accept(void)
{
    close();
    if (data.value > data.max_value) {
        data.value = data.max_value;
    }
    data.callback(data.value);
}

void window_numeric_input_show(int x, int y, int max_digits, int max_value, void (*callback)(int))
{
    window_type window = {
        WINDOW_NUMERIC_INPUT,
        0,
        draw_foreground,
        handle_input,
    };
    init(x, y, max_digits, max_value, callback);
    window_show(&window);
}
