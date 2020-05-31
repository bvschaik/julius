#include "labor_priority.h"

#include "city/labor.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"

#define MIN_DIALOG_WIDTH 320

static void button_set_priority(int new_priority, int param2);

static struct {
    int category;
    int max_items;
    int focus_button_id;
} data;

static generic_button priority_buttons[] = {
    {180, 256, 280, 25, button_set_priority, button_none, 0, 0}, // no prio
    {178, 221, 27, 27, button_set_priority, button_none, 1, 0},
    {210, 221, 27, 27, button_set_priority, button_none, 2, 0},
    {242, 221, 27, 27, button_set_priority, button_none, 3, 0},
    {274, 221, 27, 27, button_set_priority, button_none, 4, 0},
    {306, 221, 27, 27, button_set_priority, button_none, 5, 0},
    {338, 221, 27, 27, button_set_priority, button_none, 6, 0},
    {370, 221, 27, 27, button_set_priority, button_none, 7, 0},
    {402, 221, 27, 27, button_set_priority, button_none, 8, 0},
    {434, 221, 27, 27, button_set_priority, button_none, 9, 0},
};

static void init(int category)
{
    data.category = category;
    data.max_items = city_labor_max_selectable_priority(category);
}

static int get_dialog_width(void)
{
    int title_width = lang_text_get_width(50, 25, FONT_LARGE_BLACK);
    int rclick_width = lang_text_get_width(13, 3, FONT_NORMAL_BLACK);
    int dialog_width = 16 + (title_width > rclick_width ? title_width : rclick_width);
    if (dialog_width < MIN_DIALOG_WIDTH) dialog_width = MIN_DIALOG_WIDTH;
    if (dialog_width % 16 != 0) {
        // make sure the width is a multiple of 16
        dialog_width += 16 - dialog_width % 16;
    }
    return dialog_width;
}

static void draw_background(void)
{
    window_draw_underlying_window();

    graphics_in_dialog();

    int dialog_width = get_dialog_width();
    int dialog_x = 160 - (dialog_width - MIN_DIALOG_WIDTH) / 2;
    outer_panel_draw(dialog_x, 176, dialog_width / 16, 9);
    lang_text_draw_centered(50, 25, 160, 185, 320, FONT_LARGE_BLACK);
    for (int i = 0; i < 9; i++) {
        graphics_draw_rect(178 + 32 * i, 221, 27, 27, COLOR_BLACK);
        lang_text_draw_centered(50, 27 + i, 178 + 32 * i, 224, 27, FONT_LARGE_BLACK);
        if (i >= data.max_items) {
            graphics_shade_rect(179 + 32 * i, 222, 25, 25, 1);
        }
    }

    graphics_draw_rect(180, 256, 280, 25, COLOR_BLACK);
    lang_text_draw_centered(50, 26, 148, 263, 344, FONT_NORMAL_BLACK);
    lang_text_draw_centered(13, 3, 128, 296, 384, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    color_t color;
    for (int i = 0; i < 9; i++) {
        color = COLOR_BLACK;
        if (i == data.focus_button_id - 2) {
            color = COLOR_RED;
        }
        graphics_draw_rect(178 + 32 * i, 221, 27, 27, color);
    }
    color = COLOR_BLACK;
    if (data.focus_button_id == 1) {
        color = COLOR_RED;
    }
    graphics_draw_rect(180, 256, 280, 25, color);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, priority_buttons, 1 + data.max_items, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void button_set_priority(int new_priority, int param2)
{
    city_labor_set_priority(data.category, new_priority);
    window_go_back();
}

static void get_tooltip(tooltip_context *c)
{
    if (!data.focus_button_id) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    if (data.focus_button_id == 1) {
        c->text_id = 92;
    } else {
        c->text_id = 93;
    }
}

void window_labor_priority_show(int category)
{
    window_type window = {
        WINDOW_LABOR_PRIORITY,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(category);
    window_show(&window);
}
