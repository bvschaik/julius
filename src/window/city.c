#include "city.h"

#include "city/message.h"
#include "city/view.h"
#include "game/state.h"
#include "game/time.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"

#include "UI/CityBuildings.h"
#include "UI/Sidebar.h"
#include "UI/TopMenu.h"

#include "Data/CityView.h"

static void draw_background()
{
    UI_Sidebar_drawBackground();
    UI_TopMenu_drawBackground();
}

static void draw_paused_and_time_left()
{
    if (scenario_criteria_time_limit_enabled()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int totalMonths = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    } else if (scenario_criteria_survival_enabled()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int totalMonths = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    }
    if (game_state_is_paused()) {
        int width = Data_CityView.widthInPixels;
        outer_panel_draw((width - 448) / 2, 40, 28, 3);
        lang_text_draw_centered(13, 2, (width - 448) / 2, 58, 448, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground()
{
    UI_TopMenu_drawBackgroundIfNecessary();
    window_city_draw();
    UI_Sidebar_drawForeground();
    draw_paused_and_time_left();
    UI_CityBuildings_drawBuildingCost();
    if (window_is(Window_City)) {
        city_message_process_queue();
    }
}

static void draw_foreground_military()
{
    UI_TopMenu_drawBackgroundIfNecessary();
    window_city_draw();
    UI_Sidebar_drawMinimap(0);
    draw_paused_and_time_left();
}

static void handle_mouse(const mouse *m)
{
    if (UI_TopMenu_handleMouseWidget(m)) {
        return;
    }
    if (UI_Sidebar_handleMouse(m)) {
        return;
    }
    UI_CityBuildings_handleMouse(m);
}

static void handle_mouse_military(const mouse *m)
{
    UI_CityBuildings_handleMouseMilitary(m);
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = UI_TopMenu_getTooltipText(c);
    if (!text_id) {
        text_id = UI_Sidebar_getTooltipText();
    }
    if (text_id) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        return;
    }
    UI_CityBuildings_getTooltip(c);
}

void window_city_draw_all()
{
    draw_background();
    draw_foreground();
}

void window_city_draw_panels()
{
    draw_background();
}

void window_city_draw()
{
    int x, y;
    city_view_get_camera(&x, &y);
    UI_CityBuildings_drawForeground(x, y);
}

void window_city_show()
{
    window_type window = {
        Window_City,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    window_show(&window);
}

void window_city_military_show()
{
    window_type window = {
        Window_CityMilitary,
        draw_background,
        draw_foreground_military,
        handle_mouse_military,
        get_tooltip
    };
    window_show(&window);
}
