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
#include "scenario/scenario_events_parameter_data.h"
#include "scenario/action_types/action_handler.h"
#include "window/editor/map.h"
#include "window/editor/select_scenario_action_type.h"
#include "window/editor/select_city_by_type.h"
#include "window/editor/select_city_trade_route.h"
#include "window/editor/select_special_attribute_mapping.h"
#include "window/numeric_input.h"
#include "window/select_list.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 320
#define DETAILS_Y_OFFSET 128
#define DETAILS_ROW_HEIGHT 32

static void init(scenario_action_t *action);
static void button_amount(int param1, int param2);
static void button_delete(int param1, int param2);
static void button_change_type(int param1, int param2);
static void set_param_value(int value);
static void set_parameter_being_edited(int value);
static void set_resource_value(int value);
static void resource_selection(void);
static void change_parameter(xml_data_attribute_t *parameter, int param1);

static generic_button buttons[] = {
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 1, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 2, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 3, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 4, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 5, 0},
    {288, 32, 64, 14, button_delete, button_none, 0, 0},
    {32, 64, BUTTON_WIDTH, 32, button_change_type, button_none, 0, 0}
};
#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    int focus_button_id;
    int parameter_being_edited;
    int parameter_being_edited_current_value;

    scenario_action_t *action;
    scenario_action_data_t *xml_info;
} data;

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

    outer_panel_draw(16, 16, 24, 24);

    for (int i = 5; i <= 6; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
    }

    text_draw_centered(translation_for(TR_EDITOR_DELETE), 288, 40, 72, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    text_draw_centered(translation_for(data.xml_info->xml_attr.key), 32, 72, BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    int y_offset = DETAILS_Y_OFFSET;
    int button_id = 0;
    if (data.xml_info->xml_parm1.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(translation_for(data.xml_info->xml_parm1.key), data.action->parameter1, "", 64, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm2.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(translation_for(data.xml_info->xml_parm2.key), data.action->parameter2, "", 64, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm3.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(translation_for(data.xml_info->xml_parm3.key), data.action->parameter3, "", 64, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm4.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(translation_for(data.xml_info->xml_parm4.key), data.action->parameter4, "", 64, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm5.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(translation_for(data.xml_info->xml_parm5.key), data.action->parameter5, "", 64, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    lang_text_draw_centered(13, 3, 32, 32 + 16 * 20, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void button_delete(int param1, int param2)
{
    scenario_action_type_delete(data.action);
    window_go_back();
}

static void button_change_type(int param1, int param2)
{
    window_editor_select_scenario_action_type_show(data.action);
}

static void button_amount(int param1, int param2)
{
    switch (param1) {
        case 1: change_parameter(&data.xml_info->xml_parm1, param1); break;
        case 2: change_parameter(&data.xml_info->xml_parm2, param1); break;
        case 3: change_parameter(&data.xml_info->xml_parm3, param1); break;
        case 4: change_parameter(&data.xml_info->xml_parm4, param1); break;
        case 5: change_parameter(&data.xml_info->xml_parm5, param1); break;
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

static void resource_selection(void)
{
    static const uint8_t *resource_texts[RESOURCE_MAX];
    for (resource_type resource = RESOURCE_MIN_FOOD; resource < RESOURCE_MAX; resource++) {
        resource_texts[resource - 1] = resource_get_data(resource)->text;
    }
    window_select_list_show_text(screen_dialog_offset_x() + 64, screen_dialog_offset_y() + 64,
        resource_texts, RESOURCE_MAX - 1, set_resource_value);
}

static void change_parameter(xml_data_attribute_t *parameter, int param1)
{
    set_parameter_being_edited(param1);
    switch (parameter->type) {
        case PARAMETER_TYPE_NUMBER:
        case PARAMETER_TYPE_MIN_MAX_NUMBER:
            window_numeric_input_bound_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 9, parameter->min_limit, parameter->max_limit, set_param_value);
            return;
        case PARAMETER_TYPE_CHECK:
        case PARAMETER_TYPE_DIFFICULTY:
        case PARAMETER_TYPE_BOOLEAN:
        case PARAMETER_TYPE_POP_CLASS:
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_BUILDING_COUNTING:
        case PARAMETER_TYPE_ALLOWED_BUILDING:
        case PARAMETER_TYPE_STANDARD_MESSAGE:
            window_editor_select_special_attribute_mapping_show(parameter->type, set_param_value, data.parameter_being_edited_current_value);
            return;
        case PARAMETER_TYPE_ROUTE:
            window_editor_select_city_trade_route_show(set_param_value);
            return;
        case PARAMETER_TYPE_FUTURE_CITY:
            window_editor_select_city_by_type_show(set_param_value, EMPIRE_CITY_FUTURE_TRADE);
            return;
        case PARAMETER_TYPE_RESOURCE:
            resource_selection();
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
