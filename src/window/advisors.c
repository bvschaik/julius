#include "advisors.h"

#include "city/constants.h"
#include "city/culture.h"
#include "city/finance.h"
#include "city/houses.h"
#include "city/labor.h"
#include "city/migration.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "city/warning.h"
#include "core/image_group.h"
#include "figure/formation.h"
#include "game/settings.h"
#include "game/tutorial.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/window.h"
#include "input/input.h"
#include "window/city.h"
#include "window/message_dialog.h"
#include "window/advisor/chief.h"
#include "window/advisor/education.h"
#include "window/advisor/entertainment.h"
#include "window/advisor/financial.h"
#include "window/advisor/health.h"
#include "window/advisor/imperial.h"
#include "window/advisor/labor.h"
#include "window/advisor/military.h"
#include "window/advisor/population.h"
#include "window/advisor/ratings.h"
#include "window/advisor/religion.h"
#include "window/advisor/trade.h"

static void button_change_advisor(int advisor, int param2);
static void button_help(int param1, int param2);

static image_button help_button = {
    11, -7, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1
};

static generic_button advisor_buttons[] = {
    {12, 1, 40, 40, button_change_advisor, button_none, ADVISOR_LABOR, 0},
    {60, 1, 40, 40, button_change_advisor, button_none, ADVISOR_MILITARY, 0},
    {108, 1, 40, 40, button_change_advisor, button_none, ADVISOR_IMPERIAL, 0},
    {156, 1, 40, 40, button_change_advisor, button_none, ADVISOR_RATINGS, 0},
    {204, 1, 40, 40, button_change_advisor, button_none, ADVISOR_TRADE, 0},
    {252, 1, 40, 40, button_change_advisor, button_none, ADVISOR_POPULATION, 0},
    {300, 1, 40, 40, button_change_advisor, button_none, ADVISOR_HEALTH, 0},
    {348, 1, 40, 40, button_change_advisor, button_none, ADVISOR_EDUCATION, 0},
    {396, 1, 40, 40, button_change_advisor, button_none, ADVISOR_ENTERTAINMENT, 0},
    {444, 1, 40, 40, button_change_advisor, button_none, ADVISOR_RELIGION, 0},
    {492, 1, 40, 40, button_change_advisor, button_none, ADVISOR_FINANCIAL, 0},
    {540, 1, 40, 40, button_change_advisor, button_none, ADVISOR_CHIEF, 0},
    {588, 1, 40, 40, button_change_advisor, button_none, 0, 0},
};

static const advisor_window_type *(*sub_advisors[])(void) = {
    0,
    window_advisor_labor,
    window_advisor_military,
    window_advisor_imperial,
    window_advisor_ratings,
    window_advisor_trade,
    window_advisor_population,
    window_advisor_health,
    window_advisor_education,
    window_advisor_entertainment,
    window_advisor_religion,
    window_advisor_financial,
    window_advisor_chief
};

static const int ADVISOR_TO_MESSAGE_TEXT[] = {
    0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static const advisor_window_type *current_advisor_window = 0;
static advisor_type current_advisor = ADVISOR_NONE;

static int focus_button_id;
static int advisor_height;

static void set_advisor_window(void)
{
    if (sub_advisors[current_advisor]) {
        current_advisor_window = sub_advisors[current_advisor]();
    } else {
        current_advisor_window = 0;
    }
}

static void set_advisor(int advisor)
{
    current_advisor = advisor;
    setting_set_last_advisor(advisor);
    set_advisor_window();
}

static void init(void)
{
    city_labor_allocate_workers();

    city_finance_estimate_taxes();
    city_finance_estimate_wages();
    city_finance_update_interest();
    city_finance_update_salary();
    city_finance_calculate_totals();

    city_migration_determine_no_immigration_cause();

    city_houses_calculate_culture_demands();
    city_culture_update_coverage();

    city_resource_calculate_food_stocks_and_supply_wheat();
    formation_calculate_figures();

    city_ratings_update_explanations();

    set_advisor_window();
}

void window_advisors_draw_dialog_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_ADVISOR_BACKGROUND));
    graphics_in_dialog();
    image_draw(image_group(GROUP_PANEL_WINDOWS) + 13, 0, 432);

    for (int i = 0; i < 13; i++) {
        int selected_offset = 0;
        if (current_advisor && i == current_advisor - 1) {
            selected_offset = 13;
        }
        image_draw(image_group(GROUP_ADVISOR_ICONS) + i + selected_offset, 48 * i + 12, 441);
    }
    graphics_reset_dialog();
}

static void draw_background(void)
{
    window_advisors_draw_dialog_background();
    graphics_in_dialog();
    advisor_height = current_advisor_window->draw_background();
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(0, 16 * (advisor_height - 2), &help_button, 1);
    graphics_reset_dialog();

    if (current_advisor_window->draw_foreground) {
        graphics_in_dialog();
        current_advisor_window->draw_foreground();
        graphics_reset_dialog();
    }
}

static void handle_hotkeys(const hotkeys *h)
{
    if (h->show_advisor) {
        if (current_advisor == h->show_advisor) {
            window_city_show();
        } else {
            window_advisors_show_advisor(h->show_advisor);
        }
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 440, advisor_buttons, 13, &focus_button_id)) {
        return;
    }
    int button_id;
    image_buttons_handle_mouse(m_dialog, 0, 16 * (advisor_height - 2), &help_button, 1, &button_id);
    if (button_id) {
        focus_button_id = -1;
    }
    if (current_advisor_window->handle_mouse && current_advisor_window->handle_mouse(m_dialog)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_city_show();
        return;
    }
}

static void button_change_advisor(int advisor, int param2)
{
    if (advisor) {
        set_advisor(advisor);
        window_invalidate();
    } else {
        window_city_show();
    }
}

static void button_help(int param1, int param2)
{
    if (current_advisor > 0 && current_advisor < 13) {
        window_message_dialog_show(ADVISOR_TO_MESSAGE_TEXT[current_advisor], 0);
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (focus_button_id) {
        c->type = TOOLTIP_BUTTON;
        if (focus_button_id == -1) {
            c->text_id = 1; // help button
        } else {
            c->text_id = 69 + focus_button_id;
        }
        return;
    }
    int text_id = 0;
    if (current_advisor_window->get_tooltip_text) {
        text_id = current_advisor_window->get_tooltip_text();
    }
    if (text_id) {
        c->text_id = text_id;
        c->type = TOOLTIP_BUTTON;
    }
}

advisor_type window_advisors_get_advisor(void)
{
    return current_advisor;
}

void window_advisors_show(void)
{
    window_type window = {
        WINDOW_ADVISORS,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init();
    window_show(&window);
}

void window_advisors_show_checked(void)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == AVAILABLE) {
        set_advisor(setting_last_advisor());
        window_advisors_show();
    } else {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
    }
}

int window_advisors_show_advisor(advisor_type advisor)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == NOT_AVAILABLE || avail == NOT_AVAILABLE_YET) {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
        return 0;
    }
    set_advisor(advisor);
    window_advisors_show();
    return 1;
}
