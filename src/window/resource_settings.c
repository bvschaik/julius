#include "resource_settings.h"

#include "building/count.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/image_group.h"
#include "empire/city.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/building.h"
#include "window/advisor/trade.h"
#include "window/message_dialog.h"

static void button_help(int param1, int param2);
static void button_ok(int param1, int param2);
static void button_export_up_down(int is_down, int param2);

static void button_toggle_industry(int param1, int param2);
static void button_toggle_trade(int param1, int param2);
static void button_toggle_stockpile(int param1, int param2);

static image_button resource_image_buttons[] = {
    {58, 332, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1},
    {558, 335, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_ok, button_none, 0, 0, 1}
};

static arrow_button resource_arrow_buttons[] = {
    {314, 215, 17, 24, button_export_up_down, 1, 0},
    {338, 215, 15, 24, button_export_up_down, 0, 0}
};

static generic_button resource_generic_buttons[] = {
    {98, 250, 432, 30, button_toggle_industry, button_none, 0, 0},
    {98, 212, 432, 30, button_toggle_trade, button_none, 0, 0},
    {98, 288, 432, 50, button_toggle_stockpile, button_none, 0, 0},
};

static struct {
    resource_type resource;
    int focus_button_id;
} data;

static void init(resource_type resource)
{
    data.resource = resource;
}

static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    window_advisor_trade_draw_dialog_background();
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(48, 128, 34, 15);
    int image_offset = data.resource + resource_image_offset(data.resource, RESOURCE_IMAGE_ICON);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + image_offset, 58, 136);

    lang_text_draw(23, data.resource, 92, 137, FONT_LARGE_BLACK);

    if (empire_can_produce_resource(data.resource)) {
        int total_buildings = building_count_industry_total(data.resource);
        int active_buildings = building_count_industry_active(data.resource);
        if (building_count_industry_total(data.resource) <= 0) {
            lang_text_draw(54, 7, 98, 172, FONT_NORMAL_BLACK);
        }
        else if (city_resource_is_mothballed(data.resource)) {
            int width = text_draw_number(total_buildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
            if (total_buildings == 1) {
                lang_text_draw(54, 10, 98 + width, 172, FONT_NORMAL_BLACK);
            }
            else {
                lang_text_draw(54, 11, 98 + width, 172, FONT_NORMAL_BLACK);
            }
        }
        else if (total_buildings == active_buildings) {
            // not mothballed, all working
            int width = text_draw_number(total_buildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
            if (total_buildings == 1) {
                lang_text_draw(54, 8, 98 + width, 172, FONT_NORMAL_BLACK);
            }
            else {
                lang_text_draw(54, 9, 98 + width, 172, FONT_NORMAL_BLACK);
            }
        }
        else {
            // not mothballed, some working
            int width = text_draw_number(active_buildings, '@', " ", 98, 172, FONT_NORMAL_BLACK);
            width += lang_text_draw(54, 12, 98 + width, 172, FONT_NORMAL_BLACK);
            width += text_draw_number(total_buildings - active_buildings, '@', " ", 98 + width, 172, FONT_NORMAL_BLACK);
            if (active_buildings == 1) {
                lang_text_draw(54, 13, 98 + width, 172, FONT_NORMAL_BLACK);
            }
            else {
                lang_text_draw(54, 14, 98 + width, 172, FONT_NORMAL_BLACK);
            }
        }
    }
    else if (data.resource != RESOURCE_MEAT || !scenario_building_allowed(BUILDING_WHARF)) {
        // we cannot produce this good
        lang_text_draw(54, 25, 98, 172, FONT_NORMAL_BLACK);
    }

    int width = lang_text_draw_amount(8, 10, city_resource_count(data.resource), 98, 192, FONT_NORMAL_BLACK);
    lang_text_draw(54, 15, 98 + width, 192, FONT_NORMAL_BLACK);

    int trade_flags = TRADE_STATUS_NONE;
    int trade_status = city_resource_trade_status(data.resource);
    if (empire_can_import_resource(data.resource)) {
        trade_flags |= TRADE_STATUS_IMPORT;
    }
    if (empire_can_export_resource(data.resource)) {
        trade_flags |= TRADE_STATUS_EXPORT;
    }
    if (!trade_flags) {
        lang_text_draw(54, 24, 98, 212, FONT_NORMAL_BLACK);
    }
    else {
        button_border_draw(98, 212, 432, 30, data.focus_button_id == 2);
        switch (trade_status) {
        case TRADE_STATUS_NONE:
            lang_text_draw_centered(54, 18, 114, 221, 400, FONT_NORMAL_BLACK);
            break;
        case TRADE_STATUS_IMPORT:
            lang_text_draw_centered(54, 19, 114, 221, 200, FONT_NORMAL_BLACK);
            break;
        case TRADE_STATUS_EXPORT:
            lang_text_draw_centered(54, 20, 114, 221, 200, FONT_NORMAL_BLACK);
            break;
        }
    }

    if (trade_status == TRADE_STATUS_EXPORT || trade_status == TRADE_STATUS_IMPORT) {
        lang_text_draw_amount(8, 10, city_resource_export_over(data.resource), 386, 221, FONT_NORMAL_BLACK);
    }

    if (building_count_industry_total(data.resource) > 0) {
        button_border_draw(98, 250, 432, 30, data.focus_button_id == 1);
        if (city_resource_is_mothballed(data.resource)) {
            lang_text_draw_centered(54, 17, 114, 259, 400, FONT_NORMAL_BLACK);
        }
        else {
            lang_text_draw_centered(54, 16, 114, 259, 400, FONT_NORMAL_BLACK);
        }
    }

    button_border_draw(98, 288, 432, 50, data.focus_button_id == 3);
    if (city_resource_is_stockpiled(data.resource)) {
        lang_text_draw_centered(54, 26, 114, 296, 400, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 27, 114, 316, 400, FONT_NORMAL_BLACK);
    }
    else {
        lang_text_draw_centered(54, 28, 114, 296, 400, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 29, 114, 316, 400, FONT_NORMAL_BLACK);
    }

    image_buttons_draw(0, 0, resource_image_buttons, 2);
    if (trade_status == TRADE_STATUS_EXPORT || trade_status == TRADE_STATUS_IMPORT) {
        arrow_buttons_draw(0, 0, resource_arrow_buttons, 2);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    if (image_buttons_handle_mouse(m_dialog, 0, 0, resource_image_buttons, 2, 0)) {
        return;
    }
    if ((city_resource_trade_status(data.resource) == TRADE_STATUS_EXPORT || city_resource_trade_status(data.resource) == TRADE_STATUS_IMPORT)) {
        int button = 0;
        arrow_buttons_handle_mouse(m_dialog, 0, 0, resource_arrow_buttons, 2, &button);
        if (button) {
            return;
        }
    }
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, resource_generic_buttons, 3, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_advisors_show();
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_INDUSTRY, 0);
}

static void button_ok(int param1, int param2)
{
    window_advisors_show();
}

static void button_export_up_down(int is_down, int param2)
{
    city_resource_change_export_over(data.resource, is_down ? -1 : 1);
}

static void button_toggle_industry(int param1, int param2)
{
    if (building_count_industry_total(data.resource) > 0) {
        city_resource_toggle_mothballed(data.resource);
    }
}

static void button_toggle_trade(int param1, int param2)
{
    city_resource_cycle_trade_status(data.resource);
}

static void button_toggle_stockpile(int param1, int param2)
{
    city_resource_toggle_stockpiled(data.resource);
}

void window_resource_settings_show(resource_type resource)
{
    window_type window = {
        WINDOW_RESOURCE_SETTINGS,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(resource);
    window_show(&window);
}
