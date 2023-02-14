#include "edit_demand_change.h"

#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
#include "empire/trade_route.h"
#include "empire/type.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/demand_changes.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

#include <stdlib.h>
#include <string.h>

static void button_year(int param1, int param2);
static void button_resource(int param1, int param2);
static void button_route(int param1, int param2);
static void button_amount(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_save(int param1, int param2);

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year, button_none},
    {190, 152, 120, 25, button_resource, button_none},
    {420, 152, 200, 25, button_route, button_none},
    {350, 192, 100, 25, button_amount, button_none},
    {30, 230, 250, 25, button_delete, button_none},
    {320, 230, 100, 25, button_save, button_none}
};

static const uint8_t UNKNOWN[4] = { '?', '?', '?', 0 };
static const uint8_t NA[4] = { 'N', '/', 'A', 0 };

static struct {
    int id;
    editor_demand_change demand_change;
    int focus_button_id;
    int *route_ids;
    const uint8_t **route_names;
    int num_routes;
} data;

static void create_route_info(int route_id, const uint8_t *city_name)
{
    int index = route_id - 1;
    data.route_ids[index] = route_id;
    int length = string_length(city_name) + 10;
    uint8_t *dst = malloc(sizeof(uint8_t) * length);
    if (!dst) {
        return;
    }
    int offset = string_from_int(dst, route_id, 0);
    dst[offset++] = ' ';
    dst[offset++] = '-';
    dst[offset++] = ' ';
    string_copy(city_name, &dst[offset], length - offset);
    data.route_names[index] = dst;
}

static void init(int id)
{
    data.id = id;
    for (int i = 0; i < data.num_routes; i++) {
        free((uint8_t *) data.route_names[i]);
    }
    free(data.route_ids);
    free(data.route_names);
    data.num_routes = trade_route_count() - 1;
    if (!data.num_routes) {
        data.route_ids = 0;
        data.route_names = 0;
        return;
    }
    data.route_ids = malloc(sizeof(int) * data.num_routes);
    data.route_names = malloc(sizeof(uint8_t *) * data.num_routes);
    if (!data.route_ids || !data.route_names) {
        return;
    }
    memset(data.route_ids, 0, sizeof(int) * data.num_routes);
    memset(data.route_names, 0, sizeof(uint8_t *) * data.num_routes);
    scenario_editor_demand_change_get(id, &data.demand_change);
    for (int i = 1; i < trade_route_count(); i++) {
        empire_city *city = empire_city_get(empire_city_get_for_trade_route(i));
        if (city && (city->type == EMPIRE_CITY_TRADE || city->type == EMPIRE_CITY_FUTURE_TRADE)) {
            const uint8_t *city_name = empire_city_get_name(city);
            create_route_info(i, city_name);
        } else {
            create_route_info(i, UNKNOWN);
        }
    }
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static const uint8_t *get_text_for_route_id(int route_id)
{
    if (!data.num_routes) {
        return NA;
    }
    // No route selected yet
    if (route_id == 0) {
        return data.route_names[0];
    }
    for (int i = 0; i < data.num_routes; i++) {
        if (data.route_ids[i] == route_id) {
            return data.route_names[i];
        }
    }
    return UNKNOWN;
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 100, 40, 11);
    lang_text_draw(44, 94, 20, 114, FONT_LARGE_BLACK);

    button_border_draw(30, 152, 60, 25, data.focus_button_id == 1);
    text_draw_number_centered_prefix(data.demand_change.year, '+', 30, 158, 60, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + data.demand_change.year, 100, 158, FONT_NORMAL_BLACK);

    button_border_draw(190, 152, 120, 25, data.focus_button_id == 2);
    text_draw_centered(resource_get_data(data.demand_change.resource)->text, 190, 158, 120, FONT_NORMAL_BLACK, COLOR_MASK_NONE);

    lang_text_draw(44, 97, 330, 158, FONT_NORMAL_BLACK);
    button_border_draw(420, 152, 200, 25, data.focus_button_id == 3);
    text_draw_centered(get_text_for_route_id(data.demand_change.route_id), 420, 158, 200, FONT_NORMAL_BLACK, 0);

    lang_text_draw(44, 100, 60, 198, FONT_NORMAL_BLACK);
    button_border_draw(350, 192, 100, 25, data.focus_button_id == 4);
    text_draw_number_centered(data.demand_change.amount, 350, 198, 100, FONT_NORMAL_BLACK);

    button_border_draw(30, 230, 250, 25, data.focus_button_id == 5);
    lang_text_draw_centered(44, 101, 30, 236, 250, FONT_NORMAL_BLACK);

    button_border_draw(320, 230, 100, 25, data.focus_button_id == 6);
    lang_text_draw_centered(18, 3, 320, 236, 100, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 6, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_save(0, 0);
    }
}

static void set_year(int value)
{
    data.demand_change.year = value;
}

static void button_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 50, 3, 999, set_year);
}

static void set_resource(int value)
{
    data.demand_change.resource = value;
}

static void button_resource(int param1, int param2)
{
    static const uint8_t *resource_texts[RESOURCE_MAX];
    for (resource_type resource = RESOURCE_NONE; resource < RESOURCE_MAX; resource++) {
        resource_texts[resource] = resource_get_data(resource)->text;
    }
    window_select_list_show_text(screen_dialog_offset_x() + 320, screen_dialog_offset_y() + 40,
        resource_texts, RESOURCE_MAX, set_resource);
}

static void set_route_id(int index)
{
    data.demand_change.route_id = data.route_ids[index];
}

static void button_route(int param1, int param2)
{
    window_select_list_show_text(screen_dialog_offset_x() + 200, screen_dialog_offset_y() + 50,
        data.route_names, data.num_routes, set_route_id);
}

static void set_change_amount(int value)
{
    data.demand_change.amount = value;
}

static void button_amount(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 100, screen_dialog_offset_y() + 50, 3, 999, set_change_amount);
}

static void button_delete(int param1, int param2)
{
    scenario_editor_demand_change_delete(data.id);
    window_editor_demand_changes_show();
}

static void button_save(int param1, int param2)
{
    scenario_editor_demand_change_save(data.id, &data.demand_change);
    window_editor_demand_changes_show();
}

void window_editor_edit_demand_change_show(int id)
{
    window_type window = {
        WINDOW_EDITOR_EDIT_DEMAND_CHANGE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(id);
    window_show(&window);
}
