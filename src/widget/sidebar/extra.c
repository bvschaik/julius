#include "extra.h"

#include "city/labor.h"
#include "city/population.h"
#include "city/ratings.h"
#include "core/config.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/menu.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/criteria.h"
#include "scenario/property.h"

#define EXTRA_INFO_LINE_SPACE 16
#define EXTRA_INFO_HEIGHT_GAME_SPEED 64
#define EXTRA_INFO_HEIGHT_UNEMPLOYMENT 48
#define EXTRA_INFO_HEIGHT_RATINGS 176
#define EXTRA_INFO_VERTICAL_PADDING 8

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
    int x_offset;
    int y_offset;
    int width;
    int height;
    int is_collapsed;
    sidebar_extra_display info_to_display;
    int game_speed;
    int unemployment_percentage;
    int unemployment_amount;
    objective culture;
    objective prosperity;
    objective peace;
    objective favor;
    objective population;
} data;

static sidebar_extra_display calculate_displayable_info(sidebar_extra_display info_to_display, int available_height)
{
    if (data.is_collapsed || !config_get(CONFIG_UI_SIDEBAR_INFO) || info_to_display == SIDEBAR_EXTRA_DISPLAY_NONE) {
        return SIDEBAR_EXTRA_DISPLAY_NONE;
    }
    sidebar_extra_display result = SIDEBAR_EXTRA_DISPLAY_NONE;
    if (available_height >= EXTRA_INFO_HEIGHT_GAME_SPEED) {
        if (info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
            available_height -= EXTRA_INFO_HEIGHT_GAME_SPEED;
            result |= SIDEBAR_EXTRA_DISPLAY_GAME_SPEED;
        }
    } else {
        return result;
    }
    if (available_height >= EXTRA_INFO_HEIGHT_UNEMPLOYMENT) {
        if (info_to_display & SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT) {
            available_height -= EXTRA_INFO_HEIGHT_UNEMPLOYMENT;
            result |= SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT;
        }
    } else {
        return result;
    }
    if (available_height >= EXTRA_INFO_HEIGHT_RATINGS) {
        if (info_to_display & SIDEBAR_EXTRA_DISPLAY_RATINGS) {
            available_height -= EXTRA_INFO_HEIGHT_RATINGS;
            result |= SIDEBAR_EXTRA_DISPLAY_RATINGS;
        }
    }
    return result;
}

static int calculate_extra_info_height(void)
{
    if (data.info_to_display == SIDEBAR_EXTRA_DISPLAY_NONE) {
        return 0;
    }
    int height = 0;
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        height += EXTRA_INFO_HEIGHT_GAME_SPEED;
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT) {
        height += EXTRA_INFO_HEIGHT_UNEMPLOYMENT;
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_RATINGS) {
        height += EXTRA_INFO_HEIGHT_RATINGS;
    }
    return height;
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

static int update_extra_info(int is_background)
{
    int changed = 0;
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        changed |= update_extra_info_value(setting_game_speed(), &data.game_speed);
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT) {
        changed |= update_extra_info_value(city_labor_unemployment_percentage(), &data.unemployment_percentage);
        changed |= update_extra_info_value(
                       city_labor_workers_unemployed() - city_labor_workers_needed(),
                       &data.unemployment_amount
                   );
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_RATINGS) {
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

static int draw_extra_info_objective(int x_offset, int y_offset, int text_group, int text_id, objective *obj, int cut_off_at_parenthesis)
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
    int width = text_draw_number(obj->value, '@', "", x_offset + 11, y_offset + EXTRA_INFO_LINE_SPACE, font);
    text_draw_number(obj->target, '(', ")", x_offset + 11 + width, y_offset + EXTRA_INFO_LINE_SPACE, font);
    return EXTRA_INFO_LINE_SPACE * 2;
}

static void draw_extra_info_panel(void)
{
    int panel_blocks = data.height / 16;
    graphics_draw_vertical_line(data.x_offset, data.y_offset, data.y_offset + data.height, COLOR_WHITE);
    graphics_draw_vertical_line(data.x_offset + data.width - 1, data.y_offset, data.y_offset + data.height, COLOR_SIDEBAR);
    inner_panel_draw(data.x_offset + 1, data.y_offset, data.width / 16, panel_blocks);

    int y_current_line = data.y_offset;

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 2;

        lang_text_draw(45, 2, data.x_offset + 11, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_LINE_SPACE + EXTRA_INFO_VERTICAL_PADDING;

        text_draw_percentage(data.game_speed, data.x_offset + 60, y_current_line, FONT_NORMAL_GREEN);
        arrow_buttons_draw(data.x_offset, data.y_offset, arrow_buttons_speed, 2);

        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 3;
    }

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING;

        lang_text_draw(68, 148, data.x_offset + 11, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        int text_width = text_draw_percentage(data.unemployment_percentage, data.x_offset + 11, y_current_line, FONT_NORMAL_GREEN);
        text_draw_number(data.unemployment_amount, '(', ")", data.x_offset + 11 + text_width, y_current_line, FONT_NORMAL_GREEN);

        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 3;
    }

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_RATINGS) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING;

        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 1, &data.culture, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 2, &data.prosperity, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 3, &data.peace, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 4, &data.favor, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 4, 6, &data.population, 1);
    }
}

int sidebar_extra_draw_background(int x_offset, int y_offset, int width, int available_height, int is_collapsed, sidebar_extra_display info_to_display)
{
    data.is_collapsed = is_collapsed;
    data.x_offset = x_offset;
    data.y_offset = y_offset;
    data.width = width;
    data.info_to_display = calculate_displayable_info(info_to_display, available_height);
    data.height = calculate_extra_info_height();

    if (data.info_to_display != SIDEBAR_EXTRA_DISPLAY_NONE) {
        update_extra_info(1);
        draw_extra_info_panel();
    }
    return data.height;
}

static void draw_extra_info_buttons(void)
{
    if (update_extra_info(0)) {
        // Updates displayed speed % after clicking the arrows
        draw_extra_info_panel();
    } else if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        arrow_buttons_draw(data.x_offset, data.y_offset, arrow_buttons_speed, 2);
    }
}

void sidebar_extra_draw_foreground(void)
{
    draw_extra_info_buttons();
}

int sidebar_extra_handle_mouse(const mouse *m)
{
    if (!(data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED)) {
        return 0;
    }
    return arrow_buttons_handle_mouse(m, data.x_offset, data.y_offset, arrow_buttons_speed, 2, 0);
}

static void button_game_speed(int is_down, int param2)
{
    if (is_down) {
        setting_decrease_game_speed();
    } else {
        setting_increase_game_speed();
    }
}
