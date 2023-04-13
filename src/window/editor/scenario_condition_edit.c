#include "scenario_condition_edit.h"

#include "core/string.h"
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
#include "window/editor/map.h"
#include "window/numeric_input.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 320
#define DETAILS_Y_OFFSET 128
#define DETAILS_ROW_HEIGHT 32

static void init(scenario_condition_t *condition);
static void button_amount(int param1, int param2);

static generic_button buttons[] = {
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 1, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 2, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 3, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 4, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_amount, button_none, 5, 0}
};
#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    int focus_button_id;
    int target_param;

    scenario_condition_t *condition;
    scenario_condition_data_t *xml_info;
} data;

static void init(scenario_condition_t *condition)
{
    data.condition = condition;
    data.xml_info = scenario_events_parameter_data_get_conditions_xml_attributes(condition->type);
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 24, 24);

    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_CONDITION), 32, 32, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_centered(string_from_ascii(data.xml_info->xml_attr.name), 32, 64, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);

    int y_offset = DETAILS_Y_OFFSET;
    int button_id = 0;
    if (data.xml_info->xml_parm1.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(string_from_ascii(data.xml_info->xml_parm1.name), data.condition->parameter1, "", 64, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm2.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(string_from_ascii(data.xml_info->xml_parm2.name), data.condition->parameter2, "", 64, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm3.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(string_from_ascii(data.xml_info->xml_parm3.name), data.condition->parameter3, "", 64, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm4.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(string_from_ascii(data.xml_info->xml_parm4.name), data.condition->parameter4, "", 64, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }
    y_offset += DETAILS_ROW_HEIGHT;
    button_id++;

    if (data.xml_info->xml_parm5.type > PARAMETER_TYPE_UNDEFINED) {
        large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
        text_draw_label_and_number(string_from_ascii(data.xml_info->xml_parm5.name), data.condition->parameter5, "", 64, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
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

static void set_amount_param(int value)
{
    switch (data.target_param) {
        case 1: data.condition->parameter1 = value; break;
        case 2: data.condition->parameter2 = value; break;
        case 3: data.condition->parameter3 = value; break;
        case 4: data.condition->parameter4 = value; break;
        case 5: data.condition->parameter5 = value; break;
    }
}

static void button_amount(int param1, int param2)
{
    data.target_param = param1;

    int max_limit = 0;
    switch (data.target_param) {
        case 1: max_limit = data.xml_info->xml_parm1.max_limit; break;
        case 2: max_limit = data.xml_info->xml_parm2.max_limit; break;
        case 3: max_limit = data.xml_info->xml_parm3.max_limit; break;
        case 4: max_limit = data.xml_info->xml_parm4.max_limit; break;
        case 5: max_limit = data.xml_info->xml_parm5.max_limit; break;
    }

    window_numeric_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 9, max_limit, set_amount_param);
}

void window_editor_scenario_condition_edit_show(scenario_condition_t *condition)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_CONDITION_EDIT,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(condition);
    window_show(&window);
}
