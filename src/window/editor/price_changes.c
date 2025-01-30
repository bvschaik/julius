#include "price_changes.h"

#include "core/image_group_editor.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/price_change.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/edit_price_change.h"
#include "window/editor/map.h"

#include <stdlib.h>

static void button_price_change(const grid_box_item *item);
static void button_new_price_change(const generic_button *button);
static void draw_price_change_button(const grid_box_item *item);

static struct {
    const price_change_t **price_changes;
    unsigned int total_price_changes;
    unsigned int price_changes_in_use;
    unsigned int new_price_change_button_focused;
} data;

static grid_box_type price_change_buttons = {
    .x = 10,
    .y = 40,
    .width = 38 * BLOCK_SIZE,
    .height = 19 * BLOCK_SIZE,
    .num_columns = 2,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 5,
    .extend_to_hidden_scrollbar = 1,
    .on_click = button_price_change,
    .draw_item = draw_price_change_button
};

static generic_button new_price_change_button = {
    195, 340, 250, 25, button_new_price_change
};

static void limit_and_sort_list(void)
{
    data.price_changes_in_use = 0;
    for (unsigned int i = 0; i < data.total_price_changes; i++) {
        const price_change_t *price_change = scenario_price_change_get(i);
        if (!price_change->year) {
            continue;
        }
        data.price_changes[data.price_changes_in_use] = price_change;
        data.price_changes_in_use++;
    }
    for (unsigned int i = 0; i < data.price_changes_in_use; i++) {
        for (unsigned int j = data.price_changes_in_use - 1; j > 0; j--) {
            const price_change_t *current = data.price_changes[j];
            const price_change_t *prev = data.price_changes[j - 1];
            if (current->year && (!prev->year || prev->year > current->year)) {
                const price_change_t *tmp = data.price_changes[j];
                data.price_changes[j] = data.price_changes[j - 1];
                data.price_changes[j - 1] = tmp;
            }
        }
    }
}

static void update_price_changes_list(void)
{
    int current_price_changes = scenario_price_change_count_total();
    if (current_price_changes != data.total_price_changes) {
        free(data.price_changes);
        data.price_changes = 0;
        if (current_price_changes) {
            data.price_changes = malloc(current_price_changes * sizeof(price_change_t *));
            if (!data.price_changes) {
                grid_box_update_total_items(&price_change_buttons, 0);
                data.total_price_changes = 0;
                data.price_changes_in_use = 0;
                return;
            }
        }
        data.total_price_changes = current_price_changes;
    }
    limit_and_sort_list();
    grid_box_update_total_items(&price_change_buttons, data.price_changes_in_use);
}

static void draw_background(void)
{
    update_price_changes_list();

    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 0, 40, 30);
    lang_text_draw(44, 95, 20, 12, FONT_LARGE_BLACK);

    if (!data.price_changes_in_use) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_NO_PRICE_CHANGES, 0, 165, 640, FONT_LARGE_BLACK);
    }
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_NEW_PRICE_CHANGE, new_price_change_button.x + 8,
        new_price_change_button.y + 8, new_price_change_button.width - 16, FONT_NORMAL_BLACK);

    lang_text_draw_centered(13, 3, 0, 456, 640, FONT_NORMAL_BLACK);
    lang_text_draw_multiline(152, 3, 32, 376, 576, FONT_NORMAL_BLACK);

    graphics_reset_dialog();

    grid_box_request_refresh(&price_change_buttons);
}

static void draw_price_change_button(const grid_box_item *item)
{
    button_border_draw(item->x, item->y, item->width, item->height, item->is_focused);
    const price_change_t *price_change = data.price_changes[item->index];
    text_draw_number(price_change->year, '+', " ", item->x + 10, item->y + 7, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + price_change->year, item->x + 65, item->y + 7,
        FONT_NORMAL_BLACK);
    int image_id = resource_get_data(price_change->resource)->image.editor.icon;
    const image *img = image_get(image_id);
    int base_height = (item->height - img->original.height) / 2;
    image_draw(image_id, item->x + 140, item->y + base_height, COLOR_MASK_NONE, SCALE_NONE);
    int width = lang_text_draw(44, price_change->is_rise ? 104 : 103, item->x + 170, item->y + 7, FONT_NORMAL_BLACK);
    text_draw_number(price_change->amount, '@', " ", item->x + 170 + width, item->y + 7, FONT_NORMAL_BLACK, 0);
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    if (data.price_changes_in_use) {
        grid_box_draw(&price_change_buttons);
    }
    button_border_draw(new_price_change_button.x, new_price_change_button.y,
        new_price_change_button.width, new_price_change_button.height, data.new_price_change_button_focused);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (grid_box_handle_input(&price_change_buttons, m_dialog, 1) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, &new_price_change_button, 1,
            &data.new_price_change_button_focused)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_price_change(const grid_box_item *item)
{
    window_editor_edit_price_change_show(item->index);
}

static void button_new_price_change(const generic_button *button)
{
    int new_price_change_id = scenario_price_change_new();
    if (new_price_change_id >= 0) {
        window_editor_edit_price_change_show(new_price_change_id);
    }
}

void window_editor_price_changes_show(void)
{
    window_type window = {
        WINDOW_EDITOR_PRICE_CHANGES,
        draw_background,
        draw_foreground,
        handle_input
    };
    grid_box_init(&price_change_buttons, scenario_price_change_count_total());
    window_show(&window);
}
