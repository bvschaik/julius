#include "price_changes.h"

#include "core/image_group_editor.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/list_box.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/data.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_price_change.h"
#include "window/editor/map.h"

static void select_price_change(unsigned int id, int param2);
static void draw_price_change_button(const list_box_item *item);

static list_box_type price_change_list = {
    .x = 20,
    .y = 40,
    .width_blocks = 38,
    .height_blocks = 19,
    .num_columns = 2,
    .item_height = 30,
    .extend_to_hidden_scrollbar = 1,
    .on_select = select_price_change,
    .draw_item = draw_price_change_button
};

static void draw_background(void)
{
    window_editor_map_draw_all();
    list_box_request_refresh(&price_change_list);
}

static void draw_price_change_button(const list_box_item *item)
{
    button_border_draw(item->x + 5, item->y + 3, item->width - 10, item->height - 5, item->is_focused);
    editor_price_change price_change;
    scenario_editor_price_change_get(item->index, &price_change);
    if (price_change.year) {
        text_draw_number(price_change.year, '+', " ", item->x + 15, item->y + 10, FONT_NORMAL_BLACK, 0);
        lang_text_draw_year(scenario_property_start_year() + price_change.year, item->x + 70, item->y + 10,
            FONT_NORMAL_BLACK);
        image_draw(resource_get_data(price_change.resource)->image.editor.icon, item->x + 145, item->y + 7,
            COLOR_MASK_NONE, SCALE_NONE);
        int width = lang_text_draw(44, price_change.is_rise ? 104 : 103, item->x + 175, item->y + 10, FONT_NORMAL_BLACK);
        text_draw_number(price_change.amount, '@', " ", item->x + 175 + width, item->y + 10, FONT_NORMAL_BLACK, 0);
    } else {
        lang_text_draw_centered(44, 102, item->x + 5, item->y + 10, item->width - 10, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw(44, 95, 20, 12, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 0, 456, 640, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(152, 3, 32, 376, 576, FONT_NORMAL_BLACK);
    list_box_draw(&price_change_list);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (list_box_handle_input(&price_change_list, mouse_in_dialog(m), 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void select_price_change(unsigned int id, int param2)
{
    window_editor_edit_price_change_show(id);
}

void window_editor_price_changes_show(void)
{
    window_type window = {
        WINDOW_EDITOR_PRICE_CHANGES,
        draw_background,
        draw_foreground,
        handle_input
    };
    list_box_init(&price_change_list, MAX_PRICE_CHANGES);
    window_show(&window);
}
