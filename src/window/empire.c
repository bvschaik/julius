#include "empire.h"

#include "assets/assets.h"
#include "building/menu.h"
#include "city/military.h"
#include "city/warning.h"
#include "core/image_group.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "game/tutorial.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/scroll.h"
#include "scenario/empire.h"
#include "scenario/invasion.h"
#include "window/advisors.h"
#include "window/city.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/resource_settings.h"
#include "window/trade_opened.h"
#include "window/trade_prices.h"

#include <math.h>

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

typedef struct {
    int x;
    int y;
} px_point;
static px_point trade_amount_px_offsets[5] = {
    { 2, 0 },
    { 5, 2 },
    { 8, 4 },
    { 0, 3 },
    { 4, 6 },
};

static void button_help(int param1, int param2);
static void button_return_to_city(int param1, int param2);
static void button_advisor(int advisor, int param2);
static void button_show_prices(int param1, int param2);
static void button_open_trade(int param1, int param2);
static void button_show_resource_window(int resource, int param2);

static image_button image_button_help[] = {
    {0, 0, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1}
};
static image_button image_button_return_to_city[] = {
    {0, 0, 24, 24, IB_NORMAL, GROUP_CONTEXT_ICONS, 4, button_return_to_city, button_none, 0, 0, 1}
};
static image_button image_button_advisor[] = {
    {-4, 0, 24, 24, IB_NORMAL, GROUP_MESSAGE_ADVISOR_BUTTONS, 12, button_advisor, button_none, ADVISOR_TRADE, 0, 1}
};
static image_button image_button_show_prices[] = {
    {-4, 0, 24, 24, IB_NORMAL, GROUP_MESSAGE_ADVISOR_BUTTONS, 30, button_show_prices, button_none, 0, 0, 1}
};

static generic_button generic_button_trade_resource[] = {
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_WHEAT, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_VEGETABLES , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_FRUIT , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_OLIVES , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_VINES , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_MEAT, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_WINE , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_OIL , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_IRON , 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_TIMBER, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_CLAY, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_MARBLE, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_WEAPONS, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_FURNITURE, 0},
    {0, 0, 101, 27, button_show_resource_window, button_none, RESOURCE_POTTERY, 0}
};
static generic_button generic_button_open_trade[] = {
    {30, 56, 440, 26, button_open_trade, button_none, 0, 0}
};

static struct {
    int selected_button;
    int selected_city;
    int x_min, x_max, y_min, y_max;
    int x_draw_offset, y_draw_offset;
    int focus_button_id;
    int is_scrolling;
    int finished_scroll;
    int focus_resource;
} data = { 0, 1 };

static void init(void)
{
    data.selected_button = 0;
    int selected_object = empire_selected_object();
    if (selected_object) {
        data.selected_city = empire_city_get_for_object(selected_object - 1);
    } else {
        data.selected_city = 0;
    }
    data.focus_button_id = 0;
}

static void draw_paneling(void)
{
    int image_base = image_group(GROUP_EMPIRE_PANELS);
    // bottom panel background
    graphics_set_clip_rectangle(data.x_min, data.y_min, data.x_max - data.x_min, data.y_max - data.y_min);
    for (int x = data.x_min; x < data.x_max; x += 70) {
        image_draw(image_base + 3, x, data.y_max - 120, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base + 3, x, data.y_max - 80, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base + 3, x, data.y_max - 40, COLOR_MASK_NONE, SCALE_NONE);
    }

    // horizontal bar borders
    for (int x = data.x_min; x < data.x_max; x += 86) {
        image_draw(image_base + 1, x, data.y_min, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base + 1, x, data.y_max - 120, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base + 1, x, data.y_max - 16, COLOR_MASK_NONE, SCALE_NONE);
    }

    // vertical bar borders
    for (int y = data.y_min + 16; y < data.y_max; y += 86) {
        image_draw(image_base, data.x_min, y, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_base, data.x_max - 16, y, COLOR_MASK_NONE, SCALE_NONE);
    }

    // crossbars
    image_draw(image_base + 2, data.x_min, data.y_min, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, data.x_min, data.y_max - 120, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, data.x_min, data.y_max - 16, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, data.x_max - 16, data.y_min, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, data.x_max - 16, data.y_max - 120, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 2, data.x_max - 16, data.y_max - 16, COLOR_MASK_NONE, SCALE_NONE);

    graphics_reset_clip_rectangle();
}

static void draw_trade_resource(resource_type resource, int trade_max, int x_offset, int y_offset)
{
    graphics_draw_inset_rect(x_offset, y_offset, 26, 26);

    int image_id = resource + image_group(GROUP_EMPIRE_RESOURCES);
    int resource_offset = resource_image_offset(resource, RESOURCE_IMAGE_ICON);
    image_draw(image_id + resource_offset, x_offset + 1, y_offset + 1, COLOR_MASK_NONE, SCALE_NONE);

    if (data.focus_resource == resource) {
        button_border_draw(x_offset - 2, y_offset - 2, 101 + 4, 30, 1);
    }

    window_empire_draw_resource_shields(trade_max, x_offset, y_offset);
}

void window_empire_draw_resource_shields(int trade_max, int x_offset, int y_offset)
{
    int num_bronze_shields = (trade_max % 100) / 20 + 1;
    if (trade_max >= 600) {
        num_bronze_shields = 5;
    }

    int top_left_x;
    if (num_bronze_shields == 1) {
        top_left_x = x_offset + 19;
    } else if (num_bronze_shields == 2) {
        top_left_x = x_offset + 15;
    } else {
        top_left_x = x_offset + 11;
    }
    int top_left_y = y_offset - 1;
    int bronze_shield = image_group(GROUP_TRADE_AMOUNT);
    for (int i = 0; i < num_bronze_shields; i++) {
        px_point pt = trade_amount_px_offsets[i];
        image_draw(bronze_shield, top_left_x + pt.x, top_left_y + pt.y, COLOR_MASK_NONE, SCALE_NONE);
    }

    int num_gold_shields = trade_max / 100;
    if (num_gold_shields > 5) {
        num_gold_shields = 5;
    }
    top_left_x = x_offset - 1;
    top_left_y = y_offset + 22;
    int gold_shield = assets_lookup_image_id(ASSET_GOLD_SHIELD);
    for (int i = 0; i < num_gold_shields; i++) {
        image_draw(gold_shield, top_left_x + i * 3, top_left_y, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_trade_city_info(const empire_object *object, const empire_city *city)
{
    int x_offset = (data.x_min + data.x_max - 500) / 2;
    int y_offset = data.y_max - 113;
    if (city->is_open) {
        // city sells
        lang_text_draw(47, 10, x_offset + 44, y_offset + 40, FONT_NORMAL_GREEN);
        int index = 0;
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->sells_resource[resource]) {
                continue;
            }
            int trade_max = trade_route_limit(city->route_id, resource);
            draw_trade_resource(resource, trade_max, x_offset + 124 * index + 120, y_offset + 31);
            int trade_now = trade_route_traded(city->route_id, resource);
            if (trade_now > trade_max) {
                trade_max = trade_now;
            }
            int text_width = text_draw_number(trade_now, '@', "",
                x_offset + 124 * index + 150, y_offset + 40, FONT_NORMAL_GREEN, 0);
            text_width += lang_text_draw(47, 11,
                x_offset + 124 * index + 148 + text_width, y_offset + 40, FONT_NORMAL_GREEN);
            text_draw_number(trade_max, '@', "",
                x_offset + 124 * index + 138 + text_width, y_offset + 40, FONT_NORMAL_GREEN, 0);
            index++;
        }
        // city buys
        lang_text_draw(47, 9, x_offset + 44, y_offset + 71, FONT_NORMAL_GREEN);
        index = 0;
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->buys_resource[resource]) {
                continue;
            }
            int trade_max = trade_route_limit(city->route_id, resource);
            draw_trade_resource(resource, trade_max, x_offset + 124 * index + 120, y_offset + 62);
            int trade_now = trade_route_traded(city->route_id, resource);
            if (trade_now > trade_max) {
                trade_max = trade_now;
            }
            int text_width = text_draw_number(trade_now, '@', "",
                x_offset + 124 * index + 150, y_offset + 71, FONT_NORMAL_GREEN, 0);
            text_width += lang_text_draw(47, 11,
                x_offset + 124 * index + 148 + text_width, y_offset + 71, FONT_NORMAL_GREEN);
            text_draw_number(trade_max, '@', "",
                x_offset + 124 * index + 138 + text_width, y_offset + 71, FONT_NORMAL_GREEN, 0);
            index++;
        }
    } else { // trade is closed
        int index = lang_text_draw(47, 5, x_offset + 50, y_offset + 42, FONT_NORMAL_GREEN);
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->sells_resource[resource]) {
                continue;
            }
            int trade_max = trade_route_limit(city->route_id, resource);
            draw_trade_resource(resource, trade_max, x_offset + index + 60, y_offset + 33);
            index += 32;
        }
        index += lang_text_draw(47, 4, x_offset + index + 100, y_offset + 42, FONT_NORMAL_GREEN);
        for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
            if (!city->buys_resource[resource]) {
                continue;
            }
            int trade_max = trade_route_limit(city->route_id, resource);
            draw_trade_resource(resource, trade_max, x_offset + index + 110, y_offset + 33);
            index += 32;
        }
        index = lang_text_draw_amount(8, 0, city->cost_to_open,
            x_offset + 40, y_offset + 73, FONT_NORMAL_GREEN);
        lang_text_draw(47, 6, x_offset + index + 40, y_offset + 73, FONT_NORMAL_GREEN);
        int image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE) + 1 - city->is_sea_trade;
        image_draw(image_id, x_offset + 430, y_offset + 65 + 2 * city->is_sea_trade, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_city_info(const empire_object *object)
{
    int x_offset = (data.x_min + data.x_max - 240) / 2;
    int y_offset = data.y_max - 88;

    const empire_city *city = empire_city_get(data.selected_city);
    switch (city->type) {
        case EMPIRE_CITY_DISTANT_ROMAN:
            lang_text_draw_centered(47, 12, x_offset, y_offset + 42, 240, FONT_NORMAL_GREEN);
            break;
        case EMPIRE_CITY_VULNERABLE_ROMAN:
            if (city_military_distant_battle_city_is_roman()) {
                lang_text_draw_centered(47, 12, x_offset, y_offset + 42, 240, FONT_NORMAL_GREEN);
            } else {
                lang_text_draw_centered(47, 13, x_offset, y_offset + 42, 240, FONT_NORMAL_GREEN);
            }
            break;
        case EMPIRE_CITY_FUTURE_TRADE:
        case EMPIRE_CITY_DISTANT_FOREIGN:
        case EMPIRE_CITY_FUTURE_ROMAN:
            lang_text_draw_centered(47, 0, x_offset, y_offset + 42, 240, FONT_NORMAL_GREEN);
            break;
        case EMPIRE_CITY_OURS:
            lang_text_draw_centered(47, 1, x_offset, y_offset + 42, 240, FONT_NORMAL_GREEN);
            break;
        case EMPIRE_CITY_TRADE:
            draw_trade_city_info(object, city);
            break;
    }
}

static void draw_roman_army_info(const empire_object *object)
{
    int x_offset = (data.x_min + data.x_max - 240) / 2;
    int y_offset = data.y_max - 68;
    int text_id;
    if (city_military_distant_battle_roman_army_is_traveling_forth()) {
        text_id = 15;
    } else {
        text_id = 16;
    }
    lang_text_draw_multiline(47, text_id, x_offset, y_offset, 240, FONT_NORMAL_GREEN);
}

static void draw_enemy_army_info(const empire_object *object)
{
    lang_text_draw_multiline(47, 14,
        (data.x_min + data.x_max - 240) / 2,
        data.y_max - 68,
        240, FONT_NORMAL_GREEN);
}

static void draw_object_info(void)
{
    int selected_object = empire_selected_object();
    if (selected_object) {
        const empire_object *object = empire_object_get(selected_object - 1);
        switch (object->type) {
            case EMPIRE_OBJECT_CITY:
                draw_city_info(object);
                break;
            case EMPIRE_OBJECT_ROMAN_ARMY:
                if (city_military_distant_battle_roman_army_is_traveling()) {
                    if (city_military_distant_battle_roman_months_traveled() == object->distant_battle_travel_months) {
                        draw_roman_army_info(object);
                        break;
                    }
                }
            case EMPIRE_OBJECT_ENEMY_ARMY:
                if (city_military_months_until_distant_battle() > 0) {
                    if (city_military_distant_battle_enemy_months_traveled() == object->distant_battle_travel_months) {
                        draw_enemy_army_info(object);
                        break;
                    }
                }
            default:
                lang_text_draw_centered(47, 8, data.x_min, data.y_max - 48, data.x_max - data.x_min, FONT_NORMAL_GREEN);
        }
    } else {
        lang_text_draw_centered(47, 8, data.x_min, data.y_max - 48, data.x_max - data.x_min, FONT_NORMAL_GREEN);
    }
}

static void draw_background(void)
{
    int s_width = screen_width();
    int s_height = screen_height();
    data.x_min = s_width <= MAX_WIDTH ? 0 : (s_width - MAX_WIDTH) / 2;
    data.x_max = s_width <= MAX_WIDTH ? s_width : data.x_min + MAX_WIDTH;
    data.y_min = s_height <= MAX_HEIGHT ? 0 : (s_height - MAX_HEIGHT) / 2;
    data.y_max = s_height <= MAX_HEIGHT ? s_height : data.y_min + MAX_HEIGHT;

    if (data.x_min || data.y_min) {
        graphics_clear_screen();
    }
}

void draw_trade_dots(const empire_object *trade_route, int x_draw_offset, int y_draw_offset, int start_x, int start_y, int end_x, int end_y)
{
    int x_diff = end_x - start_x;
    int y_diff = end_y - start_y;
    double dist = sqrt(x_diff * x_diff + y_diff * y_diff);
    double x_factor = x_diff / dist;
    double y_factor = y_diff / dist;
    static const int dot_spacing = 15;
    int num_dots = ceil(dist / dot_spacing);
    int image_id = trade_route->type == EMPIRE_OBJECT_LAND_TRADE_ROUTE ? assets_get_image_id("UI", "LandRouteDot") : assets_get_image_id("UI", "SeaRouteDot");
    for (int j = 0; j < num_dots; j++) {
        int x = x_factor * j * 15 + start_x;
        int y = y_factor * j * 15 + start_y;
        image_draw(image_id, x_draw_offset + x, y_draw_offset + y, COLOR_MASK_NONE, SCALE_NONE);
    }
}

void window_empire_draw_trade_waypoints(const empire_object *trade_route, int x_draw_offset, int y_draw_offset)
{
    const empire_object *our_city = empire_object_get_our_city();
    const empire_object *trade_city = empire_object_get_trade_city(trade_route->trade_route_id);
    int last_x = our_city->x + 25;
    int last_y = our_city->y + 25;
    for (int i = 0; i < MAX_EMPIRE_OBJECTS; i++) {
        empire_object *obj = empire_object_get(i);
        if (obj->type != EMPIRE_OBJECT_TRADE_WAYPOINT || obj->trade_route_id != trade_route->trade_route_id) {
            continue;
        }
        draw_trade_dots(trade_route, x_draw_offset, y_draw_offset, last_x, last_y, obj->x, obj->y);
        last_x = obj->x;
        last_y = obj->y;
    }
    draw_trade_dots(trade_route, x_draw_offset, y_draw_offset, last_x, last_y, trade_city->x + 25, trade_city->y + 25);
}

static void draw_empire_object(const empire_object *obj)
{
    if (obj->type == EMPIRE_OBJECT_LAND_TRADE_ROUTE || obj->type == EMPIRE_OBJECT_SEA_TRADE_ROUTE) {
        if (!empire_city_is_trade_route_open(obj->trade_route_id)) {
            return;
        }
        if (scenario_empire_id() == SCENARIO_CUSTOM_EMPIRE) {
            window_empire_draw_trade_waypoints(obj, data.x_draw_offset, data.y_draw_offset);
        }
    }
    int x, y, image_id;
    if (scenario_empire_is_expanded()) {
        x = obj->expanded.x;
        y = obj->expanded.y;
        image_id = obj->expanded.image_id;
    } else {
        x = obj->x;
        y = obj->y;
        image_id = obj->image_id;
    }

    if (obj->type == EMPIRE_OBJECT_CITY) {
        const empire_city *city = empire_city_get(empire_city_get_for_object(obj->id));
        if (city->type == EMPIRE_CITY_DISTANT_FOREIGN ||
            city->type == EMPIRE_CITY_FUTURE_ROMAN) {
            image_id = image_group(GROUP_EMPIRE_FOREIGN_CITY);
        } else if (city->type == EMPIRE_CITY_TRADE) {
            // Fix cases where empire map still gives a blue flag for new trade cities
            // (e.g. Massilia in campaign Lugdunum)
            image_id = image_group(GROUP_EMPIRE_CITY_TRADE);
        }
    }
    if (obj->type == EMPIRE_OBJECT_BATTLE_ICON) {
        // handled later
        return;
    }
    if (obj->type == EMPIRE_OBJECT_ENEMY_ARMY) {
        if (city_military_months_until_distant_battle() <= 0) {
            return;
        }
        if (city_military_distant_battle_enemy_months_traveled() != obj->distant_battle_travel_months) {
            return;
        }
    }
    if (obj->type == EMPIRE_OBJECT_ROMAN_ARMY) {
        if (!city_military_distant_battle_roman_army_is_traveling()) {
            return;
        }
        if (city_military_distant_battle_roman_months_traveled() != obj->distant_battle_travel_months) {
            return;
        }
    }
    image_draw(image_id, data.x_draw_offset + x, data.y_draw_offset + y, COLOR_MASK_NONE, SCALE_NONE);
    const image *img = image_get(image_id);
    if (img->animation && img->animation->speed_id) {
        int new_animation = empire_object_update_animation(obj, image_id);
        image_draw(image_id + new_animation,
            data.x_draw_offset + x + img->animation->sprite_offset_x,
            data.y_draw_offset + y + img->animation->sprite_offset_y,
            COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void draw_invasion_warning(int x, int y, int image_id)
{
    image_draw(image_id, data.x_draw_offset + x, data.y_draw_offset + y, COLOR_MASK_NONE, SCALE_NONE);
}

static void draw_map(void)
{
    graphics_set_clip_rectangle(data.x_min + 16, data.y_min + 16,
        data.x_max - data.x_min - 32, data.y_max - data.y_min - 136);

    empire_set_viewport(data.x_max - data.x_min - 32, data.y_max - data.y_min - 136);

    data.x_draw_offset = data.x_min + 16;
    data.y_draw_offset = data.y_min + 16;
    empire_adjust_scroll(&data.x_draw_offset, &data.y_draw_offset);
    image_draw(image_group(GROUP_EMPIRE_MAP), data.x_draw_offset, data.y_draw_offset, COLOR_MASK_NONE, SCALE_NONE);

    empire_object_foreach(draw_empire_object);

    scenario_invasion_foreach_warning(draw_invasion_warning);

    graphics_reset_clip_rectangle();
}

static void draw_city_name(const empire_city *city)
{
    int image_base = image_group(GROUP_EMPIRE_PANELS);
    image_draw(image_base + 6, data.x_min + 2, data.y_max - 199, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 7, data.x_max - 84, data.y_max - 199, COLOR_MASK_NONE, SCALE_NONE);
    image_draw(image_base + 8, (data.x_min + data.x_max - 332) / 2, data.y_max - 181, COLOR_MASK_NONE, SCALE_NONE);
    if (city) {
        int x_offset = (data.x_min + data.x_max - 332) / 2 + 64;
        int y_offset = data.y_max - 118;
        const uint8_t *city_name = empire_city_get_name(city);
        text_draw_centered(city_name, x_offset, y_offset, 268, FONT_LARGE_BLACK, 0);
    }
}

static void draw_panel_buttons(const empire_city *city)
{
    image_buttons_draw(data.x_min + 20, data.y_max - 44, image_button_help, 1);
    image_buttons_draw(data.x_max - 44, data.y_max - 44, image_button_return_to_city, 1);
    image_buttons_draw(data.x_max - 44, data.y_max - 100, image_button_advisor, 1);
    image_buttons_draw(data.x_min + 24, data.y_max - 100, image_button_show_prices, 1);
    if (city) {
        if (city->type == EMPIRE_CITY_TRADE && !city->is_open) {
            button_border_draw((data.x_min + data.x_max - 500) / 2 + 30, data.y_max - 49, 440,
                26, data.selected_button);
        }
    }
}

static void draw_foreground(void)
{
    draw_map();

    const empire_city *city = 0;
    int selected_object = empire_selected_object();
    if (selected_object) {
        const empire_object *object = empire_object_get(selected_object - 1);
        if (object->type == EMPIRE_OBJECT_CITY) {
            data.selected_city = empire_city_get_for_object(object->id);
            city = empire_city_get(data.selected_city);
        }
    }
    draw_paneling();
    draw_city_name(city);
    draw_panel_buttons(city);
    draw_object_info();
}

static int is_outside_map(int x, int y)
{
    return (x < data.x_min + 16 || x >= data.x_max - 16 ||
        y < data.y_min + 16 || y >= data.y_max - 120);
}

static void determine_selected_object(const mouse *m)
{
    if (!m->left.went_up || data.finished_scroll || is_outside_map(m->x, m->y)) {
        data.finished_scroll = 0;
        return;
    }
    empire_select_object(m->x - data.x_min - 16, m->y - data.y_min - 16);
    window_invalidate();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    pixel_offset position;
    if (scroll_get_delta(m, &position, SCROLL_TYPE_EMPIRE)) {
        empire_scroll_map(position.x, position.y);
    }
    if (m->is_touch) {
        const touch *t = touch_get_earliest();
        if (!is_outside_map(t->current_point.x, t->current_point.y)) {
            if (t->has_started) {
                data.is_scrolling = 1;
                scroll_drag_start(1);
            }
        }
        if (t->has_ended) {
            data.is_scrolling = 0;
            data.finished_scroll = !touch_was_click(t);
            scroll_drag_end();
        }
    }
    data.focus_button_id = 0;
    data.focus_resource = 0;
    int button_id;
    image_buttons_handle_mouse(m, data.x_min + 20, data.y_max - 44, image_button_help, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 1;
    }
    image_buttons_handle_mouse(m, data.x_max - 44, data.y_max - 44, image_button_return_to_city, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 2;
    }
    image_buttons_handle_mouse(m, data.x_max - 44, data.y_max - 100, image_button_advisor, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 3;
    }
    image_buttons_handle_mouse(m, data.x_min + 24, data.y_max - 100, image_button_show_prices, 1, &button_id);
    if (button_id) {
        data.focus_button_id = 4;
    }
    button_id = 0;
    determine_selected_object(m);
    int selected_object = empire_selected_object();
    if (selected_object) {
        const empire_object *obj = empire_object_get(selected_object - 1);
        if (obj->type == EMPIRE_OBJECT_CITY) {
            data.selected_city = empire_city_get_for_object(selected_object - 1);
            const empire_city *city = empire_city_get(data.selected_city);
            if (city->type == EMPIRE_CITY_TRADE) {
                if (city->is_open) {
                    int x_offset = (data.x_min + data.x_max - 500) / 2;
                    int y_offset = data.y_max - 113;
                    int index_sell = 0;
                    int index_buy = 0;

                    // we only want to handle resource buttons that the selected city trades
                    for (int resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
                        if (city->sells_resource[resource]) {
                            generic_buttons_handle_mouse(m, x_offset + 120 + 124 * index_sell, y_offset + 31,
                                generic_button_trade_resource + resource - 1, 1, &button_id);
                            index_sell++;
                        } else if (city->buys_resource[resource]) {
                            generic_buttons_handle_mouse(m, x_offset + 120 + 124 * index_buy, y_offset + 62,
                                generic_button_trade_resource + resource - 1, 1, &button_id);
                            index_buy++;
                        }

                        if (button_id) {
                            data.focus_resource = resource;
                            // if we're focusing any button we can skip further checks
                            break;
                        }
                    }
                } else {
                    generic_buttons_handle_mouse(
                        m, (data.x_min + data.x_max - 500) / 2, data.y_max - 105,
                        generic_button_open_trade, 1, &data.selected_button);
                }
            }
        }
        // allow de-selection only for objects that are currently selected/drawn, otherwise exit empire map
        if (input_go_back_requested(m, h)) {
            switch (obj->type) {
                case EMPIRE_OBJECT_CITY:
                    empire_clear_selected_object();
                    window_invalidate();
                    break;
                case EMPIRE_OBJECT_ROMAN_ARMY:
                    if (city_military_distant_battle_roman_army_is_traveling()) {
                        if (city_military_distant_battle_roman_months_traveled() == obj->distant_battle_travel_months) {
                            empire_clear_selected_object();
                            window_invalidate();
                            break;
                        }
                    }
                case EMPIRE_OBJECT_ENEMY_ARMY:
                    if (city_military_months_until_distant_battle() > 0) {
                        if (city_military_distant_battle_enemy_months_traveled() == obj->distant_battle_travel_months) {
                            empire_clear_selected_object();
                            window_invalidate();
                            break;
                        }
                    }
                default:
                    window_city_show();
            }
        }
    } else {
        if (m->right.went_down) {
            scroll_drag_start(0);
        }
        if (m->right.went_up) {
            int has_scrolled = scroll_drag_end();
            if (!has_scrolled && input_go_back_requested(m, h)) {
                window_city_show();
            }
        }
    }
}

static int is_mouse_hit(tooltip_context *c, int x, int y, int size)
{
    int mx = c->mouse_x;
    int my = c->mouse_y;
    return x <= mx && mx < x + size && y <= my && my < y + size;
}

static int get_tooltip_resource(tooltip_context *c)
{
    const empire_city *city = empire_city_get(data.selected_city);
    // we only want to check tooltips on our own closed cities.
    // open city resource tooltips are handled by their respective buttons directly
    if (city->type != EMPIRE_CITY_TRADE || city->is_open) {
        return 0;
    }
    int x_offset = (data.x_min + data.x_max - 500) / 2;
    int y_offset = data.y_max - 113;

    int item_offset = lang_text_get_width(47, 5, FONT_NORMAL_GREEN);
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (city->sells_resource[r]) {
            if (is_mouse_hit(c, x_offset + 60 + item_offset, y_offset + 33, 26)) {
                return r;
            }
            item_offset += 32;
        }
    }
    item_offset += lang_text_get_width(47, 4, FONT_NORMAL_GREEN);
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (city->buys_resource[r]) {
            if (is_mouse_hit(c, x_offset + 110 + item_offset, y_offset + 33, 26)) {
                return r;
            }
            item_offset += 32;
        }
    }

    return 0;
}

static void get_tooltip_trade_route_type(tooltip_context *c)
{
    int selected_object = empire_selected_object();
    if (!selected_object || empire_object_get(selected_object - 1)->type != EMPIRE_OBJECT_CITY) {
        return;
    }

    data.selected_city = empire_city_get_for_object(selected_object - 1);
    const empire_city *city = empire_city_get(data.selected_city);
    if (city->type != EMPIRE_CITY_TRADE || city->is_open) {
        return;
    }

    int x_offset = (data.x_min + data.x_max + 355) / 2;
    int y_offset = data.y_max - 41;
    int y_offset_max = y_offset + 22 - 2 * city->is_sea_trade;
    if (c->mouse_x >= x_offset && c->mouse_x < x_offset + 32 &&
        c->mouse_y >= y_offset && c->mouse_y < y_offset_max) {
        c->type = TOOLTIP_BUTTON;
        c->text_group = 44;
        c->text_id = 28 + city->is_sea_trade;
    }
}

static void get_tooltip(tooltip_context *c)
{
    int resource = data.focus_resource ? data.focus_resource : get_tooltip_resource(c);
    if (resource) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = 131 + resource;
    } else if (data.focus_button_id) {
        c->type = TOOLTIP_BUTTON;
        switch (data.focus_button_id) {
            case 1: c->text_id = 1; break;
            case 2: c->text_id = 2; break;
            case 3: c->text_id = 69; break;
            case 4:
                c->text_group = 54;
                c->text_id = 2;
                break;
        }
    } else {
        get_tooltip_trade_route_type(c);
    }
}

static void button_help(int param1, int param2)
{
    window_message_dialog_show(MESSAGE_DIALOG_EMPIRE_MAP, 0);
}

static void button_return_to_city(int param1, int param2)
{
    window_city_show();
}

static void button_advisor(int advisor, int param2)
{
    window_advisors_show_advisor(advisor);
}

static void button_show_prices(int param1, int param2)
{
    window_trade_prices_show(0, 0, screen_width(), screen_height());
}

static void button_show_resource_window(int resource, int param2)
{
    window_resource_settings_show(resource);
}

static void confirmed_open_trade(int accepted, int checked)
{
    if (accepted) {
        empire_city_open_trade(data.selected_city);
        building_menu_update();
        window_trade_opened_show(data.selected_city);
    }
}

static void button_open_trade(int param1, int param2)
{
    window_popup_dialog_show(POPUP_DIALOG_OPEN_TRADE, confirmed_open_trade, 2);
}

void window_empire_show(void)
{
    window_type window = {
        WINDOW_EMPIRE,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init();
    window_show(&window);
}

void window_empire_show_checked(void)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == AVAILABLE) {
        window_empire_show();
    } else {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET, NEW_WARNING_SLOT);
    }
}
