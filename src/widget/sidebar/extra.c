#include "extra.h"

#include "city/finance.h"
#include "city/labor.h"
#include "city/military.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/request.h"
#include "city/resource.h"
#include "core/config.h"
#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "figure/formation_legion.h"
#include "game/resource.h"
#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/menu.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "translation/translation.h"
#include "window/advisor/imperial.h"
#include "window/empire.h"
#include "window/popup_dialog.h"

#include <stdlib.h>

#define EXTRA_INFO_LINE_SPACE 16
#define EXTRA_INFO_VERTICAL_PADDING 8
#define EXTRA_INFO_HEIGHT_GAME_SPEED 64
#define EXTRA_INFO_HEIGHT_UNEMPLOYMENT 48
#define EXTRA_INFO_HEIGHT_RATINGS 176
#define EXTRA_INFO_HEIGHT_REQUESTS_PANEL 48
#define EXTRA_INFO_HEIGHT_REQUESTS_MIN EXTRA_INFO_LINE_SPACE * 2 + EXTRA_INFO_HEIGHT_REQUESTS_PANEL

#define MAX_REQUESTS_TO_DISPLAY 5
#define REQUEST_MONTHS_LEFT_FOR_RED_WARNING 3

static void button_game_speed(int is_down, int param2);
static void button_handle_request(int index, int param2);

static arrow_button arrow_buttons_speed[] = {
    {11, 30, 17, 24, button_game_speed, 1, 0},
    {35, 30, 15, 24, button_game_speed, 0, 0},
};

static generic_button buttons_emperor_requests[] = {
    {2, 0, 158, 48, button_handle_request, button_none, 0, 0},
    {2, 48, 158, 48, button_handle_request, button_none, 1, 0},
    {2, 96, 158, 48, button_handle_request, button_none, 2, 0},
    {2, 144, 158, 48, button_handle_request, button_none, 3, 0},
    {2, 192, 158, 48, button_handle_request, button_none, 4, 0}
};

typedef struct {
    int value;
    int target;
} objective;

typedef struct {
    int index;
    int resource;
    int amount;
    int available;
    int time;
} request;

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
    int visible_requests;
    int active_requests;
    int request_buttons_y_offset;
    int focused_request_button_id;
    int selected_request_id;
    request requests[MAX_REQUESTS_TO_DISPLAY];
} data;

static int count_active_requests(void)
{
    int count = city_request_has_troop_request() + scenario_request_count_visible();
    return count > MAX_REQUESTS_TO_DISPLAY ? MAX_REQUESTS_TO_DISPLAY : count;
}

int sort_requests(const void *va, const void *vb)
{
    return ((request *)va)->time - ((request *)vb)->time;
}

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
    } else {
        return result;
    }
    if (available_height >= EXTRA_INFO_HEIGHT_REQUESTS_MIN) {
        if (info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS) {
            available_height -= EXTRA_INFO_HEIGHT_REQUESTS_MIN;
            result |= SIDEBAR_EXTRA_DISPLAY_REQUESTS;
        }
    }
    return result;
}

static int calculate_extra_info_height(int available_height)
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
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS) {
        height += EXTRA_INFO_HEIGHT_REQUESTS_MIN;
        int num_requests = count_active_requests();
        data.visible_requests = 1;
        while (data.visible_requests < num_requests) {
            if (height + EXTRA_INFO_HEIGHT_REQUESTS_PANEL > available_height) {
                break;
            }
            height += EXTRA_INFO_HEIGHT_REQUESTS_PANEL;
            data.visible_requests++;
        }
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
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS) {
        int new_requests = update_extra_info_value(count_active_requests(), &data.active_requests);
        int troop_requests = city_request_has_troop_request();
        if (troop_requests) {
            changed |= update_extra_info_value(RESOURCE_TROOPS, &data.requests[0].resource);
            changed |= update_extra_info_value(city_military_months_until_distant_battle(), &data.requests[0].time);
            changed |= update_extra_info_value(city_military_distant_battle_enemy_strength(), &data.requests[0].amount);
            changed |= update_extra_info_value(city_military_empire_service_legions(), &data.requests[0].available);
            data.requests[0].index = 0;
        }
        int other_requests = data.active_requests - troop_requests;
        int must_resort = 0;
        for (int i = 0; i < other_requests; i++) {
            request *slot = &data.requests[i + troop_requests];
            if (new_requests) {
                slot->index = i;
            }
            const scenario_request *r = scenario_request_get_visible(slot->index);
            changed |= update_extra_info_value(r->resource, &slot->resource);
            if (r->months_to_comply > slot->time) {
                must_resort = 1;
            }
            changed |= update_extra_info_value(r->months_to_comply, &slot->time);
            changed |= update_extra_info_value(r->amount, &slot->amount);
            if (r->resource == RESOURCE_DENARII) {
                changed |= update_extra_info_value(city_finance_treasury(), &slot->available);
            } else {
                changed |= update_extra_info_value(city_resource_count(r->resource), &slot->available);
            }
        }
        if (new_requests || must_resort) {
            qsort(data.requests + troop_requests, other_requests, sizeof(request), sort_requests);
            changed = 1;
        }
    }
    return changed;
}

static int draw_extra_info_objective(
    int x_offset, int y_offset, int text_group, int text_id, objective *obj, int cut_off_at_parenthesis)
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

static int get_text_offset_for_force_size(int force_size)
{
    if (force_size < 46) {
        return 0;
    } else if (force_size < 89) {
        return 1;
    } else {
        return 2;
    }
}

static int draw_request_buttons(int y_offset)
{
    int original_offset = y_offset;

    for (int i = 0; i < data.visible_requests; i++) {
        const request *r = &data.requests[i];
        y_offset += EXTRA_INFO_VERTICAL_PADDING;
        if (data.visible_requests < data.active_requests && i == data.visible_requests - 1) {
            text_draw_centered(translation_for(TR_SIDEBAR_EXTRA_REQUESTS_VIEW_ALL),
                data.x_offset, y_offset + EXTRA_INFO_VERTICAL_PADDING + 4, data.width, FONT_NORMAL_GREEN, 0);
            break;
        }
        if (r->resource == RESOURCE_TROOPS) {
            image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS,
                data.x_offset + 10, y_offset + 5);
            int force_text_offset = get_text_offset_for_force_size(r->amount);
            text_draw_ellipsized(translation_for(TR_SIDEBAR_EXTRA_REQUESTS_SMALL_FORCE + force_text_offset),
                data.x_offset + 32, y_offset, data.width - 34, FONT_NORMAL_WHITE, 0);
            y_offset += EXTRA_INFO_LINE_SPACE;
            lang_text_draw_amount(8, 4, r->time, data.x_offset + 26, y_offset + 4,
                r->time <= 3 ? FONT_NORMAL_RED : FONT_NORMAL_WHITE);
        } else {
            int width = data.x_offset + 10;
            width += text_draw_number(r->amount, 0, "",
                width, y_offset, FONT_NORMAL_WHITE);
            int resource_offset = r->resource + resource_image_offset(r->resource, RESOURCE_IMAGE_ICON);
            int image_id = image_group(GROUP_RESOURCE_ICONS) + resource_offset;
            const image *img = image_get(image_id);
            int image_y_offset = (EXTRA_INFO_LINE_SPACE - img->height) / 2;
            image_draw(image_id, width, y_offset + image_y_offset);
            if (r->resource != RESOURCE_DENARII) {
                width += img->width + 6;
                width += text_draw(string_from_ascii("("), width, y_offset, FONT_NORMAL_WHITE, 0) - 6;
                width += text_draw(translation_for(TR_SIDEBAR_EXTRA_REQUESTS_HAVE),
                    width, y_offset, FONT_NORMAL_WHITE, 0);
                text_draw_number(r->available, 0, ")", width, y_offset, FONT_NORMAL_WHITE);
            }
            y_offset += EXTRA_INFO_LINE_SPACE;
            lang_text_draw_amount(8, 4, r->time, data.x_offset + 4, y_offset + 4,
                r->time <= REQUEST_MONTHS_LEFT_FOR_RED_WARNING ? FONT_NORMAL_RED : FONT_NORMAL_WHITE);
        }
        y_offset += EXTRA_INFO_VERTICAL_PADDING * 3;
    }
    return y_offset - original_offset;
}

static void draw_extra_info_panel(void)
{
    int panel_blocks = data.height / 16;
    graphics_draw_vertical_line(data.x_offset, data.y_offset, data.y_offset + data.height, COLOR_WHITE);
    graphics_draw_vertical_line(data.x_offset + data.width - 1, data.y_offset,
        data.y_offset + data.height, COLOR_SIDEBAR);
    inner_panel_draw(data.x_offset + 1, data.y_offset, data.width / 16, panel_blocks);

    int y_current_line = data.y_offset + EXTRA_INFO_VERTICAL_PADDING;

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING;

        lang_text_draw(45, 2, data.x_offset + 10, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_LINE_SPACE + EXTRA_INFO_VERTICAL_PADDING;

        text_draw_percentage(data.game_speed, data.x_offset + 60, y_current_line - 2, FONT_NORMAL_GREEN);

        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 3;
    }

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_UNEMPLOYMENT) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING;

        lang_text_draw(68, 148, data.x_offset + 10, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_LINE_SPACE;

        int text_width = text_draw_percentage(data.unemployment_percentage,
            data.x_offset + 10, y_current_line, FONT_NORMAL_GREEN);
        text_draw_number(data.unemployment_amount, '(', ")",
            data.x_offset + 10 + text_width, y_current_line, FONT_NORMAL_GREEN);

        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 3;
    }

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_RATINGS) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING;

        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 1, &data.culture, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 2, &data.prosperity, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 3, &data.peace, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 53, 4, &data.favor, 0);
        y_current_line += draw_extra_info_objective(data.x_offset, y_current_line, 4, 6, &data.population, 1);

        y_current_line += EXTRA_INFO_VERTICAL_PADDING;
    }

    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS) {
        y_current_line += EXTRA_INFO_VERTICAL_PADDING + 4;
        lang_text_draw(44, 40, data.x_offset + 10, y_current_line, FONT_NORMAL_WHITE);
        y_current_line += EXTRA_INFO_VERTICAL_PADDING * 3 - 4;
        data.request_buttons_y_offset = y_current_line;

        if (data.active_requests == 0) {
            lang_text_draw_centered(44, 19, data.x_offset, y_current_line + EXTRA_INFO_VERTICAL_PADDING + 4,
                data.width, FONT_NORMAL_GREEN);
            y_current_line += EXTRA_INFO_HEIGHT_REQUESTS_PANEL;
        } else {
            y_current_line += draw_request_buttons(y_current_line);
        }
    }
}

int sidebar_extra_draw_background(int x_offset, int y_offset, int width, int available_height,
    int is_collapsed, sidebar_extra_display info_to_display)
{
    data.is_collapsed = is_collapsed;
    data.x_offset = x_offset;
    data.y_offset = y_offset;
    data.width = width;
    data.info_to_display = calculate_displayable_info(info_to_display, available_height);
    data.height = calculate_extra_info_height(available_height);

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
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) {
        arrow_buttons_draw(data.x_offset, data.y_offset, arrow_buttons_speed, 2);
    }
    if (data.info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS && data.active_requests) {
        for (int i = 0; i < data.visible_requests; i++) {
            button_border_draw(data.x_offset + 2, data.request_buttons_y_offset + i * EXTRA_INFO_HEIGHT_REQUESTS_PANEL,
                data.width - 4, EXTRA_INFO_HEIGHT_REQUESTS_PANEL, i == data.focused_request_button_id - 1);
        }
    }
}

void sidebar_extra_draw_foreground(void)
{
    draw_extra_info_buttons();
}

int sidebar_extra_handle_mouse(const mouse *m)
{
    if ((data.info_to_display & SIDEBAR_EXTRA_DISPLAY_GAME_SPEED) &&
        arrow_buttons_handle_mouse(m, data.x_offset, data.y_offset, arrow_buttons_speed, 2, 0)) {
        return 1;
    }
    if ((data.info_to_display & SIDEBAR_EXTRA_DISPLAY_REQUESTS) &&
        generic_buttons_handle_mouse(m, data.x_offset, data.request_buttons_y_offset,
            buttons_emperor_requests, data.visible_requests, &data.focused_request_button_id)) {
        return 1;
    }
    return 0;
}

static void button_game_speed(int is_down, int param2)
{
    if (is_down) {
        setting_decrease_game_speed();
    } else {
        setting_increase_game_speed();
    }
}

static void confirm_nothing(int accepted)
{}

static void confirm_send_troops(int accepted)
{
    if (accepted) {
        formation_legions_dispatch_to_distant_battle();
        window_empire_show();
    }
}

static void confirm_send_goods(int accepted)
{
    if (accepted) {
        scenario_request_dispatch(data.selected_request_id);
    }
}

static void button_handle_request(int index, int param2)
{
    if (data.active_requests > data.visible_requests && index == data.visible_requests - 1) {
        window_advisors_show_advisor(ADVISOR_IMPERIAL);
        return;
    }
    int request_index = data.requests[index].index;
    if (city_request_has_troop_request() && index != 0) {
        request_index++;
    }
    int status = city_request_get_status(request_index);
    if (status) {
        city_military_clear_empire_service_legions();
        switch (status) {
        case CITY_REQUEST_STATUS_NO_LEGIONS_AVAILABLE:
            window_popup_dialog_show(POPUP_DIALOG_NO_LEGIONS_AVAILABLE, confirm_nothing, 0);
            break;
        case CITY_REQUEST_STATUS_NO_LEGIONS_SELECTED:
            window_popup_dialog_show(POPUP_DIALOG_NO_LEGIONS_SELECTED, confirm_nothing, 0);
            break;
        case CITY_REQUEST_STATUS_CONFIRM_SEND_LEGIONS:
            window_popup_dialog_show(POPUP_DIALOG_SEND_TROOPS, confirm_send_troops, 2);
            break;
        case CITY_REQUEST_STATUS_NOT_ENOUGH_RESOURCES:
            window_popup_dialog_show(POPUP_DIALOG_NOT_ENOUGH_GOODS, confirm_nothing, 0);
            break;
        default:
            data.selected_request_id = status - 1;
            window_popup_dialog_show(POPUP_DIALOG_SEND_GOODS, confirm_send_goods, 2);
            break;
        }
    }
}
