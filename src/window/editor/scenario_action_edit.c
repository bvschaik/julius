#include "scenario_action_edit.h"

#include "core/string.h"
#include "game/resource.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/event/parameter_data.h"
#include "scenario/event/action_handler.h"
#include "window/editor/allowed_buildings.h"
#include "window/editor/custom_variables.h"
#include "window/editor/map.h"
#include "window/editor/requests.h"
#include "window/editor/select_scenario_action_type.h"
#include "window/editor/select_city_by_type.h"
#include "window/editor/select_city_trade_route.h"
#include "window/editor/select_custom_message.h"
#include "window/editor/select_special_attribute_mapping.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define DETAILS_Y_OFFSET 128
#define DETAILS_ROW_HEIGHT 32
#define MAX_TEXT_LENGTH 50

static void init(scenario_action_t *action);
static void button_amount(const generic_button *button);
static void button_delete(const generic_button *button);
static void button_change_type(const generic_button *button);
static void set_param_value(int value);
static void set_parameter_being_edited(int value);
static void set_resource_value(int value);
static void resource_selection(const generic_button *button);
static void custom_message_selection(void);
static void change_parameter(xml_data_attribute_t *parameter, const generic_button *button);

static generic_button buttons[] = {
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, 0, 1},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, 0, 2},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, 0, 3},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, 0, 4},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, 0, 5},
    {288, 32, 64, 14, button_delete},
    {32, 64, BUTTON_WIDTH, 32, button_change_type}
};

#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    unsigned int focus_button_id;
    int parameter_being_edited;
    int parameter_being_edited_current_value;

    uint8_t display_text[MAX_TEXT_LENGTH];

    scenario_action_t *action;
    scenario_action_data_t *xml_info;
} data;

static uint8_t *translation_for_param_value(parameter_type type, int value)
{
    memset(data.display_text, 0, MAX_TEXT_LENGTH);
    scenario_events_parameter_data_get_display_string_for_value(type, value, data.display_text, MAX_TEXT_LENGTH);
    return data.display_text;
}

static void init(scenario_action_t *action)
{
    data.action = action;
}

static void draw_background(void)
{
    data.xml_info = scenario_events_parameter_data_get_actions_xml_attributes(data.action->type);
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 42, 24);

    for (unsigned int i = 5; i <= 6; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
    }

    text_draw_centered(translation_for(TR_EDITOR_DELETE), 288, 40, 72, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    text_draw_centered(translation_for(data.xml_info->xml_attr.key), 32, 72, BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    unsigned int button_id = 0;
    if (data.xml_info->xml_parm1.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16,
            data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_centered(translation_for(data.xml_info->xml_parm1.key),
            buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for_param_value(data.xml_info->xml_parm1.type, data.action->parameter1),
            buttons[button_id].x + BUTTON_WIDTH / 2, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    button_id++;

    if (data.xml_info->xml_parm2.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16,
            data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_centered(translation_for(data.xml_info->xml_parm2.key),
            buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for_param_value(data.xml_info->xml_parm2.type, data.action->parameter2),
            buttons[button_id].x + BUTTON_WIDTH / 2, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    button_id++;

    if (data.xml_info->xml_parm3.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16,
            data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_centered(translation_for(data.xml_info->xml_parm3.key),
            buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for_param_value(data.xml_info->xml_parm3.type, data.action->parameter3),
            buttons[button_id].x + BUTTON_WIDTH / 2, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    button_id++;

    if (data.xml_info->xml_parm4.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16,
            data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_centered(translation_for(data.xml_info->xml_parm4.key),
            buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for_param_value(data.xml_info->xml_parm4.type, data.action->parameter4),
            buttons[button_id].x + BUTTON_WIDTH / 2, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    button_id++;

    if (data.xml_info->xml_parm5.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16,
            data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_centered(translation_for(data.xml_info->xml_parm5.key),
            buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for_param_value(data.xml_info->xml_parm5.type, data.action->parameter5),
            buttons[button_id].x + BUTTON_WIDTH / 2, buttons[button_id].y + 8, buttons[button_id].width / 2,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }

    lang_text_draw_centered(13, 3, 32, 32 + 16 * 20, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void close_window(void)
{
    scenario_action_type_init(data.action);
    window_go_back();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        close_window();
    }
}

static void button_delete(const generic_button *button)
{
    scenario_action_type_delete(data.action);
    close_window();
}

static void button_change_type(const generic_button *button)
{
    window_editor_select_scenario_action_type_show(data.action);
}

static void button_amount(const generic_button *button)
{
    switch (button->parameter1) {
        case 1: change_parameter(&data.xml_info->xml_parm1, button); break;
        case 2: change_parameter(&data.xml_info->xml_parm2, button); break;
        case 3: change_parameter(&data.xml_info->xml_parm3, button); break;
        case 4: change_parameter(&data.xml_info->xml_parm4, button); break;
        case 5: change_parameter(&data.xml_info->xml_parm5, button); break;
    }
}

static void set_param_value(int value)
{
    switch (data.parameter_being_edited) {
        case 1:
            data.action->parameter1 = value;
            return;
        case 2:
            data.action->parameter2 = value;
            return;
        case 3:
            data.action->parameter3 = value;
            return;
        case 4:
            data.action->parameter4 = value;
            return;
        case 5:
            data.action->parameter5 = value;
            return;
        default:
            return;
    }
}

static void set_parameter_being_edited(int value)
{
    data.parameter_being_edited = value;
    switch (value) {
        case 1:
            data.parameter_being_edited_current_value = data.action->parameter1;
            break;
        case 2:
            data.parameter_being_edited_current_value = data.action->parameter2;
            break;
        case 3:
            data.parameter_being_edited_current_value = data.action->parameter3;
            break;
        case 4:
            data.parameter_being_edited_current_value = data.action->parameter4;
            break;
        case 5:
            data.parameter_being_edited_current_value = data.action->parameter5;
            break;
        default:
            break;
    }
}

static void set_resource_value(int value)
{
    switch (data.parameter_being_edited) {
        case 1:
            data.action->parameter1 = value + 1;
            return;
        case 2:
            data.action->parameter2 = value + 1;
            return;
        case 3:
            data.action->parameter3 = value + 1;
            return;
        case 4:
            data.action->parameter4 = value + 1;
            return;
        case 5:
            data.action->parameter5 = value + 1;
            return;
        default:
            return;
    }
}

static void resource_selection(const generic_button *button)
{
    static const uint8_t *resource_texts[RESOURCE_MAX];
    for (resource_type resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX; resource++) {
        resource_texts[resource - 1] = resource_get_data(resource)->text;
    }
    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
        resource_texts, RESOURCE_MAX - 1, set_resource_value);
}

static void custom_message_selection(void)
{
    window_editor_select_custom_message_show(set_param_value);
}

static void set_param_custom_variable(unsigned int id)
{
    switch (data.parameter_being_edited) {
        case 1:
            data.action->parameter1 = id;
            return;
        case 2:
            data.action->parameter2 = id;
            return;
        case 3:
            data.action->parameter3 = id;
            return;
        case 4:
            data.action->parameter4 = id;
            return;
        case 5:
            data.action->parameter5 = id;
            return;
        default:
            return;
    }
}

static void custom_variable_selection(void)
{
    window_editor_custom_variables_show(set_param_custom_variable);
}

static void set_param_allowed_building(int type)
{
    switch (data.parameter_being_edited) {
        case 1:
            data.action->parameter1 = type;
            return;
        case 2:
            data.action->parameter2 = type;
            return;
        case 3:
            data.action->parameter3 = type;
            return;
        case 4:
            data.action->parameter4 = type;
            return;
        case 5:
            data.action->parameter5 = type;
            return;
        default:
            return;
    }
}

static void change_parameter(xml_data_attribute_t *parameter, const generic_button *button)
{
    set_parameter_being_edited(button->parameter1);
    switch (parameter->type) {
        case PARAMETER_TYPE_NUMBER:
        case PARAMETER_TYPE_MIN_MAX_NUMBER:
            window_numeric_input_bound_show(BUTTON_WIDTH / 2, 0, button, 9, parameter->min_limit, parameter->max_limit,
                set_param_value);
            return;
        case PARAMETER_TYPE_BOOLEAN:
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_BUILDING_COUNTING:
        case PARAMETER_TYPE_CHECK:
        case PARAMETER_TYPE_DIFFICULTY:
        case PARAMETER_TYPE_ENEMY_TYPE:
        case PARAMETER_TYPE_INVASION_TYPE:
        case PARAMETER_TYPE_POP_CLASS:
        case PARAMETER_TYPE_RATING_TYPE:
        case PARAMETER_TYPE_STANDARD_MESSAGE:
        case PARAMETER_TYPE_STORAGE_TYPE:
        case PARAMETER_TYPE_TARGET_TYPE:
        case PARAMETER_TYPE_GOD:
        case PARAMETER_TYPE_CLIMATE:
            window_editor_select_special_attribute_mapping_show(parameter->type, set_param_value, data.parameter_being_edited_current_value);
            return;
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            window_editor_allowed_buildings_select(set_param_allowed_building, data.parameter_being_edited_current_value);
            return;
        case PARAMETER_TYPE_REQUEST:
            window_editor_requests_show_with_callback(set_param_value);
            return;
        case PARAMETER_TYPE_ROUTE:
            window_editor_select_city_trade_route_show(set_param_value);
            return;
        case PARAMETER_TYPE_FUTURE_CITY:
            window_editor_select_city_by_type_show(set_param_value, EMPIRE_CITY_FUTURE_TRADE);
            return;
        case PARAMETER_TYPE_RESOURCE:
            resource_selection(button);
            return;
        case PARAMETER_TYPE_CUSTOM_MESSAGE:
            custom_message_selection();
            return;
        case PARAMETER_TYPE_CUSTOM_VARIABLE:
            custom_variable_selection();
            return;
        default:
            return;
    }
}

void window_editor_scenario_action_edit_show(scenario_action_t *action)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_ACTION_EDIT,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(action);
    window_show(&window);
}
