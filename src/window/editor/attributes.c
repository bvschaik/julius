#include "attributes.h"

#include "core/image.h"
#include "core/image_group_editor.h"
#include "core/string.h"
#include "editor/editor.h"
#include "game/resource.h"
#include "graphics/arrow_button.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/editor.h"
#include "scenario/invasion.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "widget/input_box.h"
#include "widget/minimap.h"
#include "widget/sidebar/editor.h"
#include "window/city.h"
#include "window/editor/allowed_buildings.h"
#include "window/editor/custom_messages.h"
#include "window/editor/demand_changes.h"
#include "window/editor/invasions.h"
#include "window/editor/map.h"
#include "window/editor/price_changes.h"
#include "window/editor/requests.h"
#include "window/editor/scenario_events.h"
#include "window/editor/select_custom_message.h"
#include "window/editor/special_events.h"
#include "window/editor/starting_conditions.h"
#include "window/editor/win_criteria.h"
#include "window/select_list.h"

#define BRIEF_DESC_LENGTH 64

static void button_starting_conditions(const generic_button *button);
static void button_requests(const generic_button *button);
static void button_enemy(const generic_button *button);
static void button_invasions(const generic_button *button);
static void button_allowed_buildings(const generic_button *button);
static void button_win_criteria(const generic_button *button);
static void button_special_events(const generic_button *button);
static void button_price_changes(const generic_button *button);
static void button_demand_changes(const generic_button *button);
static void button_scenario_events(const generic_button *button);
static void button_custom_messages(const generic_button *button);
static void button_change_intro(const generic_button *button);
static void button_delete_intro(const generic_button *button);
static void button_change_victory(const generic_button *button);
static void button_delete_victory(const generic_button *button);
static void button_return_to_city(const generic_button *button);
static void button_change_climate(const generic_button *button);
static void button_change_image(int forward, int param2);

static generic_button buttons[] = {
    {212, 76, 250, 30, button_starting_conditions, 0, 1},
    {212, 116, 250, 30, button_change_climate, 0, 2},
    {212, 156, 250, 30, button_requests, 0, 3},
    {212, 196, 250, 30, button_enemy, 0, 4},
    {212, 236, 250, 30, button_invasions, 0, 5},
    {212, 276, 250, 30, button_allowed_buildings, 0, 6},
    {212, 316, 250, 30, button_win_criteria, 0, 7},
    {212, 356, 250, 30, button_special_events, 0, 8},
    {212, 396, 250, 30, button_price_changes, 0, 9},
    {212, 436, 250, 30, button_demand_changes, 0, 10},
    {470,  76, 250, 30, button_scenario_events, 0, 11},
    {470, 116, 250, 30, button_custom_messages, 0, 12},
    {470, 156, 250, 30, button_change_intro, button_delete_intro, 13},
    {470, 196, 250, 30, button_change_victory, button_delete_victory, 14},
    {470, 436, 250, 30, button_return_to_city},
};
#define NUMBER_OF_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static arrow_button image_arrows[] = {
    {20, 424, 19, 24, button_change_image, 0, 0},
    {44, 424, 21, 24, button_change_image, 1, 0},
};

static struct {
    uint8_t brief_description[BRIEF_DESC_LENGTH];
    unsigned int focus_button_id;
} data;

static input_box scenario_description_input = {
    200, 40, 19, 2, FONT_NORMAL_WHITE, 1,
    data.brief_description, BRIEF_DESC_LENGTH
};

static void start(void)
{
    string_copy(scenario_brief_description(), data.brief_description, BRIEF_DESC_LENGTH);
    input_box_start(&scenario_description_input);
}

static void stop(void)
{
    input_box_stop(&scenario_description_input);
    scenario_editor_update_brief_description(data.brief_description);
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 30, 46, 28);

    button_border_draw(18, 278, 184, 144, 0);
    int group_id = editor_is_active() ? image_group(GROUP_EDITOR_SCENARIO_IMAGE) : image_group(GROUP_SCENARIO_IMAGE);
    image_draw(group_id + scenario_image_id(), 20, 280, COLOR_MASK_NONE, SCALE_NONE);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    input_box_draw(&scenario_description_input);

    button_border_draw(212, 76, 250, 30, data.focus_button_id == 1);
    lang_text_draw_centered(44, 88, 212, 85, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 76, 32, 125, FONT_NORMAL_BLACK);
    button_border_draw(212, 116, 250, 30, data.focus_button_id == 2);
    lang_text_draw_centered(44, 77 + scenario_property_climate(), 212, 125, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 40, 32, 165, FONT_NORMAL_BLACK);
    button_border_draw(212, 156, 250, 30, data.focus_button_id == 3);
    int requests = scenario_request_count_active();
    if (requests == 0) {
        lang_text_draw_centered(44, 19, 212, 165, 250, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount_centered(CUSTOM_TRANSLATION, TR_EDITOR_REQUEST, requests, 212, 165, 250,
            FONT_NORMAL_BLACK);
    }

    lang_text_draw(44, 41, 32, 205, FONT_NORMAL_BLACK);
    button_border_draw(212, 196, 250, 30, data.focus_button_id == 4);
    lang_text_draw_centered(37, scenario_property_enemy(), 212, 205, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 42, 32, 245, FONT_NORMAL_BLACK);
    button_border_draw(212, 236, 250, 30, data.focus_button_id == 5);

    int invasions = scenario_invasion_count_active();
    if (invasions == 0) {
        lang_text_draw_centered(44, 20, 212, 245, 250, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_amount_centered(CUSTOM_TRANSLATION, TR_EDITOR_INVASION, invasions, 212, 245, 250,
            FONT_NORMAL_BLACK);
    }

    button_border_draw(212, 276, 250, 30, data.focus_button_id == 6);
    lang_text_draw_centered(44, 44, 212, 285, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 316, 250, 30, data.focus_button_id == 7);
    lang_text_draw_centered(44, 45, 212, 325, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 356, 250, 30, data.focus_button_id == 8);
    lang_text_draw_centered(44, 49, 212, 365, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 396, 250, 30, data.focus_button_id == 9);
    lang_text_draw_centered(44, 95, 212, 405, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 436, 250, 30, data.focus_button_id == 10);
    lang_text_draw_centered(44, 94, 212, 445, 250, FONT_NORMAL_BLACK);

    button_border_draw(470, 76, 250, 30, data.focus_button_id == 11);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_TITLE, 470, 85, 250, FONT_NORMAL_BLACK);

    button_border_draw(470, 116, 250, 30, data.focus_button_id == 12);
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_MESSAGES_TITLE, 470, 125, 250, FONT_NORMAL_BLACK);

    button_border_draw(470, 156, 250, 30, data.focus_button_id == 13);
    if (!scenario_editor_get_custom_message_introduction()) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_SELECT_INTRO, 470, 165, 250, FONT_NORMAL_BLACK);
    } else {
        text_draw_number(scenario_editor_get_custom_message_introduction(), '@',
            " ", 470, 165, FONT_NORMAL_BLACK, 0);
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_DESELECT_INTRO, 490, 165, 230, FONT_NORMAL_BLACK);
    }

    button_border_draw(470, 196, 250, 30, data.focus_button_id == 14);
    if (!scenario_editor_get_custom_victory_message()) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_SELECT_VICTORY, 470, 205, 250, FONT_NORMAL_BLACK);
    } else {
        text_draw_number(scenario_editor_get_custom_victory_message(), '@',
            " ", 470, 205, FONT_NORMAL_BLACK, 0);
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_DESELECT_VICTORY, 490, 205, 230, FONT_NORMAL_BLACK);
    }

    if (!editor_is_active()) {
        button_border_draw(470, 436, 250, 30, data.focus_button_id == 15);
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_RETURN_TO_CITY, 470, 445, 250, FONT_NORMAL_BLACK);
    }

    arrow_buttons_draw(0, 0, image_arrows, 2);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    int active_buttons = NUMBER_OF_BUTTONS;
    if (editor_is_active()) {
        active_buttons -= 1;
    }
    if (input_box_handle_mouse(m_dialog, &scenario_description_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, active_buttons, &data.focus_button_id) ||
        arrow_buttons_handle_mouse(m_dialog, 0, 0, image_arrows, 2, 0) ||
        widget_sidebar_editor_handle_mouse_attributes(m)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        stop();
        window_editor_map_show();
    }
}

static void button_starting_conditions(const generic_button *button)
{
    stop();
    window_editor_starting_conditions_show();
}

static void button_requests(const generic_button *button)
{
    stop();
    window_editor_requests_show();
}

static void set_enemy(int enemy)
{
    scenario_editor_set_enemy(enemy);
    start();
}

static void button_enemy(const generic_button *button)
{
    stop();
    window_select_list_show(screen_dialog_offset_x(), screen_dialog_offset_y(), button, 37, 20, set_enemy);
}

static void button_invasions(const generic_button *button)
{
    stop();
    window_editor_invasions_show();
}

static void button_allowed_buildings(const generic_button *button)
{
    stop();
    window_editor_allowed_buildings_show();
}

static void button_win_criteria(const generic_button *button)
{
    stop();
    window_editor_win_criteria_show();
}

static void button_special_events(const generic_button *button)
{
    stop();
    window_editor_special_events_show();
}

static void button_price_changes(const generic_button *button)
{
    stop();
    window_editor_price_changes_show();
}

static void button_demand_changes(const generic_button *button)
{
    stop();
    window_editor_demand_changes_show();
}

static void button_scenario_events(const generic_button *button)
{
    stop();
    window_editor_scenario_events_show();
}

static void button_custom_messages(const generic_button *button)
{
    stop();
    window_editor_custom_messages_show();
}

static void button_change_intro(const generic_button *button)
{
    stop();
    if (!scenario_editor_get_custom_message_introduction()) {
        window_editor_select_custom_message_show(scenario_editor_set_custom_message_introduction);
    } else {
        scenario_editor_set_custom_message_introduction(0);
        window_request_refresh();
    }
}

static void button_delete_intro(const generic_button *button)
{
    stop();
    scenario_editor_set_custom_message_introduction(0);
}

static void button_change_victory(const generic_button *button)
{
    stop();
    if (!scenario_editor_get_custom_victory_message()) {
        window_editor_select_custom_message_show(scenario_editor_set_custom_victory_message);
    } else {
        scenario_editor_set_custom_victory_message(0);
        window_request_refresh();
    }
}

static void button_delete_victory(const generic_button *button)
{
    stop();
    scenario_editor_set_custom_victory_message(0);
}

static void button_return_to_city(const generic_button *button)
{
    stop();
    window_city_show();
}

static void button_change_climate(const generic_button *button)
{
    scenario_editor_cycle_climate();
    image_load_climate(scenario_property_climate(), editor_is_active(), 0, 0);
    widget_minimap_invalidate();
    window_request_refresh();
}

static void button_change_image(int forward, int param2)
{
    scenario_editor_cycle_image(forward);
    window_request_refresh();
}

void window_editor_attributes_show(void)
{
    window_type window = {
        WINDOW_EDITOR_ATTRIBUTES,
        draw_background,
        draw_foreground,
        handle_input
    };
    start();
    window_show(&window);
}
