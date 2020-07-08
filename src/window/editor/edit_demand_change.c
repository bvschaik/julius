#include "edit_demand_change.h"

#include "core/lang.h"
#include "core/string.h"
#include "empire/city.h"
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

#define MAX_ROUTES 20
#define NAME_LENGTH 50

static void button_year(int param1, int param2);
static void button_resource(int param1, int param2);
static void button_route(int param1, int param2);
static void button_toggle_rise(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_save(int param1, int param2);

static generic_button buttons[] = {
    {30, 152, 60, 25, button_year, button_none},
    {190, 152, 120, 25, button_resource, button_none},
    {420, 152, 200, 25, button_route, button_none},
    {350, 192, 100, 25, button_toggle_rise, button_none},
    {30, 230, 250, 25, button_delete, button_none},
    {320, 230, 100, 25, button_save, button_none}
};

static const uint8_t UNKNOWN[4] = {'?', '?', '?', 0};
static uint8_t route_display_names[MAX_ROUTES][NAME_LENGTH];

static struct {
    int id;
    editor_demand_change demand_change;
    int focus_button_id;
    int route_ids[MAX_ROUTES];
    uint8_t *route_names[MAX_ROUTES];
    int num_routes;
} data;

static void create_display_name(int route_id, const uint8_t *city_name)
{
    uint8_t *dst = route_display_names[route_id];
    int offset = string_from_int(dst, route_id, 0);
    dst[offset++] = ' ';
    dst[offset++] = '-';
    dst[offset++] = ' ';
    string_copy(city_name, &dst[offset], NAME_LENGTH - offset);
}

static void init(int id)
{
    data.id = id;
    scenario_editor_demand_change_get(id, &data.demand_change);

    data.num_routes = 0;
    for (int i = 1; i < MAX_ROUTES; i++) {
        empire_city *city = empire_city_get(empire_city_get_for_trade_route(i));
        if (city && (city->type == EMPIRE_CITY_TRADE || city->type == EMPIRE_CITY_FUTURE_TRADE)) {
            create_display_name(i, lang_get_string(21, city->name_id));

            data.route_ids[data.num_routes] = i;
            data.route_names[data.num_routes] = route_display_names[i];
            data.num_routes++;
        } else {
            create_display_name(i, UNKNOWN);
        }
    }
}

static void draw_background(void)
{
    window_editor_map_draw_all();
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
    lang_text_draw_centered(23, data.demand_change.resource, 190, 158, 120, FONT_NORMAL_BLACK);

    lang_text_draw(44, 97, 330, 158, FONT_NORMAL_BLACK);
    button_border_draw(420, 152, 200, 25, data.focus_button_id == 3);
    text_draw_centered(route_display_names[data.demand_change.route_id], 420, 158, 200, FONT_NORMAL_BLACK, 0);

    lang_text_draw(44, 100, 60, 198, FONT_NORMAL_BLACK);
    button_border_draw(350, 192, 100, 25, data.focus_button_id == 4);
    lang_text_draw_centered(44, data.demand_change.is_rise ? 99 : 98, 350, 198, 100, FONT_NORMAL_BLACK);

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
    window_select_list_show(screen_dialog_offset_x() + 320, screen_dialog_offset_y() + 40, 23, 16, set_resource);
}

static void set_route_id(int index)
{
    data.demand_change.route_id = data.route_ids[index];
}

static void button_route(int param1, int param2)
{
    window_select_list_show_text(screen_dialog_offset_x() + 200, screen_dialog_offset_y() + 50, data.route_names, data.num_routes, set_route_id);
}

static void button_toggle_rise(int param1, int param2)
{
    data.demand_change.is_rise = !data.demand_change.is_rise;
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
