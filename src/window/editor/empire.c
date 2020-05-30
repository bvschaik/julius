#include "empire.h"

#include "core/image_group_editor.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/scroll.h"
#include "scenario/editor.h"
#include "scenario/empire.h"
#include "window/editor/map.h"

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

static void button_change_empire(int is_up, int param2);
static void button_ok(int param1, int param2);

static arrow_button arrow_buttons_empire[] = {
    {8, 48, 17, 24, button_change_empire, 1, 0},
    {32, 48, 15, 24, button_change_empire, 0, 0}
};
static generic_button generic_button_ok[] = {
    {84, 48, 100, 24, button_ok, button_none, 0, 0}
};

static struct {
    int selected_button;
    int selected_city;
    int x_min, x_max, y_min, y_max;
    int x_draw_offset, y_draw_offset;
    int focus_button_id;
    int is_scrolling;
    int finished_scroll;
    int show_battle_objects;
} data;

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

static int map_viewport_width(void)
{
    return data.x_max - data.x_min - 32;
}

static int map_viewport_height(void)
{
    return data.y_max - data.y_min - 136;
}

static void draw_paneling(void)
{
    int image_base = image_group(GROUP_EDITOR_EMPIRE_PANELS);
    // bottom panel background
    graphics_set_clip_rectangle(data.x_min, data.y_min, data.x_max - data.x_min, data.y_max - data.y_min);
    for (int x = data.x_min; x < data.x_max; x += 70) {
        image_draw(image_base + 3, x, data.y_max - 120);
        image_draw(image_base + 3, x, data.y_max - 80);
        image_draw(image_base + 3, x, data.y_max - 40);
    }

    // horizontal bar borders
    for (int x = data.x_min; x < data.x_max; x += 86) {
        image_draw(image_base + 1, x, data.y_min);
        image_draw(image_base + 1, x, data.y_max - 120);
        image_draw(image_base + 1, x, data.y_max - 16);
    }

    // vertical bar borders
    for (int y = data.y_min + 16; y < data.y_max; y += 86) {
        image_draw(image_base, data.x_min, y);
        image_draw(image_base, data.x_max - 16, y);
    }

    // crossbars
    image_draw(image_base + 2, data.x_min, data.y_min);
    image_draw(image_base + 2, data.x_min, data.y_max - 120);
    image_draw(image_base + 2, data.x_min, data.y_max - 16);
    image_draw(image_base + 2, data.x_max - 16, data.y_min);
    image_draw(image_base + 2, data.x_max - 16, data.y_max - 120);
    image_draw(image_base + 2, data.x_max - 16, data.y_max - 16);

    graphics_reset_clip_rectangle();
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
        graphics_clear_screens();
    }
    draw_paneling();
}

static void draw_shadowed_number(int value, int x, int y, color_t color)
{
    text_draw_number_colored(value, '@', " ", x + 1, y - 1, FONT_SMALL_PLAIN, COLOR_BLACK);
    text_draw_number_colored(value, '@', " ", x, y, FONT_SMALL_PLAIN, color);
}

static void draw_empire_object(const empire_object *obj)
{
    int x = obj->x;
    int y = obj->y;
    int image_id = obj->image_id;

    if (!data.show_battle_objects && (
        obj->type == EMPIRE_OBJECT_BATTLE_ICON ||
        obj->type == EMPIRE_OBJECT_ROMAN_ARMY ||
        obj->type == EMPIRE_OBJECT_ENEMY_ARMY)) {
        return;
    }
    if (obj->type == EMPIRE_OBJECT_CITY) {
        const empire_city *city = empire_city_get(empire_city_get_for_object(obj->id));
        if (city->type == EMPIRE_CITY_DISTANT_FOREIGN ||
            city->type == EMPIRE_CITY_FUTURE_ROMAN) {
            image_id = image_group(GROUP_EDITOR_EMPIRE_FOREIGN_CITY);
        }
    } else if (obj->type == EMPIRE_OBJECT_BATTLE_ICON) {
        draw_shadowed_number(obj->invasion_path_id, data.x_draw_offset + x - 9, data.y_draw_offset + y - 9, COLOR_WHITE);
        draw_shadowed_number(obj->invasion_years, data.x_draw_offset + x + 15, data.y_draw_offset + y - 9, COLOR_FONT_RED);
    } else if (obj->type == EMPIRE_OBJECT_ROMAN_ARMY || obj->type == EMPIRE_OBJECT_ENEMY_ARMY) {
        draw_shadowed_number(obj->distant_battle_travel_months,
            data.x_draw_offset + x + 7, data.y_draw_offset + y - 9,
            obj->type == EMPIRE_OBJECT_ROMAN_ARMY ? COLOR_WHITE : COLOR_FONT_RED);
    }
    image_draw(image_id, data.x_draw_offset + x, data.y_draw_offset + y);
    const image *img = image_get(image_id);
    if (img->animation_speed_id) {
        int new_animation = empire_object_update_animation(obj, image_id);
        image_draw(image_id + new_animation,
            data.x_draw_offset + x + img->sprite_offset_x,
            data.y_draw_offset + y + img->sprite_offset_y);
    }
}

static void draw_map(void)
{
    int viewport_width = map_viewport_width();
    int viewport_height = map_viewport_height();
    graphics_set_clip_rectangle(data.x_min + 16, data.y_min + 16, viewport_width, viewport_height);

    empire_set_viewport(viewport_width, viewport_height);

    data.x_draw_offset = data.x_min + 16;
    data.y_draw_offset = data.y_min + 16;
    empire_adjust_scroll(&data.x_draw_offset, &data.y_draw_offset);
    image_draw(image_group(GROUP_EDITOR_EMPIRE_MAP), data.x_draw_offset, data.y_draw_offset);

    empire_object_foreach(draw_empire_object);

    graphics_reset_clip_rectangle();
}

static void draw_resource(resource_type resource, int trade_max, int x_offset, int y_offset)
{
    graphics_draw_inset_rect(x_offset, y_offset, 26, 26);
    int image_id = resource + image_group(GROUP_EDITOR_EMPIRE_RESOURCES);
    int resource_offset = resource_image_offset(resource, RESOURCE_IMAGE_ICON);
    image_draw(image_id + resource_offset, x_offset + 1, y_offset + 1);
    switch (trade_max) {
        case 15:
            image_draw(image_group(GROUP_EDITOR_TRADE_AMOUNT), x_offset + 21, y_offset - 1);
            break;
        case 25:
            image_draw(image_group(GROUP_EDITOR_TRADE_AMOUNT) + 1, x_offset + 17, y_offset - 1);
            break;
        case 40:
            image_draw(image_group(GROUP_EDITOR_TRADE_AMOUNT) + 2, x_offset + 13, y_offset - 1);
            break;
    }
}

static void draw_city_info(const empire_city *city)
{
    int x_offset = data.x_min + 28;
    int y_offset = data.y_max - 85;

    int width = lang_text_draw(21, city->name_id, x_offset, y_offset, FONT_NORMAL_WHITE);

    switch (city->type) {
        case EMPIRE_CITY_DISTANT_ROMAN:
        case EMPIRE_CITY_VULNERABLE_ROMAN:
            lang_text_draw(47, 12, x_offset + 20 + width, y_offset, FONT_NORMAL_GREEN);
            break;
        case EMPIRE_CITY_FUTURE_TRADE:
        case EMPIRE_CITY_DISTANT_FOREIGN:
        case EMPIRE_CITY_FUTURE_ROMAN:
            lang_text_draw(47, 0, x_offset + 20 + width, y_offset, FONT_NORMAL_GREEN);
            break;
        case EMPIRE_CITY_OURS: {
            width += lang_text_draw(47, 1, x_offset + 20 + width, y_offset, FONT_NORMAL_GREEN);
            int resource_x_offset = x_offset + 30 + width;
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                if (empire_object_city_sells_resource(city->empire_object_id, r)) {
                    draw_resource(r, 0, resource_x_offset, y_offset - 9);
                    resource_x_offset += 32;
                }
            }
            break;
        }
        case EMPIRE_CITY_TRADE: {
            width += lang_text_draw(47, 5, x_offset + 20 + width, y_offset, FONT_NORMAL_GREEN);
            int resource_x_offset = x_offset + 30 + width;
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                if (empire_object_city_sells_resource(city->empire_object_id, r)) {
                    draw_resource(r, trade_route_limit(city->route_id, r), resource_x_offset, y_offset - 9);
                    resource_x_offset += 32;
                }
            }
            resource_x_offset += 50;
            resource_x_offset += lang_text_draw(47, 4, resource_x_offset, y_offset, FONT_NORMAL_GREEN);
            resource_x_offset += 10;
            for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                if (empire_object_city_buys_resource(city->empire_object_id, r)) {
                    draw_resource(r, trade_route_limit(city->route_id, r), resource_x_offset, y_offset - 9);
                    resource_x_offset += 32;
                }
            }
            break;
        }
    }
}

static void draw_panel_buttons(const empire_city *city)
{
    arrow_buttons_draw(data.x_min + 20, data.y_max - 100, arrow_buttons_empire, 2);

    if (city) {
        draw_city_info(city);
    } else {
        lang_text_draw_centered(150, scenario_empire_id(), data.x_min, data.y_max - 85, data.x_max - data.x_min, FONT_NORMAL_GREEN);
    }
    lang_text_draw(151, scenario_empire_id(), data.x_min + 220, data.y_max - 45, FONT_NORMAL_GREEN);

    button_border_draw(data.x_min + 104, data.y_max - 52, 100, 24, data.focus_button_id == 1);
    lang_text_draw_centered(44, 7, data.x_min + 104, data.y_max - 45, 100, FONT_NORMAL_GREEN);
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
    draw_panel_buttons(city);
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
    if (h->toggle_editor_battle_info) {
        data.show_battle_objects = !data.show_battle_objects;
    }
    if (m->is_touch) {
        const touch *t = get_earliest_touch();
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
    if (!arrow_buttons_handle_mouse(m, data.x_min + 20, data.y_max - 100, arrow_buttons_empire, 2, 0)) {
        if (!generic_buttons_handle_mouse(m, data.x_min + 20, data.y_max - 100, generic_button_ok, 1, &data.focus_button_id)) {
            determine_selected_object(m);
            int selected_object = empire_selected_object();
            if (selected_object) {
                if (empire_object_get(selected_object - 1)->type == EMPIRE_OBJECT_CITY) {
                    data.selected_city = empire_city_get_for_object(selected_object - 1);
                }
            } else if (input_go_back_requested(m, h)) {
                window_editor_map_show();
            }
        }
    }
}

static void button_change_empire(int is_down, int param2)
{
    scenario_editor_change_empire(is_down ? -1 : 1);
    empire_load_editor(scenario_empire_id(), map_viewport_width(), map_viewport_height());
    window_request_refresh();
}

static void button_ok(int param1, int param2)
{
    window_editor_map_show();
}

void window_editor_empire_show(void)
{
    window_type window = {
        WINDOW_EDITOR_EMPIRE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}
