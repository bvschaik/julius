#include "starting_conditions.h"

#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"

static void button_rank(int param1, int param2);
static void button_start_year(int param1, int param2);
static void button_initial_funds(int param1, int param2);
static void button_rescue_loan(int param1, int param2);
static void button_wheat(int param1, int param2);
static void button_flotsam(int param1, int param2);
static void button_milestone(int param1, int param2);

static generic_button buttons[] = {
    {262, 76, 462, 106, GB_IMMEDIATE, button_rank, button_none},
    {262, 116, 462, 146, GB_IMMEDIATE, button_start_year, button_none},
    {262, 156, 462, 186, GB_IMMEDIATE, button_initial_funds,button_none},
    {262, 196, 462, 226, GB_IMMEDIATE, button_rescue_loan,button_none},
    {262, 236, 462, 266, GB_IMMEDIATE, button_wheat,button_none},
    {262, 276, 462, 306, GB_IMMEDIATE, button_flotsam, button_none, 0},
    {262, 316, 462, 346, GB_IMMEDIATE, button_milestone, button_none, 0},
    {262, 356, 462, 386, GB_IMMEDIATE, button_milestone, button_none, 1},
    {262, 396, 462, 426, GB_IMMEDIATE, button_milestone, button_none, 2}
};

static int focus_button_id;

static void draw_background(void)
{
    // TODO draw city map
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
    lang_text_draw_year(scenario_property_start_year(), 330, 125, FONT_NORMAL_BLACK); // large font??

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
    int width = text_draw_number(scenario_criteria_milestone_year(25), '+', " ", 297, 327, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_criteria_milestone_year(25), 307 + width, 327, FONT_SMALL_PLAIN);

    lang_text_draw(44, 92, 32, 365, FONT_NORMAL_BLACK);
    button_border_draw(262, 356, 200, 30, focus_button_id == 8);
    width = text_draw_number(scenario_criteria_milestone_year(50), '+', " ", 297, 367, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_criteria_milestone_year(50), 307 + width, 367, FONT_SMALL_PLAIN);

    lang_text_draw(44, 93, 32, 405, FONT_NORMAL_BLACK);
    button_border_draw(262, 396, 200, 30, focus_button_id == 9);
    width = text_draw_number(scenario_criteria_milestone_year(75), '+', " ", 297, 407, FONT_NORMAL_BLACK);
    lang_text_draw_year(scenario_property_start_year() + scenario_criteria_milestone_year(75), 307 + width, 407, FONT_SMALL_PLAIN);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (m->right.went_down) {
        window_editor_attributes_show();
        return;
    }

    const mouse *m_dialog = mouse_in_dialog(m);
    generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 9, &focus_button_id);
}

static void button_rank(int param1, int param2)
{
}

static void button_start_year(int param1, int param2)
{
}

static void button_initial_funds(int param1, int param2)
{
}

static void button_rescue_loan(int param1, int param2)
{
}

static void button_wheat(int param1, int param2)
{
}

static void button_flotsam(int param1, int param2)
{
}

static void button_milestone(int param1, int param2)
{
}

void window_editor_starting_conditions_show(void)
{
    window_type window = {
        WINDOW_EDITOR_STARTING_CONDITIONS,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
