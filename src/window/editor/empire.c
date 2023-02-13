#include "empire.h"

#include "assets/assets.h"
#include "core/image_group_editor.h"
#include "core/string.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "empire/xml.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/mouse.h"
#include "input/scroll.h"
#include "scenario/data.h"
#include "scenario/editor.h"
#include "scenario/empire.h"
#include "window/editor/map.h"
#include "window/empire.h"

#define MAX_WIDTH 2032
#define MAX_HEIGHT 1136

#define OUR_CITY -1

static void button_change_empire(int is_up, int param2);
static void button_ok(int param1, int param2);
static void button_toggle_invasions(int param1, int param2);
static void button_refresh(int param1, int param2);
static void button_cycle_preview(int param1, int param2);

static arrow_button arrow_buttons_empire[] = {
    {8, 48, 17, 24, button_change_empire, 1, 0},
    {32, 48, 15, 24, button_change_empire, 0, 0}
};
static generic_button generic_buttons[] = {
    {4, 48, 100, 24, button_ok, button_none, 0, 0},
    {124, 48, 150, 24, button_toggle_invasions, button_none, 0, 0},
    {294, 48, 150, 24, button_refresh, button_none, 0, 0},
};
static generic_button preview_button[] = {
    {0, 0, 72, 72, button_cycle_preview, button_none, 0, 0},
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
    int preview_image_group;
    int preview_button_focused;
    int picked_coord_x;
    int picked_coord_y;
    int picked_coord_enabled;
} data;

static void update_screen_size(void)
{
    int s_width = screen_width();
    int s_height = screen_height();
    data.x_min = s_width <= MAX_WIDTH ? 0 : (s_width - MAX_WIDTH) / 2;
    data.x_max = s_width <= MAX_WIDTH ? s_width : data.x_min + MAX_WIDTH;
    data.y_min = s_height <= MAX_HEIGHT ? 0 : (s_height - MAX_HEIGHT) / 2;
    data.y_max = s_height <= MAX_HEIGHT ? s_height : data.y_min + MAX_HEIGHT;
}

static int map_viewport_width(void)
{
    return data.x_max - data.x_min - 32;
}

static int map_viewport_height(void)
{
    return data.y_max - data.y_min - 136;
}

static void init(void)
{
    update_screen_size();
    data.selected_button = 0;
    data.picked_coord_enabled = 0;
    data.preview_image_group = 0;
    int selected_object = empire_selected_object();
    if (selected_object) {
        data.selected_city = empire_city_get_for_object(selected_object - 1);
    } else {
        data.selected_city = 0;
    }
    data.focus_button_id = 0;
    empire_center_on_our_city(map_viewport_width(), map_viewport_height());
}

static void draw_paneling(void)
{
    int image_base = image_group(GROUP_EDITOR_EMPIRE_PANELS);
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

static int draw_preview_image(int x, int y, int center, color_t color_mask, int draw_borders)
{
    if (!data.preview_image_group) {
        return 0;
    }

    int image_id = image_group(data.preview_image_group);
    const image *img = image_get(image_id);
    int x_offset = x + (center - img->width) / 2;
    int y_offset = y + (center - img->height) / 2;
    image_draw(image_id, x_offset, y_offset, color_mask, SCALE_NONE);
    if (img->animation && img->animation->speed_id) {
        image_draw(image_id + 1, x_offset + img->animation->sprite_offset_x,
            y_offset + img->animation->sprite_offset_y, color_mask, SCALE_NONE);
    }
    if (draw_borders) {
        graphics_draw_rect(x_offset, y_offset, img->width, img->height, COLOR_BLACK);
    }
    return 1;
}

static void draw_background(void)
{
    update_screen_size();
    if (data.x_min || data.y_min) {
        graphics_clear_screen();
    }
    draw_paneling();
    draw_preview_image(data.x_max - 92, data.y_max - 100, 72, COLOR_MASK_NONE, 0);
}

static void draw_shadowed_number(int value, int x, int y, color_t color)
{
    text_draw_number(value, '@', " ", x + 1, y - 1, FONT_SMALL_PLAIN, COLOR_BLACK);
    text_draw_number(value, '@', " ", x, y, FONT_SMALL_PLAIN, color);
}

static void draw_empire_object(const empire_object *obj)
{
    int x = obj->x;
    int y = obj->y;
    int image_id = obj->image_id;
    if (obj->type == EMPIRE_OBJECT_TRADE_WAYPOINT || obj->type == EMPIRE_OBJECT_BORDER_EDGE) {
        return;
    }
    if (!data.show_battle_objects && (
        obj->type == EMPIRE_OBJECT_BATTLE_ICON ||
        obj->type == EMPIRE_OBJECT_ROMAN_ARMY ||
        obj->type == EMPIRE_OBJECT_ENEMY_ARMY)) {
        return;
    }
    if (obj->type == EMPIRE_OBJECT_BORDER) {
        window_empire_draw_border(obj, data.x_draw_offset, data.y_draw_offset);
    }
    if (obj->type == EMPIRE_OBJECT_CITY) {
        const empire_city *city = empire_city_get(empire_city_get_for_object(obj->id));
        if (city->type == EMPIRE_CITY_DISTANT_FOREIGN ||
            city->type == EMPIRE_CITY_FUTURE_ROMAN) {
            image_id = image_group(GROUP_EDITOR_EMPIRE_FOREIGN_CITY);
        }
    } else if (obj->type == EMPIRE_OBJECT_BATTLE_ICON) {
        draw_shadowed_number(obj->invasion_path_id,
            data.x_draw_offset + x - 9, data.y_draw_offset + y - 9, COLOR_WHITE);
        draw_shadowed_number(obj->invasion_years,
            data.x_draw_offset + x + 15, data.y_draw_offset + y - 9, COLOR_FONT_RED);
    } else if (obj->type == EMPIRE_OBJECT_ROMAN_ARMY || obj->type == EMPIRE_OBJECT_ENEMY_ARMY) {
        draw_shadowed_number(obj->distant_battle_travel_months,
            data.x_draw_offset + x + 7, data.y_draw_offset + y - 9,
            obj->type == EMPIRE_OBJECT_ROMAN_ARMY ? COLOR_WHITE : COLOR_FONT_RED);
    }
    if (scenario_empire_id() == SCENARIO_CUSTOM_EMPIRE &&
        (obj->type == EMPIRE_OBJECT_LAND_TRADE_ROUTE || obj->type == EMPIRE_OBJECT_SEA_TRADE_ROUTE)) {
        window_empire_draw_trade_waypoints(obj, data.x_draw_offset, data.y_draw_offset);
    }
    if (obj->type == EMPIRE_OBJECT_ORNAMENT) {
        if (image_id < 0) {
            image_id = assets_lookup_image_id(ASSET_FIRST_ORNAMENT) - 1 - image_id;
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
    // Manually fix the Hagia Sophia
    if (obj->image_id == 3372) {
        image_id = assets_lookup_image_id(ASSET_HAGIA_SOPHIA_FIX);
        image_draw(image_id, data.x_draw_offset + x, data.y_draw_offset + y, COLOR_MASK_NONE, SCALE_NONE);
    }
}

static void show_coords(int x_offset, int y_offset, const uint8_t *title, int x_coord, int y_coord)
{
    uint8_t text[100];
    int len = 0;
    string_copy(title, text, 100);
    len = string_length(text);
    string_from_int(text + len, x_coord, 0);
    len = string_length(text);
    string_copy(string_from_ascii(", "), text + len, 100 - len);
    len += 2;
    string_from_int(text + len, y_coord, 0);

    graphics_draw_rect(x_offset, y_offset, 220, 25, COLOR_BLACK);
    graphics_fill_rect(x_offset + 1, y_offset + 1, 218, 23, COLOR_WHITE);

    text_draw_centered(text, x_offset, y_offset + 7, 220, FONT_NORMAL_BLACK, 0);
}

static int is_outside_map(int x, int y)
{
    return (x < data.x_min + 16 || x >= data.x_max - 16 ||
            y < data.y_min + 16 || y >= data.y_max - 120);
}

static void draw_coordinates(void)
{
    if (scenario.empire.id != SCENARIO_CUSTOM_EMPIRE) {
        return;
    }
    const mouse *m = mouse_get();

    int x_coord;
    int y_coord;

    if (m->x < data.x_min + 16) {
        x_coord = data.x_min + 16 - data.x_draw_offset;
    } else if (m->x >= data.x_max - 16) {
        x_coord = data.x_max - 17 - data.x_draw_offset;
    } else {
        x_coord = m->x - data.x_draw_offset;
    }
    if (m->y < data.y_min + 16) {
        y_coord = data.y_min + 16 - data.y_draw_offset;
    } else if (m->y >= data.y_max - 120) {
        y_coord = data.y_max - 121 - data.y_draw_offset;
    } else {
        y_coord = m->y - data.y_draw_offset;
    }

    show_coords(data.x_min + 20, data.y_min + 20,
        lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_CURRENT_COORDS), x_coord, y_coord);
    if (!(m->left.is_down && !is_outside_map(m->x, m->y))) {
        draw_preview_image(m->x, m->y, 0, ALPHA_FONT_SEMI_TRANSPARENT, 0);
    }

    if (data.picked_coord_enabled) {
        x_coord = data.picked_coord_x + data.x_draw_offset;
        y_coord = data.picked_coord_y + data.y_draw_offset;
        show_coords(data.x_min + 20, data.y_min + 50,
            lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SELECTED_COORDS), data.picked_coord_x, data.picked_coord_y);
        if (!draw_preview_image(x_coord, y_coord, 0, ALPHA_FONT_SEMI_TRANSPARENT, 1)) {
            graphics_draw_rect(x_coord - 3, y_coord - 3, 7, 7, COLOR_BLACK);
            graphics_draw_rect(x_coord - 1, y_coord - 1, 3, 3, COLOR_WHITE);
        }
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
    image_draw(image_group(GROUP_EDITOR_EMPIRE_MAP), data.x_draw_offset, data.y_draw_offset,
        COLOR_MASK_NONE, SCALE_NONE);

    empire_object_foreach(draw_empire_object);

    draw_coordinates();

    graphics_reset_clip_rectangle();
}

static void draw_resource(resource_type resource, int trade_max, int x_offset, int y_offset)
{
    graphics_draw_inset_rect(x_offset, y_offset, 26, 26);
    image_draw(resource_get_data(resource)->image.editor.empire, x_offset + 1, y_offset + 1,
        COLOR_MASK_NONE, SCALE_NONE);
    if (trade_max != OUR_CITY) {
        window_empire_draw_resource_shields(trade_max, x_offset, y_offset);
    }
}

static void draw_city_info(const empire_city *city)
{
    int x_offset = data.x_min + 28;
    int y_offset = data.y_max - 85;
    const uint8_t *city_name = empire_city_get_name(city);
    int width = text_draw(city_name, x_offset, y_offset, FONT_NORMAL_WHITE, 0);

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
                    draw_resource(r, OUR_CITY, resource_x_offset, y_offset - 9);
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
    int x_offset;
    if (scenario_empire_id() != SCENARIO_CUSTOM_EMPIRE) {
        arrow_buttons_draw(data.x_min + 20, data.y_max - 100, arrow_buttons_empire, 2);
        x_offset = 104;
    } else {
        x_offset = 24;
    }

    if (city) {
        draw_city_info(city);
    } else {
        lang_text_draw_centered(150, scenario_empire_id(),
            data.x_min, data.y_max - 85, data.x_max - data.x_min, FONT_NORMAL_GREEN);
    }
    lang_text_draw(151, scenario_empire_id(), data.x_min + 220, data.y_max - 45, FONT_NORMAL_GREEN);


    button_border_draw(data.x_min + x_offset, data.y_max - 52, 100, 24, data.focus_button_id == 1);
    lang_text_draw_centered(44, 7, data.x_min + x_offset, data.y_max - 45, 100, FONT_NORMAL_GREEN);

    if (scenario.empire.id == SCENARIO_CUSTOM_EMPIRE) {
        button_border_draw(data.x_min + 144, data.y_max - 52, 150, 24, data.focus_button_id == 2);
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_TOGGLE_INVASIONS,
            data.x_min + 144, data.y_max - 45, 150, FONT_NORMAL_GREEN);

        button_border_draw(data.x_min + 314, data.y_max - 52, 150, 24, data.focus_button_id == 3);
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_REFRESH_EMPIRE,
            data.x_min + 314, data.y_max - 45, 150, FONT_NORMAL_GREEN);

        int width = lang_text_get_width(CUSTOM_TRANSLATION, TR_EDITOR_CITY_PREVIEW, FONT_NORMAL_GREEN);

        if (data.x_min + 564 + width < data.x_max) {
            lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_CITY_PREVIEW,
                data.x_max - 96 - width, data.y_max - 45, FONT_NORMAL_GREEN);
        }

        button_border_draw(data.x_max - 92, data.y_max - 100, 72, 72, data.preview_button_focused);
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
    draw_panel_buttons(city);
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

static void refresh_empire(void)
{
    if (scenario.empire.id != SCENARIO_CUSTOM_EMPIRE) {
        return;
    }
    empire_xml_parse_file(scenario.empire.custom_name);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    pixel_offset position;
    if (scroll_get_delta(m, &position, SCROLL_TYPE_EMPIRE)) {
        empire_scroll_map(position.x, position.y);
    }
    if (h->toggle_editor_battle_info) {
        data.show_battle_objects = !data.show_battle_objects;
    } else if (h->f5_pressed) {
        refresh_empire();
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
    int x_offset = scenario.empire.id == SCENARIO_CUSTOM_EMPIRE ? 20 : 100;
    if (scenario.empire.id != SCENARIO_CUSTOM_EMPIRE &&
        arrow_buttons_handle_mouse(m, data.x_min + 20, data.y_max - 100, arrow_buttons_empire, 2, 0)) {
        return;
    }
    if (generic_buttons_handle_mouse(m, data.x_min + x_offset, data.y_max - 100, generic_buttons,
        scenario.empire.id == SCENARIO_CUSTOM_EMPIRE ? 3 : 1, &data.focus_button_id)) {
        return;
    }
    if (scenario.empire.id == SCENARIO_CUSTOM_EMPIRE &&
        generic_buttons_handle_mouse(m, data.x_max - 92, data.y_max - 100,
            preview_button, 1, &data.preview_button_focused)) {
        return;
    }
    determine_selected_object(m);
    int selected_object = empire_selected_object();
    if (selected_object) {
        if (empire_object_get(selected_object - 1)->type == EMPIRE_OBJECT_CITY) {
            data.selected_city = empire_city_get_for_object(selected_object - 1);
        }
        if (input_go_back_requested(m, h)) {
            empire_clear_selected_object();
            window_invalidate();
        }
    } else {
        if (m->right.went_down) {
            scroll_drag_start(0);
        }
        if (m->right.went_up) {
            int has_scrolled = scroll_drag_end();
            if (!has_scrolled && input_go_back_requested(m, h)) {
                if (data.picked_coord_enabled) {
                    data.picked_coord_enabled = 0;
                } else {
                    window_editor_map_show();
                }
            }
        }
        if (scenario.empire.id == SCENARIO_CUSTOM_EMPIRE && m->left.is_down && !is_outside_map(m->x, m->y)) {
            data.picked_coord_enabled = 1;
            data.picked_coord_x = m->x - data.x_draw_offset;
            data.picked_coord_y = m->y - data.y_draw_offset;
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

static void button_toggle_invasions(int param1, int param2)
{
    data.show_battle_objects = !data.show_battle_objects;
}

static void button_cycle_preview(int param1, int param2)
{
    switch (data.preview_image_group) {
        case GROUP_EMPIRE_CITY:
            data.preview_image_group = GROUP_EMPIRE_CITY_DISTANT_ROMAN;
            break;
        case GROUP_EMPIRE_CITY_DISTANT_ROMAN:
            data.preview_image_group = GROUP_EMPIRE_FOREIGN_CITY;
            break;
        case GROUP_EMPIRE_FOREIGN_CITY:
            data.preview_image_group = GROUP_EMPIRE_CITY_TRADE;
            break;
        case GROUP_EMPIRE_CITY_TRADE:
            data.preview_image_group = 0;
            break;
        default:
            data.preview_image_group = GROUP_EMPIRE_CITY;
            break;
    }
    window_request_refresh();
}

static void button_refresh(int param1, int param2)
{
    refresh_empire();
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
