#include "donate_to_city.h"

#include "city/finance.h"
#include "core/calc.h"
#include "game/resource.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "window/advisors.h"

#include "Data/CityInfo.h"

static void button_set_amount(int amount_id, int param2);
static void button_donate(int param1, int param2);
static void button_cancel(int param1, int param2);
static void arrow_button_amount(int is_down, int param2);

static generic_button buttons[] = {
    {336, 283, 496, 303, GB_IMMEDIATE, button_cancel, button_none, 0, 0},
    {144, 283, 304, 303, GB_IMMEDIATE, button_donate, button_none, 0, 0},
    {128, 216, 192, 236, GB_IMMEDIATE, button_set_amount, button_none, 0, 0},
    {208, 216, 272, 236, GB_IMMEDIATE, button_set_amount, button_none, 1, 0},
    {288, 216, 352, 236, GB_IMMEDIATE, button_set_amount, button_none, 2, 0},
    {368, 216, 432, 236, GB_IMMEDIATE, button_set_amount, button_none, 3, 0},
    {448, 216, 512, 236, GB_IMMEDIATE, button_set_amount, button_none, 4, 0},
};

static arrow_button arrow_buttons[] = {
    {240, 242, 17, 24, arrow_button_amount, 1, 0},
    {264, 242, 15, 24, arrow_button_amount, 0, 0},
};

static int focus_button_id;
static int focus_arrow_button_id;

static void init()
{
    if (Data_CityInfo.donateAmount > Data_CityInfo.personalSavings) {
        Data_CityInfo.donateAmount = Data_CityInfo.personalSavings;
    }
}

static void draw_background()
{
    window_advisors_draw_dialog_background();

    graphics_in_dialog();

    outer_panel_draw(64, 160, 32, 10);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_DENARII, 96, 176);
    lang_text_draw_centered(52, 16, 80, 176, 496, FONT_LARGE_BLACK);

    inner_panel_draw(112, 208, 26, 4);

    text_draw_number_centered(0, 124, 221, 64, FONT_NORMAL_WHITE);
    text_draw_number_centered(500, 204, 221, 64, FONT_NORMAL_WHITE);
    text_draw_number_centered(2000, 284, 221, 64, FONT_NORMAL_WHITE);
    text_draw_number_centered(5000, 364, 221, 64, FONT_NORMAL_WHITE);
    lang_text_draw_centered(52, 19, 444, 221, 64, FONT_NORMAL_WHITE);

    lang_text_draw(52, 17, 128, 248, FONT_NORMAL_WHITE);
    text_draw_number(Data_CityInfo.donateAmount, '@', " ", 316, 248, FONT_NORMAL_WHITE);

    lang_text_draw_centered(13, 4, 336, 288, 160, FONT_NORMAL_BLACK);
    lang_text_draw_centered(52, 18, 144, 288, 160, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground()
{
    graphics_in_dialog();

    button_border_draw(128, 216, 64, 20, focus_button_id == 3);
    button_border_draw(208, 216, 64, 20, focus_button_id == 4);
    button_border_draw(288, 216, 64, 20, focus_button_id == 5);
    button_border_draw(368, 216, 64, 20, focus_button_id == 6);
    button_border_draw(448, 216, 64, 20, focus_button_id == 7);

    button_border_draw(336, 283, 160, 20, focus_button_id == 1);
    button_border_draw(144, 283, 160, 20, focus_button_id == 2);

    arrow_buttons_draw(0, 0, arrow_buttons, 2);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    focus_arrow_button_id = 0;
    if (m->right.went_up) {
        window_advisors_show();
    } else {
        const mouse *m_dialog = mouse_in_dialog(m);
        if (!generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 7, &focus_button_id)) {
            focus_arrow_button_id = arrow_buttons_handle_mouse(m_dialog, 0, 0, arrow_buttons, 2);
        }
    }
}

static void button_set_amount(int amount_id, int param2)
{
    int amount;
    switch (amount_id) {
        case 0: amount = 0; break;
        case 1: amount = 500; break;
        case 2: amount = 2000; break;
        case 3: amount = 5000; break;
        case 4: amount = 1000000; break;
        default: return;
    }
    Data_CityInfo.donateAmount = calc_bound(amount, 0, Data_CityInfo.personalSavings);
    window_invalidate();
}

static void button_donate(int param1, int param2)
{
    city_finance_process_donation(Data_CityInfo.donateAmount);
    Data_CityInfo.personalSavings -= Data_CityInfo.donateAmount;
    city_finance_calculate_totals();
    window_advisors_show();
}

static void button_cancel(int param1, int param2)
{
    window_advisors_show();
}

static void arrow_button_amount(int is_down, int param2)
{
    if (is_down) {
        Data_CityInfo.donateAmount -= 10;
    } else {
        Data_CityInfo.donateAmount += 10;
    }
    Data_CityInfo.donateAmount = calc_bound(Data_CityInfo.donateAmount, 0, Data_CityInfo.personalSavings);
    window_invalidate();
}

static void get_tooltip(tooltip_context *c)
{
    if (!focus_button_id && !focus_arrow_button_id) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    if (focus_button_id == 1) {
        c->text_id = 98;
    } else if (focus_button_id == 2) {
        c->text_id = 99;
    } else if (focus_button_id) {
        c->text_id = 100;
    } else if (focus_arrow_button_id) {
        c->text_id = 101;
    }
}

void window_donate_to_city_show()
{
    window_type window = {
        Window_DonateToCityDialog,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    init();
    window_show(&window);
}
