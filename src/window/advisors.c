#include "advisors.h"

#include "assets/assets.h"
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
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"
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
#include "window/advisor/housing.h"

static void button_change_advisor(int advisor, int param2);
static void button_help(int param1, int param2);

static image_button help_button = {
    11, -7, 27, 27, IB_NORMAL, GROUP_CONTEXT_ICONS, 0, button_help, button_none, 0, 0, 1
};

static generic_button advisor_buttons[ADVISOR_MAX] = {
    {9, 1, 40, 40, button_change_advisor, button_none, ADVISOR_LABOR, 0},
    {54, 1, 40, 40, button_change_advisor, button_none, ADVISOR_MILITARY, 0},
    {99, 1, 40, 40, button_change_advisor, button_none, ADVISOR_IMPERIAL, 0},
    {144, 1, 40, 40, button_change_advisor, button_none, ADVISOR_RATINGS, 0},
    {189, 1, 40, 40, button_change_advisor, button_none, ADVISOR_TRADE, 0},
    {234, 1, 40, 40, button_change_advisor, button_none, ADVISOR_POPULATION, 0},
    {279, 1, 40, 40, button_change_advisor, button_none, ADVISOR_HOUSING, 0},
    {324, 1, 40, 40, button_change_advisor, button_none, ADVISOR_HEALTH, 0},
    {369, 1, 40, 40, button_change_advisor, button_none, ADVISOR_EDUCATION, 0},
    {414, 1, 40, 40, button_change_advisor, button_none, ADVISOR_ENTERTAINMENT, 0},
    {459, 1, 40, 40, button_change_advisor, button_none, ADVISOR_RELIGION, 0},
    {504, 1, 40, 40, button_change_advisor, button_none, ADVISOR_FINANCIAL, 0},
    {549, 1, 40, 40, button_change_advisor, button_none, ADVISOR_CHIEF, 0},
    {594, 1, 40, 40, button_change_advisor, button_none, 0, 0},
};

static const advisor_window_type *(*sub_advisors[])(void) = {
    0,
    window_advisor_labor,
    window_advisor_military,
    window_advisor_imperial,
    window_advisor_ratings,
    window_advisor_trade,
    window_advisor_population,
    window_advisor_housing,
    window_advisor_health,
    window_advisor_education,
    window_advisor_entertainment,
    window_advisor_religion,
    window_advisor_financial,
    window_advisor_chief,
    0,
    // sub-advisors begin here
    0,
    0,
    0,
    0,
    window_advisor_housing, // housing sub-advisor,
    0,
    0
};

static const int ADVISOR_TO_MESSAGE_TEXT[] = {
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ADVISOR_LABOR,
    MESSAGE_DIALOG_ADVISOR_MILITARY,
    MESSAGE_DIALOG_ADVISOR_IMPERIAL,
    MESSAGE_DIALOG_ADVISOR_RATINGS,
    MESSAGE_DIALOG_ADVISOR_TRADE,
    MESSAGE_DIALOG_ADVISOR_POPULATION,
    MESSAGE_DIALOG_ADVISOR_POPULATION,
    MESSAGE_DIALOG_ADVISOR_HEALTH,
    MESSAGE_DIALOG_ADVISOR_EDUCATION,
    MESSAGE_DIALOG_ADVISOR_ENTERTAINMENT,
    MESSAGE_DIALOG_ADVISOR_RELIGION,
    MESSAGE_DIALOG_ADVISOR_FINANCIAL,
    MESSAGE_DIALOG_ADVISOR_CHIEF,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ADVISOR_POPULATION,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ABOUT,
    MESSAGE_DIALOG_ADVISOR_ENTERTAINMENT
};

static int advisor_image_ids[2][ADVISOR_MAX];

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

void window_advisors_set_advisor(advisor_type advisor)
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

static void prepare_advisor_image_ids(void)
{
    if (advisor_image_ids[0][0]) {
        return;
    }
    int reduce = 0;
    for (int i = 0; i < ADVISOR_MAX; i++) {
        if (i == (ADVISOR_HOUSING - 1)) {
            reduce = 1;
            advisor_image_ids[0][ADVISOR_HOUSING - 1] = assets_get_image_id("UI",
                "Housing Advisor Button");
            advisor_image_ids[1][ADVISOR_HOUSING - 1] = assets_get_image_id("UI",
                "Housing Advisor Button Selected");
        } else {
            advisor_image_ids[0][i] = image_group(GROUP_ADVISOR_ICONS) + i - reduce;
            advisor_image_ids[1][i] = image_group(GROUP_ADVISOR_ICONS) + i - reduce + 13;
        }
    }
}

void window_advisors_draw_dialog_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_ADVISOR_BACKGROUND));
    graphics_in_dialog();
    image_draw(image_group(GROUP_PANEL_WINDOWS) + 13, 0, 432, COLOR_MASK_NONE, SCALE_NONE);

    prepare_advisor_image_ids();

    for (int i = 0; i < ADVISOR_MAX; i++) {
        int selected = current_advisor && i == (current_advisor % ADVISOR_MAX) - 1;
        image_draw(advisor_image_ids[selected][i], 45 * i + 8, 441, COLOR_MASK_NONE, SCALE_NONE);
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
    image_buttons_draw(0, BLOCK_SIZE * (advisor_height - 2), &help_button, 1);
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
    if (generic_buttons_handle_mouse(m_dialog, 0, 440, advisor_buttons, ADVISOR_MAX, &focus_button_id)) {
        return;
    }
    int button_id;
    image_buttons_handle_mouse(m_dialog, 0, BLOCK_SIZE * (advisor_height - 2), &help_button, 1, &button_id);
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
        window_advisors_set_advisor(advisor);
        window_invalidate();
    } else {
        window_city_show();
    }
}

static void button_help(int param1, int param2)
{
    if (current_advisor > 0) {
        window_message_dialog_show(ADVISOR_TO_MESSAGE_TEXT[current_advisor], 0);
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (focus_button_id) {
        c->type = TOOLTIP_BUTTON;
        if (focus_button_id == -1) {
            c->text_id = 1; // help button
        } else if (focus_button_id == ADVISOR_HOUSING) {
            c->translation_key = TR_TOOLTIP_ADVISOR_POPULATION_HOUSING_BUTTON;
        } else {
            c->text_id = 69 + focus_button_id - (focus_button_id >= ADVISOR_HOUSING ? 1 : 0);
        }
        return;
    }
    advisor_tooltip_result result = {
        .text_id = 0,
        .translation_key = 0,
        .precomposed_text = 0
    };
    if (current_advisor_window->get_tooltip_text != 0) {
        current_advisor_window->get_tooltip_text(&result);
    }
    if (result.text_id) {
        c->text_id = result.text_id;
        c->type = TOOLTIP_BUTTON;
    } else if (result.translation_key) {
        c->translation_key = result.translation_key;
        c->type = TOOLTIP_BUTTON;
    } else if (result.precomposed_text) {
        c->precomposed_text = result.precomposed_text;
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
        window_advisors_set_advisor(setting_last_advisor());
        window_advisors_show();
    } else {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET, NEW_WARNING_SLOT);
    }
}

int window_advisors_show_advisor(advisor_type advisor)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == NOT_AVAILABLE || avail == NOT_AVAILABLE_YET) {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET, NEW_WARNING_SLOT);
        return 0;
    }
    window_advisors_set_advisor(advisor);
    window_advisors_show();
    return 1;
}
