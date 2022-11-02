#include "special_events.h"

#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor_events.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"

static void button_earthquake_severity(int param1, int param2);
static void button_earthquake_year(int param1, int param2);
static void button_gladiator_toggle(int param1, int param2);
static void button_gladiator_year(int param1, int param2);
static void button_emperor_toggle(int param1, int param2);
static void button_emperor_year(int param1, int param2);
static void button_sea_trade_toggle(int param1, int param2);
static void button_land_trade_toggle(int param1, int param2);
static void button_raise_wages_toggle(int param1, int param2);
static void button_max_wages(int param1, int param2);
static void button_lower_wages_toggle(int param1, int param2);
static void button_min_wages(int param1, int param2);
static void button_contamination_toggle(int param1, int param2);
static void button_iron_mine_toggle(int param1, int param2);
static void button_clay_pit_toggle(int param1, int param2);

static generic_button buttons[] = {
    {216, 106, 100, 24, button_earthquake_severity, button_none},
    {326, 106, 150, 24, button_earthquake_year, button_none},
    {216, 136, 100, 24, button_gladiator_toggle,button_none},
    {326, 136, 150, 24, button_gladiator_year,button_none},
    {216, 166, 100, 24, button_emperor_toggle,button_none},
    {326, 166, 150, 24, button_emperor_year, button_none},
    {216, 196, 100, 24, button_sea_trade_toggle, button_none},
    {216, 226, 100, 24, button_land_trade_toggle, button_none},
    {216, 256, 100, 24, button_raise_wages_toggle, button_none},
    {465, 256, 100, 24, button_max_wages, button_none},
    {216, 286, 100, 24, button_lower_wages_toggle, button_none},
    {465, 286, 100, 24, button_min_wages, button_none},
    {216, 316, 100, 24, button_contamination_toggle, button_none},
    {216, 346, 100, 24, button_iron_mine_toggle, button_none},
    {216, 376, 100, 24, button_clay_pit_toggle, button_none},
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 32, 32, 26);

    lang_text_draw(38, 0, 32, 48, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 480, FONT_NORMAL_BLACK);

    // table header
    lang_text_draw(38, 11, 236, 92, FONT_SMALL_PLAIN);
    lang_text_draw(38, 12, 346, 92, FONT_SMALL_PLAIN);

    // earthquake
    lang_text_draw(38, 1, 36, 112, FONT_NORMAL_BLACK);
    button_border_draw(216, 106, 100, 24, focus_button_id == 1);
    lang_text_draw_centered(40, scenario_editor_earthquake_severity(), 216, 112, 100, FONT_NORMAL_BLACK);

    button_border_draw(326, 106, 150, 24, focus_button_id == 2);
    int width = text_draw_number(scenario_editor_earthquake_year(), '+', " ", 346, 112, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(
        scenario_property_start_year() + scenario_editor_earthquake_year(),
        356 + width, 112, FONT_NORMAL_BLACK);

    // gladiator revolt
    lang_text_draw(38, 2, 36, 142, FONT_NORMAL_BLACK);
    button_border_draw(216, 136, 100, 24, focus_button_id == 3);
    lang_text_draw_centered(18, scenario_editor_gladiator_revolt_enabled(), 216, 142, 100, FONT_NORMAL_BLACK);

    button_border_draw(326, 136, 150, 24, focus_button_id == 4);
    width = text_draw_number(scenario_editor_gladiator_revolt_year(), '+', " ", 346, 142, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(
        scenario_property_start_year() + scenario_editor_gladiator_revolt_year(),
        356 + width, 142, FONT_NORMAL_BLACK);

    // emperor change
    lang_text_draw(38, 3, 36, 172, FONT_NORMAL_BLACK);
    button_border_draw(216, 166, 100, 24, focus_button_id == 5);
    lang_text_draw_centered(18, scenario_editor_emperor_change_enabled(), 216, 172, 100, FONT_NORMAL_BLACK);

    button_border_draw(326, 166, 150, 24, focus_button_id == 6);
    width = text_draw_number(scenario_editor_emperor_change_year(), '+', " ", 346, 172, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(
        scenario_property_start_year() + scenario_editor_emperor_change_year(),
        356 + width, 172, FONT_NORMAL_BLACK);

    // random events
    lang_text_draw(38, 4, 36, 202, FONT_NORMAL_BLACK);
    button_border_draw(216, 196, 100, 24, focus_button_id == 7);
    lang_text_draw_centered(18, scenario_editor_sea_trade_problem_enabled(), 216, 202, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 204, FONT_SMALL_PLAIN);

    lang_text_draw(38, 5, 36, 232, FONT_NORMAL_BLACK);
    button_border_draw(216, 226, 100, 24, focus_button_id == 8);
    lang_text_draw_centered(18, scenario_editor_land_trade_problem_enabled(), 216, 232, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 234, FONT_SMALL_PLAIN);

    lang_text_draw(38, 6, 36, 262, FONT_NORMAL_BLACK);
    button_border_draw(216, 256, 100, 24, focus_button_id == 9);
    lang_text_draw_centered(18, scenario_editor_raise_wages_enabled(), 216, 262, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 264, FONT_SMALL_PLAIN);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_MAX_WAGES, 405, 264, FONT_SMALL_PLAIN);
    button_border_draw(465, 256, 50, 24, focus_button_id == 10);
    text_draw_number_centered(scenario_editor_get_max_wages(), 465, 262, 50, FONT_NORMAL_BLACK);

    lang_text_draw(38, 7, 36, 292, FONT_NORMAL_BLACK);
    button_border_draw(216, 286, 100, 24, focus_button_id == 11);
    lang_text_draw_centered(18, scenario_editor_lower_wages_enabled(), 216, 292, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 294, FONT_SMALL_PLAIN);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_MIN_WAGES, 405, 294, FONT_SMALL_PLAIN);
    button_border_draw(465, 286, 50, 24, focus_button_id == 12);
    text_draw_number_centered(scenario_editor_get_min_wages(), 465, 292, 50, FONT_NORMAL_BLACK);

    lang_text_draw(38, 8, 36, 322, FONT_NORMAL_BLACK);
    button_border_draw(216, 316, 100, 24, focus_button_id == 13);
    lang_text_draw_centered(18, scenario_editor_contaminated_water_enabled(), 216, 322, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 324, FONT_SMALL_PLAIN);

    lang_text_draw(38, 9, 36, 352, FONT_NORMAL_BLACK);
    button_border_draw(216, 346, 100, 24, focus_button_id == 14);
    lang_text_draw_centered(18, scenario_editor_iron_mine_collapse_enabled(), 216, 352, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 354, FONT_SMALL_PLAIN);

    lang_text_draw(38, 10, 36, 382, FONT_NORMAL_BLACK);
    button_border_draw(216, 376, 100, 24, focus_button_id == 15);
    lang_text_draw_centered(18, scenario_editor_clay_pit_flooded_enabled(), 216, 382, 100, FONT_NORMAL_BLACK);
    lang_text_draw(38, 13, 346, 384, FONT_SMALL_PLAIN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 15, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_earthquake_severity(int param1, int param2)
{
    scenario_editor_earthquake_cycle_severity();
    window_request_refresh();
}

static void button_earthquake_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 190, screen_dialog_offset_y() + 100,
                              3, 999, scenario_editor_earthquake_set_year);
}

static void button_gladiator_toggle(int param1, int param2)
{
    scenario_editor_gladiator_revolt_toggle_enabled();
    window_request_refresh();
}

static void button_gladiator_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 190, screen_dialog_offset_y() + 100,
                              3, 999, scenario_editor_gladiator_revolt_set_year);
}

static void button_emperor_toggle(int param1, int param2)
{
    scenario_editor_emperor_change_toggle_enabled();
    window_request_refresh();
}

static void button_emperor_year(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 190, screen_dialog_offset_y() + 100,
                              3, 999, scenario_editor_emperor_change_set_year);
}

static void button_sea_trade_toggle(int param1, int param2)
{
    scenario_editor_sea_trade_problem_toggle_enabled();
    window_request_refresh();
}

static void button_land_trade_toggle(int param1, int param2)
{
    scenario_editor_land_trade_problem_toggle_enabled();
    window_request_refresh();
}

static void button_raise_wages_toggle(int param1, int param2)
{
    scenario_editor_raise_wages_toggle_enabled();
    window_request_refresh();
}

static void set_max_wages(int amount)
{
    scenario_editor_set_max_wages(amount);
}

static void button_max_wages(int param1, int param2)
{
    window_numeric_input_show(400, 256, 2, 99, set_max_wages);
}

static void button_lower_wages_toggle(int param1, int param2)
{
    scenario_editor_lower_wages_toggle_enabled();
    window_request_refresh();
}

static void set_min_wages(int amount)
{
    scenario_editor_set_min_wages(amount);
}

static void button_min_wages(int param1, int param2)
{
    window_numeric_input_show(400, 286, 2, 99, set_min_wages);
}

static void button_contamination_toggle(int param1, int param2)
{
    scenario_editor_contaminated_water_toggle_enabled();
    window_request_refresh();
}

static void button_iron_mine_toggle(int param1, int param2)
{
    scenario_editor_iron_mine_collapse_toggle_enabled();
    window_request_refresh();
}

static void button_clay_pit_toggle(int param1, int param2)
{
    scenario_editor_clay_pit_flooded_toggle_enabled();
    window_request_refresh();
}

void window_editor_special_events_show(void)
{
    window_type window = {
        WINDOW_EDITOR_SPECIAL_EVENTS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
