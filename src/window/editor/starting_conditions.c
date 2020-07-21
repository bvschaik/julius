#include "starting_conditions.h"

#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/editor/start_year.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

static void button_rank(int param1, int param2);
static void button_start_year(int param1, int param2);
static void button_initial_funds(int param1, int param2);
static void button_rescue_loan(int param1, int param2);
static void button_wheat(int param1, int param2);
static void button_flotsam(int param1, int param2);
static void button_milestone(int milestone_pct, int param2);

static generic_button buttons[] = {
    {262, 76, 200, 30, button_rank, button_none},
    {262, 116, 200, 30, button_start_year, button_none},
    {262, 156, 200, 30, button_initial_funds,button_none},
    {262, 196, 200, 30, button_rescue_loan,button_none},
    {262, 236, 200, 30, button_wheat,button_none},
    {262, 276, 200, 30, button_flotsam, button_none, 0},
    {262, 316, 200, 30, button_milestone, button_none, 25},
    {262, 356, 200, 30, button_milestone, button_none, 50},
    {262, 396, 200, 30, button_milestone, button_none, 75}
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 28, 30, 28);

    lang_text_draw(44, 88, 32, 45, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 12, 448, 480, FONT_NORMAL_BLACK);

    lang_text_draw(44, 108, 32, 85, FONT_NORMAL_BLACK);
    button_border_draw(262, 76, 200, 30, focus_button_id == 1);
    lang_text_draw_centered(32, scenario_property_player_rank(), 262, 85, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 89, 32, 125, FONT_NORMAL_BLACK);
    button_border_draw(262, 116, 200, 30, focus_button_id == 2);
    lang_text_draw_year(scenario_property_start_year(), 330, 125, FONT_NORMAL_BLACK);

    lang_text_draw(44, 39, 32, 165, FONT_NORMAL_BLACK);
    button_border_draw(262, 156, 200, 30, focus_button_id == 3);
    text_draw_number_centered(scenario_initial_funds(), 262, 165, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 68, 32, 205, FONT_NORMAL_BLACK);
    button_border_draw(262, 196, 200, 30, focus_button_id == 4);
    text_draw_number_centered(scenario_rescue_loan(), 262, 205, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 43, 32, 245, FONT_NORMAL_BLACK);
    button_border_draw(262, 236, 200, 30, focus_button_id == 5);
    lang_text_draw_centered(18, scenario_property_rome_supplies_wheat(), 262, 245, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 80, 32, 285, FONT_NORMAL_BLACK);
    button_border_draw(262, 276, 200, 30, focus_button_id == 6);
    lang_text_draw_centered(18, scenario_map_has_flotsam(), 262, 285, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 91, 32, 325, FONT_NORMAL_BLACK);
    button_border_draw(262, 316, 200, 30, focus_button_id == 7);
    int width = text_draw_number(scenario_editor_milestone_year(25), '+', " ", 297, 327, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(25), 307 + width, 327, FONT_SMALL_PLAIN);

    lang_text_draw(44, 92, 32, 365, FONT_NORMAL_BLACK);
    button_border_draw(262, 356, 200, 30, focus_button_id == 8);
    width = text_draw_number(scenario_editor_milestone_year(50), '+', " ", 297, 367, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(50), 307 + width, 367, FONT_SMALL_PLAIN);

    lang_text_draw(44, 93, 32, 405, FONT_NORMAL_BLACK);
    button_border_draw(262, 396, 200, 30, focus_button_id == 9);
    width = text_draw_number(scenario_editor_milestone_year(75), '+', " ", 297, 407, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(75), 307 + width, 407, FONT_SMALL_PLAIN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 9, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_rank(int param1, int param2)
{
    window_select_list_show(screen_dialog_offset_x() + 40, screen_dialog_offset_y() + 56,
                            32, 10, scenario_editor_set_player_rank);
}

static void button_start_year(int param1, int param2)
{
    window_editor_start_year_show();
}

static void button_initial_funds(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 120, screen_dialog_offset_y() + 56,
                              5, 99999, scenario_editor_set_initial_funds);
}

static void button_rescue_loan(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 120, screen_dialog_offset_y() + 56,
                              5, 99999, scenario_editor_set_rescue_loan);
}

static void button_wheat(int param1, int param2)
{
    scenario_editor_toggle_rome_supplies_wheat();
}

static void button_flotsam(int param1, int param2)
{
    scenario_editor_toggle_flotsam();
}

static int dialog_milestone_pct;
static void set_milestone_year(int value)
{
    scenario_editor_set_milestone_year(dialog_milestone_pct, value);
}

static void button_milestone(int milestone_pct, int param2)
{
    dialog_milestone_pct = milestone_pct;
    window_numeric_input_show(screen_dialog_offset_x() + 120, screen_dialog_offset_y() + 210,
                              3, 999, set_milestone_year);
}

void window_editor_starting_conditions_show(void)
{
    window_type window = {
        WINDOW_EDITOR_STARTING_CONDITIONS,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
