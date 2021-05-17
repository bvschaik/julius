#include "trade_prices.h"

#include "building/caravanserai.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/trade_policy.h"
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


static void draw_background(void)
{
    window_draw_underlying_window();

    graphics_in_dialog();

    int has_caravanserai = building_monument_working(BUILDING_CARAVANSERAI);
    int has_lighthouse = building_monument_working(BUILDING_LIGHTHOUSE);
    trade_policy land_policy = city_trade_policy_get(LAND_TRADE_POLICY);
    trade_policy sea_policy = city_trade_policy_get(SEA_TRADE_POLICY);

    int has_land_trade_policy = has_caravanserai && land_policy && land_policy != TRADE_POLICY_3;
    int has_sea_trade_policy = has_lighthouse && sea_policy && sea_policy != TRADE_POLICY_3;
    int same_policy = land_policy == sea_policy;

    int window_height = 11;
    int line_buy_position = 230;
    int line_sell_position = 270;
    int number_margin = 25;
    int button_position = 295;
    int icon_shift = 126;
    int price_shift = 120;
    int four_line = 0;
    int no_policy = !has_land_trade_policy && !has_sea_trade_policy;

    if (((has_sea_trade_policy && !has_land_trade_policy) ||
        (!has_sea_trade_policy && has_land_trade_policy) ||
        (has_sea_trade_policy && has_land_trade_policy && !same_policy))) {
        window_height = 17;
        line_sell_position = 305;
        button_position = 390;
        four_line = 1;
        icon_shift = 66;
        price_shift = 60;
    }

    graphics_shade_rect(17, 53, 622, 334, 0);
    outer_panel_draw(16, 144, 38, window_height);

    lang_text_draw(54, 21, 26, 153, FONT_LARGE_BLACK);

    if (four_line) {
        lang_text_draw_centered(54, 22, 26, line_buy_position, 608, FONT_NORMAL_BLACK);
        lang_text_draw_centered(54, 23, 26, line_sell_position, 608, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw(54, 22, 26, line_buy_position, FONT_NORMAL_BLACK);
        lang_text_draw(54, 23, 26, line_sell_position, FONT_NORMAL_BLACK);
    }


    for (int i = 1; i < 16; i++) {
        int image_offset = i + resource_image_offset(i, RESOURCE_IMAGE_ICON);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + image_offset, icon_shift + 30 * i, 194);

        if (!four_line || no_policy) {
            if (no_policy) {
                text_draw_number_centered(trade_price_buy(i, 0), price_shift + 30 * i, line_buy_position, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(i, 0), price_shift + 30 * i, line_sell_position, 30, FONT_SMALL_PLAIN);
            } else {
                text_draw_number_centered_colored(trade_price_buy(i, 1), price_shift + 30 * i, line_buy_position, 30, FONT_SMALL_PLAIN, land_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(i, 1), price_shift + 30 * i, line_sell_position, 30, FONT_SMALL_PLAIN, land_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            }
        } else {
            if (has_land_trade_policy) {
                text_draw_number_centered_colored(trade_price_buy(i, 1), price_shift + 30 * i, line_buy_position + number_margin, 30, FONT_SMALL_PLAIN, land_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(i, 1), price_shift + 30 * i, line_sell_position + number_margin, 30, FONT_SMALL_PLAIN, land_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            } else {
                text_draw_number_centered(trade_price_buy(i, 1), price_shift + 30 * i, line_buy_position + number_margin, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(i, 1), price_shift + 30 * i, line_sell_position + number_margin, 30, FONT_SMALL_PLAIN);
            }
            if (has_sea_trade_policy) {
                text_draw_number_centered_colored(trade_price_buy(i, 0), price_shift + 30 * i, line_buy_position + 2 * number_margin, 30, FONT_SMALL_PLAIN, sea_policy == TRADE_POLICY_1 ? COLOR_MASK_PURPLE : COLOR_MASK_DARK_GREEN);
                text_draw_number_centered_colored(trade_price_sell(i, 0), price_shift + 30 * i, line_sell_position + 2 * number_margin, 30, FONT_SMALL_PLAIN, sea_policy == TRADE_POLICY_1 ? COLOR_MASK_DARK_GREEN : COLOR_MASK_PURPLE);
            } else {
                text_draw_number_centered(trade_price_buy(i, 0), price_shift + 30 * i, line_buy_position + 2 * number_margin, 30, FONT_SMALL_PLAIN);
                text_draw_number_centered(trade_price_sell(i, 0), price_shift + 30 * i, line_sell_position + 2 * number_margin, 30, FONT_SMALL_PLAIN);
            }
        }
    }

    if (four_line) {
        int y_pos_buy = line_buy_position + number_margin - 5;
        int y_pos_sell = line_sell_position + number_margin - 5;

        int image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE) + 1;

        image_draw(image_id, 60, y_pos_buy);
        image_draw(image_id, 60, y_pos_sell);

        image_id = image_group(GROUP_EMPIRE_TRADE_ROUTE_TYPE);
        if (!same_policy) {
            y_pos_buy += number_margin;
            y_pos_sell += number_margin;
        }
        image_draw(image_id, 60, y_pos_buy);
        image_draw(image_id, 60, y_pos_sell);
    }

    lang_text_draw_centered(13, 1, 16, button_position, 608, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (input_go_back_requested(m, h)) {
        window_advisors_show();
    }
}

static int get_tooltip_resource(tooltip_context *c)
{
    int x_base = screen_dialog_offset_x() + 124;
    int y = screen_dialog_offset_y() + 192;
    int x_mouse = c->mouse_x;
    int y_mouse = c->mouse_y;

    for (int i = 1; i < 16; i++) {
        int x = x_base + 30 * i;
        if (x <= x_mouse && x + 24 > x_mouse && y <= y_mouse && y + 24 > y_mouse) {
            return i;
        }
    }
    return 0;
}

static void get_tooltip(tooltip_context *c)
{
    int resource = get_tooltip_resource(c);
    if (!resource) {
        return;
    }
    c->type = TOOLTIP_BUTTON;
    c->text_id = 131 + resource;
}

void window_trade_prices_show(void)
{
    window_type window = {
        WINDOW_TRADE_PRICES,
        draw_background,
        0,
        handle_input,
        get_tooltip
    };
    window_show(&window);
}
