#include "demand_changes.h"

#include "core/image_group_editor.h"
#include "empire/trade_route.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
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

static void button_demand_change(unsigned int id, unsigned int mouse_x, unsigned int mouse_y);
static void draw_demand_change_button(const grid_box_item *item);

static grid_box_type demand_change_buttons = {
    .x = 20,
    .y = 40,
    .width = 38 * BLOCK_SIZE,
    .height = 19 * BLOCK_SIZE,
    .num_columns = 2,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 5,
    .extend_to_hidden_scrollbar = 1,
    .on_click = button_demand_change,
    .draw_item = draw_demand_change_button
};

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 23);
    lang_text_draw(44, 94, 20, 14, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 342, 640, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    grid_box_request_refresh(&demand_change_buttons);
}

static int calc_current_trade(editor_demand_change *from_change, int idx)
{
    int amount = trade_route_limit(from_change->route_id, from_change->resource);
    for (int i = 0; i < MAX_DEMAND_CHANGES && i <= idx; i++) {
        editor_demand_change change;
        scenario_editor_demand_change_get(i, &change);
        if (change.resource != from_change->resource || change.route_id != from_change->route_id)
            continue;
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

static void draw_demand_change_button(const grid_box_item *item)
{
    button_border_draw(item->x, item->y, item->width, item->height, item->is_focused);
    editor_demand_change demand_change;
    scenario_editor_demand_change_get(item->index, &demand_change);
    if (demand_change.year) {
        text_draw_number(demand_change.year, '+', " ", item->x + 10, item->y + 7, FONT_NORMAL_BLACK, 0);
        lang_text_draw_year(scenario_property_start_year() + demand_change.year, item->x + 35, item->y + 7,
            FONT_NORMAL_BLACK);
        image_draw(resource_get_data(demand_change.resource)->image.editor.icon, item->x + 115, item->y + 4,
            COLOR_MASK_NONE, SCALE_NONE);
        int width = lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_SHORT_ROUTE_TEXT, item->x + 140, item->y + 7,
            FONT_NORMAL_BLACK);
        width += text_draw_number(demand_change.route_id, '@', " ", item->x + 140 + width, item->y + 7,
            FONT_NORMAL_BLACK, 0);
        int amount = calc_current_trade(&demand_change, item->index);
        width += text_draw_number(amount, '@', " ", item->x + 140 + width, item->y + 7, FONT_NORMAL_BLACK, 0);
        int last_amount = calc_current_trade(&demand_change, item->index - 1);
        width += text_draw_number(amount - last_amount, '(', ")", item->x + 140 + width, item->y + 7,
            FONT_NORMAL_BLACK, 0);
    } else {
        lang_text_draw_centered(44, 96, item->x, item->y + 7, item->width, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    grid_box_draw(&demand_change_buttons);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (grid_box_handle_input(&demand_change_buttons, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_demand_change(unsigned int id, unsigned int mouse_x, unsigned int mouse_y)
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
    grid_box_init(&demand_change_buttons, MAX_DEMAND_CHANGES);
    window_show(&window);
}
