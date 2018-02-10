#include "hold_festival.h"

#include "building/warehouse.h"
#include "city/constants.h"
#include "city/finance.h"
#include "city/gods.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "window/advisors.h"
#include "window/message_dialog.h"

#include "Data/CityInfo.h"

static void button_god(int god, int param2);
static void button_size(int size, int param2);
static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_hold_festival(int param1, int param2);

static image_button imageButtonsBottom[] = {
    {58, 316, 27, 27, IB_NORMAL, 134, 0, button_help, button_none, 0, 0, 1},
    {558, 319, 24, 24, IB_NORMAL, 134, 4, button_close, button_none, 0, 0, 1},
    {358, 317, 34, 34, IB_NORMAL, 96, 0, button_hold_festival, button_none, 1, 0, 1},
    {400, 317, 34, 34, IB_NORMAL, 96, 4, button_close, button_none, 0, 0, 1},
};

static generic_button buttonsGodsSize[] = {
    {70, 96, 150, 186, GB_IMMEDIATE, button_god, button_none, 0, 0},
    {170, 96, 250, 186, GB_IMMEDIATE, button_god, button_none, 1, 0},
    {270, 96, 350, 186, GB_IMMEDIATE, button_god, button_none, 2, 0},
    {370, 96, 450, 186, GB_IMMEDIATE, button_god, button_none, 3, 0},
    {470, 96, 550, 186, GB_IMMEDIATE, button_god, button_none, 4, 0},
    {102, 216, 532, 242, GB_IMMEDIATE, button_size, button_none, 1, 0},
    {102, 246, 532, 272, GB_IMMEDIATE, button_size, button_none, 2, 0},
    {102, 276, 532, 302, GB_IMMEDIATE, button_size, button_none, 3, 0},
};

static int focus_button_id;
static int focus_image_button_id;

static void draw_buttons()
{
    int width;

    // small festival
    button_border_draw(102, 216, 430, 26, focus_button_id == 6);
    width = lang_text_draw(58, 31, 110, 224, FONT_NORMAL_BLACK);
    lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostSmall, 110 + width, 224, FONT_NORMAL_BLACK);

    // large festival
    button_border_draw(102, 246, 430, 26, focus_button_id == 7);
    width = lang_text_draw(58, 32, 110, 254, FONT_NORMAL_BLACK);
    lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostLarge, 110 + width, 254, FONT_NORMAL_BLACK);

    // grand festival
    button_border_draw(102, 276, 430, 26, focus_button_id == 8);
    width = lang_text_draw(58, 33, 110, 284, FONT_NORMAL_BLACK);
    width += lang_text_draw_amount(8, 0, Data_CityInfo.festivalCostGrand,
        110 + width, 284, FONT_NORMAL_BLACK);
    width += lang_text_draw_amount(8, 10, Data_CityInfo.festivalWineGrand,
        120 + width, 284, FONT_NORMAL_BLACK);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WINE,
        120 + width, 279);
    
    // greying out of buttons
    if (city_finance_out_of_money()) {
        graphics_shade_rect(104, 218, 426, 22, 0);
        graphics_shade_rect(104, 248, 426, 22, 0);
        graphics_shade_rect(104, 278, 426, 22, 0);
    } else if (Data_CityInfo.festivalNotEnoughWine) {
        graphics_shade_rect(104, 278, 426, 22, 0);
    }
}

static void draw_background()
{
    window_advisors_draw_dialog_background();

    graphics_in_dialog();

    outer_panel_draw(48, 48, 34, 20);
    lang_text_draw_centered(58, 25 + Data_CityInfo.festivalGod, 48, 60, 544, FONT_LARGE_BLACK);
    for (int god = 0; god < MAX_GODS; god++) {
        if (god == Data_CityInfo.festivalGod) {
            button_border_draw(100 * god + 66, 92, 90, 100, 1);
            image_draw(image_group(GROUP_PANEL_WINDOWS) + god + 21, 100 * god + 70, 96);
        } else {
            image_draw(image_group(GROUP_PANEL_WINDOWS) + god + 16, 100 * god + 70, 96);
        }
    }
    draw_buttons();
    lang_text_draw(58, 30 + Data_CityInfo.festivalSize, 180, 322, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_foreground()
{
    graphics_in_dialog();
    draw_buttons();
    image_buttons_draw(0, 0, imageButtonsBottom, 4);
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_up) {
        window_advisors_show();
        return;
    }

    const mouse *m_dialog = mouse_in_dialog(m);
    image_buttons_handle_mouse(m_dialog, 0, 0, imageButtonsBottom, 4, &focus_image_button_id);
    generic_buttons_handle_mouse(m_dialog, 0, 0, buttonsGodsSize, 8, &focus_button_id);
    if (focus_image_button_id) {
        focus_button_id = 0;
    }
}

static void button_god(int god, int param2)
{
    Data_CityInfo.festivalGod = god;
    window_invalidate();
}

static void button_size(int size, int param2)
{
    if (!city_finance_out_of_money()) {
        if (size != FESTIVAL_GRAND || !Data_CityInfo.festivalNotEnoughWine) {
            Data_CityInfo.festivalSize = size;
            window_invalidate();
        }
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MessageDialog_EntertainmentAdvisor, 1);
}

static void button_close(int param1, int param2)
{
    window_advisors_show();
}

static void button_hold_festival(int param1, int param2)
{
    if (city_finance_out_of_money()) {
        return;
    }
    city_gods_schedule_festival();
    window_advisors_show();
}

static void get_tooltip(struct TooltipContext *c)
{
    if (!focus_image_button_id && (!focus_button_id || focus_button_id > 5)) {
        return;
    }
    c->type = TooltipType_Button;
    // image buttons
    switch (focus_image_button_id) {
        case 1: c->textId = 1; break;
        case 2: c->textId = 2; break;
        case 3: c->textId = 113; break;
        case 4: c->textId = 114; break;
    }
    // gods
    switch (focus_button_id) {
        case 1: c->textId = 115; break;
        case 2: c->textId = 116; break;
        case 3: c->textId = 117; break;
        case 4: c->textId = 118; break;
        case 5: c->textId = 119; break;
    }
}

void window_hold_festival_show()
{
    window_type window = {
        Window_HoldFestivalDialog,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    window_show(&window);
}
