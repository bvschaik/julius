#include "advisors.h"

#include "city/constants.h"
#include "city/culture.h"
#include "city/finance.h"
#include "city/labor.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "city/warning.h"
#include "figure/formation.h"
#include "game/settings.h"
#include "game/tutorial.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/window.h"
#include "window/city.h"

#include "UI/Advisors_private.h"
#include "UI/MessageDialog.h"

static void button_change_advisor(int advisor, int param2);
static void button_help(int param1, int param2);

static image_button helpButton = {
    11, -7, 27, 27, IB_NORMAL, 134, 0, button_help, button_none, 0, 0, 1
};

static generic_button advisorButtons[] = {
    {12, 1, 52, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_LABOR, 0},
    {60, 1, 100, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_MILITARY, 0},
    {108, 1, 148, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_IMPERIAL, 0},
    {156, 1, 196, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_RATINGS, 0},
    {204, 1, 244, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_TRADE, 0},
    {252, 1, 292, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_POPULATION, 0},
    {300, 1, 340, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_HEALTH, 0},
    {348, 1, 388, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_EDUCATION, 0},
    {396, 1, 436, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_ENTERTAINMENT, 0},
    {444, 1, 484, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_RELIGION, 0},
    {492, 1, 532, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_FINANCIAL, 0},
    {540, 1, 580, 41, GB_IMMEDIATE, button_change_advisor, button_none, ADVISOR_CHIEF, 0},
    {588, 1, 624, 41, GB_IMMEDIATE, button_change_advisor, button_none, 0, 0},
};

static struct {
    void (*draw_background)(int *height);
    void (*draw_foreground)();
    void (*handle_mouse)(const mouse *m);
    int (*get_tooltip)();
} sub_windows[] = {
    {0, 0, 0, 0},
    {UI_Advisor_Labor_drawBackground, UI_Advisor_Labor_drawForeground, UI_Advisor_Labor_handleMouse, UI_Advisor_Labor_getTooltip},
    {UI_Advisor_Military_drawBackground, UI_Advisor_Military_drawForeground, UI_Advisor_Military_handleMouse, 0},
    {UI_Advisor_Imperial_drawBackground, UI_Advisor_Imperial_drawForeground, UI_Advisor_Imperial_handleMouse, UI_Advisor_Imperial_getTooltip},
    {UI_Advisor_Ratings_drawBackground, UI_Advisor_Ratings_drawForeground, UI_Advisor_Ratings_handleMouse, UI_Advisor_Ratings_getTooltip},
    {UI_Advisor_Trade_drawBackground, UI_Advisor_Trade_drawForeground, UI_Advisor_Trade_handleMouse, UI_Advisor_Trade_getTooltip},
    {UI_Advisor_Population_drawBackground, UI_Advisor_Population_drawForeground, UI_Advisor_Population_handleMouse, UI_Advisor_Population_getTooltip},
    {UI_Advisor_Health_drawBackground, 0, 0, 0},
    {UI_Advisor_Education_drawBackground, 0, 0, 0},
    {UI_Advisor_Entertainment_drawBackground, UI_Advisor_Entertainment_drawForeground, UI_Advisor_Entertainment_handleMouse, UI_Advisor_Entertainment_getTooltip},
    {UI_Advisor_Religion_drawBackground, 0, 0, 0},
    {UI_Advisor_Financial_drawBackground, UI_Advisor_Financial_drawForeground, UI_Advisor_Financial_handleMouse, UI_Advisor_Financial_getTooltip},
    {UI_Advisor_Chief_drawBackground, 0, 0, 0},
};

static const int advisorToMessageTextId[] = {
    0, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static advisor_type currentAdvisor = ADVISOR_NONE;

static int focusButtonId;
static int advisorHeight;

static void init()
{
    city_labor_allocate_workers();

    city_finance_estimate_taxes();
    city_finance_estimate_wages();
    city_finance_update_interest();
    city_finance_update_salary();
    city_finance_calculate_totals();

    city_culture_calculate_demands();
    city_culture_update_coverage();

    city_resource_calculate_food_stocks_and_supply_wheat();
    formation_calculate_figures();

    city_ratings_update_explanations();

    if (currentAdvisor == ADVISOR_ENTERTAINMENT) {
        UI_Advisor_Entertainment_init();
    }
}

void window_advisors_draw_dialog_background()
{
    image_draw_fullscreen_background(image_group(GROUP_ADVISOR_BACKGROUND));
    graphics_in_dialog();
    image_draw(image_group(GROUP_PANEL_WINDOWS) + 13, 0, 432);

    for (int i = 0; i < 13; i++) {
        int selected_offset = 0;
        if (currentAdvisor && i == currentAdvisor - 1) {
            selected_offset = 13;
        }
        image_draw(image_group(GROUP_ADVISOR_ICONS) + i + selected_offset, 48 * i + 12, 441);
    }
    graphics_reset_dialog();
}

static void draw_background()
{
    window_advisors_draw_dialog_background();
    sub_windows[currentAdvisor].draw_background(&advisorHeight);
}

static void draw_foreground()
{
    graphics_in_dialog();
    image_buttons_draw(0, 16 * (advisorHeight - 2), &helpButton, 1);
    graphics_reset_dialog();

    if (sub_windows[currentAdvisor].draw_foreground) {
        sub_windows[currentAdvisor].draw_foreground();
    }
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 440, advisorButtons, 13, &focusButtonId)) {
        return;
    }
    int button_id;
    image_buttons_handle_mouse(m_dialog, 0, 16 * (advisorHeight - 2), &helpButton, 1, &button_id);
    if (button_id) {
        focusButtonId = -1;
        return;
    }
    if (m->right.went_up) {
        window_city_show();
        return;
    }

    if (sub_windows[currentAdvisor].handle_mouse) {
        sub_windows[currentAdvisor].handle_mouse(m);
    }
}

static void button_change_advisor(int advisor, int param2)
{
    if (advisor) {
        currentAdvisor = advisor;
        setting_set_last_advisor(advisor);
        window_invalidate();
    } else {
        window_city_show();
    }
}

static void button_help(int param1, int param2)
{
    if (currentAdvisor > 0 && currentAdvisor < 13) {
        UI_MessageDialog_show(advisorToMessageTextId[currentAdvisor], 1);
    }
}

static void get_tooltip(struct TooltipContext *c)
{
    if (focusButtonId) {
        c->type = TooltipType_Button;
        if (focusButtonId == -1) {
            c->textId = 1; // help button
        } else {
            c->textId = 69 + focusButtonId;
        }
        return;
    }
    int textId = 0;
    if (sub_windows[currentAdvisor].get_tooltip) {
        textId = sub_windows[currentAdvisor].get_tooltip();
    }
    if (textId) {
        c->textId = textId;
        c->type = TooltipType_Button;
    }
}

advisor_type window_advisors_get_advisor()
{
    return currentAdvisor;
}

void window_advisors_show()
{
    window_type window = {
        Window_Advisors,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    init();
    window_show(&window);
}

void window_advisors_show_checked()
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == AVAILABLE) {
        currentAdvisor = setting_last_advisor();
        window_advisors_show();
    } else {
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
    }
}

void window_advisors_show_advisor(advisor_type advisor)
{
    tutorial_availability avail = tutorial_advisor_empire_availability();
    if (avail == NOT_AVAILABLE || avail == NOT_AVAILABLE_YET) {
        if (window_is(Window_MessageDialog)) {
            UI_MessageDialog_close();
            window_city_show();
        }
        city_warning_show(avail == NOT_AVAILABLE ? WARNING_NOT_AVAILABLE : WARNING_NOT_AVAILABLE_YET);
        return;
    }
    currentAdvisor = advisor;
    setting_set_last_advisor(advisor);
    window_advisors_show();
}
