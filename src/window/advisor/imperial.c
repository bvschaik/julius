#include "imperial.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/military.h"
#include "city/ratings.h"
#include "city/resource.h"
#include "empire/city.h"
#include "figure/formation_legion.h"
#include "graphics/generic_button.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "window/donate_to_city.h"
#include "window/empire.h"
#include "window/gift_to_emperor.h"
#include "window/popup_dialog.h"
#include "window/set_salary.h"

#define ADVISOR_HEIGHT 27

enum {
    STATUS_NOT_ENOUGH_RESOURCES = -1,
    STATUS_CONFIRM_SEND_LEGIONS = -2,
    STATUS_NO_LEGIONS_SELECTED = -3,
    STATUS_NO_LEGIONS_AVAILABLE = -4,
};

static void button_donate_to_city(int param1, int param2);
static void button_set_salary(int param1, int param2);
static void button_gift_to_emperor(int param1, int param2);
static void button_request(int index, int param2);

static generic_button imperial_buttons[] = {
    {320, 367, 250, 20, button_donate_to_city, button_none, 0, 0},
    {70, 393, 500, 20, button_set_salary, button_none, 0, 0},
    {320, 341, 250, 20, button_gift_to_emperor, button_none, 0, 0},
    {38, 96, 560, 40, button_request, button_none, 0, 0},
    {38, 138, 560, 40, button_request, button_none, 1, 0},
    {38, 180, 560, 40, button_request, button_none, 2, 0},
    {38, 222, 560, 40, button_request, button_none, 3, 0},
    {38, 264, 560, 40, button_request, button_none, 4, 0},
};

static int focus_button_id;
static int selected_request_id;

static void draw_request(int index, const scenario_request *request)
{
    if (index >= 5) {
        return;
    }

    button_border_draw(38, 96 + 42 * index, 560, 40, 0);
    text_draw_number(request->amount, '@', " ", 40, 102 + 42 * index, FONT_NORMAL_WHITE);
    int resource_offset = request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource_offset, 110, 100 + 42 * index);
    lang_text_draw(23, request->resource, 150, 102 + 42 * index, FONT_NORMAL_WHITE);

    int width = lang_text_draw_amount(8, 4, request->months_to_comply, 310, 102 + 42 * index, FONT_NORMAL_WHITE);
    lang_text_draw(12, 2, 310 + width, 102 + 42 * index, FONT_NORMAL_WHITE);

    if (request->resource == RESOURCE_DENARII) {
        // request for money
        int treasury = city_finance_treasury();
        width = text_draw_number(treasury, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE);
        width += lang_text_draw(52, 44, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        if (treasury < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    } else {
        // normal goods request
        int amount_stored = city_resource_count(request->resource);
        width = text_draw_number(amount_stored, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE);
        width += lang_text_draw(52, 43, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        if (amount_stored < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    }
}

static int draw_background(void)
{
    city_emperor_calculate_gift_costs();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 2, 10, 10);

    text_draw(scenario_player_name(), 60, 12, FONT_LARGE_BLACK, 0);

    int width = lang_text_draw(52, 0, 60, 44, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_favor(), '@', " ", 60 + width, 44, FONT_NORMAL_BLACK);

    lang_text_draw_multiline(52, city_rating_favor() / 5 + 22, 60, 60, 544, FONT_NORMAL_BLACK);

    inner_panel_draw(32, 90, 36, 14);

    int num_requests = 0;
    if (city_military_months_until_distant_battle() > 0 && !city_military_distant_battle_roman_army_is_traveling_forth()) {
        // can send to distant battle
        button_border_draw(38, 96, 560, 40, 0);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS, 50, 106);
        width = lang_text_draw(52, 72, 80, 102, FONT_NORMAL_WHITE);
        lang_text_draw(21, empire_city_get(city_military_distant_battle_city())->name_id, 80 + width, 102, FONT_NORMAL_WHITE);
        int strength_text_id;
        int enemy_strength = city_military_distant_battle_enemy_strength();
        if (enemy_strength < 46) {
            strength_text_id = 73;
        } else if (enemy_strength < 89) {
            strength_text_id = 74;
        } else {
            strength_text_id = 75;
        }
        width = lang_text_draw(52, strength_text_id, 80, 120, FONT_NORMAL_WHITE);
        lang_text_draw_amount(8, 4, city_military_months_until_distant_battle(), 80 + width, 120, FONT_NORMAL_WHITE);
        num_requests = 1;
    }
    num_requests = scenario_request_foreach_visible(num_requests, draw_request);
    if (!num_requests) {
        lang_text_draw_multiline(52, 21, 64, 160, 512, FONT_NORMAL_WHITE);
    }

    return ADVISOR_HEIGHT;
}

static int get_request_status(int index)
{
    int num_requests = 0;
    if (city_military_months_until_distant_battle() > 0 && !city_military_distant_battle_roman_army_is_traveling_forth()) {
        num_requests = 1;
        if (index == 0) {
            if (city_military_total_legions() <= 0) {
                return STATUS_NO_LEGIONS_AVAILABLE;
            } else if (city_military_empire_service_legions() <= 0) {
                return STATUS_NO_LEGIONS_SELECTED;
            } else {
                return STATUS_CONFIRM_SEND_LEGIONS;
            }
        }
    }
    const scenario_request *request = scenario_request_get_visible(index - num_requests);
    if (request) {
        if (request->resource == RESOURCE_DENARII) {
            if (city_finance_treasury() <= request->amount) {
                return STATUS_NOT_ENOUGH_RESOURCES;
            }
        } else {
            if (city_resource_count(request->resource) < request->amount) {
                return STATUS_NOT_ENOUGH_RESOURCES;
            }
        }
        return request->id + 1;
    }
    return 0;
}

static void draw_foreground(void)
{
    inner_panel_draw(64, 324, 32, 6);

    lang_text_draw(32, city_emperor_rank(), 72, 338, FONT_LARGE_BROWN);

    int width = lang_text_draw(52, 1, 72, 372, FONT_NORMAL_WHITE);
    text_draw_money(city_emperor_personal_savings(), 80 + width, 372, FONT_NORMAL_WHITE);

    button_border_draw(320, 367, 250, 20, focus_button_id == 1);
    lang_text_draw_centered(52, 2, 320, 372, 250, FONT_NORMAL_WHITE);

    button_border_draw(70, 393, 500, 20, focus_button_id == 2);
    width = lang_text_draw(52, city_emperor_salary_rank() + 4, 120, 398, FONT_NORMAL_WHITE);
    width += text_draw_number(city_emperor_salary_amount(), '@', " ", 120 + width, 398, FONT_NORMAL_WHITE);
    lang_text_draw(52, 3, 120 + width, 398, FONT_NORMAL_WHITE);

    button_border_draw(320, 341, 250, 20, focus_button_id == 3);
    lang_text_draw_centered(52, 49, 320, 346, 250, FONT_NORMAL_WHITE);

    // Request buttons
    if (get_request_status(0)) {
        button_border_draw(38, 96, 560, 40, focus_button_id == 4);
    }
    if (get_request_status(1)) {
        button_border_draw(38, 138, 560, 40, focus_button_id == 5);
    }
    if (get_request_status(2)) {
        button_border_draw(38, 180, 560, 40, focus_button_id == 6);
    }
    if (get_request_status(3)) {
        button_border_draw(38, 222, 560, 40, focus_button_id == 7);
    }
    if (get_request_status(4)) {
        button_border_draw(38, 264, 560, 40, focus_button_id == 8);
    }
}

static int handle_mouse(const mouse *m)
{
    return generic_buttons_handle_mouse(m, 0, 0, imperial_buttons, 8, &focus_button_id);
}

static void button_donate_to_city(int param1, int param2)
{
    window_donate_to_city_show();
}

static void button_set_salary(int param1, int param2)
{
    window_set_salary_show();
}

static void button_gift_to_emperor(int param1, int param2)
{
    window_gift_to_emperor_show();
}

static void confirm_nothing(int accepted)
{
}

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
        scenario_request_dispatch(selected_request_id);
    }
}

static void button_request(int index, int param2)
{
    int status = get_request_status(index);
    if (status) {
        city_military_clear_empire_service_legions();
        switch (status) {
            case STATUS_NO_LEGIONS_AVAILABLE:
                window_popup_dialog_show(POPUP_DIALOG_NO_LEGIONS_AVAILABLE, confirm_nothing, 0);
                break;
            case STATUS_NO_LEGIONS_SELECTED:
                window_popup_dialog_show(POPUP_DIALOG_NO_LEGIONS_SELECTED, confirm_nothing, 0);
                break;
            case STATUS_CONFIRM_SEND_LEGIONS:
                window_popup_dialog_show(POPUP_DIALOG_SEND_TROOPS, confirm_send_troops, 2);
                break;
            case STATUS_NOT_ENOUGH_RESOURCES:
                window_popup_dialog_show(POPUP_DIALOG_NOT_ENOUGH_GOODS, confirm_nothing, 0);
                break;
            default:
                selected_request_id = status - 1;
                window_popup_dialog_show(POPUP_DIALOG_SEND_GOODS, confirm_send_goods, 2);
                break;
        }
    }
}

static int get_tooltip_text(void)
{
    if (focus_button_id && focus_button_id <= 2) {
        return 93 + focus_button_id;
    } else if (focus_button_id == 3) {
        return 131;
    } else {
        return 0;
    }
}

const advisor_window_type *window_advisor_imperial(void)
{
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
