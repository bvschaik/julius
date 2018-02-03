#include "labor_priority.h"

#include "city/labor.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "window/advisor/labor.h"

#include "UI/Tooltip.h"

static void button_set_priority(int new_priority, int param2);

static struct {
    int category;
    int max_items;
    int focus_button_id;
} data;

static generic_button priority_buttons[] = {
    {180, 256, 460, 281, GB_IMMEDIATE, button_set_priority, button_none, 0, 0}, // no prio
    {178, 221, 205, 248, GB_IMMEDIATE, button_set_priority, button_none, 1, 0},
    {210, 221, 237, 248, GB_IMMEDIATE, button_set_priority, button_none, 2, 0},
    {242, 221, 269, 248, GB_IMMEDIATE, button_set_priority, button_none, 3, 0},
    {274, 221, 301, 248, GB_IMMEDIATE, button_set_priority, button_none, 4, 0},
    {306, 221, 333, 248, GB_IMMEDIATE, button_set_priority, button_none, 5, 0},
    {338, 221, 365, 248, GB_IMMEDIATE, button_set_priority, button_none, 6, 0},
    {370, 221, 397, 248, GB_IMMEDIATE, button_set_priority, button_none, 7, 0},
    {402, 221, 429, 248, GB_IMMEDIATE, button_set_priority, button_none, 8, 0},
    {434, 221, 461, 248, GB_IMMEDIATE, button_set_priority, button_none, 9, 0},
};

static void init(int category)
{
    data.category = category;
    data.max_items = city_labor_max_selectable_priority(category);
}

static void draw_background()
{
    graphics_in_dialog();

    window_advisor_labor_draw_dialog_background();

    outer_panel_draw(160, 176, 20, 9);
    lang_text_draw_centered(50, 25, 160, 185, 320, FONT_LARGE_BLACK);
    for (int i = 0; i < 9; i++) {
        graphics_draw_rect(178 + 32 * i, 221, 27, 27, COLOR_BLACK);
        lang_text_draw_centered(50, 27 + i, 178 + 32 * i, 224, 27, FONT_LARGE_BLACK);
        if (i >= data.max_items) {
            graphics_shade_rect(179 + 32 * i, 222, 25, 25, 1);
        }
    }

    graphics_draw_rect(180, 256, 280, 25, COLOR_BLACK);
    lang_text_draw_centered(50, 26, 180, 263, 280, FONT_NORMAL_BLACK);
    lang_text_draw_centered(13, 3, 160, 296, 320, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

static void draw_foreground()
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

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        window_advisors_show();
    } else {
        generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, priority_buttons, 1 + data.max_items, &data.focus_button_id);
    }
}

static void button_set_priority(int new_priority, int param2)
{
    city_labor_set_priority(data.category, new_priority);
    window_advisors_show();
}

static void get_tooltip(struct TooltipContext *c)
{
    if (!data.focus_button_id) {
        return;
    }
    c->type = TooltipType_Button;
    if (data.focus_button_id == 1) {
        c->textId = 92;
    } else {
        c->textId = 93;
    }
}

void window_labor_priority_show(int category)
{
    window_type window = {
        Window_LaborPriorityDialog,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    init(category);
    window_show(&window);
}
