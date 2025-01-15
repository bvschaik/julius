#include "scenario_event_details.h"

#include "core/string.h"
#include "editor/editor.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/scenario_event.h"
#include "scenario/scenario_events_controller.h"
#include "scenario/scenario_events_parameter_data.h"
#include "window/editor/map.h"
#include "window/editor/scenario_action_edit.h"
#include "window/editor/scenario_condition_edit.h"
#include "window/editor/select_special_attribute_mapping.h"
#include "window/numeric_input.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define SHORT_BUTTON_LEFT_PADDING 128
#define SHORT_BUTTON_WIDTH 480
#define EVENT_REPEAT_Y_OFFSET 96
#define DETAILS_Y_OFFSET 224
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 10
#define MAX_TEXT_LENGTH 75

enum {
    SCENARIO_EVENT_DETAILS_SET_MAX_REPEATS = 0,
    SCENARIO_EVENT_DETAILS_SET_REPEAT_MIN,
    SCENARIO_EVENT_DETAILS_SET_REPEAT_MAX,
    SCENARIO_EVENT_DETAILS_SET_TRIGGER,
    SCENARIO_EVENT_DETAILS_ADD_CONDITION,
    SCENARIO_EVENT_DETAILS_ADD_ACTION,
};

static void init(int event_id);
static void init_scroll_list(void);
static void on_scroll(void);
static void button_click(int index, int param2);
static void button_amount(int param1, int param2);
static void button_trigger_change(int param1, int param2);
static void button_add(int param1, int param2);
static void button_delete_event(int param1, int param2);
static void add_new_condition(void);
static void add_new_action(void);

static scrollbar_type scrollbar = {
    640, DETAILS_Y_OFFSET, DETAILS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {580, 32, 64, 14, button_delete_event, button_none, 10, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET, SHORT_BUTTON_WIDTH, 14, button_trigger_change, button_none, SCENARIO_EVENT_DETAILS_SET_TRIGGER, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET + 32, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_MAX_REPEATS, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET + 64, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_REPEAT_MIN, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET + 96, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_REPEAT_MAX, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (0 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 0, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (1 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 1, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (2 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 2, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (3 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 3, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (4 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 4, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (5 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 5, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (6 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 6, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (7 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 7, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (8 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 8, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (9 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 9, 0},
    {SHORT_BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (10 * DETAILS_ROW_HEIGHT), SHORT_BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_add, button_none, SCENARIO_EVENT_DETAILS_ADD_CONDITION, 0},
    {SHORT_BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (11 * DETAILS_ROW_HEIGHT), SHORT_BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_add, button_none, SCENARIO_EVENT_DETAILS_ADD_ACTION, 0}
};
#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

typedef enum {
    SUB_ITEM_TYPE_UNDEFINED,
    SUB_ITEM_TYPE_CONDITION,
    SUB_ITEM_TYPE_ACTION
} sub_item_type;

typedef struct {
    sub_item_type sub_type;
    int id;
    int type;
    int parameter1;
    int parameter2;
    int parameter3;
    int parameter4;
    int parameter5;
    xml_data_attribute_t *xml_attr;
    xml_data_attribute_t *xml_parm1;
    xml_data_attribute_t *xml_parm2;
    xml_data_attribute_t *xml_parm3;
    xml_data_attribute_t *xml_parm4;
    xml_data_attribute_t *xml_parm5;
    uint8_t text[MAX_TEXT_LENGTH];
} sub_item_entry_t;

static struct {
    unsigned int focus_button_id;
    unsigned int total_sub_items;
    unsigned int conditions_count;
    unsigned int actions_count;

    scenario_event_t *event;

    sub_item_entry_t list[MAX_VISIBLE_ROWS];
    uint8_t display_text[MAX_TEXT_LENGTH];
} data;

static void populate_list(int offset)
{
    // Ensure we dont offset past the end or beginning of the list.
    if (data.total_sub_items - offset < MAX_VISIBLE_ROWS) {
        offset = data.total_sub_items - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        unsigned int target_id = i + offset;
        if (target_id < data.conditions_count) {
            int condition_id = target_id;
            sub_item_entry_t entry;
            scenario_condition_t *current = scenario_event_get_condition(data.event, condition_id);
            entry.sub_type = SUB_ITEM_TYPE_CONDITION;
            entry.id = condition_id;
            entry.type = current->type;
            entry.parameter1 = current->parameter1;
            entry.parameter2 = current->parameter2;
            entry.parameter3 = current->parameter3;
            entry.parameter4 = current->parameter4;
            entry.parameter5 = current->parameter5;

            if (entry.type != CONDITION_TYPE_UNDEFINED) {
                scenario_condition_data_t *xml_info = scenario_events_parameter_data_get_conditions_xml_attributes(current->type);
                entry.xml_attr = &xml_info->xml_attr;
                entry.xml_parm1 = &xml_info->xml_parm1;
                entry.xml_parm2 = &xml_info->xml_parm2;
                entry.xml_parm3 = &xml_info->xml_parm3;
                entry.xml_parm4 = &xml_info->xml_parm4;
                entry.xml_parm5 = &xml_info->xml_parm5;
            }
            memset(entry.text, 0, MAX_TEXT_LENGTH);
            scenario_events_parameter_data_get_display_string_for_condition(current, entry.text, MAX_TEXT_LENGTH);

            data.list[i] = entry;
        } else if ((target_id - data.conditions_count) < data.actions_count) {
            int action_id = target_id - data.conditions_count;
            sub_item_entry_t entry;
            scenario_action_t *current = scenario_event_get_action(data.event, action_id);
            entry.sub_type = SUB_ITEM_TYPE_ACTION;
            entry.id = action_id;
            entry.type = current->type;
            entry.parameter1 = current->parameter1;
            entry.parameter2 = current->parameter2;
            entry.parameter3 = current->parameter3;
            entry.parameter4 = current->parameter4;
            entry.parameter5 = current->parameter5;

            if (entry.type != ACTION_TYPE_UNDEFINED) {
            scenario_action_data_t *xml_info = scenario_events_parameter_data_get_actions_xml_attributes(current->type);
                entry.xml_attr = &xml_info->xml_attr;
                entry.xml_parm1 = &xml_info->xml_parm1;
                entry.xml_parm2 = &xml_info->xml_parm2;
                entry.xml_parm3 = &xml_info->xml_parm3;
                entry.xml_parm4 = &xml_info->xml_parm4;
                entry.xml_parm5 = &xml_info->xml_parm5;
            }
            memset(entry.text, 0, MAX_TEXT_LENGTH);
            scenario_events_parameter_data_get_display_string_for_action(current, entry.text, MAX_TEXT_LENGTH);

            data.list[i] = entry;
        } else {
            sub_item_entry_t entry;
            entry.sub_type = SUB_ITEM_TYPE_UNDEFINED;
            data.list[i] = entry;
        }
    }
}

static void init(int event_id)
{
    data.event = scenario_event_get(event_id);

    init_scroll_list();
}

static void init_scroll_list(void)
{
    data.conditions_count = data.event->conditions.size;
    data.actions_count = data.event->actions.size;
    data.total_sub_items = data.conditions_count + data.actions_count;

    scrollbar_init(&scrollbar, 0, data.total_sub_items);
    populate_list(0);
}

static uint8_t *translation_for_param_value(parameter_type type, int value)
{
    memset(data.display_text, 0, MAX_TEXT_LENGTH);
    scenario_events_parameter_data_get_display_string_for_value(type, value, data.display_text, MAX_TEXT_LENGTH);
    return data.display_text;
}

static int color_from_state(event_state state)
{
    if (!editor_is_active()) {
        if (state == EVENT_STATE_ACTIVE) {
            return COLOR_MASK_GREEN;
        } else if (state == EVENT_STATE_PAUSED) {
            return COLOR_MASK_RED;
        } else if (state == EVENT_STATE_DISABLED) {
            return COLOR_MASK_GREY;
        }
    }
    return COLOR_MASK_NONE;
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(0, 0, 42, 38);

    lang_text_draw_centered(13, 3, 48, 16, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENTS_TITLE), 16, 32, 320, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_ID),
        data.event->id, "", 336, 40, FONT_NORMAL_PLAIN, COLOR_BLACK);
    if (!editor_is_active()) {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_STATE_UNDEFINED + data.event->state),
            420, 40, 80, FONT_NORMAL_GREEN, color_from_state(data.event->state));
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_EXECUTION_COUNT),
            data.event->execution_count, "", 40, 72, FONT_NORMAL_PLAIN, COLOR_BLACK);
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_MONTHS_UNTIL_ACTIVE),
            data.event->triggers_until_active, "", 336, 72, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }
    
    int button_id = 0;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw_centered(translation_for(TR_EDITOR_DELETE), buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width + 8,
        FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    button_id++;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw(translation_for(TR_EDITOR_SCENARIO_EVENT_TRIGGER),
        buttons[button_id].x + 8, buttons[button_id].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    text_draw_right_aligned(translation_for_param_value(PARAMETER_TYPE_EVENT_TRIGGER_TYPE, data.event->trigger),
        buttons[button_id].x, buttons[button_id].y + 8, buttons[button_id].width - 8,
        FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    button_id++;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    if (scenario_event_can_repeat(data.event) == 0) {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_DOES_NOT_REPEAT), 32, buttons[button_id].y + 8,
            SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    } else if (data.event->max_number_of_repeats > 0) {
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_MAX_NUM_REPEATS),
            data.event->max_number_of_repeats, "", SHORT_BUTTON_LEFT_PADDING + 16, buttons[button_id].y + 8,
            FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    } else {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_MAX_NUM_REPEATS), 32, buttons[button_id].y + 8,
            SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEATS_FOREVER), 240, buttons[button_id].y + 8,
            SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }

    button_id++;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEAT_MIN_TRIGGERS),
        data.event->repeat_triggers_min, "", SHORT_BUTTON_LEFT_PADDING + 16, buttons[button_id].y + 8,
        FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    button_id++;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEAT_MAX_TRIGGERS),
        data.event->repeat_triggers_max, "", SHORT_BUTTON_LEFT_PADDING + 16, buttons[button_id].y + 8,
        FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    button_id = 5;
    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i].sub_type != SUB_ITEM_TYPE_UNDEFINED) {
            large_label_draw(buttons[i + button_id].x, buttons[i + button_id].y,
                buttons[i + button_id].width / 16, data.focus_button_id == i + button_id + 1 ? 1 : 0);

            if (data.focus_button_id == (i + button_id + 1) && data.list[i].type) {
                button_border_draw(BUTTON_LEFT_PADDING, buttons[i + button_id].y, BUTTON_WIDTH, DETAILS_ROW_HEIGHT, 1);
            }

            color_t font_color = COLOR_MASK_BUILDING_GHOST_RED;
            if (data.list[i].sub_type == SUB_ITEM_TYPE_ACTION) {
                font_color = COLOR_MASK_NONE;
            }

            if (data.list[i].type) {
                text_draw(data.list[i].text, 48, buttons[i + button_id].y + 8, FONT_NORMAL_GREEN, font_color);
            } else {
                text_draw_centered(translation_for(TR_EDITOR_DELETED), 48, buttons[i + button_id].y + 8,
                    SHORT_BUTTON_WIDTH, FONT_NORMAL_PLAIN, font_color);
            }
        }
    }

    button_id = 15;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_CONDITION_ADD), buttons[button_id].x, buttons[button_id].y + 8,
        buttons[button_id].width, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    button_id++;
    large_label_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width / 16, data.focus_button_id == button_id + 1 ? 1 : 0);
    if (data.focus_button_id == button_id + 1) {
        button_border_draw(buttons[button_id].x, buttons[button_id].y, buttons[button_id].width, buttons[button_id].height + 8, 1);
    }
    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_ACTION_ADD), buttons[button_id].x, buttons[button_id].y + 8,
        buttons[button_id].width, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void on_scroll(void)
{
    window_request_refresh();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
    populate_list(scrollbar.scroll_position);
}

static void set_amount_max_repeats(int value)
{
    data.event->max_number_of_repeats = value;
}

static void set_amount_repeat_min(int value)
{
    data.event->repeat_triggers_min = value;
}

static void set_amount_repeat_max(int value)
{
    data.event->repeat_triggers_max = value;
}

static void add_new_condition(void)
{
    condition_types type = CONDITION_TYPE_TIME_PASSED;
    scenario_condition_t *condition = scenario_event_condition_create(data.event, type);
    condition->parameter1 = 1;
    init_scroll_list();
    window_request_refresh();
}

static void add_new_action(void)
{
    action_types type = ACTION_TYPE_ADJUST_FAVOR;
    scenario_event_action_create(data.event, type);
    init_scroll_list();
    window_request_refresh();
}

static void button_delete_event(int param1, int param2)
{
    scenario_event_delete(data.event);
    window_go_back();
}

static void button_click(int index, int param2)
{
    if (index > MAX_VISIBLE_ROWS ||
        index >= (int) data.total_sub_items) {
        return;
    }

    if (data.list[index].sub_type == SUB_ITEM_TYPE_ACTION) {
        scenario_action_t *action = scenario_event_get_action(data.event, data.list[index].id);
        if (action->type != ACTION_TYPE_UNDEFINED) {
            window_editor_scenario_action_edit_show(action);
        }
    } else {
        scenario_condition_t *condition = scenario_event_get_condition(data.event, data.list[index].id);
        if (condition->type != CONDITION_TYPE_UNDEFINED) {
            window_editor_scenario_condition_edit_show(condition);
        }
    }
}

static void set_event_trigger_value(int value)
{
    data.event->trigger = value;
}

static void button_trigger_change(int param1, int param2)
{
    window_editor_select_special_attribute_mapping_show(PARAMETER_TYPE_EVENT_TRIGGER_TYPE, set_event_trigger_value, data.event->trigger);
}

static void button_amount(int param1, int param2)
{
    if (param1 == SCENARIO_EVENT_DETAILS_SET_MAX_REPEATS) {
        window_numeric_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 3, 100000, set_amount_max_repeats);
    } else if (param1 == SCENARIO_EVENT_DETAILS_SET_REPEAT_MIN) {
        window_numeric_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 3, 1000, set_amount_repeat_min);
    } else if (param1 == SCENARIO_EVENT_DETAILS_SET_REPEAT_MAX) {
        window_numeric_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 3, 1000, set_amount_repeat_max);
    }
}

static void button_add(int param1, int param2)
{
    if (param1 == SCENARIO_EVENT_DETAILS_ADD_CONDITION) {
        add_new_condition();
    } else if (param1 == SCENARIO_EVENT_DETAILS_ADD_ACTION) {
        add_new_action();
    }
}

void window_editor_scenario_event_details_show(int event_id)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_EVENT_DETAILS,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(event_id);
    window_show(&window);
}
