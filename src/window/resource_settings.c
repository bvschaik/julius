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
#include "translation/translation.h"
#include "window/advisor/trade.h"
#include "window/message_dialog.h"
#include "window/empire.h"

static void button_help(int param1, int param2);
static void button_ok(int param1, int param2);
static void button_trade_up_down(int trade_type, int is_down);

static void button_toggle_industry(int param1, int param2);
static void button_toggle_trade(int status, int param2);
static void button_toggle_stockpile(int param1, int param2);

static image_button resource_image_buttons[] = {
    {26, 332, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1},
    {590, 335, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_ok, button_none, 0, 0, 1}
};

static arrow_button import_amount_arrow_buttons[] = {
    {265, 215, 17, 24, button_trade_up_down, TRADE_STATUS_IMPORT, 1},
    {289, 215, 15, 24, button_trade_up_down, TRADE_STATUS_IMPORT, 0}
};

static arrow_button export_amount_arrow_buttons[] = {
    {557, 215, 17, 24, button_trade_up_down, TRADE_STATUS_EXPORT, 1},
    {581, 215, 15, 24, button_trade_up_down, TRADE_STATUS_EXPORT, 0}
};

static generic_button resource_generic_buttons[] = {
    {66, 250, 496, 30, button_toggle_industry, button_none, 0, 0},
    {30, 212, 286, 30, button_toggle_trade, button_none, TRADE_STATUS_IMPORT, 0},
    {322, 212, 286, 30, button_toggle_trade, button_none, TRADE_STATUS_EXPORT, 0},
    {66, 288, 496, 50, button_toggle_stockpile, button_none, 0, 0},
};

static struct {
    resource_type resource;
    int focus_button_id;
    int focus_image_button_id;
} data;

static void init(resource_type resource)
{
    data.resource = resource;
}

static void draw_background(void)
{
    window_draw_underlying_window();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 128, 38, 15);
    image_draw(resource_get_data(data.resource)->image.icon, 26, 136, COLOR_MASK_NONE, SCALE_NONE);

    text_draw(resource_get_data(data.resource)->text, 60, 137, FONT_LARGE_BLACK, COLOR_MASK_NONE);

    int total_buildings = building_count_total(resource_get_data(data.resource)->industry);

    if (empire_can_produce_resource(data.resource)) {
        int active_buildings = building_count_active(resource_get_data(data.resource)->industry);
        if (total_buildings <= 0) {
            lang_text_draw(54, 7, 66, 172, FONT_NORMAL_BLACK);
        } else if (city_resource_is_mothballed(data.resource)) {
            int width = text_draw_number(total_buildings, '@', " ", 66, 172, FONT_NORMAL_BLACK, 0);
            if (total_buildings == 1) {
                lang_text_draw(54, 10, 66 + width, 172, FONT_NORMAL_BLACK);
            } else {
                lang_text_draw(54, 11, 66 + width, 172, FONT_NORMAL_BLACK);
            }
        } else if (total_buildings == active_buildings) {
            // not mothballed, all working
            int width = text_draw_number(total_buildings, '@', " ", 66, 172, FONT_NORMAL_BLACK, 0);
            if (total_buildings == 1) {
                lang_text_draw(54, 8, 66 + width, 172, FONT_NORMAL_BLACK);
            } else {
                lang_text_draw(54, 9, 66 + width, 172, FONT_NORMAL_BLACK);
            }
        } else {
            // not mothballed, some working
            int width = text_draw_number(active_buildings, '@', " ", 66, 172, FONT_NORMAL_BLACK, 0);
            width += lang_text_draw(54, 12, 66 + width, 172, FONT_NORMAL_BLACK);
            width += text_draw_number(total_buildings - active_buildings, '@', " ",
                66 + width, 172, FONT_NORMAL_BLACK, 0);
            if (active_buildings == 1) {
                lang_text_draw(54, 13, 66 + width, 172, FONT_NORMAL_BLACK);
            } else {
                lang_text_draw(54, 14, 66 + width, 172, FONT_NORMAL_BLACK);
            }
        }
    } else {
        // we cannot produce this good
        lang_text_draw(54, 25, 66, 172, FONT_NORMAL_BLACK);
    }

    int width = lang_text_draw_amount(8, 10, city_resource_count(data.resource), 66, 192, FONT_NORMAL_BLACK);
    lang_text_draw(54, 15, 66 + width, 192, FONT_NORMAL_BLACK);

    int can_import_potentially = empire_can_import_resource_potentially(data.resource);
    int can_export_potentially = empire_can_export_resource_potentially(data.resource);
    int trade_flags = TRADE_STATUS_NONE;
    int trade_status = city_resource_trade_status(data.resource);
    if (empire_can_import_resource(data.resource)) {
        trade_flags |= TRADE_STATUS_IMPORT;
    }
    if (empire_can_export_resource(data.resource)) {
        trade_flags |= TRADE_STATUS_EXPORT;
    }
    if (trade_flags & TRADE_STATUS_IMPORT) {
        button_border_draw(30, 212, 286, 30, data.focus_button_id == 2);
        if (trade_status & TRADE_STATUS_IMPORT) {
            int x_offset = 32 + (215 - lang_text_get_width(54, 5, FONT_NORMAL_BLACK)) / 2;
            int width = lang_text_draw(54, 5, x_offset, 221, FONT_NORMAL_BLACK);
            int trade_quantity = city_resource_import_over(data.resource);
            if (trade_quantity == 0) {
                text_draw(translation_for(TR_ADVISOR_TRADE_NO_LIMIT), x_offset + width, 221, FONT_NORMAL_BLACK, 0);
            } else {
                text_draw_number(trade_quantity, 0, " ", x_offset + width, 221, FONT_NORMAL_BLACK, 0);
            }
            arrow_buttons_draw(0, 0, import_amount_arrow_buttons, 2);   
        } else {
            text_draw_centered(translation_for(TR_ADVISOR_TRADE_NOT_IMPORTING), 32, 221, 282, FONT_NORMAL_BLACK, 0);             
        }
    } else {
        if (!can_import_potentially) {
            text_draw_centered(translation_for(TR_ADVISOR_TRADE_NO_SELLERS), 34, 221, 282, FONT_NORMAL_BLACK, 0);
        } else {
            button_border_draw(30, 212, 286, 30, data.focus_button_id == 2);
            text_draw_centered(translation_for(TR_ADVISOR_OPEN_TO_IMPORT), 34, 221, 282, FONT_NORMAL_BLACK, 0);
        }
    }
    if (trade_flags & TRADE_STATUS_EXPORT) {
        button_border_draw(322, 212, 286, 30, data.focus_button_id == 3);
        if (trade_status & TRADE_STATUS_EXPORT) {
            int x_offset = 324 + (220 - lang_text_get_width(54, 6, FONT_NORMAL_BLACK)) / 2;
            int width = lang_text_draw(54, 6, x_offset, 221, FONT_NORMAL_BLACK);
            int trade_quantity = city_resource_export_over(data.resource);
            text_draw_number(trade_quantity, 0, " ", x_offset + width, 221, FONT_NORMAL_BLACK, 0);
            arrow_buttons_draw(0, 0, export_amount_arrow_buttons, 2);
        } else {
            text_draw_centered(translation_for(TR_ADVISOR_TRADE_NOT_EXPORTING), 324, 221, 282, FONT_NORMAL_BLACK, 0);
        }
    } else {
        if (!can_export_potentially) {
            text_draw_centered(translation_for(TR_ADVISOR_TRADE_NO_BUYERS), 324, 221, 282, FONT_NORMAL_BLACK, 0);
        } else {
            button_border_draw(322, 212, 286, 30, data.focus_button_id == 3);
            text_draw_centered(translation_for(TR_ADVISOR_OPEN_TO_EXPORT), 324, 221, 282, FONT_NORMAL_BLACK, 0);
        }
    }

    if (total_buildings > 0) {
        button_border_draw(66, 250, 496, 30, data.focus_button_id == 1);
        if (city_resource_is_mothballed(data.resource)) {
            lang_text_draw_centered(54, 17, 82, 259, 464, FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_centered(54, 16, 82, 259, 464, FONT_NORMAL_BLACK);
        }
    }

    button_border_draw(66, 288, 496, 50, data.focus_button_id == 4);
    if (city_resource_is_stockpiled(data.resource)) {
        lang_text_draw_centered(54, 26, 82, 296, 464, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 27, 82, 316, 464, FONT_NORMAL_BLACK);
    }
    else {
        lang_text_draw_centered(54, 28, 82, 296, 464, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 29, 82, 316, 464, FONT_NORMAL_BLACK);
    }

    image_buttons_draw(0, 0, resource_image_buttons, 2);

    graphics_reset_dialog();
}

static int needs_to_open_trade_route(int status)
{
    if (status == TRADE_STATUS_IMPORT) {
        return empire_can_import_resource_potentially(data.resource) && !empire_can_import_resource(data.resource);
    }
    if (status == TRADE_STATUS_EXPORT) {
        return empire_can_export_resource_potentially(data.resource) && !empire_can_export_resource(data.resource);
    }
    return 0;
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    if (image_buttons_handle_mouse(m_dialog, 0, 0, resource_image_buttons, 2, &data.focus_image_button_id)) {
        return;
    }
    if (city_resource_trade_status(data.resource) & TRADE_STATUS_IMPORT) {
        int button = 0;
        arrow_buttons_handle_mouse(m_dialog, 0, 0, import_amount_arrow_buttons, 2, &button);
        if (button) {
            return;
        }
    }
    if (city_resource_trade_status(data.resource) & TRADE_STATUS_EXPORT) {
        int button = 0;
        arrow_buttons_handle_mouse(m_dialog, 0, 0, export_amount_arrow_buttons, 2, &button);
        if (button) {
            return;
        }
    }
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, resource_generic_buttons, 4, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_INDUSTRY, 0);
}

static void button_ok(int param1, int param2)
{
    window_go_back();
}

static void button_trade_up_down(int trade_type, int is_down)
{
    if (trade_type == TRADE_STATUS_IMPORT) {
        city_resource_change_import_over(data.resource, is_down ? -1 : 1);
    } else if (trade_type == TRADE_STATUS_EXPORT) {
        city_resource_change_export_over(data.resource, is_down ? -1 : 1);
    }
}

static void button_toggle_industry(int param1, int param2)
{
    if (building_count_total(resource_get_data(data.resource)->industry) > 0) {
        city_resource_toggle_mothballed(data.resource);
    }
}

static void button_toggle_trade(int status, int param2)
{
    if (needs_to_open_trade_route(status)) {
        window_empire_show();
        return;
    }
    city_resource_cycle_trade_status(data.resource, status);
}

static void button_toggle_stockpile(int param1, int param2)
{
    city_resource_toggle_stockpiled(data.resource);
}

void get_tooltip(tooltip_context *c)
{
    if ((data.focus_button_id == 2 && needs_to_open_trade_route(TRADE_STATUS_IMPORT)) ||
        (data.focus_button_id == 3 && needs_to_open_trade_route(TRADE_STATUS_EXPORT))) {
        c->type = TOOLTIP_BUTTON;
        c->text_group = 54;
        c->text_id = 30;
        return;
    }
    if (data.focus_image_button_id) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = data.focus_image_button_id;
    }
}

void window_resource_settings_show(resource_type resource)
{
    window_type window = {
        WINDOW_RESOURCE_SETTINGS,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(resource);
    window_show(&window);
}
