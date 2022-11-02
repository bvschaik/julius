#include "imperial.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/military.h"
#include "city/ratings.h"
#include "city/request.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/lang.h"
#include "core/string.h"
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
#include "translation/translation.h"
#include "window/donate_to_city.h"
#include "window/empire.h"
#include "window/gift_to_emperor.h"
#include "window/popup_dialog.h"
#include "window/resource_settings.h"
#include "window/set_salary.h"

#define ADVISOR_HEIGHT 27
#define RESOURCE_INFO_MAX_TEXT 200

static void button_donate_to_city(int param1, int param2);
static void button_set_salary(int param1, int param2);
static void button_gift_to_emperor(int param1, int param2);
static void button_request(int index, int param2);
static void button_request_resource(int index, int param2);

static generic_button imperial_buttons[] = {
    {320, 367, 250, 20, button_donate_to_city, button_none, 0, 0},
    {70, 393, 500, 20, button_set_salary, button_none, 0, 0},
    {320, 341, 250, 20, button_gift_to_emperor, button_none, 0, 0},
    {38, 96, 560, 40, button_request, button_request_resource, 0, 0},
    {38, 138, 560, 40, button_request, button_request_resource, 1, 0},
    {38, 180, 560, 40, button_request, button_request_resource, 2, 0},
    {38, 222, 560, 40, button_request, button_request_resource, 3, 0},
    {38, 264, 560, 40, button_request, button_request_resource, 4, 0},
};

static int focus_button_id;
static int selected_request_id;
static int selected_resource;
static uint8_t tooltip_resource_info[RESOURCE_INFO_MAX_TEXT];

static void draw_request(int index, const scenario_request *request)
{
    if (index >= 5) {
        return;
    }

    button_border_draw(38, 96 + 42 * index, 560, 40, 0);
    text_draw_number(request->amount, '@', " ", 40, 102 + 42 * index, FONT_NORMAL_WHITE, 0);
    int resource_offset = request->resource + resource_image_offset(request->resource, RESOURCE_IMAGE_ICON);
    image_draw(image_group(GROUP_RESOURCE_ICONS) + resource_offset, 110, 100 + 42 * index, COLOR_MASK_NONE, SCALE_NONE);
    lang_text_draw(23, request->resource, 150, 102 + 42 * index, FONT_NORMAL_WHITE);

    int width = lang_text_draw_amount(8, 4, request->months_to_comply, 310, 102 + 42 * index, FONT_NORMAL_WHITE);
    lang_text_draw(12, 2, 310 + width, 102 + 42 * index, FONT_NORMAL_WHITE);

    if (request->resource == RESOURCE_DENARII) {
        // request for money
        int treasury = city_finance_treasury();
        width = text_draw_number(treasury, '@', " ", 40, 120 + 42 * index, FONT_NORMAL_WHITE, 0);
        width += lang_text_draw(52, 44, 40 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        if (treasury < request->amount) {
            lang_text_draw(52, 48, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, 120 + 42 * index, FONT_NORMAL_WHITE);
        }
    } else {
        // normal goods request
        int using_granaries;
        int amount_stored = city_resource_get_amount_including_granaries(request->resource,
            request->amount, &using_granaries);
        int y_offset = 120 + 42 * index;
        width = text_draw_number(amount_stored, '@', " ", 40, y_offset, FONT_NORMAL_WHITE, 0);
        if (using_granaries) {
            width += text_draw(translation_for(TR_ADVISOR_IN_STORAGE), 40 + width, y_offset, FONT_NORMAL_WHITE, 0);
        } else {
            width += lang_text_draw(52, 43, 40 + width, y_offset, FONT_NORMAL_WHITE);
        }
        if (amount_stored < request->amount) {
            lang_text_draw(52, 48, 80 + width, y_offset, FONT_NORMAL_WHITE);
        } else {
            lang_text_draw(52, 47, 80 + width, y_offset, FONT_NORMAL_WHITE);
        }
    }
}

static int draw_background(void)
{
    city_emperor_calculate_gift_costs();

    outer_panel_draw(0, 0, 40, ADVISOR_HEIGHT);
    image_draw(image_group(GROUP_ADVISOR_ICONS) + 2, 10, 10, COLOR_MASK_NONE, SCALE_NONE);

    text_draw(scenario_player_name(), 60, 12, FONT_LARGE_BLACK, 0);

    int width = lang_text_draw(52, 0, 60, 44, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_favor(), '@', " ", 60 + width, 44, FONT_NORMAL_BLACK, 0);

    lang_text_draw_multiline(52, city_rating_favor() / 5 + 22, 60, 60, 544, FONT_NORMAL_BLACK);

    inner_panel_draw(32, 90, 36, 14);

    int num_requests = 0;
    if (city_request_has_troop_request()) {
        // can send to distant battle
        button_border_draw(38, 96, 560, 40, 0);
        image_draw(image_group(GROUP_RESOURCE_ICONS) + RESOURCE_WEAPONS, 50, 106, COLOR_MASK_NONE, SCALE_NONE);
        width = lang_text_draw(52, 72, 80, 102, FONT_NORMAL_WHITE);
        empire_city *city = empire_city_get(city_military_distant_battle_city());
        const uint8_t *city_name = empire_city_get_name(city);
        text_draw(city_name, 80 + width, 102, FONT_NORMAL_WHITE, 0);
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
    width += text_draw_number(city_emperor_salary_amount(), '@', " ", 120 + width, 398, FONT_NORMAL_WHITE, 0);
    lang_text_draw(52, 3, 120 + width, 398, FONT_NORMAL_WHITE);

    button_border_draw(320, 341, 250, 20, focus_button_id == 3);
    lang_text_draw_centered(52, 49, 320, 346, 250, FONT_NORMAL_WHITE);

    // Request buttons
    for (int i = 0; i < CITY_REQUEST_MAX_ACTIVE; i++) {
        if (city_request_get_status(i)) {
            button_border_draw(38, 96 + i * 42, 560, 40, focus_button_id == i + 4);
        }
    }
}

static int handle_mouse(const mouse *m)
{
    int request_count = city_request_has_troop_request() + scenario_request_count_visible();
    request_count = calc_bound(request_count, 0, CITY_REQUEST_MAX_ACTIVE);
    return generic_buttons_handle_mouse(m, 0, 0, imperial_buttons, 3 + request_count, &focus_button_id);
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

static void confirm_nothing(int accepted, int checked)
{}

static void confirm_send_troops(int accepted, int checked)
{
    if (accepted) {
        formation_legions_dispatch_to_distant_battle();
        window_empire_show();
    }
}

static void confirm_send_goods(int accepted, int checked)
{
    if (accepted) {
        scenario_request_dispatch(selected_request_id);
        if (!checked && city_resource_is_stockpiled(selected_resource)) {
            city_resource_toggle_stockpiled(selected_resource);
        }
    }
}

void button_request(int index, int param2)
{
    int status = city_request_get_status(index);
    if (!status) {
        return;
    }
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
            selected_resource = city_get_request_resource(index);
            selected_request_id = (status - CITY_REQUEST_STATUS_MAX) & ~CITY_REQUEST_STATUS_RESOURCES_FROM_GRANARY;
            if (status & CITY_REQUEST_STATUS_RESOURCES_FROM_GRANARY) {
                window_popup_dialog_show_confirmation(
                    translation_for(TR_ADVISOR_DISPATCHING_FOOD_FROM_GRANARIES_TITLE),
                    translation_for(TR_ADVISOR_DISPATCHING_FOOD_FROM_GRANARIES_TEXT),
                    city_resource_is_stockpiled(selected_resource) ? translation_for(TR_ADVISOR_KEEP_STOCKPILING) : 0,
                    confirm_send_goods);
            } else {
                window_popup_dialog_show_confirmation(
                    lang_get_string(5, POPUP_DIALOG_SEND_GOODS),
                    lang_get_string(5, POPUP_DIALOG_SEND_GOODS + 1),
                    city_resource_is_stockpiled(selected_resource) ? translation_for(TR_ADVISOR_KEEP_STOCKPILING) : 0,
                    confirm_send_goods);
            }
            break;
    }
}

// Used for showing the resource settings window on right click
void button_request_resource(int index, int param2)
{
    // Make sure there's a request pending at this index
    if (!city_request_get_status(index)) {
        return;
    }

    // index 0 is used for a troop request if it exists
    if (!index && city_request_has_troop_request()) {
        return;
    }
    selected_resource = city_get_request_resource(index);
    
    // we can't manage money with the resource settings window
    if (selected_resource == RESOURCE_DENARII) {
        return;
    }

    window_resource_settings_show(selected_resource);
}

static void write_resource_storage_tooltip(advisor_tooltip_result *r, int resource)
{
    int amount_warehouse = city_resource_count(resource);
    int amount_granary = city_resource_count_food_on_granaries(resource) / RESOURCE_GRANARY_ONE_LOAD;
    uint8_t *text = tooltip_resource_info;
    text += string_from_int(text, amount_warehouse, 0);
    *text = ' ';
    text++;
    text = string_copy(lang_get_string(52, 43), text, RESOURCE_INFO_MAX_TEXT - (int) (text - tooltip_resource_info));
    *text = '\n';
    text++;
    text += string_from_int(text, amount_granary, 0);
    *text = ' ';
    text++;
    text = string_copy(translation_for(TR_ADVISOR_FROM_GRANARIES), text, RESOURCE_INFO_MAX_TEXT - (int) (text - tooltip_resource_info));
    r->precomposed_text = tooltip_resource_info;
}

static void get_tooltip_text(advisor_tooltip_result *r)
{
    if (focus_button_id && focus_button_id <= 2) {
        r->text_id = 93 + focus_button_id;
    } else if (focus_button_id == 3) {
        r->text_id = 131;
    } else if (focus_button_id >= 4 && focus_button_id <= 8) {
        int index = focus_button_id - 4;
        int request_status = city_request_get_status(index);
        if (request_status == CITY_REQUEST_STATUS_NOT_ENOUGH_RESOURCES || request_status >= CITY_REQUEST_STATUS_MAX) {
            const scenario_request *request = scenario_request_get_visible(index - city_request_has_troop_request());
            int using_granaries;
            city_resource_get_amount_including_granaries(request->resource, request->amount, &using_granaries);
            if (using_granaries) {
                write_resource_storage_tooltip(r, request->resource);
            }
        }
    }
}

const advisor_window_type *window_advisor_imperial(void)
{
    city_resource_calculate_warehouse_stocks();
    city_resource_calculate_food_stocks_and_supply_wheat();
    static const advisor_window_type window = {
        draw_background,
        draw_foreground,
        handle_mouse,
        get_tooltip_text
    };
    return &window;
}
