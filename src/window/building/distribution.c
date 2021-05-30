#include "distribution.h"

#include "building/building.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "window/building_info.h"

static void go_to_orders(int param1, int param2);
static void toggle_resource_state(int index, int param2);
static void granary_orders(int param1, int param2);
static void warehouse_orders(int index, int param2);

static generic_button go_to_orders_button[] = {
    {0, 0, 304, 20, go_to_orders, button_none, 0, 0}
};

static generic_button orders_resource_buttons[] = {
    {0, 0, 210, 22, toggle_resource_state, button_none, 1, 0},
    {0, 22, 210, 22, toggle_resource_state, button_none, 2, 0},
    {0, 44, 210, 22, toggle_resource_state, button_none, 3, 0},
    {0, 66, 210, 22, toggle_resource_state, button_none, 4, 0},
    {0, 88, 210, 22, toggle_resource_state, button_none, 5, 0},
    {0, 110, 210, 22, toggle_resource_state, button_none, 6, 0},
    {0, 132, 210, 22, toggle_resource_state, button_none, 7, 0},
    {0, 154, 210, 22, toggle_resource_state, button_none, 8, 0},
    {0, 176, 210, 22, toggle_resource_state, button_none, 9, 0},
    {0, 198, 210, 22, toggle_resource_state, button_none, 10, 0},
    {0, 220, 210, 22, toggle_resource_state, button_none, 11, 0},
    {0, 242, 210, 22, toggle_resource_state, button_none, 12, 0},
    {0, 264, 210, 22, toggle_resource_state, button_none, 13, 0},
    {0, 286, 210, 22, toggle_resource_state, button_none, 14, 0},
    {0, 308, 210, 22, toggle_resource_state, button_none, 15, 0},
};

static generic_button granary_order_buttons[] = {
    {0, 0, 304, 20, granary_orders, button_none, 0, 0},
    {314, 0, 20, 20, granary_orders, button_none, 1, 0},
};

static generic_button warehouse_order_buttons[] = {
    {0, 0, 304, 20, warehouse_orders, button_none, 0, 0},
    {0, -22, 304, 20, warehouse_orders, button_none, 1, 0},
    {314, 0, 20, 20, warehouse_orders, button_none, 2, 0},
};

static struct {
    int focus_button_id;
    int orders_focus_button_id;
    int resource_focus_button_id;
    int building_id;
} data = {0, 0, 0, 0};

void window_building_draw_dock(building_info_context *c)
{
    c->help_id = 83;
    window_building_play_sound(c, "wavs/dock.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(101, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->data.dock.trade_ship_id) {
        if (c->worker_percentage <= 0) {
            window_building_draw_description(c, 101, 2);
        } else if (c->worker_percentage < 50) {
            window_building_draw_description(c, 101, 3);
        } else if (c->worker_percentage < 75) {
            window_building_draw_description(c, 101, 4);
        } else {
            window_building_draw_description(c, 101, 5);
        }
    } else {
        if (c->worker_percentage <= 0) {
            window_building_draw_description(c, 101, 6);
        } else if (c->worker_percentage < 50) {
            window_building_draw_description(c, 101, 7);
        } else if (c->worker_percentage < 75) {
            window_building_draw_description(c, 101, 8);
        } else {
            window_building_draw_description(c, 101, 9);
        }
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_market(building_info_context *c)
{
    c->help_id = 2;
    window_building_play_sound(c, "wavs/market.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(97, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 97, 2);
    } else {
        int image_id = image_group(GROUP_RESOURCE_ICONS);
        if (b->data.market.inventory[INVENTORY_WHEAT] || b->data.market.inventory[INVENTORY_VEGETABLES] ||
            b->data.market.inventory[INVENTORY_FRUIT] || b->data.market.inventory[INVENTORY_MEAT]) {
            // food stocks
            image_draw(image_id + RESOURCE_WHEAT, c->x_offset + 32, c->y_offset + 64);
            text_draw_number(b->data.market.inventory[INVENTORY_WHEAT], '@', " ",
                c->x_offset + 64, c->y_offset + 70, FONT_NORMAL_BLACK);

            image_draw(image_id + RESOURCE_VEGETABLES, c->x_offset + 142, c->y_offset + 64);
            text_draw_number(b->data.market.inventory[INVENTORY_VEGETABLES], '@', " ",
                c->x_offset + 174, c->y_offset + 70, FONT_NORMAL_BLACK);

            image_draw(image_id + RESOURCE_FRUIT, c->x_offset + 252, c->y_offset + 64);
            text_draw_number(b->data.market.inventory[INVENTORY_FRUIT], '@', " ",
                c->x_offset + 284, c->y_offset + 70, FONT_NORMAL_BLACK);

            image_draw(image_id + RESOURCE_MEAT +
                resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
                c->x_offset + 362, c->y_offset + 64);
            text_draw_number(b->data.market.inventory[INVENTORY_MEAT], '@', " ",
                c->x_offset + 394, c->y_offset + 70, FONT_NORMAL_BLACK);
        } else {
            window_building_draw_description_at(c, 48, 97, 4);
        }
        // good stocks
        image_draw(image_id + RESOURCE_POTTERY, c->x_offset + 32, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_POTTERY], '@', " ",
            c->x_offset + 64, c->y_offset + 110, FONT_NORMAL_BLACK);

        image_draw(image_id + RESOURCE_FURNITURE, c->x_offset + 142, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_FURNITURE], '@', " ",
            c->x_offset + 174, c->y_offset + 110, FONT_NORMAL_BLACK);

        image_draw(image_id + RESOURCE_OIL, c->x_offset + 252, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_OIL], '@', " ",
            c->x_offset + 284, c->y_offset + 110, FONT_NORMAL_BLACK);

        image_draw(image_id + RESOURCE_WINE, c->x_offset + 362, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_WINE], '@', " ",
            c->x_offset + 394, c->y_offset + 110, FONT_NORMAL_BLACK);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_granary(building_info_context *c)
{
    c->help_id = 3;
    window_building_play_sound(c, "wavs/granary.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(98, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description_at(c, 40, 69, 25);
    } else if (scenario_property_rome_supplies_wheat()) {
        window_building_draw_description_at(c, 40, 98, 4);
    } else {
        int total_stored = 0;
        for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
            total_stored += b->data.granary.resource_stored[i];
        }
        int width = lang_text_draw(98, 2, c->x_offset + 34, c->y_offset + 40, FONT_NORMAL_BLACK);
        lang_text_draw_amount(8, 16, total_stored, c->x_offset + 34 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

        width = lang_text_draw(98, 3, c->x_offset + 220, c->y_offset + 40, FONT_NORMAL_BLACK);
        lang_text_draw_amount(8, 16, b->data.granary.resource_stored[RESOURCE_NONE],
            c->x_offset + 220 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

        int image_id = image_group(GROUP_RESOURCE_ICONS);
        // wheat
        image_draw(image_id + RESOURCE_WHEAT, c->x_offset + 34, c->y_offset + 68);
        width = text_draw_number(b->data.granary.resource_stored[RESOURCE_WHEAT], '@', " ",
            c->x_offset + 68, c->y_offset + 75, FONT_NORMAL_BLACK);
        lang_text_draw(23, RESOURCE_WHEAT, c->x_offset + 68 + width, c->y_offset + 75, FONT_NORMAL_BLACK);

        // vegetables
        image_draw(image_id + RESOURCE_VEGETABLES, c->x_offset + 34, c->y_offset + 92);
        width = text_draw_number(b->data.granary.resource_stored[RESOURCE_VEGETABLES], '@', " ",
            c->x_offset + 68, c->y_offset + 99, FONT_NORMAL_BLACK);
        lang_text_draw(23, RESOURCE_VEGETABLES, c->x_offset + 68 + width, c->y_offset + 99, FONT_NORMAL_BLACK);

        // fruit
        image_draw(image_id + RESOURCE_FRUIT, c->x_offset + 240, c->y_offset + 68);
        width = text_draw_number(b->data.granary.resource_stored[RESOURCE_FRUIT], '@', " ",
            c->x_offset + 274, c->y_offset + 75, FONT_NORMAL_BLACK);
        lang_text_draw(23, RESOURCE_FRUIT, c->x_offset + 274 + width, c->y_offset + 75, FONT_NORMAL_BLACK);

        // meat/fish
        image_draw(image_id + RESOURCE_MEAT + resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->x_offset + 240, c->y_offset + 92);
        width = text_draw_number(b->data.granary.resource_stored[RESOURCE_MEAT], '@', " ",
            c->x_offset + 274, c->y_offset + 99, FONT_NORMAL_BLACK);
        lang_text_draw(23, RESOURCE_MEAT, c->x_offset + 274 + width, c->y_offset + 99, FONT_NORMAL_BLACK);
    }
    // cartpusher state
    int cartpusher = b->figure_id;
    int has_cart_orders = cartpusher && figure_get(cartpusher)->state == FIGURE_STATE_ALIVE;
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, has_cart_orders ? 5 : 4);
    window_building_draw_employment(c, 142);
    if (has_cart_orders) {
        int resource = figure_get(cartpusher)->resource_id;
        image_draw(image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 32, c->y_offset + 190);
        lang_text_draw_multiline(99, 17, c->x_offset + 64, c->y_offset + 193,
            BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
    }
}

void window_building_draw_granary_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
}

int window_building_handle_mouse_granary(const mouse *m, building_info_context *c)
{
    return generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
}

void window_building_draw_granary_orders(building_info_context *c)
{
    c->help_id = 3;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(98, 6, c->x_offset, y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

static void draw_accept_none_button(int x, int y, int focused)
{
    uint8_t refuse_button_text[] = {'x', 0};
    button_border_draw(x, y, 20, 20, focused ? 1 : 0);
    text_draw_centered(refuse_button_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
}

void window_building_draw_granary_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    // empty button
    button_border_draw(c->x_offset + 80, y_offset + 404, BLOCK_SIZE * (c->width_blocks - 10), 20,
        data.orders_focus_button_id == 1 ? 1 : 0);
    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);
    if (storage->empty_all) {
        lang_text_draw_centered(98, 8, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(98, 9, c->x_offset, y_offset + 384,
            BLOCK_SIZE * c->width_blocks, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(98, 7, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 2);

    const resource_list *list = city_resource_get_available_foods();
    for (int i = 0; i < list->size; i++) {
        int resource = list->items[i];
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 408, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);

        int state = storage->resource_state[resource];
        if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        } else if (state == BUILDING_STORAGE_STATE_GETTING) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 10, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        }
    }
}

int window_building_handle_mouse_granary_orders(const mouse *m, building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_resource_buttons, city_resource_get_available_foods()->size,
        &data.resource_focus_button_id)) {
        return 1;
    }
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404,
        granary_order_buttons, 2, &data.orders_focus_button_id);
}

void window_building_get_tooltip_granary_orders(int *group_id, int *text_id)
{
    if (data.orders_focus_button_id == 2) {
        *group_id = 143;
        *text_id = 1;
    }
}

void window_building_draw_warehouse(building_info_context *c)
{
    c->help_id = 4;
    window_building_play_sound(c, "wavs/warehouse.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(99, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else {
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            int x, y;
            if (r <= 5) {
                x = c->x_offset + 20;
                y = c->y_offset + 24 * (r - 1) + 36;
            } else if (r <= 10) {
                x = c->x_offset + 170;
                y = c->y_offset + 24 * (r - 6) + 36;
            } else {
                x = c->x_offset + 320;
                y = c->y_offset + 24 * (r - 11) + 36;
            }
            int amount = building_warehouse_get_amount(b, r);
            int image_id = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
            image_draw(image_id, x, y);
            int width = text_draw_number(amount, '@', " ", x + 24, y + 7, FONT_SMALL_PLAIN);
            lang_text_draw(23, r, x + 24 + width, y + 7, FONT_SMALL_PLAIN);
        }
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 168, c->width_blocks - 2, 5);
    window_building_draw_employment(c, 173);
    // cartpusher state
    int cartpusher = b->figure_id;
    if (cartpusher && figure_get(cartpusher)->state == FIGURE_STATE_ALIVE) {
        int resource = figure_get(cartpusher)->resource_id;
        image_draw(image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 32, c->y_offset + 220);
        lang_text_draw_multiline(99, 17, c->x_offset + 64, c->y_offset + 223,
            BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
    } else if (b->num_workers) {
        // cartpusher is waiting for orders
        lang_text_draw_multiline(99, 15, c->x_offset + 32, c->y_offset + 223,
            BLOCK_SIZE * (c->width_blocks - 3), FONT_NORMAL_BROWN);
    }

    if (c->warehouse_space_text == 1) { // full
        lang_text_draw_multiline(99, 13, c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 93,
            BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    } else if (c->warehouse_space_text == 2) {
        lang_text_draw_multiline(99, 14, c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 93,
            BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    }
}

void window_building_draw_warehouse_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(99, 2, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
}

int window_building_handle_mouse_warehouse(const mouse *m, building_info_context *c)
{
    return generic_buttons_handle_mouse(m, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
}

void window_building_draw_warehouse_orders(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    c->help_id = 4;
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(99, 3, c->x_offset, y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_warehouse_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    // emptying button
    button_border_draw(c->x_offset + 80, y_offset + 404, BLOCK_SIZE * (c->width_blocks - 10),
        20, data.orders_focus_button_id == 1 ? 1 : 0);
    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);
    if (storage->empty_all) {
        lang_text_draw_centered(99, 5, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(99, 6, c->x_offset, y_offset + 426, BLOCK_SIZE * c->width_blocks, FONT_SMALL_PLAIN);
    } else {
        lang_text_draw_centered(99, 4, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    // trade center
    button_border_draw(c->x_offset + 80, y_offset + 382, BLOCK_SIZE * (c->width_blocks - 10),
        20, data.orders_focus_button_id == 2 ? 1 : 0);
    int is_trade_center = c->building_id == city_buildings_get_trade_center();
    lang_text_draw_centered(99, is_trade_center ? 11 : 12, c->x_offset + 80, y_offset + 386,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 3);

    const resource_list *list = city_resource_get_available();
    for (int i = 0; i < list->size; i++) {
        int resource = list->items[i];
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 408, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);

        int state = storage->resource_state[resource];
        if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        } else if (state == BUILDING_STORAGE_STATE_GETTING) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        }
    }
}

int window_building_handle_mouse_warehouse_orders(const mouse *m, building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_resource_buttons, city_resource_get_available()->size,
        &data.resource_focus_button_id)) {
        return 1;
    }
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404,
        warehouse_order_buttons, 3, &data.orders_focus_button_id);
}

void window_building_get_tooltip_warehouse_orders(int *group_id, int *text_id)
{
    if (data.orders_focus_button_id == 3) {
        *group_id = 15;
        *text_id = 1;
    }
}

static void go_to_orders(int param1, int param2)
{
    window_building_info_show_storage_orders();
}

static void toggle_resource_state(int index, int param2)
{
    building *b = building_get(data.building_id);
    int resource;
    if (b->type == BUILDING_WAREHOUSE) {
        resource = city_resource_get_available()->items[index-1];
    } else {
        resource = city_resource_get_available_foods()->items[index-1];
    }
    building_storage_cycle_resource_state(b->storage_id, resource);
    window_invalidate();
}

static void granary_orders(int index, int param2)
{
    int storage_id = building_get(data.building_id)->storage_id;
    if (index == 0) {
        building_storage_toggle_empty_all(storage_id);
    } else if (index == 1) {
        building_storage_accept_none(storage_id);
    }
    window_invalidate();
}

static void warehouse_orders(int index, int param2)
{
    if (index == 0) {
        int storage_id = building_get(data.building_id)->storage_id;
        building_storage_toggle_empty_all(storage_id);
    } else if (index == 1) {
        city_buildings_set_trade_center(data.building_id);
    } else if (index == 2) {
        int storage_id = building_get(data.building_id)->storage_id;
        building_storage_accept_none(storage_id);
    }
    window_invalidate();
}
