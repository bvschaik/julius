#include "hold_festival.h"

#include "assets/assets.h"
#include "city/constants.h"
#include "city/festival.h"
#include "city/finance.h"
#include "city/gods.h"
#include "core/image_group.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/advisors.h"
#include "window/message_dialog.h"

static void button_god(int god, int param2);
static void button_size(int size, int param2);
static void button_help(int param1, int param2);
static void button_close(int param1, int param2);
static void button_hold_festival(int param1, int param2);

static image_button image_buttons_bottom[] = {
    {58, 316, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1},
    {558, 319, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_close, button_none, 0, 0, 1},
    {494, 317, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_close, button_none, 0, 0, 1},
    {452, 317, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_hold_festival, button_none, 1, 0, 1},
};

static generic_button buttons_gods_size[] = {
    {70, 96, 80, 90, button_god, button_none, 0, 0},
    {170, 96, 80, 90, button_god, button_none, 1, 0},
    {270, 96, 80, 90, button_god, button_none, 2, 0},
    {370, 96, 80, 90, button_god, button_none, 3, 0},
    {470, 96, 80, 90, button_god, button_none, 4, 0},
    {102, 216, 430, 26, button_size, button_none, 1, 0},
    {102, 246, 430, 26, button_size, button_none, 2, 0},
    {102, 276, 430, 26, button_size, button_none, 3, 0},
};

static int focus_button_id;
static int focus_image_button_id;

static void draw_buttons(void)
{
    font_t font;
    color_t color;
    int wine_image_id;

    // greying out of buttons
    if (city_finance_out_of_money()) {
        font = FONT_NORMAL_PLAIN;
        color = COLOR_FONT_LIGHT_GRAY;
        wine_image_id = assets_get_image_id("UI", "Grand Festival Wine Disabled");
    } else {
        font = FONT_NORMAL_BLACK;
        color = COLOR_MASK_NONE;
        wine_image_id = image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WINE;
    }
    // small festival
    button_border_draw(102, 216, 430, 26, color == COLOR_MASK_NONE && focus_button_id == 6);
    int width = lang_text_draw_colored(58, 31, 110, 224, font, color);
    lang_text_draw_amount_colored(8, 0, city_festival_small_cost(), 110 + width, 224, font, color);

    // large festival
    button_border_draw(102, 246, 430, 26, color == COLOR_MASK_NONE && focus_button_id == 7);
    width = lang_text_draw_colored(58, 32, 110, 254, font, color);
    lang_text_draw_amount_colored(8, 0, city_festival_large_cost(), 110 + width, 254, font, color);

    if (city_festival_out_of_wine() && !city_finance_out_of_money()) {
        font = FONT_NORMAL_PLAIN;
        color = COLOR_FONT_LIGHT_GRAY;
        wine_image_id = assets_get_image_id("UI", "Grand Festival Wine Disabled");        
    }

    // grand festival
    button_border_draw(102, 276, 430, 26, color == COLOR_MASK_NONE && focus_button_id == 8);
    width = lang_text_draw_colored(58, 33, 110, 284, font, color);
    width += lang_text_draw_amount_colored(8, 0, city_festival_grand_cost(), 110 + width, 284, font, color);
    width += lang_text_draw_amount_colored(8, 10, city_festival_grand_wine(), 120 + width, 284, font, color);
    image_draw(wine_image_id, 120 + width, 279, COLOR_MASK_NONE, SCALE_NONE);
}

static void draw_background(void)
{
    window_advisors_draw_dialog_background();

    graphics_in_dialog();

    outer_panel_draw(48, 48, 34, 20);
    lang_text_draw_centered(58, 25 + city_festival_selected_god(), 48, 60, 544, FONT_LARGE_BLACK);
    for (int god = 0; god < MAX_GODS; god++) {
        if (god == city_festival_selected_god()) {
            button_border_draw(100 * god + 66, 92, 90, 100, 1);
            image_draw(image_group(GROUP_PANEL_WINDOWS) + god + 21, 100 * god + 70, 96, COLOR_MASK_NONE, SCALE_NONE);
        } else {
            image_draw(image_group(GROUP_PANEL_WINDOWS) + god + 16, 100 * god + 70, 96, COLOR_MASK_NONE, SCALE_NONE);
        }
    }
    draw_buttons();

    if (!city_finance_out_of_money()) {
        lang_text_draw_centered(58, 30 + city_festival_selected_size(), 28, 322, 544, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_OUT_OF_MONEY, 28, 322, 544, FONT_NORMAL_BLACK);
    }

    graphics_reset_dialog();
}

static int active_image_buttons(void)
{
    return city_finance_out_of_money() ? 3 : 4;
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (!city_finance_out_of_money()) {
        button_border_draw(102, 216, 430, 26, focus_button_id == 6);
        button_border_draw(102, 246, 430, 26, focus_button_id == 7);
        if (!city_festival_out_of_wine()) {
            button_border_draw(102, 276, 430, 26, focus_button_id == 8);
        }
    }
    image_buttons_draw(0, 0, image_buttons_bottom, active_image_buttons());
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    int handled = 0;
    handled |= image_buttons_handle_mouse(m_dialog, 0, 0, image_buttons_bottom, active_image_buttons(),
        &focus_image_button_id);
    handled |= generic_buttons_handle_mouse(m_dialog, 0, 0, buttons_gods_size, 8, &focus_button_id);
    if (focus_image_button_id) {
        focus_button_id = 0;
    }
    if (!handled && input_go_back_requested(m, h)) {
        window_advisors_show();
    }
}

static void button_god(int god, int param2)
{
    city_festival_select_god(god);
    window_invalidate();
}

static void button_size(int size, int param2)
{
    if (!city_finance_out_of_money()) {
        if (city_festival_select_size(size)) {
            window_invalidate();
        }
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_ADVISOR_RELIGION, 0);
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
    city_festival_schedule();
    window_advisors_show();
}

static void get_tooltip(tooltip_context *c)
{
    if (!focus_image_button_id && (!focus_button_id || focus_button_id > 5)) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    // image buttons
    switch (focus_image_button_id) {
        case 1: c->text_id = 1; break;
        case 2: c->text_id = 2; break;
        case 3: c->text_id = 114; break;
        case 4: c->text_id = 113; break;
    }
    // gods
    switch (focus_button_id) {
        case 1: c->text_id = 115; break;
        case 2: c->text_id = 116; break;
        case 3: c->text_id = 117; break;
        case 4: c->text_id = 118; break;
        case 5: c->text_id = 119; break;
    }
}

void window_hold_festival_show(void)
{
    window_type window = {
        WINDOW_HOLD_FESTIVAL,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}
