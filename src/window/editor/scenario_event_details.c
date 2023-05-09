#include "scenario_event_details.h"

#include "core/string.h"
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
#include "window/numeric_input.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define SHORT_BUTTON_LEFT_PADDING 128
#define SHORT_BUTTON_WIDTH 480
#define EVENT_REPEAT_Y_OFFSET 64
#define DETAILS_Y_OFFSET 160
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 10

enum {
    SCENARIO_EVENT_DETAILS_SET_MAX_REPEATS = 0,
    SCENARIO_EVENT_DETAILS_SET_REPEAT_MIN,
    SCENARIO_EVENT_DETAILS_SET_REPEAT_MAX,
    SCENARIO_EVENT_DETAILS_ADD_CONDITION,
    SCENARIO_EVENT_DETAILS_ADD_ACTION,
};

static void init(int event_id);
static void init_scroll_list(void);
static void on_scroll(void);
static void button_click(int param1, int param2);
static void button_amount(int param1, int param2);
static void button_add(int param1, int param2);
static void button_delete_event(int param1, int param2);
static void add_new_condition(void);
static void add_new_action(void);

static scrollbar_type scrollbar = {
    640, DETAILS_Y_OFFSET, DETAILS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {540, 32, 64, 14, button_delete_event, button_none, 10, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_MAX_REPEATS, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET + 32, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_REPEAT_MIN, 0},
    {SHORT_BUTTON_LEFT_PADDING, EVENT_REPEAT_Y_OFFSET + 64, SHORT_BUTTON_WIDTH, 14, button_amount, button_none, SCENARIO_EVENT_DETAILS_SET_REPEAT_MAX, 0},
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
} sub_item_entry_t;

static struct {
    int focus_button_id;
    int total_sub_items;
    int conditions_count;
    int actions_count;

    scenario_event_t *event;

    sub_item_entry_t list[MAX_VISIBLE_ROWS];
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
        int target_id = i + offset;
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

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 42, 36);

    for (int i = 0; i < 4; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
    }
    for (int i = 14; i < 16; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
    }
    
    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENTS_TITLE), 32, 32, SHORT_BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_ID), data.event->id, "", 400, 40, FONT_NORMAL_PLAIN, COLOR_BLACK);
    text_draw_centered(translation_for(TR_EDITOR_DELETE), buttons[0].x, buttons[0].y + 8, buttons[0].width + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    if (data.focus_button_id == 1) {
        button_border_draw(buttons[0].x, buttons[0].y, buttons[0].width, buttons[0].height + 8, 1);
    }

    int y_offset = EVENT_REPEAT_Y_OFFSET;
    if (scenario_event_can_repeat(data.event) == 0) {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_DOES_NOT_REPEAT), 32, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    } else if (data.event->max_number_of_repeats > 0) {
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_MAX_NUM_REPEATS), data.event->max_number_of_repeats, "",
            SHORT_BUTTON_LEFT_PADDING + 16, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    } else {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_MAX_NUM_REPEATS), 32, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEATS_FOREVER), 240, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    }

    y_offset += DETAILS_ROW_HEIGHT;
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEAT_MIN_MONTHS), data.event->repeat_months_min, "",
        SHORT_BUTTON_LEFT_PADDING + 16, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
    
    y_offset += DETAILS_ROW_HEIGHT;
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_REPEAT_MAX_MONTHS), data.event->repeat_months_max, "",
        SHORT_BUTTON_LEFT_PADDING + 16, y_offset + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    y_offset = DETAILS_Y_OFFSET;
    int i_button_offset = 4;
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i].sub_type != SUB_ITEM_TYPE_UNDEFINED) {
            large_label_draw(buttons[i + i_button_offset].x, buttons[i + i_button_offset].y, buttons[i + i_button_offset].width / 16, data.focus_button_id == i + i_button_offset + 1 ? 1 : 0);
            if (data.focus_button_id == (i + i_button_offset + 1)
                && data.list[i].type) {
                button_border_draw(BUTTON_LEFT_PADDING, y_offset, BUTTON_WIDTH, DETAILS_ROW_HEIGHT, 1);
            }

            color_t font_color = COLOR_MASK_BUILDING_GHOST_RED;
            if (data.list[i].sub_type == SUB_ITEM_TYPE_ACTION) {
                font_color = COLOR_MASK_NONE;
            }

            if (data.list[i].type) {
                text_draw(translation_for(data.list[i].xml_attr->key), 48, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                if (data.list[i].xml_parm1->type != PARAMETER_TYPE_UNDEFINED) {
                    text_draw_number(data.list[i].parameter1, ' ', " ", 336, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                }
                if (data.list[i].xml_parm2->type != PARAMETER_TYPE_UNDEFINED) {
                    text_draw_number(data.list[i].parameter2, ' ', " ", 400, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                }
                if (data.list[i].xml_parm3->type != PARAMETER_TYPE_UNDEFINED) {
                    text_draw_number(data.list[i].parameter3, ' ', " ", 464, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                }
                if (data.list[i].xml_parm4->type != PARAMETER_TYPE_UNDEFINED) {
                    text_draw_number(data.list[i].parameter4, ' ', " ", 528, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                }
                if (data.list[i].xml_parm5->type != PARAMETER_TYPE_UNDEFINED) {
                    text_draw(string_from_ascii("..."), 592, y_offset + 8, FONT_NORMAL_GREEN, font_color);
                }
            } else {
                text_draw_centered(translation_for(TR_EDITOR_DELETED), 48, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_PLAIN, font_color);
            }
        }

        y_offset += DETAILS_ROW_HEIGHT;
    }

    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_CONDITION_ADD), 32, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    y_offset += DETAILS_ROW_HEIGHT;
    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_ACTION_ADD), 32, y_offset + 8, SHORT_BUTTON_WIDTH, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

    lang_text_draw_centered(13, 3, 48, 16 * 34, BUTTON_WIDTH, FONT_NORMAL_BLACK);

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
    data.event->repeat_months_min = value;
}

static void set_amount_repeat_max(int value)
{
    data.event->repeat_months_max = value;
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

static void button_click(int param1, int param2)
{
    if (param1 > MAX_VISIBLE_ROWS ||
        param1 >= data.total_sub_items) {
        return;
    }

    if (data.list[param1].sub_type == SUB_ITEM_TYPE_ACTION) {
        scenario_action_t *action = scenario_event_get_action(data.event, data.list[param1].id);
        if (action->type != ACTION_TYPE_UNDEFINED) {
            window_editor_scenario_action_edit_show(action);
        }
    } else {
        scenario_condition_t *condition = scenario_event_get_condition(data.event, data.list[param1].id);
        if (condition->type != CONDITION_TYPE_UNDEFINED) {
            window_editor_scenario_condition_edit_show(condition);
        }
    }
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
