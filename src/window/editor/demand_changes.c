#include "demand_changes.h"

#include "core/image_group_editor.h"
#include "empire/trade_route.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/data.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_demand_change.h"
#include "window/editor/map.h"

static void button_demand_change(int id, int param2);

static generic_button buttons[] = {
    {20, 42, 290, 25, button_demand_change, button_none, 0, 0},
    {20, 72, 290, 25, button_demand_change, button_none, 1, 0},
    {20, 102, 290, 25, button_demand_change, button_none, 2, 0},
    {20, 132, 290, 25, button_demand_change, button_none, 3, 0},
    {20, 162, 290, 25, button_demand_change, button_none, 4, 0},
    {20, 192, 290, 25, button_demand_change, button_none, 5, 0},
    {20, 222, 290, 25, button_demand_change, button_none, 6, 0},
    {20, 252, 290, 25, button_demand_change, button_none, 7, 0},
    {20, 282, 290, 25, button_demand_change, button_none, 8, 0},
    {20, 312, 290, 25, button_demand_change, button_none, 9, 0},
    {320, 42, 290, 25, button_demand_change, button_none, 10, 0},
    {320, 72, 290, 25, button_demand_change, button_none, 11, 0},
    {320, 102, 290, 25, button_demand_change, button_none, 12, 0},
    {320, 132, 290, 25, button_demand_change, button_none, 13, 0},
    {320, 162, 290, 25, button_demand_change, button_none, 14, 0},
    {320, 192, 290, 25, button_demand_change, button_none, 15, 0},
    {320, 222, 290, 25, button_demand_change, button_none, 16, 0},
    {320, 252, 290, 25, button_demand_change, button_none, 17, 0},
    {320, 282, 290, 25, button_demand_change, button_none, 18, 0},
    {320, 312, 290, 25, button_demand_change, button_none, 19, 0},
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static int calc_current_trade(editor_demand_change *change, int idx)
{
    int amount = trade_route_limit(change->route_id, change->resource);
    for (int i = 0; i < MAX_DEMAND_CHANGES && i <= idx; i++) {
        editor_demand_change change;
        scenario_editor_demand_change_get(i, &change);
        if (change.amount == DEMAND_CHANGE_LEGACY_IS_RISE) {
            if (amount == 0) {
                amount = 15;
            } else if (amount == 15) {
                amount = 25;
            } else if (amount == 25) {
                amount = 40;
            }
        } else if (change.amount == DEMAND_CHANGE_LEGACY_IS_FALL) {
            if (amount == 40) {
                amount = 25;
            } else if (amount == 25) {
                amount = 15;
            } else if (amount == 15) {
                amount = 0;
            }
        } else {
            amount = change.amount;
        }
    }
    return amount;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 23);
    lang_text_draw(44, 94, 20, 14, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 342, 640, FONT_NORMAL_BLACK);

    for (int i = 0; i < MAX_DEMAND_CHANGES; i++) {
        int x, y;
        if (i < 10) {
            x = 20;
            y = 42 + 30 * i;
        } else {
            x = 320;
            y = 42 + 30 * (i - 10);
        }
        button_border_draw(x, y, 290, 25, focus_button_id == i + 1);
        editor_demand_change demand_change;
        scenario_editor_demand_change_get(i, &demand_change);
        if (demand_change.year) {
            text_draw_number(demand_change.year, '+', " ", x + 10, y + 6, FONT_NORMAL_BLACK, 0);
            lang_text_draw_year(scenario_property_start_year() + demand_change.year, x + 35, y + 6, FONT_NORMAL_BLACK);
            int offset = demand_change.resource + resource_image_offset(demand_change.resource, RESOURCE_IMAGE_ICON);
            image_draw(image_group(GROUP_EDITOR_RESOURCE_ICONS) + offset, x + 115, y + 3, COLOR_MASK_NONE, SCALE_NONE);
            int width = lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_SHORT_ROUTE_TEXT, x + 140, y + 6, FONT_NORMAL_BLACK);
            width += text_draw_number(demand_change.route_id, '@', " ", x + 140 + width, y + 6, FONT_NORMAL_BLACK, 0);
            int amount = calc_current_trade(&demand_change, i);
            width += text_draw_number(amount, '@', " ", x + 140 + width, y + 6, FONT_NORMAL_BLACK, 0);
            int last_amount = calc_current_trade(&demand_change, i - 1);
            width += text_draw_number(amount - last_amount, '(', ")", x + 140 + width, y + 6, FONT_NORMAL_BLACK, 0);
        } else {
            lang_text_draw_centered(44, 96, x, y + 6, 290, FONT_NORMAL_BLACK);
        }
    }

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 20, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_demand_change(int id, int param2)
{
    window_editor_edit_demand_change_show(id);
}

void window_editor_demand_changes_show(void)
{
    window_type window = {
        WINDOW_EDITOR_DEMAND_CHANGES,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
