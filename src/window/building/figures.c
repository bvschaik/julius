#include "figures.h"

#include "building/building.h"
#include "city/view.h"
#include "core/config.h"
#include "empire/city.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/phrase.h"
#include "figure/trader.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "widget/city.h"

static void select_figure(int index, int param2);

static const int FIGURE_TYPE_TO_BIG_FIGURE_IMAGE[] = {
    8, 13, 13, 9, 4, 13, 8, 16, 7, 4,
    18, 42, 26, 41, 8, 1, 33, 10, 11, 25,
    8, 25, 15, 15, 15, 60, 12, 14, 5, 52,
    52, 2, 3, 6, 6, 13, 8, 8, 17, 12,
    58, 21, 50, 8, 8, 8, 28, 30, 23, 8,
    8, 8, 34, 39, 33, 43, 27, 48, 63, 8,
    8, 8, 8, 8, 53, 8, 38, 62, 54, 55,
    56, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    57, 74, 75, 76, 90, 58, 72, 99, 88, 89,
    10, 11, 59, 32, 33, 34, 35, 37, 12, 13,
    14, 15, 80, 71, 94, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114,
    70, 55, 51, 47, 91, 16, 17
};

static generic_button figure_buttons[] = {
    {26, 46, 50, 50, select_figure, button_none, 0, 0},
    {86, 46, 50, 50, select_figure, button_none, 1, 0},
    {146, 46, 50, 50, select_figure, button_none, 2, 0},
    {206, 46, 50, 50, select_figure, button_none, 3, 0},
    {266, 46, 50, 50, select_figure, button_none, 4, 0},
    {326, 46, 50, 50, select_figure, button_none, 5, 0},
    {386, 46, 50, 50, select_figure, button_none, 6, 0},
};

static struct {
    color_t figure_images[7][48*48];
    int focus_button_id;
    building_info_context *context_for_callback;
} data;

static int big_people_image(figure_type type)
{
    return image_group(GROUP_BIG_PEOPLE) + FIGURE_TYPE_TO_BIG_FIGURE_IMAGE[type] - 1;
}

static int inventory_to_resource_id(int value)
{
    switch (value) {
        case INVENTORY_WHEAT: return RESOURCE_WHEAT;
        case INVENTORY_VEGETABLES: return RESOURCE_VEGETABLES;
        case INVENTORY_FRUIT: return RESOURCE_FRUIT;
        case INVENTORY_MEAT: return RESOURCE_MEAT;
        case INVENTORY_WINE: return RESOURCE_WINE;
        case INVENTORY_OIL: return RESOURCE_OIL;
        case INVENTORY_FURNITURE: return RESOURCE_FURNITURE;
        case INVENTORY_POTTERY: return RESOURCE_POTTERY;
        default: return RESOURCE_NONE;
    }
}

static figure *get_head_of_caravan(figure *f)
{
    while (f->type == FIGURE_TRADE_CARAVAN_DONKEY) {
        f = figure_get(f->leading_figure_id);
    }
    return f;
}

static void draw_trader(building_info_context *c, figure *f)
{
    f = get_head_of_caravan(f);
    const empire_city *city = empire_city_get(f->empire_city_id);
    int width = lang_text_draw(64, f->type, c->x_offset + 40, c->y_offset + 110, FONT_SMALL_BLACK);
    lang_text_draw(21, city->name_id, c->x_offset + 40 + width, c->y_offset + 110, FONT_SMALL_BLACK);

    width = lang_text_draw(129, 1, c->x_offset + 40, c->y_offset + 130, FONT_SMALL_BLACK);
    lang_text_draw_amount(8, 10, f->type == FIGURE_TRADE_SHIP ? 12 : 8, c->x_offset + 40 + width, c->y_offset + 130, FONT_SMALL_BLACK);

    int trader_id = f->trader_id;
    if (f->type == FIGURE_TRADE_SHIP) {
        int text_id;
        switch (f->action_state) {
            case FIGURE_ACTION_114_TRADE_SHIP_ANCHORED: text_id = 6; break;
            case FIGURE_ACTION_112_TRADE_SHIP_MOORED: text_id = 7; break;
            case FIGURE_ACTION_115_TRADE_SHIP_LEAVING: text_id = 8; break;
            default: text_id = 9; break;
        }
        lang_text_draw(129, text_id, c->x_offset + 40, c->y_offset + 150, FONT_SMALL_BLACK);
    } else {
        int text_id;
        switch (f->action_state) {
            case FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING:
                text_id = 12;
                break;
            case FIGURE_ACTION_102_TRADE_CARAVAN_TRADING:
                text_id = 10;
                break;
            case FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING:
                if (trader_has_traded(trader_id)) {
                    text_id = 11;
                } else {
                    text_id = 13;
                }
                break;
            default:
                text_id = 11;
                break;
        }
        lang_text_draw(129, text_id, c->x_offset + 40, c->y_offset + 150, FONT_SMALL_BLACK);
    }
    if (trader_has_traded(trader_id)) {
        // bought
        width = lang_text_draw(129, 4, c->x_offset + 40, c->y_offset + 170, FONT_SMALL_BLACK);
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (trader_bought_resources(trader_id, r)) {
                width += text_draw_number(trader_bought_resources(trader_id, r),
                    '@', " ", c->x_offset + 40 + width, c->y_offset + 170, FONT_SMALL_BLACK);
                int image_id = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
                image_draw(image_id, c->x_offset + 40 + width, c->y_offset + 167);
                width += 25;
            }
        }
        // sold
        width = lang_text_draw(129, 5, c->x_offset + 40, c->y_offset + 200, FONT_SMALL_BLACK);
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (trader_sold_resources(trader_id, r)) {
                width += text_draw_number(trader_sold_resources(trader_id, r),
                    '@', " ", c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
                int image_id = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
                image_draw(image_id, c->x_offset + 40 + width, c->y_offset + 197);
                width += 25;
            }
        }
    } else { // nothing sold/bought (yet)
        // buying
        width = lang_text_draw(129, 2, c->x_offset + 40, c->y_offset + 170, FONT_SMALL_BLACK);
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (city->buys_resource[r]) {
                int image_id = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
                image_draw(image_id, c->x_offset + 40 + width, c->y_offset + 167);
                width += 25;
            }
        }
        // selling
        width = lang_text_draw(129, 3, c->x_offset + 40, c->y_offset + 200, FONT_SMALL_BLACK);
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (city->sells_resource[r]) {
                int image_id = image_group(GROUP_RESOURCE_ICONS) + r + resource_image_offset(r, RESOURCE_IMAGE_ICON);
                image_draw(image_id, c->x_offset + 40 + width, c->y_offset + 197);
                width += 25;
            }
        }
    }
}

static void draw_enemy(building_info_context *c, figure *f)
{
    int image_id = 8;
    int enemy_type = formation_get(f->formation_id)->enemy_type;
    switch (f->type) {
        case FIGURE_ENEMY43_SPEAR:
            switch (enemy_type) {
                case ENEMY_5_PERGAMUM: image_id = 44; break;
                case ENEMY_6_SELEUCID: image_id = 46; break;
                case ENEMY_7_ETRUSCAN: image_id = 32; break;
                case ENEMY_8_GREEK: image_id = 36; break;
            }
            break;
        case FIGURE_ENEMY44_SWORD:
            switch (enemy_type) {
                case ENEMY_5_PERGAMUM: image_id = 45; break;
                case ENEMY_6_SELEUCID: image_id = 47; break;
                case ENEMY_9_EGYPTIAN: image_id = 29; break;
            }
            break;
        case FIGURE_ENEMY45_SWORD:
            switch (enemy_type) {
                case ENEMY_7_ETRUSCAN: image_id = 31; break;
                case ENEMY_8_GREEK: image_id = 37; break;
                case ENEMY_10_CARTHAGINIAN: image_id = 22; break;
            }
            break;
        case FIGURE_ENEMY49_FAST_SWORD:
            switch (enemy_type) {
                case ENEMY_0_BARBARIAN: image_id = 21; break;
                case ENEMY_1_NUMIDIAN: image_id = 20; break;
                case ENEMY_4_GOTH: image_id = 35; break;
            }
            break;
        case FIGURE_ENEMY50_SWORD:
            switch (enemy_type) {
                case ENEMY_2_GAUL: image_id = 40; break;
                case ENEMY_3_CELT: image_id = 24; break;
            }
            break;
        case FIGURE_ENEMY51_SPEAR:
            switch (enemy_type) {
                case ENEMY_1_NUMIDIAN: image_id = 20; break;
            }
            break;
    }
    image_draw(image_group(GROUP_BIG_PEOPLE) + image_id - 1, c->x_offset + 28, c->y_offset + 112);

    lang_text_draw(65, f->name, c->x_offset + 90, c->y_offset + 108, FONT_LARGE_BROWN);
    lang_text_draw(37, scenario_property_enemy() + 20, c->x_offset + 92, c->y_offset + 149, FONT_SMALL_BLACK);
}

static void draw_animal(building_info_context *c, figure *f)
{
    image_draw(big_people_image(f->type), c->x_offset + 28, c->y_offset + 112);
    lang_text_draw(64, f->type, c->x_offset + 92, c->y_offset + 139, FONT_SMALL_BLACK);
}

static void draw_cartpusher(building_info_context *c, figure *f)
{
    image_draw(big_people_image(f->type), c->x_offset + 28, c->y_offset + 112);

    lang_text_draw(65, f->name, c->x_offset + 90, c->y_offset + 108, FONT_LARGE_BROWN);
    int width = lang_text_draw(64, f->type, c->x_offset + 92, c->y_offset + 139, FONT_SMALL_BLACK);

    if (f->action_state != FIGURE_ACTION_132_DOCKER_IDLING && f->resource_id) {
        int resource = f->resource_id;
        image_draw(image_group(GROUP_RESOURCE_ICONS) +
            resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 92 + width, c->y_offset + 135);
    }

    lang_text_draw_multiline(130, 21 * c->figure.sound_id + c->figure.phrase_id + 1,
        c->x_offset + 90, c->y_offset + 160, 16 * (c->width_blocks - 8), FONT_SMALL_BLACK);

    if (!f->building_id) {
        return;
    }
    building *source_building = building_get(f->building_id);
    building *target_building = building_get(f->destination_building_id);
    int is_returning = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_27_CARTPUSHER_RETURNING:
        case FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY:
        case FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD:
        case FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE:
        case FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE:
        case FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING:
        case FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING:
            is_returning = 1;
            break;
    }
    if (f->action_state != FIGURE_ACTION_132_DOCKER_IDLING) {
        if (is_returning) {
            width = lang_text_draw(129, 16, c->x_offset + 40, c->y_offset + 200, FONT_SMALL_BLACK);
            width += lang_text_draw(41, source_building->type, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
            width += lang_text_draw(129, 14, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
            lang_text_draw(41, target_building->type, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
        } else {
            width = lang_text_draw(129, 15, c->x_offset + 40, c->y_offset + 200, FONT_SMALL_BLACK);
            width += lang_text_draw(41, target_building->type, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
            width += lang_text_draw(129, 14, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
            lang_text_draw(41, source_building->type, c->x_offset + 40 + width, c->y_offset + 200, FONT_SMALL_BLACK);
        }
    }
}

static void draw_market_buyer(building_info_context *c, figure *f)
{
    image_draw(big_people_image(f->type), c->x_offset + 28, c->y_offset + 112);

    lang_text_draw(65, f->name, c->x_offset + 90, c->y_offset + 108, FONT_LARGE_BROWN);
    int width = lang_text_draw(64, f->type, c->x_offset + 92, c->y_offset + 139, FONT_SMALL_BLACK);

    if (f->action_state == FIGURE_ACTION_145_MARKET_BUYER_GOING_TO_STORAGE) {
        width += lang_text_draw(129, 17, c->x_offset + 90 + width, c->y_offset + 139, FONT_SMALL_BLACK);
        int resource = inventory_to_resource_id(f->collecting_item_id);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 90 + width, c->y_offset + 135);
    } else if (f->action_state == FIGURE_ACTION_146_MARKET_BUYER_RETURNING) {
        width += lang_text_draw(129, 18, c->x_offset + 90 + width, c->y_offset + 139, FONT_SMALL_BLACK);
        int resource = inventory_to_resource_id(f->collecting_item_id);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + resource + resource_image_offset(resource, RESOURCE_IMAGE_ICON),
            c->x_offset + 90 + width, c->y_offset + 135);
    }
    if (c->figure.phrase_id >= 0) {
        lang_text_draw_multiline(130, 21 * c->figure.sound_id + c->figure.phrase_id + 1,
            c->x_offset + 90, c->y_offset + 160, 16 * (c->width_blocks - 8), FONT_SMALL_BLACK);
    }
}

static void draw_normal_figure(building_info_context *c, figure *f)
{
    int image_id = big_people_image(f->type);
    if (f->action_state == FIGURE_ACTION_74_PREFECT_GOING_TO_FIRE ||
        f->action_state == FIGURE_ACTION_75_PREFECT_AT_FIRE) {
        image_id = image_group(GROUP_BIG_PEOPLE) + 18;
    }
    image_draw(image_id, c->x_offset + 28, c->y_offset + 112);

    lang_text_draw(65, f->name, c->x_offset + 90, c->y_offset + 108, FONT_LARGE_BROWN);
    lang_text_draw(64, f->type, c->x_offset + 92, c->y_offset + 139, FONT_SMALL_BLACK);

    if (c->figure.phrase_id >= 0) {
        lang_text_draw_multiline(130, 21 * c->figure.sound_id + c->figure.phrase_id + 1,
            c->x_offset + 90, c->y_offset + 160, 16 * (c->width_blocks - 8), FONT_SMALL_BLACK);
    }
}

static void draw_figure_info(building_info_context *c, int figure_id)
{
    button_border_draw(c->x_offset + 24, c->y_offset + 102, 16 * (c->width_blocks - 3), 122, 0);

    figure *f = figure_get(figure_id);
    int type = f->type;
    if (type == FIGURE_TRADE_CARAVAN || type == FIGURE_TRADE_CARAVAN_DONKEY || type == FIGURE_TRADE_SHIP) {
        draw_trader(c, f);
    } else if (type >= FIGURE_ENEMY43_SPEAR && type <= FIGURE_ENEMY53_AXE) {
        draw_enemy(c, f);
    } else if (type == FIGURE_FISHING_BOAT || type == FIGURE_SHIPWRECK || figure_is_herd(f)) {
        draw_animal(c, f);
    } else if (type == FIGURE_CART_PUSHER || type == FIGURE_WAREHOUSEMAN || type == FIGURE_DOCKER) {
        draw_cartpusher(c, f);
    } else if (type == FIGURE_MARKET_BUYER) {
        draw_market_buyer(c, f);
    } else {
        draw_normal_figure(c, f);
    }
}

void window_building_draw_figure_list(building_info_context *c)
{
    inner_panel_draw(c->x_offset + 16, c->y_offset + 40, c->width_blocks - 2, 12);
    if (c->figure.count <= 0) {
        lang_text_draw_centered(70, 0, c->x_offset, c->y_offset + 120, 16 * c->width_blocks, FONT_SMALL_BLACK);
    } else {
        for (int i = 0; i < c->figure.count; i++) {
            button_border_draw(c->x_offset + 60 * i + 25, c->y_offset + 45, 52, 52, i == c->figure.selected_index);
            graphics_draw_from_buffer(c->x_offset + 27 + 60 * i, c->y_offset + 47, 48, 48, data.figure_images[i]);
        }
        draw_figure_info(c, c->figure.figure_ids[c->figure.selected_index]);
    }
    c->figure.drawn = 1;
}

static void draw_figure_in_city(int figure_id, pixel_coordinate *coord)
{
    int x_cam, y_cam;
    city_view_get_camera(&x_cam, &y_cam);

    int grid_offset = figure_get(figure_id)->grid_offset;
    int x, y;
    city_view_grid_offset_to_xy_view(grid_offset, &x, &y);

    city_view_set_camera(x - 2, y - 6);

    widget_city_draw_for_figure(figure_id, coord);

    city_view_set_camera(x_cam, y_cam);
}

void window_building_prepare_figure_list(building_info_context *c)
{
    if (c->figure.count > 0) {
        pixel_coordinate coord = {0, 0};
        if (config_get(CONFIG_UI_ZOOM)) {
            graphics_set_active_canvas(CANVAS_CITY);
        }
        for (int i = 0; i < c->figure.count; i++) {
            draw_figure_in_city(c->figure.figure_ids[i], &coord);
            graphics_save_to_buffer(coord.x, coord.y, 48, 48, data.figure_images[i]);
        }
        graphics_set_active_canvas(CANVAS_UI);
        widget_city_draw();
    }
}

int window_building_handle_mouse_figure_list(const mouse *m, building_info_context *c)
{
    data.context_for_callback = c;
    int handled = generic_buttons_handle_mouse(m, c->x_offset, c->y_offset, figure_buttons, c->figure.count, &data.focus_button_id);
    data.context_for_callback = 0;
    return handled;
}

static void select_figure(int index, int param2)
{
    data.context_for_callback->figure.selected_index = index;
    window_building_play_figure_phrase(data.context_for_callback);
    window_invalidate();
}

void window_building_play_figure_phrase(building_info_context *c)
{
    int figure_id = c->figure.figure_ids[c->figure.selected_index];
    figure *f = figure_get(figure_id);
    c->figure.sound_id = figure_phrase_play(f);
    c->figure.phrase_id = f->phrase_id;
}
