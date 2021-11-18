#include "win_criteria.h"

#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/criteria.h"
#include "scenario/editor.h"
#include "scenario/property.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"

enum {
    RATING_CULTURE,
    RATING_PROSPERITY,
    RATING_PEACE,
    RATING_FAVOR
};

static void button_rating_toggle(int rating, int param2);
static void button_rating_value(int rating, int param2);
static void button_time_limit_toggle(int param1, int param2);
static void button_time_limit_years(int param1, int param2);
static void button_survival_toggle(int param1, int param2);
static void button_survival_years(int param1, int param2);
static void button_population_toggle(int param1, int param2);
static void button_population_value(int param1, int param2);
static void button_open_play_toggle(int param1, int param2);

static generic_button buttons[] = {
    {316, 132, 80, 30, button_rating_toggle, button_none, RATING_CULTURE},
    {416, 132, 180, 30, button_rating_value, button_none, RATING_CULTURE},
    {316, 172, 80, 30, button_rating_toggle, button_none, RATING_PROSPERITY},
    {416, 172, 180, 30, button_rating_value, button_none, RATING_PROSPERITY},
    {316, 212, 80, 30, button_rating_toggle, button_none, RATING_PEACE},
    {416, 212, 180, 30, button_rating_value, button_none, RATING_PEACE},
    {316, 252, 80, 30, button_rating_toggle, button_none, RATING_FAVOR},
    {416, 252, 180, 30, button_rating_value, button_none, RATING_FAVOR},
    {316, 292, 80, 30, button_time_limit_toggle, button_none},
    {416, 292, 180, 30, button_time_limit_years, button_none},
    {316, 332, 80, 30, button_survival_toggle, button_none},
    {416, 332, 180, 30, button_survival_years, button_none},
    {316, 372, 80, 30, button_population_toggle, button_none},
    {416, 372, 180, 30, button_population_value, button_none},
    {316, 92, 80, 30, button_open_play_toggle, button_none},
};

static int focus_button_id;

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 32, 38, 26);

    lang_text_draw(44, 48, 26, 42, FONT_LARGE_BLACK);
    lang_text_draw_centered(13, 3, 16, 424, 608, FONT_NORMAL_BLACK);

    int is_open_play = scenario_is_open_play();
    // advanced feature: open play
    lang_text_draw(44, 107, 66, 101, FONT_NORMAL_BLACK);
    button_border_draw(316, 92, 80, 30, focus_button_id == 15);
    lang_text_draw_centered(18, is_open_play, 316, 101, 80, FONT_NORMAL_BLACK);
    // button_border_draw(416, 92, 180, 30, focus_button_id == 16);
    // text_draw_number_centered(scenario_open_play_id(), 416, 101, 180, FONT_NORMAL_BLACK);

    lang_text_draw(44, 50, 66, 141, FONT_NORMAL_BLACK);
    button_border_draw(316, 132, 80, 30, focus_button_id == 1);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_culture_enabled(),
        316, 141, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 132, 180, 30, focus_button_id == 2);
    text_draw_number_centered(scenario_criteria_culture(), 416, 141, 180, FONT_NORMAL_BLACK);

    lang_text_draw(44, 51, 66, 181, FONT_NORMAL_BLACK);
    button_border_draw(316, 172, 80, 30, focus_button_id == 3);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_prosperity_enabled(),
        316, 181, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 172, 180, 30, focus_button_id == 4);
    text_draw_number_centered(scenario_criteria_prosperity(), 416, 181, 180, FONT_NORMAL_BLACK);

    lang_text_draw(44, 52, 66, 221, FONT_NORMAL_BLACK);
    button_border_draw(316, 212, 80, 30, focus_button_id == 5);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_peace_enabled(),
        316, 221, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 212, 180, 30, focus_button_id == 6);
    text_draw_number_centered(scenario_criteria_peace(), 416, 221, 180, FONT_NORMAL_BLACK);

    lang_text_draw(44, 53, 66, 261, FONT_NORMAL_BLACK);
    button_border_draw(316, 252, 80, 30, focus_button_id == 7);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_favor_enabled(),
        316, 261, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 252, 180, 30, focus_button_id == 8);
    text_draw_number_centered(scenario_criteria_favor(), 416, 261, 180, FONT_NORMAL_BLACK);

    lang_text_draw(44, 54, 66, 301, FONT_NORMAL_BLACK);
    button_border_draw(316, 292, 80, 30, focus_button_id == 9);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_time_limit_enabled(),
        316, 301, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 292, 180, 30, focus_button_id == 10);
    int width = text_draw_number(scenario_criteria_time_limit_years(), '+', " ", 436, 301, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + scenario_criteria_time_limit_years(),
        446 + width, 301, FONT_NORMAL_BLACK);

    lang_text_draw(44, 55, 66, 341, FONT_NORMAL_BLACK);
    button_border_draw(316, 332, 80, 30, focus_button_id == 11);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_survival_enabled(),
        316, 341, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 332, 180, 30, focus_button_id == 12);
    width = text_draw_number(scenario_criteria_survival_years(), '+', " ", 436, 341, FONT_NORMAL_BLACK, 0);
    lang_text_draw_year(scenario_property_start_year() + scenario_criteria_survival_years(),
        446 + width, 341, FONT_NORMAL_BLACK);

    lang_text_draw(44, 56, 66, 381, FONT_NORMAL_BLACK);
    button_border_draw(316, 372, 80, 30, focus_button_id == 13);
    lang_text_draw_centered(18, is_open_play ? 0 : scenario_criteria_population_enabled(),
        316, 381, 80, FONT_NORMAL_BLACK);
    button_border_draw(416, 372, 180, 30, focus_button_id == 14);
    text_draw_number_centered(scenario_criteria_population(), 416, 381, 180, FONT_NORMAL_BLACK);

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

static void button_rating_toggle(int rating, int param2)
{
    switch (rating) {
        case RATING_CULTURE:
            scenario_editor_toggle_culture();
            break;
        case RATING_PROSPERITY:
            scenario_editor_toggle_prosperity();
            break;
        case RATING_PEACE:
            scenario_editor_toggle_peace();
            break;
        case RATING_FAVOR:
            scenario_editor_toggle_favor();
            break;
    }
}

static void button_rating_value(int rating, int param2)
{
    void (*callback)(int);
    switch (rating) {
        case RATING_CULTURE:
            callback = scenario_editor_set_culture;
            break;
        case RATING_PROSPERITY:
            callback = scenario_editor_set_prosperity;
            break;
        case RATING_PEACE:
            callback = scenario_editor_set_peace;
            break;
        case RATING_FAVOR:
            callback = scenario_editor_set_favor;
            break;
        default:
            return;
    }
    window_numeric_input_show(screen_dialog_offset_x() + 280, screen_dialog_offset_y() + 100,
                              3, 100, callback);
}

static void button_time_limit_toggle(int param1, int param2)
{
    scenario_editor_toggle_time_limit();
}

static void button_time_limit_years(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 280, screen_dialog_offset_y() + 200,
                              3, 999, scenario_editor_set_time_limit);
}

static void button_survival_toggle(int param1, int param2)
{
    scenario_editor_toggle_survival_time();
}

static void button_survival_years(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 280, screen_dialog_offset_y() + 200,
                              3, 999, scenario_editor_set_survival_time);
}

static void button_population_toggle(int param1, int param2)
{
    scenario_editor_toggle_population();
}

static void button_population_value(int param1, int param2)
{
    window_numeric_input_show(screen_dialog_offset_x() + 280, screen_dialog_offset_y() + 200,
                              5, 99999, scenario_editor_set_population);
}

static void button_open_play_toggle(int param1, int param2)
{
    scenario_editor_toggle_open_play();
}

void window_editor_win_criteria_show(void)
{
    window_type window = {
        WINDOW_EDITOR_WIN_CRITERIA,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}
