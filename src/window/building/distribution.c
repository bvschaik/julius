#include "distribution.h"

#include "building/building.h"
#include "building/dock.h"
#include "building/market.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/military.h"
#include "city/resource.h"
#include "empire/city.h"
#include "empire/object.h"
#include "figure/figure.h"
#include "game/resource.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "window/building_info.h"

#include <math.h>

static void go_to_orders(int param1, int param2);
static void toggle_resource_state(int index, int param2);
static void toggle_partial_resource_state(int index, int param2);
static void granary_orders(int param1, int param2);
static void dock_toggle_route(int param1, int param2);
static void warehouse_orders(int index, int param2);
static void market_orders(int index, int param2);
static void storage_toggle_permissions(int index, int param2);
static void init_dock_permission_buttons();
static void draw_dock_permission_buttons(int x_offset, int y_offset, int dock_id);
static void on_dock_cities_scroll(void);
static int dock_cities_scroll_position(void);
static void dock_cities_set_scroll_position(int);

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

static generic_button orders_partial_resource_buttons[] = {
    {210, 0, 28, 22, toggle_partial_resource_state, button_none, 1, 0},
    {210, 22, 28, 22, toggle_partial_resource_state, button_none, 2, 0},
    {210, 44, 28, 22, toggle_partial_resource_state, button_none, 3, 0},
    {210, 66, 28, 22, toggle_partial_resource_state, button_none, 4, 0},
    {210, 88, 28, 22, toggle_partial_resource_state, button_none, 5, 0},
    {210, 110, 28, 22, toggle_partial_resource_state, button_none, 6, 0},
    {210, 132, 28, 22, toggle_partial_resource_state, button_none, 7, 0},
    {210, 154, 28, 22, toggle_partial_resource_state, button_none, 8, 0},
    {210, 176, 28, 22, toggle_partial_resource_state, button_none, 9, 0},
    {210, 198, 28, 22, toggle_partial_resource_state, button_none, 10, 0},
    {210, 220, 28, 22, toggle_partial_resource_state, button_none, 11, 0},
    {210, 242, 28, 22, toggle_partial_resource_state, button_none, 12, 0},                                                   
    {210, 264, 28, 22, toggle_partial_resource_state, button_none, 13, 0},
    {210, 286, 28, 22, toggle_partial_resource_state, button_none, 14, 0},
    {210, 308, 28, 22, toggle_partial_resource_state, button_none, 15, 0},
};

static generic_button warehouse_distribution_permissions_buttons[] = {
     {0, 0, 20, 22, storage_toggle_permissions, button_none, 1, 0},
     {96, 0, 20, 22, storage_toggle_permissions, button_none, 2, 0},
     {228, 0, 20, 22, storage_toggle_permissions, button_none, 3, 0},

};

static generic_button granary_distribution_permissions_buttons[] = {
     {0, 0, 20, 22, storage_toggle_permissions, button_none, 1, 0},
     {96, 0, 20, 22, storage_toggle_permissions, button_none, 4, 0},
};

static generic_button dock_distribution_permissions_buttons[20];

static int dock_distribution_permissions_buttons_count;

static scrollbar_type dock_scrollbar = { 0, 0, 0, on_dock_cities_scroll};

static generic_button granary_order_buttons[] = {
    {0, 0, 304, 20, granary_orders, button_none, 0, 0},
    {314, 0, 20, 20, granary_orders, button_none, 1, 0},
};

static generic_button market_order_buttons[] = {
    {314, 0, 20, 20, market_orders, button_none, 0, 0},
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
    int permission_focus_button_id;
    int building_id;
    int partial_resource_focus_button_id;
    int tooltip_id;
    int dock_scrollbar_position;
    int dock_max_cities_visible;
} data;

uint8_t warehouse_full_button_text[] = "32";
uint8_t warehouse_3quarters_button_text[] = "24";
uint8_t warehouse_half_button_text[] = "16";
uint8_t warehouse_quarter_button_text[] = "8";
uint8_t granary_full_button_text[] = "24";
uint8_t granary_3quarters_button_text[] = "18";
uint8_t granary_half_button_text[] = "12";
uint8_t granary_quarter_button_text[] = "6";

static void draw_accept_none_button(int x, int y, int focused)
{
    uint8_t refuse_button_text[] = { 'x', 0 };
    button_border_draw(x, y, 20, 20, focused ? 1 : 0);
    text_draw_centered(refuse_button_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
}

static void draw_permissions_buttons(int x, int y, int buttons)
{
    uint8_t permission_button_text[] = { 'x', 0 };
    int offsets[] = { 96, 132, 96 };
    for (int i = 0; i < buttons; i++)
    {
        button_border_draw(x, y, 20, 20, data.permission_focus_button_id == i+1 ? 1 : 0);
        if (building_storage_get_permission(i, building_get(data.building_id))) {
            text_draw_centered(permission_button_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
        }
        x += offsets[i];
    }
}

static void draw_granary_permissions_buttons(int x, int y, int buttons)
{
    uint8_t permission_button_text[] = { 'x', 0 };
    int offsets[] = { 96, 132, 96 };
    for (int i = 0; i < buttons; i++)
    {
        int permission = granary_distribution_permissions_buttons[i].parameter1 - 1;
        button_border_draw(x, y, 20, 20, data.permission_focus_button_id == i + 1 ? 1 : 0);
        if (building_storage_get_permission(permission, building_get(data.building_id))) {
            text_draw_centered(permission_button_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
        }
        x += offsets[i];
    }
}

static void init_dock_permission_buttons()
{
    dock_distribution_permissions_buttons_count = 0;
    for (int route_id = 0; route_id < 20; route_id++) {
        int city_id = -1;
        if (is_sea_trade_route(route_id) && empire_city_is_trade_route_open(route_id)) {
            city_id = empire_city_get_for_trade_route(route_id);
            if (city_id != -1) {
                generic_button button = {0, 0, 210, 22, dock_toggle_route, button_none, route_id, city_id};
                dock_distribution_permissions_buttons[dock_distribution_permissions_buttons_count] = button;
                dock_distribution_permissions_buttons_count++;
            }
        }
    }
}

static void draw_dock_permission_buttons(int x_offset, int y_offset, int dock_id)
{
    int button_order = 0;
    for (int i = 0; i < dock_distribution_permissions_buttons_count; i++) {
        if (i < dock_scrollbar.scroll_position || i - dock_scrollbar.scroll_position >= data.dock_max_cities_visible) {
            continue;
        }
        generic_button *button = &dock_distribution_permissions_buttons[i];
        int scrollbar_shown = dock_distribution_permissions_buttons_count > data.dock_max_cities_visible;
        button->x = scrollbar_shown ? 160 : 190;
        button->y = 22 * (i - dock_scrollbar.scroll_position);
        button_border_draw(x_offset + button->x, y_offset + button->y, button->width, button->height, data.permission_focus_button_id == i + 1 ? 1 : 0);
        int state = building_dock_can_trade_with_route(dock_distribution_permissions_buttons[i].parameter1, dock_id);
        if (state) {
            lang_text_draw_centered(99, 7, x_offset + button->x, y_offset + button->y + 5, button->width, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw_centered(99, 8, x_offset + button->x, y_offset + button->y + 5, button->width, FONT_NORMAL_RED);
        }
        empire_city *city = empire_city_get(button->parameter2);
        lang_text_draw(21, city->name_id, x_offset + (scrollbar_shown ? 10 : 30), y_offset + 4 + button->y, FONT_NORMAL_WHITE);
    }
}

void window_building_draw_dock(building_info_context *c)
{
    c->help_id = 83;
    window_building_play_sound(c, "wavs/dock.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(101, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);

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
    init_dock_permission_buttons();
    text_draw_centered(translation_for(TR_BUILDING_DOCK_CITIES_CONFIG_DESC), c->x_offset, c->y_offset + 240, 16 * c->width_blocks, FONT_NORMAL_BLACK, 0);
    int panel_height = c->height_blocks - 21;
    data.dock_max_cities_visible = floor(panel_height * 16 / 22);
    int scrollbar_shown = dock_distribution_permissions_buttons_count > data.dock_max_cities_visible;
    int panel_width;
    if (scrollbar_shown) {
        panel_width = c->width_blocks - 5;
    } else {
        panel_width = c->width_blocks - 2;
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 270, panel_width, panel_height);
    dock_scrollbar.x = c->x_offset + (c->width_blocks - 4) * 16;
    dock_scrollbar.y = c->y_offset + 270;
    dock_scrollbar.height = panel_height * 16;
    scrollbar_init(&dock_scrollbar, dock_cities_scroll_position(), dock_distribution_permissions_buttons_count - data.dock_max_cities_visible);
    if (!dock_distribution_permissions_buttons_count) {
        text_draw_centered(translation_for(TR_BUILDING_DOCK_CITIES_NO_ROUTES), c->x_offset + 16, c->y_offset + 270 + panel_height * 16 / 2 - 7, panel_width * 16, FONT_SMALL_BLACK, 0);
    }
}

void window_building_draw_dock_foreground(building_info_context* c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    draw_dock_permission_buttons(c->x_offset + 16, c->y_offset + 270 + 5, c->building_id);
    scrollbar_draw(&dock_scrollbar);
}

void window_building_draw_dock_orders(building_info_context* c)
{
    c->help_id = 83;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(101, 0, c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_dock_orders_foreground(building_info_context* c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 1);
    for (int i = 0; i < 15; i++) {
        resource_type resource = i + 1;
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 408, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);
        building* b = building_get(c->building_id);
        int state = building_market_is_good_accepted(i, b);
        if (state) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        }
        else {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        }
    }
}

int window_building_handle_mouse_dock(const mouse* m, building_info_context* c)
{
    int handled;

    data.building_id = c->building_id;
    handled = generic_buttons_handle_mouse(
        m, c->x_offset + 16, c->y_offset + 270 + 5,
        dock_distribution_permissions_buttons, dock_distribution_permissions_buttons_count, &data.permission_focus_button_id);
    if (handled) {
        return handled;
    }

    handled = scrollbar_handle_mouse(&dock_scrollbar, m);
    if (handled) {
        return handled;
    }

    handled = generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
    return handled;
}

int window_building_handle_mouse_dock_orders(const mouse* m, building_info_context* c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_resource_buttons, 15,
        &data.resource_focus_button_id)) {
        return 1;
    }
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404, market_order_buttons, 1, &data.orders_focus_button_id);
}

static void on_dock_cities_scroll()
{
    dock_cities_set_scroll_position(dock_scrollbar.scroll_position);
    window_invalidate();
}

static int dock_cities_scroll_position()
{
    return data.dock_scrollbar_position;
}

void dock_cities_set_scroll_position(int scroll_position)
{
    data.dock_scrollbar_position = scroll_position;
}

void window_building_draw_stocks(building_info_context* c, building* b, int draw_goods, int always_show_food) 
{
    int image_id = image_group(GROUP_RESOURCE_ICONS);
    font_t font;

    if (always_show_food || b->data.market.inventory[INVENTORY_WHEAT] || b->data.market.inventory[INVENTORY_VEGETABLES] ||
        b->data.market.inventory[INVENTORY_FRUIT] || b->data.market.inventory[INVENTORY_MEAT]) {
        // food stocks
        font = building_market_is_good_accepted(INVENTORY_WHEAT, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_WHEAT, c->x_offset + 32, c->y_offset + 64);
        text_draw_number(b->data.market.inventory[INVENTORY_WHEAT], '@', " ",
            c->x_offset + 64, c->y_offset + 70, font);

        font = building_market_is_good_accepted(INVENTORY_VEGETABLES, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_VEGETABLES, c->x_offset + 142, c->y_offset + 64);
        text_draw_number(b->data.market.inventory[INVENTORY_VEGETABLES], '@', " ",
            c->x_offset + 174, c->y_offset + 70, font);

        font = building_market_is_good_accepted(INVENTORY_FRUIT, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_FRUIT, c->x_offset + 252, c->y_offset + 64);
        text_draw_number(b->data.market.inventory[INVENTORY_FRUIT], '@', " ",
            c->x_offset + 284, c->y_offset + 70, font);

        font = building_market_is_good_accepted(INVENTORY_MEAT, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_MEAT +
            resource_image_offset(RESOURCE_MEAT, RESOURCE_IMAGE_ICON),
            c->x_offset + 362, c->y_offset + 64);
        text_draw_number(b->data.market.inventory[INVENTORY_MEAT], '@', " ",
            c->x_offset + 394, c->y_offset + 70, font);
    }
    else {
        window_building_draw_description_at(c, 48, 97, 4);
    }
    // good stocks
    if (draw_goods) {
        font = building_market_is_good_accepted(INVENTORY_POTTERY, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_POTTERY, c->x_offset + 32, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_POTTERY], '@', " ",
            c->x_offset + 64, c->y_offset + 110, font);

        font = building_market_is_good_accepted(INVENTORY_FURNITURE, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_FURNITURE, c->x_offset + 142, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_FURNITURE], '@', " ",
            c->x_offset + 174, c->y_offset + 110, font);

        font = building_market_is_good_accepted(INVENTORY_OIL, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_OIL, c->x_offset + 252, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_OIL], '@', " ",
            c->x_offset + 284, c->y_offset + 110, font);

        font = building_market_is_good_accepted(INVENTORY_WINE, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
        image_draw(image_id + RESOURCE_WINE, c->x_offset + 362, c->y_offset + 104);
        text_draw_number(b->data.market.inventory[INVENTORY_WINE], '@', " ",
            c->x_offset + 394, c->y_offset + 110, font);
    }
}

void window_building_draw_market(building_info_context* c)
{
    c->help_id = 2;
    window_building_play_sound(c, "wavs/market.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(97, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    building* b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    }
    else if (b->num_workers <= 0) {
        window_building_draw_description(c, 97, 2);
    }
    else {
        window_building_draw_stocks(c, b, 1, 0);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_draw_market_foreground(building_info_context* c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
}

void window_building_draw_market_orders(building_info_context* c)
{
    c->help_id = 3;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    text_draw_centered(translation_for(TR_MARKET_SPECIAL_ORDERS_HEADER), c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_market_orders_foreground(building_info_context* c, int food_only, int limit_resource, int resource_id_1, int resource_id_2)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    int resource_max = INVENTORY_MAX;
    int resource_min = INVENTORY_WHEAT;
    resource_type resources[8] = { RESOURCE_WHEAT,RESOURCE_VEGETABLES,RESOURCE_FRUIT,RESOURCE_MEAT,RESOURCE_WINE,RESOURCE_OIL,RESOURCE_FURNITURE,RESOURCE_POTTERY };

    if (food_only) {
        resource_max = 4;
        for (int i = 4; i < 8; ++i) {
            resources[i] = 0;
        }        
    } else if (limit_resource) {
        int i = 1;
        if (!resource_id_2) {
            resources[0] = resource_id_1;
        } else {
            resources[0] = resource_id_1;
            resources[1] = resource_id_2;
            i++;
        }
        for (; i < 8; ++i) {
            resources[i] = 0;
        }
    }

    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 1);

    for (int i = resource_min; i < resource_max; i++) {
        resource_type resource = resources[i];
        if (!resource) {
            continue;
        }
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 408, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);
        building *b = building_get(c->building_id);
        int state = building_market_is_good_accepted(i,b);
        if (state) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        }
    }
}

void window_building_handle_mouse_market(const mouse* m, building_info_context* c)
{
    generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
}

void window_building_handle_mouse_market_orders(const mouse* m, building_info_context* c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_resource_buttons, INVENTORY_MAX,
        &data.resource_focus_button_id)) {
        return;
    }
    generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404, market_order_buttons, 1, &data.orders_focus_button_id);
}


void window_building_draw_granary(building_info_context *c)
{
    c->help_id = 3;
    data.building_id = c->building_id;    
    window_building_play_sound(c, "wavs/granary.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(98, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
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
    image_draw(image_group(GROUP_FIGURE_MARKET_LADY) + 4, c->x_offset + 28, c->y_offset + 19 * c->height_blocks - 133);
    image_draw(image_group(GROUP_FIGURE_TOWER_SENTRY) + 4, c->x_offset + 128, c->y_offset + 19 * c->height_blocks - 133);
    if (has_cart_orders) {
        int resource = figure_get(cartpusher)->resource_id;
        image_draw(image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 32, c->y_offset + 190);
        lang_text_draw_multiline(99, 17, c->x_offset + 64, c->y_offset + 193,
            16 * (c->width_blocks - 5), FONT_SMALL_BLACK);
    }
}

void window_building_draw_granary_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    draw_granary_permissions_buttons(c->x_offset + 58, c->y_offset + 19 * c->height_blocks - 122, 2);

}

int window_building_handle_mouse_granary(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 58, c->y_offset + 19 * c->height_blocks - 122,
        granary_distribution_permissions_buttons, 2, &data.permission_focus_button_id)) {
    }
    return generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);


}

void window_building_draw_granary_orders(building_info_context *c)
{
    c->help_id = 3;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(98, 6, c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}


void window_building_draw_granary_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    // empty button
    button_border_draw(c->x_offset + 80, y_offset + 404, 16 * (c->width_blocks - 10), 20,
        data.orders_focus_button_id == 1 ? 1 : 0);
    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);
    if (storage->empty_all) {
        lang_text_draw_centered(98, 8, c->x_offset + 80, y_offset + 408,
            16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(98, 9, c->x_offset, y_offset + 384,
            16 * c->width_blocks, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(98, 7, c->x_offset + 80, y_offset + 408,
            16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 2);

    const resource_list *list = city_resource_get_available_foods();
    for (int i = 0; i < list->size; i++) {
        int resource = list->items[i];
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 420, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);
        button_border_draw(c->x_offset + 390, y_offset + 46 + 22 * i, 28, 22, data.partial_resource_focus_button_id == i + 1);

        int state = storage->resource_state[resource];
        if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_full_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
            text_draw_centered(granary_3quarters_button_text, c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_HALF) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_half_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_quarter_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER) {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        } else if (state == BUILDING_STORAGE_STATE_GETTING) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_full_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_3QUARTERS) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
            text_draw_centered(granary_3quarters_button_text, c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_HALF) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_half_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_QUARTER) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(granary_quarter_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
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

    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_partial_resource_buttons, city_resource_get_available()->size,
        &data.partial_resource_focus_button_id)) {
        return 1;
    }
    
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404, granary_order_buttons, 2, &data.orders_focus_button_id);
}

void window_building_get_tooltip_granary_orders(int *group_id, int *text_id, int *translation)
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
    lang_text_draw_centered(99, 0, c->x_offset, c->y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);
    data.building_id = c->building_id;
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
            16 * (c->width_blocks - 5), FONT_SMALL_BLACK);
    } else if (b->num_workers) {
        // cartpusher is waiting for orders
        lang_text_draw_multiline(99, 15, c->x_offset + 32, c->y_offset + 223,
            16 * (c->width_blocks - 3), FONT_SMALL_BLACK);
    }

    //if (c->warehouse_space_text == 1) { // full
    //    lang_text_draw_multiline(99, 13, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 93,
    //        16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    //}  else if (c->warehouse_space_text == 2) {
    //    lang_text_draw_multiline(99, 14, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 93,
    //        16 * (c->width_blocks - 4), FONT_NORMAL_BLACK);
    //}
    image_draw(image_group(GROUP_FIGURE_MARKET_LADY) + 4, c->x_offset + 32, c->y_offset + 16 * c->height_blocks - 93);
    image_draw(image_group(GROUP_FIGURE_TRADE_CARAVAN) + 4, c->x_offset + 128, c->y_offset + 16 * c->height_blocks - 93);
    image_draw(image_group(GROUP_FIGURE_SHIP) + 4, c->x_offset + 216, c->y_offset + 16 * c->height_blocks - 110);
}

void window_building_draw_warehouse_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        16 * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(99, 2, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 30,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);

    draw_permissions_buttons(c->x_offset + 64, c->y_offset + 16 * c->height_blocks - 75, 3);

}

int window_building_handle_mouse_warehouse(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 80, c->y_offset + 16 * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id)) {
    }
    if (generic_buttons_handle_mouse(m, c->x_offset + 64, c->y_offset + 16 * c->height_blocks - 75,
        warehouse_distribution_permissions_buttons, 3, &data.permission_focus_button_id)) {
    }
    return 0;
}

void window_building_draw_warehouse_orders(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    c->help_id = 4;
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(99, 3, c->x_offset, y_offset + 10, 16 * c->width_blocks, FONT_LARGE_BLACK);
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - 2, 21);
}

void window_building_draw_warehouse_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    // emptying button
    button_border_draw(c->x_offset + 80, y_offset + 404, 16 * (c->width_blocks - 10),
        20, data.orders_focus_button_id == 1 ? 1 : 0);
    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);
    if (storage->empty_all) {
        lang_text_draw_centered(99, 5, c->x_offset + 80, y_offset + 408,
            16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(99, 6, c->x_offset, y_offset + 426, 16 * c->width_blocks, FONT_SMALL_PLAIN);
    } else {
        lang_text_draw_centered(99, 4, c->x_offset + 80, y_offset + 408,
            16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    // trade center
    button_border_draw(c->x_offset + 80, y_offset + 382, 16 * (c->width_blocks - 10),
        20, data.orders_focus_button_id == 2 ? 1 : 0);
    int is_trade_center = c->building_id == city_buildings_get_trade_center();
    lang_text_draw_centered(99, is_trade_center ? 11 : 12, c->x_offset + 80, y_offset + 386,
        16 * (c->width_blocks - 10), FONT_NORMAL_BLACK);

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 3);

    const resource_list *list = city_resource_get_available();
    for (int i = 0; i < list->size; i++) {
        int resource = list->items[i];
        int image_id = image_group(GROUP_RESOURCE_ICONS) + resource +
            resource_image_offset(resource, RESOURCE_IMAGE_ICON);
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i);
        image_draw(image_id, c->x_offset + 420, y_offset + 46 + 22 * i);
        lang_text_draw(23, resource, c->x_offset + 72, y_offset + 50 + 22 * i, FONT_NORMAL_WHITE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22, data.resource_focus_button_id == i + 1);
        button_border_draw(c->x_offset + 390, y_offset + 46 + 22 * i, 28, 22, data.partial_resource_focus_button_id == i + 1);


        int state = storage->resource_state[resource];
        if (state == BUILDING_STORAGE_STATE_ACCEPTING) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_full_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
            text_draw_centered(warehouse_3quarters_button_text, c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_HALF) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_half_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER) {
            lang_text_draw(99, 7, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_quarter_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_NOT_ACCEPTING || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF || state == BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER) {
            lang_text_draw(99, 8, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_RED);
        } else if (state == BUILDING_STORAGE_STATE_GETTING) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_full_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_3QUARTERS) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
            text_draw_centered(warehouse_3quarters_button_text, c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_HALF) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_half_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
        } else if (state == BUILDING_STORAGE_STATE_GETTING_QUARTER) {
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, c->x_offset + 186, y_offset + 49 + 22 * i);
            lang_text_draw(99, 9, c->x_offset + 230, y_offset + 51 + 22 * i, FONT_NORMAL_WHITE);
	    text_draw_centered(warehouse_quarter_button_text,c->x_offset + 394, y_offset + 51 + 22 * i, 20, FONT_NORMAL_BLACK, 0);
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
    if (generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
        orders_partial_resource_buttons, city_resource_get_available()->size,
        &data.partial_resource_focus_button_id)) {
        return 1;
    }
    return generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404,
        warehouse_order_buttons, 3, &data.orders_focus_button_id);
}

void window_building_warehouse_get_tooltip_distribution_permissions(int* translation)
{
    switch (data.permission_focus_button_id) {
    case 1:
        *translation = TR_TOOLTIP_BUTTON_ACCEPT_MARKET_LADIES;
        break;
    case 2:
        *translation = TR_TOOLTIP_BUTTON_ACCEPT_TRADE_CARAVAN;
        break;
    case 3:
        *translation = TR_TOOLTIP_BUTTON_ACCEPT_TRADE_SHIPS;
        break;
    default:
        break;
    }

}

void window_building_granary_get_tooltip_distribution_permissions(int* translation)
{
    switch (data.permission_focus_button_id) {
    case 1:
        *translation = TR_TOOLTIP_BUTTON_ACCEPT_MARKET_LADIES;
        break;
    case 2:
        *translation = TR_TOOLTIP_BUTTON_ACCEPT_QUARTERMASTER;
    default:
        break;
    }

}


void window_building_get_tooltip_warehouse_orders(int *group_id, int *text_id, int *translation)
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
    if (building_has_market_inventory(b->type)) {
        building_market_toggle_good_accepted(index - 1, b);
    }
    else {
        if (b->type == BUILDING_WAREHOUSE) {
            resource = city_resource_get_available()->items[index - 1];
        }
        else {
            resource = city_resource_get_available_foods()->items[index - 1];
        }
        building_storage_cycle_resource_state(b->storage_id, resource);
    }
    window_invalidate();
}

static void market_orders(int index, int param2)
{
    building *b = building_get(data.building_id);
    if (index == 0) {
        building_market_unaccept_all_goods(b);
    }
    window_invalidate();
}

static void storage_toggle_permissions(int index, int param2)
{
    building* b = building_get(data.building_id);
    building_storage_set_permission(index-1,b);
    window_invalidate();
}

static void toggle_partial_resource_state(int index, int param2)
{
    building *b = building_get(data.building_id);
    int resource;
    if (b->type == BUILDING_WAREHOUSE) {
        resource = city_resource_get_available()->items[index-1];
    } else {
        resource = city_resource_get_available_foods()->items[index-1];
    }
    building_storage_cycle_partial_resource_state(b->storage_id, resource);
    window_invalidate();
}

static void dock_toggle_route(int route_id, int city_id)
{
    int can_trade = building_dock_can_trade_with_route(route_id, data.building_id);
    building_dock_set_can_trade_with_route(route_id, data.building_id, !can_trade);
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

void window_building_draw_mess_hall(building_info_context* c)
{
    building* b = building_get(c->building_id);
    int mess_hall_fulfillment_display = 100 - city_mess_hall_food_missing_month();
    int food_stress = city_mess_hall_food_stress();
    int hunger_text;

    window_building_play_sound(c, "wavs/warehouse2.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
   
    text_draw_centered(translation_for(TR_BUILDING_MESS_HALL), c->x_offset, c->y_offset + 12, 16 * c->width_blocks, FONT_LARGE_BLACK, 0);
    window_building_draw_stocks(c, b, 0, 1);
    if (city_military_total_soldiers_in_city() > 0) {
        int width = text_draw(translation_for(TR_BUILDING_MESS_HALL_FULFILLMENT), c->x_offset + 32, c->y_offset + 106, FONT_NORMAL_BLACK, 0);
        text_draw_percentage(mess_hall_fulfillment_display, c->x_offset + 32 + width, c->y_offset + 106, FONT_NORMAL_BLACK);
        width = text_draw(translation_for(TR_BUILDING_MESS_HALL_TROOP_HUNGER), c->x_offset + 32, c->y_offset + 126, FONT_NORMAL_BLACK, 0);
        if (food_stress < 3) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_1;
        }
        else if (food_stress > 80) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_5;
        }
        else if (food_stress > 60) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_4;
        }
        else if (food_stress > 40) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_3;
        }
        else {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_2;
        }

        text_draw(translation_for(hunger_text), c->x_offset + 32 + width, c->y_offset + 126, FONT_NORMAL_BLACK, 0);

        width = text_draw(translation_for(TR_BUILDING_MESS_HALL_MONTHS_FOOD_STORED), c->x_offset + 32, c->y_offset + 150, FONT_NORMAL_BLACK, 0);
        text_draw_number(city_mess_hall_months_food_stored(), '@', " ", c->x_offset + 32 + width, c->y_offset + 150, FONT_NORMAL_BLACK);

        if (city_mess_hall_food_types() == 2) {
            text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_FOOD_TYPES_BONUS_1), c->x_offset + 32, c->y_offset + 175, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
        else if (city_mess_hall_food_types() >= 3) {
            text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_FOOD_TYPES_BONUS_2), c->x_offset + 32, c->y_offset + 175, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
    }
    else {
        text_draw_centered(translation_for(TR_BUILDING_MESS_HALL_NO_SOLDIERS), c->x_offset, c->y_offset + 150, 16 * (c->width_blocks), FONT_NORMAL_BLACK, 0);
    }
    text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_DESC), c->x_offset + 32, c->y_offset + 226, 16 * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        
    inner_panel_draw(c->x_offset + 16, c->y_offset + 308, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 308);
    window_building_draw_market_foreground(c);
}
