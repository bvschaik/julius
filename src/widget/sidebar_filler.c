#include "sidebar_filler.h"

#include "building/menu.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/config.h"
#include "core/direction.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/orientation.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/arrow_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/menu.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "map/orientation.h"
#include "scenario/property.h"
#include "scenario/criteria.h"
#include "sound/effect.h"
#include "widget/city.h"
#include "widget/minimap.h"
#include "widget/sidebar_constants.h"
#include "window/advisors.h"
#include "window/build_menu.h"
#include "window/city.h"
#include "window/empire.h"
#include "window/message_dialog.h"
#include "window/message_list.h"
#include "window/mission_briefing.h"
#include "window/overlay_menu.h"

#define EXTRA_INFO_LINE_SPACE 32
#define EXTRA_INFO_HEIGHT_GAME_SPEED 64
#define EXTRA_INFO_HEIGHT_UNEMPLOYMENT 112
#define EXTRA_INFO_HEIGHT_RATINGS 272
#define EXTRA_INFO_TOP_PADDING 10;
#define EXTRA_INFO_SPEED_PADDING 26;
#define EXTRA_INFO_UNEMPLOYMENT_TOP_PADDING 20;
#define EXTRA_INFO_UNEMPLOYMENT_BOTTOM_PADDING 22;

static void button_game_speed(int is_down, int param2);

static arrow_button arrow_buttons_speed[] = {
    {11, 30, 17, 24, button_game_speed, 1, 0},
    {35, 30, 15, 24, button_game_speed, 0, 0},
};

typedef struct {
    int value;
    int target;
} objective;

static struct {
    int height;
    int game_speed;
    int unemployment_percentage;
    int unemployment_amount;
    objective culture;
    objective prosperity;
    objective peace;
    objective favor;
    objective population;
} data;

int sidebar_filler_calculate_extra_info_height(int is_collapsed)
{
    if (is_collapsed || !config_get(CONFIG_UI_SIDEBAR_INFO)) {
        data.height = 0;
    } else {
        int available_height = screen_height() - FILLER_Y_OFFSET;
        if (available_height >= EXTRA_INFO_HEIGHT_RATINGS) {
            data.height = EXTRA_INFO_HEIGHT_RATINGS;
        } else if (available_height >= EXTRA_INFO_HEIGHT_UNEMPLOYMENT) {
            data.height = EXTRA_INFO_HEIGHT_UNEMPLOYMENT;
        } else if (available_height >= EXTRA_INFO_HEIGHT_GAME_SPEED) {
            data.height = EXTRA_INFO_HEIGHT_GAME_SPEED;
        } else {
            data.height = 0;
        }
    }
    return data.height;
}

static void set_extra_info_objectives(void)
{
    data.culture.target = 0;
    data.prosperity.target = 0;
    data.peace.target = 0;
    data.favor.target = 0;
    data.population.target = 0;

    if (scenario_is_open_play()) {
        return;
    }
    if (scenario_criteria_culture_enabled()) {
        data.culture.target = scenario_criteria_culture();
    }
    if (scenario_criteria_prosperity_enabled()) {
        data.prosperity.target = scenario_criteria_prosperity();
    }
    if (scenario_criteria_peace_enabled()) {
        data.peace.target = scenario_criteria_peace();
    }
    if (scenario_criteria_favor_enabled()) {
        data.favor.target = scenario_criteria_favor();
    }
    if (scenario_criteria_population_enabled()) {
        data.population.target = scenario_criteria_population();
    }
}

static int update_extra_info_value(int value, int *field)
{
    if (value == *field) {
        return 0;
    } else {
        *field = value;
        return 1;
    }
}

int sidebar_filler_update_extra_info(int height, int is_background)
{
    int changed = 0;
    if (height >= EXTRA_INFO_HEIGHT_GAME_SPEED) {
        changed |= update_extra_info_value(setting_game_speed(), &data.game_speed);
    }
    if (height >= EXTRA_INFO_HEIGHT_UNEMPLOYMENT) {
        changed |= update_extra_info_value(city_labor_unemployment_percentage(), &data.unemployment_percentage);
        changed |= update_extra_info_value(
            city_labor_workers_unemployed() - city_labor_workers_needed(),
            &data.unemployment_amount
        );
    }
    if (height >= EXTRA_INFO_HEIGHT_RATINGS) {
        if (is_background) {
            set_extra_info_objectives();
        }
        changed |= update_extra_info_value(city_rating_culture(), &data.culture.value);
        changed |= update_extra_info_value(city_rating_prosperity(), &data.prosperity.value);
        changed |= update_extra_info_value(city_rating_peace(), &data.peace.value);
        changed |= update_extra_info_value(city_rating_favor(), &data.favor.value);
        changed |= update_extra_info_value(city_population(), &data.population.value);
    }
    return changed;
}

static void draw_extra_info_objective(int x_offset, int y_offset, int text_group, int text_id, objective *obj, int cut_off_at_parenthesis)
{
    if (cut_off_at_parenthesis) {
        // Exception for Chinese: the string for "population" includes the hotkey " (6)"
        // To fix that: cut the string off at the '('
        uint8_t tmp[100];
        string_copy(lang_get_string(text_group, text_id), tmp, 100);
        for (int i = 0; i < 100 && tmp[i]; i++) {
            if (tmp[i] == '(') {
                tmp[i] = 0;
                break;
            }
        }
        text_draw(tmp, x_offset + 11, y_offset, FONT_NORMAL_WHITE, 0);
    } else {
        lang_text_draw(text_group, text_id, x_offset + 11, y_offset, FONT_NORMAL_WHITE);
    }
    font_t font = obj->value >= obj->target ? FONT_NORMAL_GREEN : FONT_NORMAL_RED;
    int width = text_draw_number(obj->value, '@', "", x_offset + 11, y_offset + 16, font);
    text_draw_number(obj->target, '(', ")", x_offset + 11 + width, y_offset + 16, font);
}

void sidebar_filler_draw_extra_info_panel(int x_offset, int extra_info_height)
{
    // Borders of the extra_info panel - extends to bottom of screen
    int panel_blocks = extra_info_height / 16;
    graphics_draw_vertical_line(x_offset, FILLER_Y_OFFSET, FILLER_Y_OFFSET + extra_info_height, COLOR_WHITE);
    graphics_draw_vertical_line(x_offset + SIDEBAR_EXPANDED_WIDTH - 1, FILLER_Y_OFFSET, FILLER_Y_OFFSET + extra_info_height, COLOR_SIDEBAR);
    inner_panel_draw(x_offset + 1, FILLER_Y_OFFSET, SIDEBAR_EXPANDED_WIDTH / 16, panel_blocks);

    // This var is basically a cursor for drawing stuff on the sidebar; starting from FILLER_Y_OFFSET, we work our way down, adding info to the sidebar_filler
    int y_current_line = FILLER_Y_OFFSET + EXTRA_INFO_TOP_PADDING;

    // Game speed modifier in extra_info panel
    lang_text_draw(45, 2, x_offset + 11, y_current_line, FONT_NORMAL_WHITE);
    y_current_line += EXTRA_INFO_SPEED_PADDING;

    text_draw_percentage(data.game_speed, x_offset + 60, y_current_line, FONT_NORMAL_GREEN);
    arrow_buttons_draw(x_offset, FILLER_Y_OFFSET, arrow_buttons_speed, 2);
    y_current_line += EXTRA_INFO_LINE_SPACE; 
    
    // Unemployment info in extra_info panel
    if (extra_info_height >= EXTRA_INFO_HEIGHT_UNEMPLOYMENT) {
        lang_text_draw(68, 148, x_offset + 11, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_UNEMPLOYMENT_TOP_PADDING;

        int width = text_draw_percentage(data.unemployment_percentage, x_offset + 11, y_current_line, FONT_NORMAL_GREEN);
        text_draw_number(data.unemployment_amount, '(', ")", x_offset + 11 + width, y_current_line, FONT_NORMAL_GREEN);
        y_current_line += EXTRA_INFO_UNEMPLOYMENT_BOTTOM_PADDING;
    }

    // Objective value info on extra_info panel (culture, prosperity, peace, etc)
    if (extra_info_height >= EXTRA_INFO_HEIGHT_RATINGS) {
        draw_extra_info_objective(x_offset, y_current_line, 53, 1, &data.culture, 0);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        draw_extra_info_objective(x_offset, y_current_line, 53, 2, &data.prosperity, 0);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        draw_extra_info_objective(x_offset, y_current_line, 53, 3, &data.peace, 0);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        draw_extra_info_objective(x_offset, y_current_line, 53, 4, &data.favor, 0);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        draw_extra_info_objective(x_offset, y_current_line, 4, 6, &data.population, 1);
        y_current_line += EXTRA_INFO_LINE_SPACE;
    }
}

void sidebar_filler_draw_extra_info_buttons(int x_offset, int is_collapsed)
{
    int extra_info_height = data.height;
    if (!extra_info_height) {
        return;
    }

    graphics_set_clip_rectangle(x_offset, TOP_MENU_HEIGHT,
            screen_width() - x_offset,
            screen_height() - TOP_MENU_HEIGHT);

    if (sidebar_filler_update_extra_info(extra_info_height, 0)) {
        sidebar_filler_draw_extra_info_panel(x_offset, extra_info_height);
    } else {
        arrow_buttons_draw(x_offset, FILLER_Y_OFFSET, arrow_buttons_speed, 2);
    }

    graphics_reset_clip_rectangle();
}

int sidebar_filler_extra_info_height_game_speed_check(void)
{
  return data.height >= EXTRA_INFO_HEIGHT_GAME_SPEED;
}

static void button_game_speed(int is_down, int param2)
{
    if (is_down) {
        setting_decrease_game_speed();
    } else {
        setting_increase_game_speed();
    }
}

arrow_button *sidebar_filler_get_arrow_buttons_speed(void)
{
  return arrow_buttons_speed;
}

void sidebar_filler_draw_sidebar_filler(int x_offset, int y_offset, int is_collapsed)
{
    // relief images below panel
    int image_base = image_group(GROUP_SIDE_PANEL);
    int y_max = screen_height();
    while (y_offset < y_max) {
        if (y_max - y_offset <= 120) {
            image_draw(image_base + 2 + is_collapsed, x_offset, y_offset);
            y_offset += 120;
        } else {
            image_draw(image_base + 4 + is_collapsed, x_offset, y_offset);
            y_offset += 285;
        }
    }
}
