#include "city.h"

#include "building/construction.h"
#include "city/message.h"
#include "city/victory.h"
#include "city/view.h"
#include "game/state.h"
#include "game/time.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"
#include "widget/city.h"
#include "widget/sidebar.h"
#include "widget/top_menu.h"

static int selected_legion_formation_id;

static void draw_background(void)
{
    widget_sidebar_draw_background();
    widget_top_menu_draw(1);
}

static int center_in_city(int element_width_pixels)
{
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    int margin = (width - element_width_pixels) / 2;
    return x + margin;
}

static void draw_paused_and_time_left(void)
{
    if (scenario_criteria_time_limit_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    } else if (scenario_criteria_survival_enabled() && !city_victory_has_won()) {
        int years;
        if (scenario_criteria_max_year() <= game_time_year() + 1) {
            years = 0;
        } else {
            years = scenario_criteria_max_year() - game_time_year() - 1;
        }
        int total_months = 12 - game_time_month() + 12 * years;
        label_draw(1, 25, 15, 1);
        int width = lang_text_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
        text_draw_number(total_months, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
    }
    if (game_state_is_paused()) {
        int x_offset = center_in_city(448);
        outer_panel_draw(x_offset, 40, 28, 3);
        lang_text_draw_centered(13, 2, x_offset, 58, 448, FONT_NORMAL_BLACK);
    }
}

static void draw_foreground(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_draw_foreground();
    if (window_is(WINDOW_CITY) || window_is(WINDOW_CITY_MILITARY)) {
        draw_paused_and_time_left();
    }
    widget_city_draw_construction_cost_and_size();
    if (window_is(WINDOW_CITY)) {
        city_message_process_queue();
    }
}

static void draw_foreground_military(void)
{
    widget_top_menu_draw(0);
    window_city_draw();
    widget_sidebar_draw_foreground_military();
    draw_paused_and_time_left();
}

static void handle_mouse(const mouse *m)
{
    if (!building_construction_in_progress()) {
        if (widget_top_menu_handle_mouse(m)) {
            return;
        }
        if (widget_sidebar_handle_mouse(m)) {
            return;
        }
    }
    widget_city_handle_mouse(m);
}

static void handle_mouse_military(const mouse *m)
{
    widget_city_handle_mouse_military(m, selected_legion_formation_id);
}

static void get_tooltip(tooltip_context *c)
{
    int text_id = widget_top_menu_get_tooltip_text(c);
    if (!text_id) {
        text_id = widget_sidebar_get_tooltip_text();
    }
    if (text_id) {
        c->type = TOOLTIP_BUTTON;
        c->text_id = text_id;
        return;
    }
    widget_city_get_tooltip(c);
}

void window_city_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_city_draw_panels(void)
{
    draw_background();
}

void window_city_draw(void)
{
    widget_city_draw();
}

void window_city_show(void)
{
    window_type window = {
        WINDOW_CITY,
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip
    };
    window_show(&window);
}

void window_city_military_show(int legion_formation_id)
{
    selected_legion_formation_id = legion_formation_id;
    window_type window = {
        WINDOW_CITY_MILITARY,
        draw_background,
        draw_foreground_military,
        handle_mouse_military,
        get_tooltip
    };
    window_show(&window);
}
