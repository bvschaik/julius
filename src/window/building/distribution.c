#include "distribution.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/dock.h"
#include "building/granary.h"
#include "building/industry.h"
#include "building/market.h"
#include "building/monument.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/military.h"
#include "city/resource.h"
#include "city/trade_policy.h"
#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "empire/object.h"
#include "figure/figure.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "sound/speech.h"
#include "translation/translation.h"
#include "window/building_info.h"
#include "window/option_popup.h"

#include <math.h>

static void go_to_orders(int param1, int param2);
static void toggle_resource_state(int index, int param2);
static void toggle_partial_resource_state(int index, int param2);
static void granary_orders(int index, int param2);
static void dock_toggle_route(int route_id, int city_id);
static void warehouse_orders(int index, int param2);
static void market_orders(int index, int param2);
static void storage_toggle_permissions(int index, int param2);
static void button_stockpiling(int param1, int param2);
static void init_dock_permission_buttons(void);
static void draw_dock_permission_buttons(int x_offset, int y_offset, int dock_id);
static void on_scroll(void);

static void button_caravanserai_policy(int selected_policy, int param2);

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
    {0, 330, 210, 22, toggle_resource_state, button_none, 16, 0},
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
    {210, 330, 28, 22, toggle_partial_resource_state, button_none, 16, 0},
};

static generic_button warehouse_distribution_permissions_buttons[] = {
     {0, 0, 20, 22, storage_toggle_permissions, button_none, 1, 0},
     {96, 0, 20, 22, storage_toggle_permissions, button_none, 2, 0},
     {228, 0, 20, 22, storage_toggle_permissions, button_none, 3, 0},
     {324, 0, 20, 22, storage_toggle_permissions, button_none, 5, 0},
};

static generic_button granary_distribution_permissions_buttons[] = {
     {0, 0, 20, 22, storage_toggle_permissions, button_none, 1, 0},
     {96, 0, 20, 22, storage_toggle_permissions, button_none, 4, 0},
     {192, 0, 20, 22, storage_toggle_permissions, button_none, 2, 0},
     {324, 0, 20, 22, storage_toggle_permissions, button_none, 3, 0},
};

static generic_button dock_distribution_permissions_buttons[20];

static int dock_distribution_permissions_buttons_count;

static scrollbar_type scrollbar = { .on_scroll_callback = on_scroll };

static generic_button granary_order_buttons[] = {
    {0, 0, 304, 20, granary_orders, button_none, 0, 0},
    {314, 0, 20, 20, granary_orders, button_none, 1, 0},
};

static generic_button market_order_buttons[] = {
    {314, 0, 20, 20, market_orders, button_none, 0, 0},
};

static generic_button warehouse_order_buttons[] = {
    {0, 0, 304, 20, warehouse_orders, button_none, 0, 0},
    {314, 0, 20, 20, warehouse_orders, button_none, 1, 0},
};

static generic_button go_to_caravanserai_action_button[] = {
        {0, 0, 400, 100, button_caravanserai_policy, button_none, 0, 0}
};

static image_button image_buttons_maintain[] = {
    {324, 0, 30, 19, IB_NORMAL, 0, 0, storage_toggle_permissions, button_none, 5, 0, 1, "UI", "Maintain_1"},
    {324, 0, 30, 19, IB_NORMAL, 0, 0, storage_toggle_permissions, button_none, 5, 0, 1, "UI", "Stop_Maintain_1"},
};

static struct {
    int title;
    int subtitle;
    const char *base_image_name;
    option_menu_item items[4];
    const char *wav_file;
} land_trade_policy = {
    TR_BUILDING_CARAVANSERAI_POLICY_TITLE,
    TR_BUILDING_CARAVANSERAI_POLICY_TEXT,
    "Trade Policy",
    {
        { TR_BUILDING_CARAVANSERAI_NO_POLICY },
        { TR_BUILDING_CARAVANSERAI_POLICY_1_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_1 },
        { TR_BUILDING_CARAVANSERAI_POLICY_2_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_2 },
        { TR_BUILDING_CARAVANSERAI_POLICY_3_TITLE, TR_BUILDING_CARAVANSERAI_POLICY_3 }
    },
    "wavs/market4.wav"
};

static generic_button primary_product_producer_button_stockpiling[] = {
    {360, 3, 24, 24, button_stockpiling, button_none, 0, 0}
};

static struct {
    int focus_button_id;
    int orders_focus_button_id;
    int resource_focus_button_id;
    int permission_focus_button_id;
    int building_id;
    int partial_resource_focus_button_id;
    int tooltip_id;
    int dock_max_cities_visible;
    int caravanserai_focus_button_id;
    int primary_product_stockpiling_id;
    int image_button_focus_id;
    int showing_special_orders;
    resource_list available_resources;
} data;

uint8_t warehouse_full_button_text[] = "32";
uint8_t warehouse_3quarters_button_text[] = "24";
uint8_t warehouse_half_button_text[] = "16";
uint8_t warehouse_quarter_button_text[] = "8";
uint8_t granary_full_button_text[] = "32";
uint8_t granary_3quarters_button_text[] = "24";
uint8_t granary_half_button_text[] = "16";
uint8_t granary_quarter_button_text[] = "8";

static void draw_accept_none_button(int x, int y, int focused)
{
    uint8_t refuse_button_text[] = { 'x', 0 };
    button_border_draw(x, y, 20, 20, focused ? 1 : 0);
    text_draw_centered(refuse_button_text, x + 1, y + 4, 20, FONT_NORMAL_BLACK, 0);
}

static void draw_permissions_buttons(int x, int y, int buttons, building_info_context *c)
{
    building_storage_permission_states rules[] = {
        BUILDING_STORAGE_PERMISSION_MARKET,
        BUILDING_STORAGE_PERMISSION_TRADERS,
        BUILDING_STORAGE_PERMISSION_DOCK,
    };
    int offsets[] = { 96, 132, 96, 96 };
    int dx = x;
    for (int i = 0; i < buttons; i++)
    {
        int rule_id = rules[i];
        button_border_draw(dx, y, 20, 20, data.permission_focus_button_id == i + 1 ? 1 : 0);
        if (building_storage_get_permission(rule_id, building_get(data.building_id))) {
            image_draw(assets_get_image_id("UI", "Allowed_Walker_Check"), dx + 4, y + 4,
                COLOR_MASK_NONE, SCALE_NONE);
        }
        dx += offsets[i];
    }
    building *b = building_get(c->building_id);
    int button = 1;
    if (building_storage_get_permission(BUILDING_STORAGE_PERMISSION_WORKER, b)) {
        button = 2;
    }
    image_buttons_draw(x, y, image_buttons_maintain, button);
}

static void draw_granary_permissions_buttons(int x, int y, int buttons)
{
    int offsets[] = { 96, 96, 132, 96 };
    for (int i = 0; i < buttons; i++) {
        int permission = granary_distribution_permissions_buttons[i].parameter1 - 1;
        button_border_draw(x, y, 20, 20, data.permission_focus_button_id == i + 1 ? 1 : 0);
        if (building_storage_get_permission(permission, building_get(data.building_id))) {
            image_draw(assets_get_image_id("UI", "Allowed_Walker_Check"), x + 4, y + 4,
                COLOR_MASK_NONE, SCALE_NONE);
        }
        x += offsets[i];
    }
}

static void init_dock_permission_buttons(void)
{
    dock_distribution_permissions_buttons_count = 0;
    for (int route_id = 0; route_id < 20; route_id++) {
        int city_id = -1;
        if (is_sea_trade_route(route_id) && empire_city_is_trade_route_open(route_id)) {
            city_id = empire_city_get_for_trade_route(route_id);
            if (city_id != -1) {
                generic_button button = { 0, 0, 210, 22, dock_toggle_route, button_none, route_id, city_id };
                dock_distribution_permissions_buttons[dock_distribution_permissions_buttons_count] = button;
                dock_distribution_permissions_buttons_count++;
            }
        }
    }
}

static void draw_dock_permission_buttons(int x_offset, int y_offset, int dock_id)
{
    for (int i = 0; i < dock_distribution_permissions_buttons_count; i++) {
        if (i < scrollbar.scroll_position || i - scrollbar.scroll_position >= data.dock_max_cities_visible) {
            continue;
        }
        generic_button *button = &dock_distribution_permissions_buttons[i];
        int scrollbar_shown = dock_distribution_permissions_buttons_count > data.dock_max_cities_visible;
        button->x = scrollbar_shown ? 160 : 190;
        button->y = 22 * (i - scrollbar.scroll_position);
        button_border_draw(x_offset + button->x, y_offset + button->y, button->width, button->height,
            data.permission_focus_button_id == i + 1 ? 1 : 0);
        int state = building_dock_can_trade_with_route(dock_distribution_permissions_buttons[i].parameter1, dock_id);
        if (state) {
            lang_text_draw_centered(99, 7, x_offset + button->x, y_offset + button->y + 5, button->width,
                FONT_NORMAL_WHITE);
        } else {
            lang_text_draw_centered(99, 8, x_offset + button->x, y_offset + button->y + 5, button->width,
                FONT_NORMAL_RED);
        }
        empire_city *city = empire_city_get(button->parameter2);
        const uint8_t *city_name = empire_city_get_name(city);
        int x = x_offset + (scrollbar_shown ? 10 : 30);
        int y = y_offset + 4 + button->y;
        text_draw(city_name, x, y, FONT_NORMAL_WHITE, 0);
    }
}

void window_building_draw_dock(building_info_context *c)
{
    c->help_id = 83;

    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(101, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    building *b = building_get(c->building_id);

    if (b->has_plague) {
        window_building_play_sound(c, "wavs/clinic.wav");
        if (b->sickness_doctor_cure == 99) {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_FUMIGATION_DESC);
        } else {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_DOCK_PLAGUE_DESC);
        }
    } else {
        window_building_play_sound(c, "wavs/dock.wav");
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
    }

    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
    init_dock_permission_buttons();
    text_draw_centered(translation_for(TR_BUILDING_DOCK_CITIES_CONFIG_DESC), c->x_offset, c->y_offset + 240,
        BLOCK_SIZE * c->width_blocks, FONT_NORMAL_BLACK, 0);
    int panel_height = c->height_blocks - 21;
    data.dock_max_cities_visible = panel_height * BLOCK_SIZE / 22;
    int scrollbar_shown = dock_distribution_permissions_buttons_count > data.dock_max_cities_visible;
    int panel_width;
    if (scrollbar_shown) {
        panel_width = c->width_blocks - 5;
    } else {
        panel_width = c->width_blocks - 2;
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 270, panel_width, panel_height);
    if (data.showing_special_orders || data.building_id != c->building_id) {
        scrollbar.x = c->x_offset + (c->width_blocks - 4) * BLOCK_SIZE;
        scrollbar.y = c->y_offset + 270;
        scrollbar.height = panel_height * BLOCK_SIZE;
        scrollbar.scrollable_width = (c->width_blocks - 5) * BLOCK_SIZE;
        scrollbar.elements_in_view = data.dock_max_cities_visible;
        scrollbar_init(&scrollbar, 0, dock_distribution_permissions_buttons_count);
        data.showing_special_orders = 0;
    }
    if (!dock_distribution_permissions_buttons_count) {
        text_draw_centered(translation_for(TR_BUILDING_DOCK_CITIES_NO_ROUTES), c->x_offset + 16,
            c->y_offset + 270 + panel_height * BLOCK_SIZE / 2 - 7, panel_width * BLOCK_SIZE, FONT_NORMAL_BROWN, 0);
    }
}

void window_building_draw_dock_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    draw_dock_permission_buttons(c->x_offset + 16, c->y_offset + 275, c->building_id);
    scrollbar_draw(&scrollbar);
}

int window_building_handle_mouse_dock(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;
    data.permission_focus_button_id = 0;
    data.focus_button_id = 0;
    return scrollbar_handle_mouse(&scrollbar, m, 1) ||
        generic_buttons_handle_mouse(m, c->x_offset + 16, c->y_offset + 270 + 5, dock_distribution_permissions_buttons,
            dock_distribution_permissions_buttons_count, &data.permission_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
            go_to_orders_button, 1, &data.focus_button_id);
}

static int has_food_stocks(const building *b)
{
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (b->resources[r] && resource_is_inventory(r)) {
            return 1;
        }
    }
    return 0;
}

static void window_building_draw_stocks(building_info_context *c, building *b, int draw_goods,
    int always_show_food, int draw_higher)
{
    font_t font;

    int x_offset = 32;
    int y_offset = draw_higher ? 44 : 64;

    // food stocks
    if (always_show_food || has_food_stocks(b)) {
        const resource_list *list = city_resource_get_potential_foods();
        for (int i = 0; i < list->size; i++) {
            resource_type r = list->items[i];
            if (!resource_is_inventory(r) || (list->size > 4 && !b->resources[r])) {
                continue;
            }
            font = building_distribution_is_good_accepted(r, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
            image_draw(resource_get_data(r)->image.icon, c->x_offset + x_offset, c->y_offset + y_offset,
                COLOR_MASK_NONE, SCALE_NONE);
            text_draw_number(b->resources[r], '@', " ",
                c->x_offset + x_offset + 32, c->y_offset + y_offset + 6, font, 0);
            x_offset += 110;
        }
    } else {
        window_building_draw_description_at(c, 48, 97, 4);
    }

    x_offset = 32;
    y_offset += 40;

    // good stocks
    if (draw_goods) {
        for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
            if (!resource_is_inventory(r)) {
                continue;
            }
            font = building_distribution_is_good_accepted(r, b) ? FONT_NORMAL_BLACK : FONT_NORMAL_RED;
            image_draw(resource_get_data(r)->image.icon, c->x_offset + x_offset, c->y_offset + y_offset,
                COLOR_MASK_NONE, SCALE_NONE);
            text_draw_number(b->resources[r], '@', " ",
                c->x_offset + x_offset + 32, c->y_offset + y_offset + 6, font, 0);
            x_offset += 110;
        }
    }
}

void window_building_draw_market(building_info_context *c)
{
    c->help_id = 2;
    data.showing_special_orders = 0;
    window_building_play_sound(c, "wavs/market.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
    lang_text_draw_centered(97, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);
    building *b = building_get(c->building_id);

    if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else if (b->num_workers <= 0) {
        window_building_draw_description(c, 97, 2);
    } else {
        window_building_draw_stocks(c, b, 1, 0, 0);
    }
    inner_panel_draw(c->x_offset + 16, c->y_offset + 136, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 142);
}

void window_building_distributor_draw_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
}

static void set_distributed_resources(building_type type)
{
    for (int i = 0; i < data.available_resources.size; i++) {
        data.available_resources.items[i] = RESOURCE_NONE;
    }
    data.available_resources.size = 0;
    const resource_list *list = city_resource_get_potential();
    for (int i = 0; i < list->size; i++) {
        if (building_distribution_resource_is_handled(list->items[i], type)) {
            data.available_resources.items[data.available_resources.size++] = list->items[i];
        }
    }
}

void window_building_draw_distributor_orders(building_info_context *c, const uint8_t *title)
{
    building_type type = building_get(c->building_id)->type;
    c->help_id = type == BUILDING_DOCK ? 83 : 3;
    int y_offset = window_building_get_vertical_offset(c, 28);
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    text_draw_centered(title, c->x_offset, y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK, 0);

    if (!data.showing_special_orders || data.building_id != c->building_id) {
        set_distributed_resources(type);

        scrollbar.x = c->x_offset + (c->width_blocks - 3) * BLOCK_SIZE;
        scrollbar.y = y_offset + 42;
        scrollbar.height = 21 * BLOCK_SIZE;
        scrollbar.scrollable_width = (c->width_blocks - 2) * BLOCK_SIZE;
        scrollbar.elements_in_view = 21 * BLOCK_SIZE / 22;
        scrollbar_init(&scrollbar, 0, data.available_resources.size);

        data.showing_special_orders = 1;
    }

    int scrollbar_shown = scrollbar.max_scroll_position > 0;
    
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - (scrollbar_shown ? 4 : 2), 21);
}

void window_building_draw_distributor_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 1);
    building *b = building_get(c->building_id);
    int lang_group, lang_active_id, lang_inactive_id;
    switch (b->type) {
        case BUILDING_MARKET:
            lang_group = CUSTOM_TRANSLATION;
            lang_active_id = TR_MARKET_TRADING;
            lang_inactive_id = TR_MARKET_NOT_TRADING;
            break;
        case BUILDING_SMALL_TEMPLE_VENUS:
        case BUILDING_LARGE_TEMPLE_VENUS:
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_CERES:
            lang_group = CUSTOM_TRANSLATION;
            lang_active_id = TR_TEMPLE_DISTRIBUTING;
            lang_inactive_id = TR_TEMPLE_NOT_DISTRIBUTING;
            break;
        case BUILDING_TAVERN:
            lang_group = CUSTOM_TRANSLATION;
            lang_active_id = TR_TAVERN_FETCHING;
            lang_inactive_id = TR_TAVERN_NOT_FETCHING;
            break;
        case BUILDING_CARAVANSERAI:
        case BUILDING_MESS_HALL:
            lang_group = 99;
            lang_active_id = 10;
            lang_inactive_id = 8;
            break;
        default:
            lang_group = 99;
            lang_active_id = 7;
            lang_inactive_id = 8;
            break;
    }

    scrollbar_draw(&scrollbar);

    int scrollbar_shown = scrollbar.max_scroll_position > 0;

    for (int i = 0; i < scrollbar.elements_in_view && i < data.available_resources.size; i++) {
        resource_type resource = data.available_resources.items[i + scrollbar.scroll_position];
        int image_id = resource_get_data(resource)->image.icon;
        image_draw(image_id, c->x_offset + 32, y_offset + 46 + 22 * i, COLOR_MASK_NONE, SCALE_NONE);
        if (!scrollbar_shown) {
            image_draw(image_id, c->x_offset + 408, y_offset + 46 + 22 * i, COLOR_MASK_NONE, SCALE_NONE);
        }
        text_draw(resource_get_data(resource)->text, c->x_offset + 72, y_offset + 50 + 22 * i,
            FONT_NORMAL_WHITE, COLOR_MASK_NONE);
        button_border_draw(c->x_offset + 180, y_offset + 46 + 22 * i, 210, 22,
            data.resource_focus_button_id == i + 1);
        if (building_distribution_is_good_accepted(resource, b)) {
            lang_text_draw_centered(lang_group, lang_active_id,
                c->x_offset + 180, y_offset + 51 + 22 * i, 210, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw_centered(lang_group, lang_inactive_id,
                c->x_offset + 180, y_offset + 51 + 22 * i, 210, FONT_NORMAL_RED);
        }
    }
}

int window_building_handle_mouse_distributor(const mouse *m, building_info_context *c)
{
    return generic_buttons_handle_mouse(
        m, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id);
}

int window_building_handle_mouse_distributor_orders(const mouse *m, building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;

    int buttons_to_show = data.available_resources.size < scrollbar.elements_in_view ?
        data.available_resources.size : scrollbar.elements_in_view;

    return scrollbar_handle_mouse(&scrollbar, m, 1) ||
        generic_buttons_handle_mouse(m, c->x_offset + 180, y_offset + 46,
            orders_resource_buttons, buttons_to_show, &data.resource_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404, market_order_buttons, 1,
            &data.orders_focus_button_id);
}

int window_building_handle_mouse_primary_product_producer(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;
    return generic_buttons_handle_mouse(m, c->x_offset, c->y_offset + BLOCK_SIZE * c->height_blocks - 40,
            primary_product_producer_button_stockpiling, 1, &data.primary_product_stockpiling_id);
}

void window_building_draw_primary_product_stockpiling(building_info_context *c)
{
    int x = c->x_offset + primary_product_producer_button_stockpiling->x;
    int y = c->y_offset + primary_product_producer_button_stockpiling->y + BLOCK_SIZE * c->height_blocks - 40;
    button_border_draw(x, y, 20, 20, data.primary_product_stockpiling_id);
    if (building_stockpiling_enabled(building_get(c->building_id))) {
        image_draw(assets_get_image_id("UI", "Warehousing"), x + 4, y + 4, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void generate_granary_resource_list(building *granary)
{
    for (int i = 0; i < data.available_resources.size; i++) {
        data.available_resources.items[i] = RESOURCE_NONE;
    }
    data.available_resources.size = 0;
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (granary->resources[r] > 0) {
            data.available_resources.items[data.available_resources.size++] = r;
        }
    }
}

void window_building_draw_granary(building_info_context *c)
{
    c->help_id = 3;
    data.building_id = c->building_id;
    data.showing_special_orders = 0;

    building *b = building_get(c->building_id);

    int y_offset_blocks = 0;
    if (!b->has_plague && c->has_road_access) {
        generate_granary_resource_list(b);
        y_offset_blocks = ((data.available_resources.size - 1) / 2 - 3) * 2 + 2;
    }
    c->height_blocks = 22 + y_offset_blocks;    
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);

    lang_text_draw_centered(98, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    if (b->has_plague) {
        window_building_play_sound(c, "wavs/clinic.wav");
    } else {
        window_building_play_sound(c, "wavs/granary.wav");
    }

    if (b->has_plague) {
        if (b->sickness_doctor_cure == 99) {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_FUMIGATION_DESC);
        } else {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_GRANARY_PLAGUE_DESC);
        }
    } else if (!c->has_road_access) {
        window_building_draw_description_at(c, 40, 69, 25);
    } else if (scenario_property_rome_supplies_wheat()) {
        window_building_draw_description_at(c, 40, 98, 4);
    } else {
        if (data.available_resources.size == 0) {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUILDING_GRANARY_NO_FOOD, c->x_offset, c->y_offset + 63,
                BLOCK_SIZE * c->width_blocks, FONT_NORMAL_BLACK);
        } else {
            int total_stored = 0;
            int x;
            int y = c->y_offset + 31;
            for (int i = 0; i < data.available_resources.size; i++) {
                if (i & 1) {
                    x = c->x_offset + 240;
                } else {
                    x = c->x_offset + 20;
                    y += BLOCK_SIZE * 2;
                }
                resource_type r = data.available_resources.items[i];
                int amount = b->resources[r];
                total_stored += amount;
                image_draw(resource_get_data(r)->image.icon, x, y, COLOR_MASK_NONE, SCALE_NONE);
                int width = text_draw_number(amount, '@', " ", x + 24, y + 7, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
                text_draw(resource_get_data(r)->text, x + 24 + width, y + 7, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
            }
            int width = lang_text_draw(98, 2, c->x_offset + 34, c->y_offset + 40, FONT_NORMAL_BLACK);
            lang_text_draw_amount(8, 16, total_stored, c->x_offset + 34 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

            width = lang_text_draw(98, 3, c->x_offset + 220, c->y_offset + 40, FONT_NORMAL_BLACK);
            lang_text_draw_amount(8, 16, b->resources[RESOURCE_NONE],
                c->x_offset + 220 + width, c->y_offset + 40, FONT_NORMAL_BLACK);
        }
    }
    int y_offset = 160 + y_offset_blocks * BLOCK_SIZE;

    // cartpusher state
    inner_panel_draw(c->x_offset + 16, c->y_offset + y_offset + 8, c->width_blocks - 2, 6);
    window_building_draw_employment(c, y_offset + 13);
    int cartpusher = b->figure_id;
    figure *f = figure_get(cartpusher);
    if (cartpusher && f->state == FIGURE_STATE_ALIVE) {
        int resource = f->resource_id;
        if (resource) {
            image_draw(resource_get_data(resource)->image.icon,
                c->x_offset + 32, c->y_offset + y_offset + 60, COLOR_MASK_NONE, SCALE_NONE);
            if (f->action_state == FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE) {
                lang_text_draw_multiline(98, 9, c->x_offset + 64, c->y_offset + y_offset + 63,
                    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
            } else if (f->loads_sold_or_carrying) {
                text_draw_multiline(translation_for(TR_WINDOW_BUILDING_DISTRIBUTION_CART_PUSHER_RETURNING_WITH),
                    c->x_offset + 64, c->y_offset + y_offset + 63,
    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN, 0);
            } else {
                lang_text_draw_multiline(99, 17, c->x_offset + 64, c->y_offset + y_offset + 63,
                    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
            }
        } else {
            text_draw_multiline(translation_for(TR_WINDOW_BUILDING_DISTRIBUTION_GRANARY_CART_PUSHER_GETTING),
                c->x_offset + 64, c->y_offset + y_offset + 63,
                BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN, 0);
        }
    } else if (b->num_workers) {
        // cartpusher is waiting for orders
        lang_text_draw_multiline(99, 15, c->x_offset + 32, c->y_offset + y_offset + 63,
            BLOCK_SIZE * (c->width_blocks - 3), FONT_NORMAL_BROWN);
    }

    // Permissions image
    image_draw(image_group(GROUP_FIGURE_MARKET_LADY) + 4, c->x_offset + 28, c->y_offset + 19 * c->height_blocks - 140,
        COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_FIGURE_TOWER_SENTRY) + 4,
        c->x_offset + 128, c->y_offset + 19 * c->height_blocks - 140, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_FIGURE_TRADE_CARAVAN) + 4,
        c->x_offset + 218, c->y_offset + 19 * c->height_blocks - 140, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_FIGURE_SHIP) + 4, c->x_offset + 308, c->y_offset + 19 * c->height_blocks - 160,
        COLOR_MASK_NONE, SCALE_NONE);
}

void window_building_draw_granary_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(98, 5, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    draw_granary_permissions_buttons(c->x_offset + 58, c->y_offset + 19 * c->height_blocks - 122, 4);
}

int window_building_handle_mouse_granary(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;
    if (generic_buttons_handle_mouse(m, c->x_offset + 58, c->y_offset + 19 * c->height_blocks - 122,
        granary_distribution_permissions_buttons, 4, &data.permission_focus_button_id)) {
    }
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
    if (!data.showing_special_orders || data.building_id != c->building_id) {
        scrollbar.x = c->x_offset + (c->width_blocks - 3) * BLOCK_SIZE;
        scrollbar.y = y_offset + 42;
        scrollbar.height = 21 * BLOCK_SIZE;
        scrollbar.scrollable_width = (c->width_blocks - 2) * BLOCK_SIZE;
        scrollbar.elements_in_view = 21 * BLOCK_SIZE / 22;
        scrollbar_init(&scrollbar, 0, city_resource_get_available_foods()->size);
        data.showing_special_orders = 1;
    }

    int scrollbar_shown = scrollbar.max_scroll_position > 0;
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - (scrollbar_shown ? 4 : 2), 21);
}

static void draw_button_from_state(int state, int x, int y, building_type type, resource_type resource)
{
    switch (state) {
        case BUILDING_STORAGE_STATE_GETTING:
        case BUILDING_STORAGE_STATE_GETTING_3QUARTERS:
        case BUILDING_STORAGE_STATE_GETTING_HALF:
        case BUILDING_STORAGE_STATE_GETTING_QUARTER:
            {
                int image_width = image_get(image_group(GROUP_CONTEXT_ICONS) + 12)->width + 15;
                int group_number;
                if (resource_is_food(resource)) { 
                    // Check whether to use "getting goods" or "getting food"
                    group_number = 10;
                } else {
                    group_number = 9;
                }
                int text_width = lang_text_get_width(99, group_number, FONT_NORMAL_WHITE);
                int start_x = x + (210 - image_width - text_width) / 2;
                image_draw(image_group(GROUP_CONTEXT_ICONS) + 12, start_x, y - 2, COLOR_MASK_NONE, SCALE_NONE);
                lang_text_draw(99, group_number, start_x + image_width, y, FONT_NORMAL_WHITE);
                break;
            }
        case BUILDING_STORAGE_STATE_NOT_ACCEPTING:
        case BUILDING_STORAGE_STATE_NOT_ACCEPTING_3QUARTERS:
        case BUILDING_STORAGE_STATE_NOT_ACCEPTING_HALF:
        case BUILDING_STORAGE_STATE_NOT_ACCEPTING_QUARTER:
            lang_text_draw_centered(99, 8, x, y, 210, FONT_NORMAL_RED);
            break;
        default:
            lang_text_draw_centered(99, 7, x, y, 210, FONT_NORMAL_WHITE);
            break;
    }
    uint8_t *button_text = warehouse_full_button_text;
    switch (state) {
        case BUILDING_STORAGE_STATE_ACCEPTING:
        case BUILDING_STORAGE_STATE_GETTING:
            button_text = type == BUILDING_GRANARY ? granary_full_button_text : warehouse_full_button_text;
            break;
        case BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS:
        case BUILDING_STORAGE_STATE_GETTING_3QUARTERS:
            button_text = type == BUILDING_GRANARY ? granary_3quarters_button_text : warehouse_3quarters_button_text;
            break;
        case BUILDING_STORAGE_STATE_ACCEPTING_HALF:
        case BUILDING_STORAGE_STATE_GETTING_HALF:
            button_text = type == BUILDING_GRANARY ? granary_half_button_text : warehouse_half_button_text;
            break;
        case BUILDING_STORAGE_STATE_ACCEPTING_QUARTER:
        case BUILDING_STORAGE_STATE_GETTING_QUARTER:
            button_text = type == BUILDING_GRANARY ? granary_quarter_button_text : warehouse_quarter_button_text;
            break;
        default:
            break;
    }
    text_draw_centered(button_text, x + 214, y, 20, FONT_NORMAL_BLACK, 0);
}

static void draw_resource_orders_buttons(int x, int y, const resource_list *list, building_type type,
    const building_storage *storage)
{
    int scrollbar_shown = scrollbar.max_scroll_position > 0;

    for (int i = 0; i < scrollbar.elements_in_view && i < list->size - scrollbar.scroll_position; i++) {
        resource_type resource = list->items[i + scrollbar.scroll_position];
        int image_id = resource_get_data(resource)->image.icon;
        int y_offset = y + 22 * i;
        image_draw(image_id, x, y_offset, COLOR_MASK_NONE, SCALE_NONE);
        if (!scrollbar_shown) {
            image_draw(image_id, x + 396, y_offset, COLOR_MASK_NONE, SCALE_NONE);
        }
        text_draw(resource_get_data(resource)->text, x + 40, y_offset + 4, FONT_NORMAL_WHITE, COLOR_MASK_NONE);
        button_border_draw(x + 148, y_offset, 210, 22, data.resource_focus_button_id == i + 1);
        button_border_draw(x + 358, y_offset, 28, 22, data.partial_resource_focus_button_id == i + 1);

        draw_button_from_state(storage->resource_state[resource], x + 148, y_offset + 5, type, resource);
    }
}

void window_building_draw_granary_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);
    // empty button
    button_border_draw(c->x_offset + 80, y_offset + 404, BLOCK_SIZE * (c->width_blocks - 10), 20,
        data.orders_focus_button_id == 1 ? 1 : 0);
    if (storage->empty_all) {
        lang_text_draw_centered(98, 8, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(98, 9, c->x_offset, y_offset + 384,
            BLOCK_SIZE * c->width_blocks, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(98, 7, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    scrollbar_draw(&scrollbar);

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 2);

    draw_resource_orders_buttons(c->x_offset + 24, y_offset + 46, city_resource_get_available_foods(), BUILDING_GRANARY,
        storage);
}

int window_building_handle_mouse_granary_orders(const mouse *m, building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;

    int buttons_to_show = city_resource_get_available_foods()->size < scrollbar.elements_in_view ?
        city_resource_get_available()->size : scrollbar.elements_in_view;
    
    return scrollbar_handle_mouse(&scrollbar, m, 1) ||
        generic_buttons_handle_mouse(m, c->x_offset + 172, y_offset + 46, orders_resource_buttons, buttons_to_show,
            &data.resource_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 172, y_offset + 46, orders_partial_resource_buttons,
            buttons_to_show, &data.partial_resource_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404,
            granary_order_buttons, 2, &data.orders_focus_button_id);
}

void window_building_get_tooltip_granary_orders(int *group_id, int *text_id, int *translation)
{
    if (data.orders_focus_button_id == 2) {
        *group_id = 143;
        *text_id = 1;
    }
}

static void generate_warehouse_resource_list(building *warehouse)
{
    for (int i = 0; i < data.available_resources.size; i++) {
        data.available_resources.items[i] = RESOURCE_NONE;
    }
    data.available_resources.size = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (building_warehouse_get_amount(warehouse, r) > 0) {
            data.available_resources.items[data.available_resources.size++] = r;
        }
    }
}

void window_building_draw_warehouse(building_info_context *c)
{
    c->help_id = 4;
    data.building_id = c->building_id;
    data.showing_special_orders = 0;

    building *b = building_get(c->building_id);

    int y_offset_blocks = 0;
    if (!b->has_plague && c->has_road_access) {
        generate_warehouse_resource_list(b);
        y_offset_blocks = ((data.available_resources.size - 1) / 2 - 3) * 2 + 2;
    }
    c->height_blocks = 22 + y_offset_blocks;
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);

    lang_text_draw_centered(99, 0, c->x_offset, c->y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    if (b->has_plague) {
        window_building_play_sound(c, "wavs/clinic.wav");
    } else {
        window_building_play_sound(c, "wavs/warehouse.wav");
    }

    if (b->has_plague) {
        if (b->sickness_doctor_cure == 99) {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_FUMIGATION_DESC);
        } else {
            window_building_draw_description(c, CUSTOM_TRANSLATION, TR_BUILDING_WAREHOUSE_PLAGUE_DESC);
        }
    } else if (!c->has_road_access) {
        window_building_draw_description(c, 69, 25);
    } else {
        if (data.available_resources.size == 0) {
            lang_text_draw_centered(CUSTOM_TRANSLATION, TR_BUILDING_WAREHOUSE_NO_GOODS, c->x_offset, c->y_offset + 63,
                BLOCK_SIZE * c->width_blocks, FONT_NORMAL_BLACK);
        } else {
            int total_stored = 0;
            int x;
            int y = c->y_offset + 31;
            for (int i = 0; i < data.available_resources.size; i++) {
                if (i & 1) {
                    x = c->x_offset + 240;
                } else {
                    x = c->x_offset + 20;
                    y += BLOCK_SIZE * 2;
                }
                resource_type r = data.available_resources.items[i];
                int amount = building_warehouse_get_amount(b, r);
                total_stored += amount;
                image_draw(resource_get_data(r)->image.icon, x, y, COLOR_MASK_NONE, SCALE_NONE);
                int width = text_draw_number(amount, '@', " ", x + 24, y + 7, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
                text_draw(resource_get_data(r)->text, x + 24 + width, y + 7, FONT_NORMAL_BLACK, COLOR_MASK_NONE);
            }
            int width = lang_text_draw(98, 2, c->x_offset + 34, c->y_offset + 40, FONT_NORMAL_BLACK);
            lang_text_draw_amount(8, 16, total_stored, c->x_offset + 34 + width, c->y_offset + 40, FONT_NORMAL_BLACK);

            width = lang_text_draw(98, 3, c->x_offset + 220, c->y_offset + 40, FONT_NORMAL_BLACK);
            lang_text_draw_amount(8, 16, 32 - total_stored,
                c->x_offset + 220 + width, c->y_offset + 40, FONT_NORMAL_BLACK);
        }
    }
    int y_offset = 160 + y_offset_blocks * BLOCK_SIZE;
    inner_panel_draw(c->x_offset + 16, c->y_offset + y_offset + 8, c->width_blocks - 2, 6);
    window_building_draw_employment(c, y_offset + 13);
    // cartpusher state
    int cartpusher = b->figure_id;
    figure *f = figure_get(cartpusher);

    if (cartpusher && f->state == FIGURE_STATE_ALIVE) {
        int resource = f->resource_id;
        if (resource) {
            image_draw(resource_get_data(resource)->image.icon,
                c->x_offset + 32, c->y_offset + y_offset + 60, COLOR_MASK_NONE, SCALE_NONE);
            if (f->action_state == FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE) {
                lang_text_draw_multiline(99, 16, c->x_offset + 64, c->y_offset + y_offset + 63,
                    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
            } else if (f->loads_sold_or_carrying) {
                text_draw_multiline(translation_for(TR_WINDOW_BUILDING_DISTRIBUTION_CART_PUSHER_RETURNING_WITH),
                    c->x_offset + 64, c->y_offset + y_offset + 63,
                    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN, 0);
            } else {
                lang_text_draw_multiline(99, 17, c->x_offset + 64, c->y_offset + y_offset + 63,
                    BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN);
            }
        } else {
            image_draw(resource_get_data(f->collecting_item_id)->image.icon,
                c->x_offset + 32, c->y_offset + y_offset + 60, COLOR_MASK_NONE, SCALE_NONE);
            text_draw_multiline(translation_for(TR_WINDOW_BUILDING_DISTRIBUTION_CART_PUSHER_GETTING),
                c->x_offset + 64, c->y_offset + y_offset + 63, BLOCK_SIZE * (c->width_blocks - 5), FONT_NORMAL_BROWN, 0);
        }
    } else if (b->num_workers) {
        // cartpusher is waiting for orders
        lang_text_draw_multiline(99, 15, c->x_offset + 32, c->y_offset + y_offset + 63,
            BLOCK_SIZE * (c->width_blocks - 3), FONT_NORMAL_BROWN);
    }

    image_draw(image_group(GROUP_FIGURE_MARKET_LADY) + 4,
        c->x_offset + 32, c->y_offset + BLOCK_SIZE * c->height_blocks - 83, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_FIGURE_TRADE_CARAVAN) + 4,
        c->x_offset + 128, c->y_offset + BLOCK_SIZE * c->height_blocks - 83, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_group(GROUP_FIGURE_SHIP) + 4,
        c->x_offset + 216, c->y_offset + BLOCK_SIZE * c->height_blocks - 100, COLOR_MASK_NONE, SCALE_NONE);
}

void window_building_draw_warehouse_foreground(building_info_context *c)
{
    button_border_draw(c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        BLOCK_SIZE * (c->width_blocks - 10), 20, data.focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(99, 2, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 30,
        BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);

    draw_permissions_buttons(c->x_offset + 64, c->y_offset + BLOCK_SIZE * c->height_blocks - 75, 3, c);
}

int window_building_handle_mouse_warehouse(const mouse *m, building_info_context *c)
{
    data.building_id = c->building_id;

    if (generic_buttons_handle_mouse(m, c->x_offset + 80, c->y_offset + BLOCK_SIZE * c->height_blocks - 34,
        go_to_orders_button, 1, &data.focus_button_id)) {
    }
    if (generic_buttons_handle_mouse(m, c->x_offset + 64, c->y_offset + BLOCK_SIZE * c->height_blocks - 75,
        warehouse_distribution_permissions_buttons, 3, &data.permission_focus_button_id)) {
    }

    int button = 1;
    building *b = building_get(c->building_id);

    if (building_storage_get_permission(BUILDING_STORAGE_PERMISSION_WORKER, b)) {
        button = 2;
    }
    if (image_buttons_handle_mouse(m, c->x_offset + 64, c->y_offset + BLOCK_SIZE * c->height_blocks - 75,
        image_buttons_maintain, button, &data.image_button_focus_id)) {
        return 1;
    }

    return 0;
}

void window_building_draw_warehouse_orders(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);
    c->help_id = 4;
    outer_panel_draw(c->x_offset, y_offset, 29, 28);
    lang_text_draw_centered(99, 3, c->x_offset, y_offset + 10, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK);

    if (!data.showing_special_orders || data.building_id != c->building_id) {
        scrollbar.x = c->x_offset + (c->width_blocks - 3) * BLOCK_SIZE;
        scrollbar.y = y_offset + 42;
        scrollbar.height = 21 * BLOCK_SIZE;
        scrollbar.scrollable_width = (c->width_blocks - 2) * BLOCK_SIZE;
        scrollbar.elements_in_view = 21 * BLOCK_SIZE / 22;
        scrollbar_init(&scrollbar, 0, city_resource_get_available()->size);

        data.showing_special_orders = 1;
    }

    int scrollbar_shown = scrollbar.max_scroll_position > 0;
    inner_panel_draw(c->x_offset + 16, y_offset + 42, c->width_blocks - (scrollbar_shown ? 4 : 2), 21);
}

void window_building_draw_warehouse_orders_foreground(building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    const building_storage *storage = building_storage_get(building_get(c->building_id)->storage_id);

    // emptying button
    button_border_draw(c->x_offset + 80, y_offset + 404, BLOCK_SIZE * (c->width_blocks - 10),
        20, data.orders_focus_button_id == 1 ? 1 : 0);
    if (storage->empty_all) {
        lang_text_draw_centered(99, 5, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
        lang_text_draw_centered(99, 6, c->x_offset, y_offset + 426, BLOCK_SIZE * c->width_blocks, FONT_SMALL_PLAIN);
    } else {
        lang_text_draw_centered(99, 4, c->x_offset + 80, y_offset + 408,
            BLOCK_SIZE * (c->width_blocks - 10), FONT_NORMAL_BLACK);
    }

    // accept none button
    draw_accept_none_button(c->x_offset + 394, y_offset + 404, data.orders_focus_button_id == 2);

    scrollbar_draw(&scrollbar);

    draw_resource_orders_buttons(c->x_offset + 24, y_offset + 46, city_resource_get_available(), BUILDING_WAREHOUSE,
        storage);
}

int window_building_handle_mouse_warehouse_orders(const mouse *m, building_info_context *c)
{
    int y_offset = window_building_get_vertical_offset(c, 28);

    data.building_id = c->building_id;

    int buttons_to_show = city_resource_get_available()->size < scrollbar.elements_in_view ?
        city_resource_get_available()->size : scrollbar.elements_in_view;
    
    return scrollbar_handle_mouse(&scrollbar, m, 1) ||
        generic_buttons_handle_mouse(m, c->x_offset + 172, y_offset + 46, orders_resource_buttons, buttons_to_show,
            &data.resource_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 172, y_offset + 46, orders_partial_resource_buttons,
            buttons_to_show, &data.partial_resource_focus_button_id) ||
        generic_buttons_handle_mouse(m, c->x_offset + 80, y_offset + 404,
            warehouse_order_buttons, 2, &data.orders_focus_button_id);
}

void window_building_warehouse_get_tooltip_distribution_permissions(int *translation)
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
    if (data.image_button_focus_id) {
        building *b = building_get(data.building_id);
        if (building_storage_get_permission(BUILDING_STORAGE_PERMISSION_WORKER, b)) {
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_WORKERS; 
        } else {
            *translation = TR_TOOLTIP_BUTTON_REJECT_WORKERS;
        }
    }
}

void window_building_granary_get_tooltip_distribution_permissions(int *translation)
{
    switch (data.permission_focus_button_id) {
        case 1:
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_MARKET_LADIES;
            break;
        case 2:
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_QUARTERMASTER;
            break;
        case 3:
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_TRADE_CARAVAN;
            break;
        case 4:
            *translation = TR_TOOLTIP_BUTTON_ACCEPT_TRADE_SHIPS;
            break;
        default:
            break;
    }
}

const uint8_t *window_building_dock_get_tooltip(building_info_context *c)
{
    int x_offset = c->x_offset + 16;
    int y_offset = c->y_offset + 270;
    const building *dock = building_get(c->building_id);
    if (dock->type != BUILDING_DOCK) {
        return 0;
    }
    int width = dock_distribution_permissions_buttons_count > data.dock_max_cities_visible ? 140 : 170;
    int height = 20;
    const mouse *m = mouse_get();

    for (int i = 0; i < dock_distribution_permissions_buttons_count; i++) {
        if (i < scrollbar.scroll_position || i - scrollbar.scroll_position >= data.dock_max_cities_visible) {
            continue;
        }
        int y_pos = y_offset + 22 * (i - scrollbar.scroll_position);
        if (m->x < x_offset || m->y < y_pos || m->x > x_offset + width || m->y > y_pos + height) {
            continue;
        }
        empire_city *city = empire_city_get(dock_distribution_permissions_buttons[i].parameter2);
        if (!city) {
            return 0;
        }
        static uint8_t text[400];
        uint8_t *cursor = text;
        cursor = string_copy(lang_get_string(47, 5), cursor, 400 - (int) (cursor - text));
        cursor = string_copy(string_from_ascii(": "), cursor, 400 - (int) (cursor - text));
        int traded = 0;
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->sells_resource[resource]) {
                continue;
            }
            if (traded > 0) {
                cursor = string_copy(string_from_ascii(", "), cursor, 400 - (int) (cursor - text));                
            }
            traded++;
            cursor = string_copy(resource_get_data(resource)->text, cursor, 400 - (int) (cursor - text));
        }
        if (traded == 0) {
            cursor = string_copy(lang_get_string(23, 0), cursor, 400 - (int) (cursor - text));
        }
        cursor = string_copy(string_from_ascii("\n"), cursor, 400 - (int) (cursor - text));
        cursor = string_copy(lang_get_string(47, 4), cursor, 400 - (int) (cursor - text));
        cursor = string_copy(string_from_ascii(": "), cursor, 400 - (int) (cursor - text));
        traded = 0;
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->buys_resource[resource]) {
                continue;
            }
            if (traded > 0) {
                cursor = string_copy(string_from_ascii(", "), cursor, 400 - (int) (cursor - text));
            }
            traded++;
            cursor = string_copy(resource_get_data(resource)->text, cursor, 400 - (int) (cursor - text));
        }
        if (traded == 0) {
            cursor = string_copy(lang_get_string(23, 0), cursor, 400 - (int) (cursor - text));
        }
        return text;
    }
    return 0;
}

void window_building_primary_product_producer_stockpiling_tooltip(int *translation)
{
    if (data.primary_product_stockpiling_id) {
        if (building_stockpiling_enabled(building_get(data.building_id))) {
            *translation = TR_TOOLTIP_BUTTON_STOCKPILING_OFF;
        } else {
            *translation = TR_TOOLTIP_BUTTON_STOCKPILING_ON;
        }
    }
}

void window_building_get_tooltip_warehouse_orders(int *group_id, int *text_id, int *translation)
{
    if (data.orders_focus_button_id == 2) {
        *group_id = 15;
        *text_id = 1;
    }
}

static void on_scroll(void)
{
    window_invalidate();
}

static void go_to_orders(int param1, int param2)
{
    window_building_info_show_storage_orders();
}

static void toggle_resource_state(int index, int param2)
{
    building *b = building_get(data.building_id);
    index += scrollbar.scroll_position - 1;
    resource_type resource;
    if (building_has_supplier_inventory(b->type) || b->type == BUILDING_DOCK) {
        resource = data.available_resources.items[index];
        building_distribution_toggle_good_accepted(resource, b);
    } else {
        if (b->type == BUILDING_WAREHOUSE) {
            resource = city_resource_get_available()->items[index];
        } else {
            resource = city_resource_get_available_foods()->items[index];
        }
        building_storage_cycle_resource_state(b->storage_id, resource);
    }
    window_invalidate();
}

static void market_orders(int index, int param2)
{
    building *b = building_get(data.building_id);
    if (index == 0) {
        building_distribution_unaccept_all_goods(b);
    }
    window_invalidate();
}

static void storage_toggle_permissions(int index, int param2)
{
    building *b = building_get(data.building_id);
    building_storage_set_permission(index - 1, b);
    window_invalidate();
}

static void toggle_partial_resource_state(int index, int param2)
{
    building *b = building_get(data.building_id);
    int resource;
    if (b->type == BUILDING_WAREHOUSE) {
        resource = city_resource_get_available()->items[index + scrollbar.scroll_position - 1];
    } else {
        resource = city_resource_get_available_foods()->items[index + scrollbar.scroll_position - 1];
    }
    building_storage_cycle_partial_resource_state(b->storage_id, resource);
    window_invalidate();
}

static void button_stockpiling(int param1, int param2)
{
    building *b = building_get(data.building_id);
    if (building_is_primary_product_producer(b->type)) {
        building_stockpiling_toggle(b);
    }
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
        int storage_id = building_get(data.building_id)->storage_id;
        building_storage_accept_none(storage_id);
    }
    window_invalidate();
}

void window_building_draw_mess_hall(building_info_context *c)
{
    building *b = building_get(c->building_id);
    int mess_hall_fulfillment_display = 100 - city_mess_hall_food_missing_month();
    int food_stress = city_mess_hall_food_stress();
    int hunger_text;

    window_building_play_sound(c, "wavs/warehouse2.wav");
    outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);

    text_draw_centered(translation_for(TR_BUILDING_MESS_HALL),
        c->x_offset, c->y_offset + 12, BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK, 0);
    window_building_draw_stocks(c, b, 0, 1, 0);
    if (city_military_total_soldiers_in_city() > 0) {
        int width = text_draw(translation_for(TR_BUILDING_MESS_HALL_FULFILLMENT),
            c->x_offset + 32, c->y_offset + 106, FONT_NORMAL_BLACK, 0);
        text_draw_percentage(mess_hall_fulfillment_display,
            c->x_offset + 32 + width, c->y_offset + 106, FONT_NORMAL_BLACK);
        width = text_draw(translation_for(TR_BUILDING_MESS_HALL_TROOP_HUNGER),
            c->x_offset + 32, c->y_offset + 126, FONT_NORMAL_BLACK, 0);
        if (food_stress < 3) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_1;
        } else if (food_stress > 80) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_5;
        } else if (food_stress > 60) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_4;
        } else if (food_stress > 40) {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_3;
        } else {
            hunger_text = TR_BUILDING_MESS_HALL_TROOP_HUNGER_2;
        }

        text_draw(translation_for(hunger_text), c->x_offset + 32 + width, c->y_offset + 126, FONT_NORMAL_BLACK, 0);

        width = text_draw(translation_for(TR_BUILDING_MESS_HALL_MONTHS_FOOD_STORED), c->x_offset + 32,
            c->y_offset + 150, FONT_NORMAL_BLACK, 0);
        text_draw_number(city_mess_hall_months_food_stored(), '@', " ", c->x_offset + 32 + width, c->y_offset + 150,
            FONT_NORMAL_BLACK, 0);

        if (city_mess_hall_food_types() == 2) {
            text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_FOOD_TYPES_BONUS_1), c->x_offset + 32,
                c->y_offset + 175, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        } else if (city_mess_hall_food_types() >= 3) {
            text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_FOOD_TYPES_BONUS_2), c->x_offset + 32,
                c->y_offset + 175, BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
    } else {
        text_draw_centered(translation_for(TR_BUILDING_MESS_HALL_NO_SOLDIERS), c->x_offset, c->y_offset + 150,
            BLOCK_SIZE * (c->width_blocks), FONT_NORMAL_BLACK, 0);
    }
    text_draw_multiline(translation_for(TR_BUILDING_MESS_HALL_DESC), c->x_offset + 32, c->y_offset + 226,
        BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);

    inner_panel_draw(c->x_offset + 16, c->y_offset + 308, c->width_blocks - 2, 4);
    window_building_draw_employment(c, 308);
    window_building_distributor_draw_foreground(c);
}

static void window_building_draw_monument_caravanserai_construction_process(building_info_context *c)
{
    window_building_draw_monument_construction_process(c, TR_BUILDING_CARAVANSERAI_PHASE_1,
        TR_BUILDING_CARAVANSERAI_PHASE_1_TEXT, TR_BUILDING_MONUMENT_CONSTRUCTION_DESC);
}

int window_building_handle_mouse_caravanserai(const mouse *m, building_info_context *c)
{
    return generic_buttons_handle_mouse(
        m, c->x_offset + 32, c->y_offset + 150,
        go_to_caravanserai_action_button, 1, &data.caravanserai_focus_button_id);
}

void window_building_draw_caravanserai_foreground(building_info_context *c)
{
    int id = assets_get_image_id("UI", "Image Border Medium");
    image_draw_border(id, c->x_offset + 32, c->y_offset + 150,
        data.caravanserai_focus_button_id == 1 ? COLOR_BORDER_RED : COLOR_BORDER_GREEN);
}

static void apply_policy(int selected_policy)
{
    if (selected_policy == NO_POLICY) {
        return;
    }
    city_trade_policy_set(LAND_TRADE_POLICY, selected_policy);
    sound_speech_play_file(land_trade_policy.wav_file);
    city_finance_process_sundry(TRADE_POLICY_COST);
}

static void button_caravanserai_policy(int selected_policy, int param2)
{
    if (building_monument_working(BUILDING_CARAVANSERAI)) {
        window_option_popup_show(land_trade_policy.title, land_trade_policy.subtitle,
            &land_trade_policy.items[1], 3, apply_policy, city_trade_policy_get(LAND_TRADE_POLICY),
            TRADE_POLICY_COST, OPTION_MENU_SMALL_ROW);
    }
}

void window_building_draw_caravanserai(building_info_context *c)
{
    building *b = building_get(c->building_id);

    if (b->data.monument.phase == MONUMENT_FINISHED) {
        window_building_play_sound(c, "wavs/market2.wav");
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);

        window_building_draw_stocks(c, b, 0, 1, 1);

        if (building_monument_has_labour_problems(b)) {
            text_draw_multiline(translation_for(TR_BUILDING_CARAVANSERAI_NEEDS_WORKERS),
                c->x_offset + 22, c->y_offset + 76, 15 * c->width_blocks, FONT_NORMAL_BLACK, 0);
        } else {
            text_draw_multiline(translation_for(TR_BUILDING_CARAVANSERAI_DESC), c->x_offset + 32, c->y_offset + 76,
                BLOCK_SIZE * (c->width_blocks - 4), FONT_NORMAL_BLACK, 0);
        }
        if (!land_trade_policy.items[0].image_id) {
            int base_policy_image = assets_get_image_id("UI",
                land_trade_policy.base_image_name);
            land_trade_policy.items[0].image_id = base_policy_image;
            land_trade_policy.items[1].image_id = base_policy_image + 1;
            land_trade_policy.items[2].image_id = base_policy_image + 2;
            land_trade_policy.items[3].image_id = base_policy_image + 3;
        }

        trade_policy policy = city_trade_policy_get(LAND_TRADE_POLICY);

        text_draw_multiline(translation_for(land_trade_policy.items[policy].header),
            c->x_offset + 160, c->y_offset + 156, 260, FONT_NORMAL_BLACK, 0);
        if (policy != NO_POLICY) {
            text_draw_multiline(translation_for(land_trade_policy.items[policy].desc),
                c->x_offset + 160, c->y_offset + 181, 260, FONT_NORMAL_BLACK, 0);
        }
        image_draw(land_trade_policy.items[policy].image_id, c->x_offset + 32, c->y_offset + 150,
            COLOR_MASK_NONE, SCALE_NONE);

        inner_panel_draw(c->x_offset + 16, c->y_offset + 270, c->width_blocks - 2, 4);
        window_building_draw_employment(c, 278);

        if (c->height_blocks >= 38) {
            image_draw_border(assets_get_image_id("UI", "Large_Banner_Border"),
                c->x_offset + 32, c->y_offset + 350, COLOR_MASK_NONE);
            image_draw(assets_get_image_id("UI", "Caravanserai Banner"),
                c->x_offset + 37, c->y_offset + 355, COLOR_MASK_NONE, SCALE_NONE);
        }
    } else {
        outer_panel_draw(c->x_offset, c->y_offset, c->width_blocks, c->height_blocks);
        window_building_draw_monument_caravanserai_construction_process(c);
    }

    text_draw_centered(translation_for(TR_BUILDING_CARAVANSERAI), c->x_offset, c->y_offset + 12,
        BLOCK_SIZE * c->width_blocks, FONT_LARGE_BLACK, 0);
}
