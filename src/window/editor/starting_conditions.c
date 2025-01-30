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

static void button_rank(const generic_button *button);
static void button_caesar_salary(const generic_button *button);
static void button_start_year(const generic_button *button);
static void button_initial_funds(const generic_button *button);
static void button_rescue_loan(const generic_button *button);
static void button_wheat(const generic_button *button);
static void button_flotsam(const generic_button *button);
static void button_milestone(const generic_button *button);

static generic_button buttons[] = {
    {262, 48, 200, 30, button_rank},
    {262, 88, 200, 30, button_caesar_salary},
    {262, 128, 200, 30, button_start_year},
    {262, 168, 200, 30, button_initial_funds},
    {262, 208, 200, 30, button_rescue_loan},
    {262, 248, 200, 30, button_wheat},
    {262, 288, 200, 30, button_flotsam},
    {262, 328, 200, 30, button_milestone, 0, 25},
    {262, 368, 200, 30, button_milestone, 0, 50},
    {262, 408, 200, 30, button_milestone, 0, 75}
};

static struct {
    int dialog_milestone_pct;
    unsigned int focus_button_id;
} data;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 30, 30);

    lang_text_draw(44, 88, 32, 17, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 12, 452, 480, FONT_NORMAL_BLACK);

    lang_text_draw(44, 108, 32, 57, FONT_NORMAL_BLACK);
    button_border_draw(262, 48, 200, 30, data.focus_button_id == 1);
    lang_text_draw_centered(32, scenario_property_player_rank(), 262, 57, 200, FONT_NORMAL_BLACK);

    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_CAESAR_SALARY, 32, 97, FONT_NORMAL_BLACK);
    button_border_draw(262, 88, 200, 30, data.focus_button_id == 2);
    text_draw_number_centered(scenario_property_caesar_salary(), 262, 97, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 89, 32, 137, FONT_NORMAL_BLACK);
    button_border_draw(262, 128, 200, 30, data.focus_button_id == 3);
    lang_text_draw_year(scenario_property_start_year(), 330, 137, FONT_NORMAL_BLACK);

    lang_text_draw(44, 39, 32, 177, FONT_NORMAL_BLACK);
    button_border_draw(262, 168, 200, 30, data.focus_button_id == 4);
    text_draw_number_centered(scenario_initial_funds(), 262, 177, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 68, 32, 217, FONT_NORMAL_BLACK);
    button_border_draw(262, 208, 200, 30, data.focus_button_id == 5);
    text_draw_number_centered(scenario_rescue_loan(), 262, 217, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 43, 32, 257, FONT_NORMAL_BLACK);
    button_border_draw(262, 248, 200, 30, data.focus_button_id == 6);
    lang_text_draw_centered(18, scenario_property_rome_supplies_wheat(), 262, 257, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 80, 32, 297, FONT_NORMAL_BLACK);
    button_border_draw(262, 288, 200, 30, data.focus_button_id == 7);
    lang_text_draw_centered(18, scenario_map_has_flotsam(), 262, 297, 200, FONT_NORMAL_BLACK);

    lang_text_draw(44, 91, 32, 337, FONT_NORMAL_BLACK);
    button_border_draw(262, 328, 200, 30, data.focus_button_id == 8);
    int width = text_draw_number(scenario_editor_milestone_year(25), '+', " ", 297, 339, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(25),
        307 + width, 339, FONT_SMALL_PLAIN);

    lang_text_draw(44, 92, 32, 377, FONT_NORMAL_BLACK);
    button_border_draw(262, 368, 200, 30, data.focus_button_id == 9);
    width = text_draw_number(scenario_editor_milestone_year(50), '+', " ", 297, 379, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(50),
        307 + width, 379, FONT_SMALL_PLAIN);

    lang_text_draw(44, 93, 32, 417, FONT_NORMAL_BLACK);
    button_border_draw(262, 408, 200, 30, data.focus_button_id == 10);
    width = text_draw_number(scenario_editor_milestone_year(75), '+', " ", 297, 419, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + scenario_editor_milestone_year(75),
        307 + width, 419, FONT_SMALL_PLAIN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 10, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_editor_attributes_show();
    }
}

static void button_rank(const generic_button *button)
{
    window_select_list_show(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
                            32, 11, scenario_editor_set_player_rank);
}

static void button_caesar_salary(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 5, 60000, scenario_editor_set_caesar_salary);
}

static void button_start_year(const generic_button *button)
{
    window_editor_start_year_show();
}

static void button_initial_funds(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 5, 99999, scenario_editor_set_initial_funds);
}

static void button_rescue_loan(const generic_button *button)
{
    window_numeric_input_show(0, 0, button, 5, 99999, scenario_editor_set_rescue_loan);
}

static void button_wheat(const generic_button *button)
{
    scenario_editor_toggle_rome_supplies_wheat();
}

static void button_flotsam(const generic_button *button)
{
    scenario_editor_toggle_flotsam();
}

static void set_milestone_year(int value)
{
    scenario_editor_set_milestone_year(data.dialog_milestone_pct, value);
}

static void button_milestone(const generic_button *button)
{
    data.dialog_milestone_pct = button->parameter1;
    window_numeric_input_show(0, 0, button, 3, 999, set_milestone_year);
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
