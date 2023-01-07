#include "trade_prices.h"

#include "building/caravanserai.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "city/trade_policy.h"
#include "empire/city.h"
#include "empire/trade_prices.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/advisors.h"

static struct {
    int x;
    int y;
    int width;
    int height;
    int full_screen;
} shade;

static struct {
    int four_line;
    int window_width;
} data;

static void init(int shade_x, int shade_y, int shade_width, int shade_height)
{
    shade.full_screen = shade_x == 0 && shade_y == 0 && shade_width == screen_width() && shade_height == screen_height();
    if (!shade.full_screen) {
        shade.x = shade_x;
        shade.y = shade_y;
        shade.width = shade_width;
        shade.height = shade_height;
    }
    int has_caravanserai = building_monument_working(BUILDING_CARAVANSERAI);
    int has_lighthouse = building_monument_working(BUILDING_LIGHTHOUSE);
    trade_policy land_policy = city_trade_policy_get(LAND_TRADE_POLICY);
    trade_policy sea_policy = city_trade_policy_get(SEA_TRADE_POLICY);

    int has_land_trade_policy = has_caravanserai && land_policy && land_policy != TRADE_POLICY_3;
    int has_sea_trade_policy = has_lighthouse && sea_policy && sea_policy != TRADE_POLICY_3;
    int same_policy = land_policy == sea_policy;
    data.four_line = ((has_sea_trade_policy && !has_land_trade_policy) ||
        (!has_sea_trade_policy && has_land_trade_policy) ||
        (has_sea_trade_policy && has_land_trade_policy && !same_policy));

    city_resource_determine_available();
    data.window_width = (data.four_line ? 4 : 9) + city_resource_get_potential()->size * 2;
}

static void draw_background(void)
{
    window_draw_underlying_window();

    if (shade.full_screen) {
        graphics_shade_rect(0, 0, screen_width(), screen_height(), 8);
    } else {
        graphics_shade_rect(shade.x + screen_dialog_offset_x(), shade.y + screen_dialog_offset_y(),
            shade.width, shade.height, 8);
    }

    int has_caravanserai = building_monument_working(BUILDING_CARAVANSERAI);
    int has_lighthouse = building_monument_working(BUILDING_LIGHTHOUSE);
    trade_policy land_policy = city_trade_policy_get(LAND_TRADE_POLICY);
    trade_policy sea_policy = city_trade_policy_get(SEA_TRADE_POLICY);

    int has_land_trade_policy = has_caravanserai && land_policy && land_policy != TRADE_POLICY_3;
    int has_sea_trade_policy = has_lighthouse && sea_policy && sea_policy != TRADE_POLICY_3;
    int same_policy = land_policy == sea_policy;
    int window_height = 11;
    int line_buy_position = 86;
    int line_sell_position = 126;
    int number_margin = 25;
    int button_position = 151;
    int icon_shift = 142;
    int price_shift = 136;
    int no_policy = !has_land_trade_policy && !has_sea_trade_policy;

    if (data.four_line) {
        window_height = 17;
        line_sell_position = 161;
        button_position = 244;
        icon_shift = 52;
        price_shift = 46;
    }

    graphics_in_dialog_with_size(data.window_width * BLOCK_SIZE, window_height * BLOCK_SIZE);
    outer_panel_draw(0, 0, data.window_width, window_height);

    lang_text_draw_centered(54, 21, 0, 9, data.window_width * BLOCK_SIZE, FONT_LARGE_BLACK);

    if (data.four_line) {
        lang_text_draw_centered(54, 22, 0, line_buy_position,
            data.window_width * BLOCK_SIZE, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 23, 0, line_sell_position,
            data.window_width * BLOCK_SIZE, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw(54, 22, 10, line_buy_position, FONT_NORMAL_BLACK);
        lang_text_draw(54, 23, 10, line_sell_position, FONT_NORMAL_BLACK);
    }

    const resource_list *list = city_resource_get_potential();
    int resource_offset = BLOCK_SIZE * 2;

    for (int i = 0; i < list->size; i++) {
        resource_type r = list->items[i];
        image_draw(resource_get_data(r)->image.icon, icon_shift + i * resource_offset,
            50, COLOR_MASK_NONE, SCALE_NONE);

        if (!data.four_line || no_policy) {
            if (no_policy) {
                text_draw_number_centered(trade_price_buy(r, 0),
                    price_shift + i * resource_offset, line_buy_position, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(r, 0),
                    price_shift + i * resource_offset, line_sell_position, 30, FONT_SMALL_PLAIN);
            } else {
                text_draw_number_centered_colored(trade_price_buy(r, 1),
                    price_shift + i * resource_offset, line_buy_position, 30, FONT_SMALL_PLAIN,
                    land_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(r, 1),
                    price_shift + i * resource_offset, line_sell_position, 30, FONT_SMALL_PLAIN,
                    land_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            }
        } else {
            if (has_land_trade_policy) {
                text_draw_number_centered_colored(trade_price_buy(r, 1),
                    price_shift + i * resource_offset, line_buy_position + number_margin, 30, FONT_SMALL_PLAIN,
                    land_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(r, 1),
                    price_shift + i * resource_offset, line_sell_position + number_margin, 30, FONT_SMALL_PLAIN,
                    land_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            } else {
                text_draw_number_centered(trade_price_buy(r, 1),
                    price_shift + i * resource_offset, line_buy_position + number_margin, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(r, 1),
                    price_shift + i * resource_offset, line_sell_position + number_margin, 30, FONT_SMALL_PLAIN);
            }
            if (has_sea_trade_policy) {
                text_draw_number_centered_colored(trade_price_buy(r, 0),
                    price_shift + i * resource_offset, line_buy_position + 2 * number_margin, 30, FONT_SMALL_PLAIN,
                    sea_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(r, 0),
                    price_shift + i * resource_offset, line_sell_position + 2 * number_margin, 30, FONT_SMALL_PLAIN,
                    sea_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            } else {
                text_draw_number_centered(trade_price_buy(r, 0),
                    price_shift + i * resource_offset, line_buy_position + 2 * number_margin, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(r, 0),
                    price_shift + i * resource_offset, line_sell_position + 2 * number_margin, 30, FONT_SMALL_PLAIN);
            }
        }
    }

    if (data.four_line) {
        int y_pos_buy = line_buy_position + number_margin - 5;
        int y_pos_sell = line_sell_position + number_margin - 5;

        int image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE) + 1;

        image_draw(image_id, 16, y_pos_buy, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_id, 16, y_pos_sell, COLOR_MASK_NONE, SCALE_NONE);

        image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE);
        if (!same_policy) {
            y_pos_buy += number_margin;
            y_pos_sell += number_margin;
        }
        image_draw(image_id, 16, y_pos_buy, COLOR_MASK_NONE, SCALE_NONE);
        image_draw(image_id, 16, y_pos_sell, COLOR_MASK_NONE, SCALE_NONE);
    }

    lang_text_draw_centered(13, 1, 0, button_position, data.window_width * BLOCK_SIZE, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static resource_type get_tooltip_resource(tooltip_context *c)
{
    int x_base = 52 + (screen_width() - data.window_width * BLOCK_SIZE) / 2;
    if (!data.four_line) {
        x_base += 90;
    }
    int y = 48 + (screen_height() - (data.four_line ? 17 : 11) * BLOCK_SIZE) / 2;
    int x_mouse = c->mouse_x;
    int y_mouse = c->mouse_y;

    const resource_list *list = city_resource_get_potential();

    for (int i = 0; i < list->size; i++) {
        int x = x_base + i * BLOCK_SIZE * 2;
        if (x <= x_mouse && x + 24 > x_mouse && y <= y_mouse && y + 24 > y_mouse) {
            return list->items[i];
        }
    }
    return RESOURCE_NONE;
}

static void get_tooltip(tooltip_context *c)
{
    resource_type resource = get_tooltip_resource(c);
    if (resource == RESOURCE_NONE) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    c->precomposed_text = resource_get_data(resource)->text;
}

void window_trade_prices_show(int shade_x, int shade_y, int shade_width, int shade_height)
{
    init(shade_x, shade_y, shade_width, shade_height);
    window_type window = {
        WINDOW_TRADE_PRICES,
        draw_background,
        0,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}
