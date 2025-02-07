#include "scenario_event_details.h"

#include "assets/assets.h"
#include "core/lang.h"
#include "core/log.h"
#include "core/string.h"
#include "editor/editor.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/grid_box.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/event/event.h"
#include "scenario/event/controller.h"
#include "scenario/event/parameter_data.h"
#include "widget/input_box.h"
#include "window/editor/map.h"
#include "window/editor/scenario_action_edit.h"
#include "window/editor/scenario_condition_edit.h"
#include "window/numeric_input.h"
#include "window/popup_dialog.h"
#include "window/select_list.h"

#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define SHORT_BUTTON_LEFT_PADDING 128
#define SHORT_BUTTON_WIDTH 480
#define EVENT_REPEAT_Y_OFFSET 96
#define DETAILS_Y_OFFSET 192
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 10
#define MAX_TEXT_LENGTH 120

enum {
    REPEAT_MIN = 0,
    REPEAT_MAX = 1
};

enum {
    EVENT_REPEAT_NEVER = 0,
    EVENT_REPEAT_FOREVER = 1,
    EVENT_REPEAT_TIMES = 2
};

enum {
    DO_NOT_DISABLE = 0,
    DISABLE_ON_NO_REPEAT = 1,
    DISABLE_ON_NO_SELECTION = 2
};

enum {
    SELECT_CONDITIONS = 0,
    SELECT_ACTIONS = 1
};

typedef enum {
    CHECKBOX_NO_SELECTION = 0,
    CHECKBOX_SOME_SELECTED = 1,
    CHECKBOX_ALL_SELECTED = 2
} checkbox_selection_type;

static void button_add_new_condition(const generic_button *button);
static void button_delete_selected(const generic_button *button);
static void button_add_new_action(const generic_button *button);
static void button_delete_event(const generic_button *button);
static void button_repeat_type(const generic_button *button);
static void button_repeat_times(const generic_button *button);
static void button_repeat_between(const generic_button *button);
static void button_set_selected_to_group(const generic_button *button);
static void button_ok(const generic_button *button);

static void draw_condition_button(const grid_box_item *item);
static void draw_action_button(const grid_box_item *item);
static void click_condition_button(const grid_box_item *item);
static void click_action_button(const grid_box_item *item);
static void handle_condition_tooltip(const grid_box_item *item, tooltip_context *c);
static void handle_action_tooltip(const grid_box_item *item, tooltip_context *c);

static void button_select_all_none(const generic_button *button);

typedef struct {
    int group_id;
    scenario_condition_t *condition;
} condition_list_item;

static struct {
    uint8_t event_name[EVENT_NAME_LENGTH];
    scenario_event_t *event;
    int repeat_type;
    struct {
        condition_list_item *list;
        unsigned int active;
        uint8_t *selected;
        checkbox_selection_type selection_type;
        unsigned int available;
        struct {
            uint8_t **names;
            unsigned int available;
        } groups;
    } conditions;
    struct {
        scenario_action_t **list;
        unsigned int active;
        uint8_t *selected;
        checkbox_selection_type selection_type;
        unsigned int available;
    } actions;
    grid_box_type *focused_grid_box;
    struct {
        unsigned int top;
        unsigned int select_all_none;
        unsigned int bottom;
    } focus_button;
    int do_not_ask_again_for_delete;
} data;

static input_box event_name_input = {
    100, 40, 32, 2, FONT_NORMAL_WHITE, 1, data.event_name, EVENT_NAME_LENGTH
};

static grid_box_type conditions_grid_box = {
    .x = 16,
    .y = 188,
    .width = 18 * BLOCK_SIZE,
    .height = 13 * BLOCK_SIZE + 2,
    .num_columns = 1,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 4,
    .extend_to_hidden_scrollbar = 1,
    .draw_item = draw_condition_button,
    .on_click = click_condition_button,
    .handle_tooltip = handle_condition_tooltip
};

static grid_box_type actions_grid_box = {
    .x = 320,
    .y = 188,
    .width = 18 * BLOCK_SIZE,
    .height = 13 * BLOCK_SIZE + 2,
    .num_columns = 1,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 4,
    .extend_to_hidden_scrollbar = 1,
    .draw_item = draw_action_button,
    .on_click = click_action_button,
    .handle_tooltip = handle_action_tooltip
};

#define NUM_TOP_BUTTONS (sizeof(top_buttons) / sizeof(generic_button))

static generic_button top_buttons[] = {
    {100, 80, 220, 20, button_repeat_type, 0, EVENT_REPEAT_NEVER},
    {100, 105, 220, 20, button_repeat_type, 0, EVENT_REPEAT_FOREVER},
    {100, 130, 20, 20, button_repeat_type, 0, EVENT_REPEAT_TIMES},
    {130, 128, 190, 25, button_repeat_times},
    {410, 128, 50, 25, button_repeat_between, 0, REPEAT_MIN, DISABLE_ON_NO_REPEAT},
    {500, 128, 50, 25, button_repeat_between, 0, REPEAT_MAX, DISABLE_ON_NO_REPEAT},
    {144, 163, 155, 20, button_set_selected_to_group, 0, 0, DISABLE_ON_NO_SELECTION}
};

static generic_button select_all_none_buttons[] = {
    {16, 163, 20, 20, button_select_all_none, 0, SELECT_CONDITIONS},
    {320, 163, 20, 20, button_select_all_none, 0, SELECT_ACTIONS}
};

#define NUM_BOTTOM_BUTTONS (sizeof(bottom_buttons) / sizeof(generic_button))

static generic_button bottom_buttons[] = {
    {16, 409, 192, 25, button_add_new_condition},
    {224, 409, 192, 25, button_delete_selected, 0, 0, DISABLE_ON_NO_SELECTION},
    {432, 409, 192, 25, button_add_new_action},
    {16, 439, 200, 25, button_delete_event},
    {524, 439, 100, 25, button_ok},
};

static unsigned int count_maximum_needed_list_items(void)
{
    unsigned int total_items = 0;
    scenario_condition_group_t *group;
    array_foreach(data.event->condition_groups, group) {
        if (group->conditions.size > 0) {
            total_items += group->conditions.size + 1;
        }
    }
    return total_items;
}

static void update_visible_conditions_and_actions(void)
{
    unsigned int max_needed_items = count_maximum_needed_list_items();
    if (max_needed_items > data.conditions.available) {
        free(data.conditions.list);
        free(data.conditions.selected);
        data.conditions.list = calloc(max_needed_items, sizeof(condition_list_item));
        data.conditions.selected = calloc(max_needed_items, sizeof(uint8_t));

        if (!data.conditions.list) {
            log_error("Unable to create conditions list - out of memory. The game will probably crash.", 0, 0);
            data.conditions.available = 0;
        } else {
            data.conditions.available = max_needed_items;
        }
    }
    data.conditions.active = 0;
    if (data.conditions.available) {
        memset(data.conditions.list, 0, data.conditions.available * sizeof(condition_list_item));
        scenario_condition_group_t *group;
        scenario_condition_t *condition;
        for (unsigned int i = 1; i < data.event->condition_groups.size; i++) {
            group = array_item(data.event->condition_groups, i);
            if (group->conditions.size > 0) {
                data.conditions.list[data.conditions.active].group_id = i;
                data.conditions.active++;
                array_foreach(group->conditions, condition) {
                    if (condition->type != CONDITION_TYPE_UNDEFINED) {
                        data.conditions.list[data.conditions.active].group_id = i;
                        data.conditions.list[data.conditions.active].condition = condition;
                        data.conditions.active++;
                    }
                }
            }
        }
        group = array_item(data.event->condition_groups, 0);
        if (data.conditions.active && group->conditions.size > 0) {
            data.conditions.list[data.conditions.active].group_id = 0;
            data.conditions.active++;
        }
        array_foreach(group->conditions, condition) {
            if (condition->type != CONDITION_TYPE_UNDEFINED) {
                data.conditions.list[data.conditions.active].group_id = 0;
                data.conditions.list[data.conditions.active].condition = condition;
                data.conditions.active++;
            }
        }
    }
    grid_box_update_total_items(&conditions_grid_box, data.conditions.active);

    if (data.event->actions.size > data.actions.available) {
        free(data.actions.list);
        free(data.actions.selected);
        data.actions.list = calloc(data.event->actions.size, sizeof(scenario_action_t *));
        data.actions.selected = calloc(data.event->actions.size, sizeof(uint8_t));

        if (!data.actions.list) {
            log_error("Unable to create actions list - out of memory. The game will probably crash.", 0, 0);
            data.actions.available = 0;
        } else {
            data.actions.available = data.event->actions.size;
        }
    }
    data.actions.active = 0;
    if (data.actions.available) {
        scenario_action_t *action;
        array_foreach(data.event->actions, action) {
            if (action->type != ACTION_TYPE_UNDEFINED) {
                data.actions.list[data.actions.active] = action;
                data.actions.active++;
            }
        }
    }
    grid_box_update_total_items(&actions_grid_box, data.actions.active);
}

static void update_groups(void)
{
    for (unsigned int i = 0; i < data.conditions.groups.available; i++) {
        free(data.conditions.groups.names[i]);
    }
    free(data.conditions.groups.names);
    data.conditions.groups.names = calloc(data.event->condition_groups.size + 1, sizeof(uint8_t *));
    if (!data.conditions.groups.names) {
        log_error("Unable to create groups list - out of memory. The game will probably crash.", 0, 0);
        data.conditions.groups.available = 0;
        return;
    }
    data.conditions.groups.available = data.event->condition_groups.size + 1;
    const uint8_t *text = translation_for(TR_EDITOR_SCENARIO_EVENTS_NO_GROUP);
    int length = string_length(text) + 1;
    data.conditions.groups.names[0] = calloc(length, sizeof(uint8_t *));
    string_copy(text, data.conditions.groups.names[0], length);

    for (unsigned int i = 1; i < data.event->condition_groups.size; i++) {
        text = translation_for(TR_EDITOR_SCENARIO_EVENTS_GROUP);
        length = string_length(text) + 11;
        data.conditions.groups.names[i] = calloc(length, sizeof(uint8_t *));
        uint8_t *cursor = string_copy(text, data.conditions.groups.names[i], length);
        string_from_int(cursor, i, 0);
    }

    text = translation_for(TR_EDITOR_SCENARIO_EVENTS_NEW_GROUP);
    length = string_length(text) + 1;
    data.conditions.groups.names[data.event->condition_groups.size] = calloc(length, sizeof(uint8_t *));
    string_copy(text, data.conditions.groups.names[data.event->condition_groups.size], length);
}

static void select_no_conditions(void)
{
    for (unsigned int i = 0; i < data.conditions.active; i++) {
        data.conditions.selected[i] = 0;
    }
    data.conditions.selection_type = CHECKBOX_NO_SELECTION;
}

static void select_no_actions(void)
{
    for (unsigned int i = 0; i < data.actions.active; i++) {
        data.actions.selected[i] = 0;
    }
    data.actions.selection_type = CHECKBOX_NO_SELECTION;
}

static void start_input(void)
{
    string_copy(data.event->name, data.event_name, EVENT_NAME_LENGTH);
    input_box_start(&event_name_input);
}

static void stop_input(void)
{
    input_box_stop(&event_name_input);
    string_copy(data.event_name, data.event->name, EVENT_NAME_LENGTH);
}

static void prepare_event(int event_id)
{
    data.event = scenario_event_get(event_id);

    if (data.event->repeat_months_min > data.event->repeat_months_max) {
        data.event->repeat_months_min = data.event->repeat_months_max;
    }
    if (data.event->repeat_months_min == 0) {
        data.repeat_type = EVENT_REPEAT_NEVER;
        data.event->repeat_months_min = 1;
        if (data.event->repeat_months_max == 0) {
            data.event->repeat_months_max = 1;
        }
    } else if (data.event->max_number_of_repeats == 0) {
        data.repeat_type = EVENT_REPEAT_FOREVER;
        data.event->max_number_of_repeats = 1;
    } else {
        data.repeat_type = EVENT_REPEAT_TIMES;
    }

    update_groups();
}

static void init(int event_id)
{
    prepare_event(event_id);
    start_input();
    grid_box_init(&conditions_grid_box, count_maximum_needed_list_items());
    grid_box_init(&actions_grid_box, data.event->actions.size);
    select_no_conditions();
    select_no_actions();
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
    update_visible_conditions_and_actions();

    window_editor_map_draw_all();

    graphics_in_dialog();

    // Helper debug text during city mode
    if (!editor_is_active()) {
        text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_STATE_UNDEFINED + data.event->state),
            420, 40, 80, FONT_NORMAL_GREEN, color_from_state(data.event->state));
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_EXECUTION_COUNT),
            data.event->execution_count, "", 40, 72, FONT_NORMAL_PLAIN, COLOR_BLACK);
        text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENT_MONTHS_UNTIL_ACTIVE),
            data.event->months_until_active, "", 336, 72, FONT_NORMAL_PLAIN, COLOR_BLACK);
    }

    // Refresh lists
    grid_box_request_refresh(&conditions_grid_box);
    grid_box_request_refresh(&actions_grid_box);

    outer_panel_draw(0, 0, 40, 30);

    // Title and ID
    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENTS_TITLE), 0, 13, 640, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_ID),
        data.event->id, "", 16, 13, FONT_NORMAL_PLAIN, COLOR_BLACK);

    // "Name" string
    lang_text_draw_right_aligned(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENT_NAME, 0, event_name_input.y + 8,
        event_name_input.x - 10, FONT_NORMAL_BLACK);

    // Top buttons
    // Repeat type selected checkbox
    const generic_button *btn = &top_buttons[data.repeat_type];
    text_draw(string_from_ascii("x"), btn->x + 6, btn->y + 3, FONT_NORMAL_BLACK, 0);

    btn = &top_buttons[0];
    // Repeat type - Never
    lang_text_draw_right_aligned(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT, 0, btn->y + 3, btn->x - 16, FONT_NORMAL_BLACK);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_DO_NOT_REPEAT, btn->x + 30, btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat forever
    btn = &top_buttons[1];
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FOREVER, btn->x + 30, btn->y + 3, FONT_NORMAL_BLACK);

    // Repeat N times
    btn = &top_buttons[3];
    int repeat_times = data.event->max_number_of_repeats;
    if (repeat_times < 1) {
        repeat_times = 1;
    }
    if (repeat_times > 2) {
        text_draw_label_and_number_centered(lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TEXT), repeat_times,
            (const char *) lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_TIMES),
            btn->x, btn->y + 6, btn->width, FONT_NORMAL_BLACK, 0);
    } else {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_ONCE + repeat_times - 1, btn->x, btn->y + 6,
            btn->width, FONT_NORMAL_BLACK);
    }

    // Repeat interval text
    font_t enabled_font = data.repeat_type == EVENT_REPEAT_NEVER ? FONT_NORMAL_PLAIN : FONT_NORMAL_BLACK;
    color_t enabled_color = data.repeat_type == EVENT_REPEAT_NEVER ? COLOR_FONT_LIGHT_GRAY : COLOR_MASK_NONE;

    btn = &top_buttons[4];
    lang_text_draw_right_aligned(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FREQUENCY, 0, btn->y - 20, top_buttons[0].x + 450,
        FONT_NORMAL_BLACK);
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_BETWEEN, top_buttons[0].x + 240, btn->y + 6,
        enabled_font, enabled_color);
    text_draw_number_centered_colored(data.event->repeat_months_min, btn->x, btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_AND, btn->x + btn->width,
        btn->y + 6, btn[1].x - (btn->x + btn->width), enabled_font, enabled_color);
    btn = &top_buttons[5];
    text_draw_number_centered_colored(data.event->repeat_months_max, btn->x, btn->y + 6,
        btn->width, enabled_font, enabled_color);
    lang_text_draw_colored(CUSTOM_TRANSLATION, TR_EDITOR_REPEAT_FREQUENCY_MONTHS, btn->x + btn->width + 10,
        btn->y + 6, enabled_font, enabled_color);

    // Checkmarks for select all/none buttons for conditions
    int checkmark_id = assets_lookup_image_id(ASSET_UI_SELECTION_CHECKMARK);
    const image *img = image_get(checkmark_id);
    if (data.conditions.selection_type == CHECKBOX_SOME_SELECTED) {
        text_draw(string_from_ascii("-"), select_all_none_buttons[0].x + 8, select_all_none_buttons[0].y + 4,
            FONT_NORMAL_BLACK, 0);
    } else if (data.conditions.selection_type == CHECKBOX_ALL_SELECTED) {
        image_draw(checkmark_id, select_all_none_buttons[0].x + (20 - img->original.width) / 2,
             select_all_none_buttons[0].y + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    // "Set selected to group..." option label
    color_t color = data.conditions.selection_type == CHECKBOX_NO_SELECTION ? COLOR_FONT_LIGHT_GRAY : 0;
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_SET_TO_GROUP,
        top_buttons[6].x, top_buttons[6].y + 5, top_buttons[6].width, FONT_SMALL_PLAIN, color);

    // Condition grid box label
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_CONDITION,
        select_all_none_buttons[0].x + select_all_none_buttons[0].width + 6, conditions_grid_box.y - 20,
        FONT_NORMAL_BLACK);

    // Checkmarks for select all/none buttons for actions
    if (data.actions.selection_type == CHECKBOX_SOME_SELECTED) {
        text_draw(string_from_ascii("-"), select_all_none_buttons[1].x + 8, select_all_none_buttons[1].y + 4,
            FONT_NORMAL_BLACK, 0);
    } else if (data.actions.selection_type == CHECKBOX_ALL_SELECTED) {
        image_draw(checkmark_id, select_all_none_buttons[1].x + (20 - img->original.width) / 2,
             select_all_none_buttons[1].y + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    // Action grid box label
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_ACTION,
        select_all_none_buttons[1].x + select_all_none_buttons[1].width + 6, actions_grid_box.y - 20,
        FONT_NORMAL_BLACK);

    // No conditions
    int y_offset = conditions_grid_box.y + conditions_grid_box.height / 2 - 10;
    if (data.conditions.active == 0) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_NO_CONDITIONS, conditions_grid_box.x,
            y_offset, conditions_grid_box.width, FONT_NORMAL_BLACK);
    }

    // No actions
    if (data.actions.active == 0) {
        lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_NO_ACTIONS, actions_grid_box.x,
            y_offset, actions_grid_box.width, FONT_NORMAL_BLACK);
    }

    // New condition button label
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_CONDITION_ADD,
        bottom_buttons[0].x, bottom_buttons[0].y + 6, bottom_buttons[0].width, FONT_NORMAL_BLACK);

    // Delete selected button label
    color = data.conditions.selection_type == CHECKBOX_NO_SELECTION &&
        data.actions.selection_type == CHECKBOX_NO_SELECTION ? COLOR_FONT_LIGHT_GRAY : COLOR_RED;
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED,
        bottom_buttons[1].x, bottom_buttons[1].y + 6, bottom_buttons[1].width, FONT_NORMAL_PLAIN, color);

    // Add action button label
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_ACTION_ADD,
        bottom_buttons[2].x, bottom_buttons[2].y + 6, bottom_buttons[2].width, FONT_NORMAL_BLACK);

    // Bottom buttons
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_DELETE, bottom_buttons[3].x, bottom_buttons[3].y + 6,
        bottom_buttons[3].width, FONT_NORMAL_PLAIN, COLOR_RED);
    lang_text_draw_centered(18, 3, bottom_buttons[4].x, bottom_buttons[4].y + 6, bottom_buttons[4].width,
        FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_condition_button(const grid_box_item *item)
{
    const condition_list_item *list_item = &data.conditions.list[item->index];
    if (!list_item->condition) {
        text_draw(data.conditions.groups.names[list_item->group_id], item->x + 4, item->y + 7,
            FONT_NORMAL_BLACK, 0);
        return;
    }
    int selection_button_y_offset = (item->height - 20) / 2;
    button_border_draw(item->x, item->y + selection_button_y_offset, 20, 20,
        item->is_focused && item->mouse.x < 20);

    if (data.conditions.selected && data.conditions.selected[item->index]) {
        int checkmark_id = assets_lookup_image_id(ASSET_UI_SELECTION_CHECKMARK);
        const image *img = image_get(checkmark_id);
        image_draw(checkmark_id, item->x + (20 - img->original.width) / 2,
            item->y + selection_button_y_offset + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    button_border_draw(item->x + 24, item->y, item->width - 24, item->height, item->is_focused && item->mouse.x >= 24);

    const scenario_condition_t *condition = data.conditions.list[item->index].condition;
    uint8_t text[MAX_TEXT_LENGTH];
    scenario_events_parameter_data_get_display_string_for_condition(condition, text, MAX_TEXT_LENGTH);
    if (text_get_width(text, FONT_NORMAL_BLACK) > item->width - 32) {
        text_draw_ellipsized(text, item->x + 28, item->y + 7, item->width - 32, FONT_NORMAL_BLACK, 0);
    } else {
        text_draw_centered(text, item->x + 28, item->y + 7, item->width - 32, FONT_NORMAL_BLACK, 0);
    }
}

static void draw_action_button(const grid_box_item *item)
{
    int selection_button_y_offset = (item->height - 20) / 2;
    button_border_draw(item->x, item->y + selection_button_y_offset, 20, 20,
        item->is_focused && item->mouse.x < 20);

    if (data.actions.selected && data.actions.selected[item->index]) {
        int checkmark_id = assets_lookup_image_id(ASSET_UI_SELECTION_CHECKMARK);
        const image *img = image_get(checkmark_id);
        image_draw(checkmark_id, item->x + (20 - img->original.width) / 2,
            item->y + selection_button_y_offset + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    button_border_draw(item->x + 24, item->y, item->width - 24, item->height, item->is_focused && item->mouse.x >= 24);
    const scenario_action_t *action = data.actions.list[item->index];
    uint8_t text[MAX_TEXT_LENGTH];
    scenario_events_parameter_data_get_display_string_for_action(action, text, MAX_TEXT_LENGTH);
    if (text_get_width(text, FONT_NORMAL_BLACK) > item->width - 32) {
        text_draw_ellipsized(text, item->x + 28, item->y + 7, item->width - 32, FONT_NORMAL_BLACK, 0);
    } else {
        text_draw_centered(text, item->x + 28, item->y + 7, item->width - 32, FONT_NORMAL_BLACK, 0);
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    input_box_draw(&event_name_input);

    for (unsigned int i = 0; i < NUM_TOP_BUTTONS; i++) {
        int focus = data.focus_button.top == i + 1;
        if ((top_buttons[i].parameter2 == DISABLE_ON_NO_REPEAT && data.repeat_type == EVENT_REPEAT_NEVER) ||
            (top_buttons[i].parameter2 == DISABLE_ON_NO_SELECTION &&
                data.conditions.selection_type == CHECKBOX_NO_SELECTION)) {
            focus = 0;
        }
        int width = i < 2 ? 20 : top_buttons[i].width;
        button_border_draw(top_buttons[i].x, top_buttons[i].y, width, top_buttons[i].height, focus);
    }

    for (unsigned int i = 0; i < 2; i++) {
        int focus = data.focus_button.select_all_none == i + 1;
        if (i == 0 && data.conditions.active == 0) {
            focus = 0;
        } else if (i == 1 && data.actions.active == 0) {
            focus = 0;
        }
        button_border_draw(select_all_none_buttons[i].x, select_all_none_buttons[i].y, 20, 20, focus);
    }

    for (unsigned int i = 0; i < NUM_BOTTOM_BUTTONS; i++) {
        int focus = data.focus_button.bottom == i + 1;
        if (bottom_buttons[i].parameter2 == DISABLE_ON_NO_SELECTION &&
            data.conditions.selection_type == CHECKBOX_NO_SELECTION &&
            data.actions.selection_type == CHECKBOX_NO_SELECTION) {
            focus = 0;
        }
        button_border_draw(bottom_buttons[i].x, bottom_buttons[i].y, bottom_buttons[i].width, bottom_buttons[i].height,
            focus);
    }

    grid_box_draw(&conditions_grid_box);
    grid_box_draw(&actions_grid_box);

    graphics_reset_dialog();
}

static void get_focused_grid_box(const mouse *m)
{
    if (!data.focused_grid_box) {
        data.focused_grid_box = &conditions_grid_box;
    }
    if (m->x >= conditions_grid_box.x && m->x < conditions_grid_box.x + conditions_grid_box.width &&
        m->y >= conditions_grid_box.y && m->y < conditions_grid_box.y + conditions_grid_box.height) {
        data.focused_grid_box = &conditions_grid_box;
    } else if (m->x >= actions_grid_box.x && m->x < actions_grid_box.x + actions_grid_box.width &&
        m->y >= actions_grid_box.y && m->y < actions_grid_box.y + actions_grid_box.height) {
        data.focused_grid_box = &actions_grid_box;
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    get_focused_grid_box(m_dialog);
    if (input_box_handle_mouse(m_dialog, &event_name_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, top_buttons, NUM_TOP_BUTTONS, &data.focus_button.top) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, select_all_none_buttons, 2, &data.focus_button.select_all_none) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, bottom_buttons, NUM_BOTTOM_BUTTONS, &data.focus_button.bottom) ||
        grid_box_handle_input(data.focused_grid_box, m_dialog, 1)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_ok(0);
    }
}

static void button_repeat_type(const generic_button *button)
{
    int repeat_type = button->parameter1;
    if (data.repeat_type == repeat_type) {
        return;
    }
    data.repeat_type = repeat_type;
    window_request_refresh();
}

static void set_repeat_times(int value)
{
    data.event->max_number_of_repeats = value;
}

static void button_repeat_times(const generic_button *button)
{
    data.repeat_type = EVENT_REPEAT_TIMES;
    stop_input();
    window_numeric_input_bound_show(0, 0, button,
        3, 1, 999, set_repeat_times);
}

static void set_repeat_interval_min(int value)
{
    data.event->repeat_months_min = value;
    if (data.event->repeat_months_max < value) {
        data.event->repeat_months_max = value;
    }
}

static void set_repeat_interval_max(int value)
{
    data.event->repeat_months_max = value;
    if (data.event->repeat_months_min > value) {
        data.event->repeat_months_min = value;
    }
}

static void button_repeat_between(const generic_button *button)
{
    int amount_type = button->parameter1;
    if (data.repeat_type == EVENT_REPEAT_NEVER) {
        return;
    }

    stop_input();
    window_numeric_input_bound_show(0, 0, button,
        2, 1, 999, amount_type == REPEAT_MIN ? set_repeat_interval_min : set_repeat_interval_max);
}

static void select_all_conditions(void)
{
    for (unsigned int i = 0; i < data.conditions.active; i++) {
        data.conditions.selected[i] = 1;
    }
    data.conditions.selection_type = CHECKBOX_ALL_SELECTED;
}

static void select_all_actions(void)
{
    for (unsigned int i = 0; i < data.actions.active; i++) {
        data.actions.selected[i] = 1;
    }
    data.actions.selection_type = CHECKBOX_ALL_SELECTED;
}

static void button_select_all_none(const generic_button *button)
{
    unsigned int select_type = button->parameter1;
    if (select_type == SELECT_CONDITIONS) {
        if (data.conditions.active == 0) {
            return;
        }
        if (data.conditions.selection_type == CHECKBOX_ALL_SELECTED) {
            select_no_conditions();
        } else {
            select_all_conditions();
        }
    } else {
        if (data.actions.active == 0) {
            return;
        }
        if (data.actions.selection_type == CHECKBOX_ALL_SELECTED) {
            select_no_actions();
        } else {
            select_all_actions();
        }
    }
    window_request_refresh();
}

static void pack_and_clear_conditions(scenario_condition_group_t *group)
{
    array_pack(group->conditions);
    if (group->type == FULFILLMENT_TYPE_ANY && group->conditions.size == 0) {
        array_clear(group->conditions);
    }
}

static void set_selected_to_group(int group_id)
{
    scenario_condition_group_t *group;
    // New group
    if (group_id >= data.event->condition_groups.size) {
        group = array_advance(data.event->condition_groups);
        if (!group) {
            log_error("Unable to create new group - memory full. The game will probably crash", 0, 0);
            return;
        }
    } else {
        group = array_item(data.event->condition_groups, group_id);
    }
    for (unsigned int i = 0; i < data.conditions.active; i++) {
        if (!data.conditions.selected[i]) {
            continue;
        }
        // Ignore conditions that are already in the group
        if (data.conditions.list[i].group_id == group_id || !data.conditions.list[i].condition) {
            continue;
        }
        scenario_condition_t *condition = array_advance(group->conditions);
        if (!condition) {
            log_error("Unable to add condition to group - memory full. The game will probably crash", 0, 0);
            return;
        }
        *condition = *data.conditions.list[i].condition;
        data.conditions.list[i].condition->type = CONDITION_TYPE_UNDEFINED;
    }
    array_foreach_callback(data.event->condition_groups, pack_and_clear_conditions);
    array_pack(data.event->condition_groups);
    update_groups();
    select_no_conditions();
    window_request_refresh();
}

static void button_set_selected_to_group(const generic_button *button)
{
    if (data.conditions.selection_type == CHECKBOX_NO_SELECTION) {
        return;
    }
    stop_input();
    window_select_list_show_text(screen_dialog_offset_x(), screen_dialog_offset_y(), button,
        (const uint8_t **) data.conditions.groups.names, data.conditions.groups.available,
        set_selected_to_group);
}

static void button_add_new_condition(const generic_button *button)
{
    condition_types type = CONDITION_TYPE_TIME_PASSED;
    scenario_condition_t *condition = scenario_event_condition_create(array_item(data.event->condition_groups, 0), type);
    condition->parameter1 = 1;
    select_no_conditions();
    window_request_refresh();
}

static void delete_selected(int is_ok, int checked)
{
    if (!is_ok) {
        return;
    }
    if (checked) {
        data.do_not_ask_again_for_delete = 1;
    }
    if (data.conditions.selection_type != CHECKBOX_NO_SELECTION) {
        for (unsigned int i = 0; i < data.conditions.active; i++) {
            if (!data.conditions.selected[i] || !data.conditions.list[i].condition) {
                continue;
            }
            data.conditions.list[i].condition->type = CONDITION_TYPE_UNDEFINED;
        }
        array_foreach_callback(data.event->condition_groups, pack_and_clear_conditions);
        array_pack(data.event->condition_groups);
        update_groups();
    }
    if (data.actions.selection_type != CHECKBOX_NO_SELECTION) {
        for (unsigned int i = 0; i < data.actions.active; i++) {
            if (!data.actions.selected[i]) {
                continue;
            }
            data.actions.list[i]->type = ACTION_TYPE_UNDEFINED;
        }
        array_pack(data.event->actions);
    }
    select_no_conditions();
    select_no_actions();
    window_request_refresh();
}

static void button_delete_selected(const generic_button *button)
{
    if (data.conditions.selection_type == CHECKBOX_NO_SELECTION &&
        data.actions.selection_type == CHECKBOX_NO_SELECTION) {
        return;
    }
    if (!data.do_not_ask_again_for_delete) {
        const uint8_t *title = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED_CONFIRM_TITLE);
        const uint8_t *text = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED_CONFIRM_TEXT);
        const uint8_t *check_text = lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DO_NOT_ASK_AGAIN);
        stop_input();
        window_popup_dialog_show_confirmation(title, text, check_text, delete_selected);
    } else {
        delete_selected(1, 1);
    }
}

static void button_add_new_action(const generic_button *button)
{
    action_types type = ACTION_TYPE_ADJUST_FAVOR;
    scenario_event_action_create(data.event, type);
    select_no_actions();
    window_request_refresh();
}

static void button_delete_event(const generic_button *button)
{
    scenario_event_delete(data.event);
    stop_input();
    window_go_back();
}

static void set_repeat_type(void)
{
    if (data.repeat_type == EVENT_REPEAT_NEVER) {
        data.event->repeat_months_min = 0;
        data.event->repeat_months_max = 0;
    } else if (data.repeat_type == EVENT_REPEAT_FOREVER) {
        data.event->max_number_of_repeats = 0;
    }
}

static void button_ok(const generic_button *button)
{
    stop_input();
    set_repeat_type();
    window_go_back();
}

static void update_conditions_selection_type(void)
{
    uint8_t some_selected = 0;
    uint8_t all_selected = 1;
    for (unsigned int i = 0; i < data.conditions.active; i++) {
        if (!data.conditions.list[i].condition) {
            continue;
        }
        some_selected |= data.conditions.selected[i];
        all_selected &= data.conditions.selected[i];
        if (some_selected != all_selected) {
            data.conditions.selection_type = CHECKBOX_SOME_SELECTED;
            return;
        }
    }
    data.conditions.selection_type = some_selected ? CHECKBOX_ALL_SELECTED : CHECKBOX_NO_SELECTION;
}

static void update_actions_selection_type(void)
{
    uint8_t some_selected = 0;
    uint8_t all_selected = 1;
    for (unsigned int i = 0; i < data.actions.active; i++) {
        some_selected |= data.actions.selected[i];
        all_selected &= data.actions.selected[i];
        if (some_selected != all_selected) {
            data.actions.selection_type = CHECKBOX_SOME_SELECTED;
            return;
        }
    }
    data.actions.selection_type = some_selected ? CHECKBOX_ALL_SELECTED : CHECKBOX_NO_SELECTION;
}

static void click_condition_button(const grid_box_item *item)
{
    const condition_list_item *list_item = &data.conditions.list[item->index];
    if (!list_item->condition) {
        return;
    }

    if (list_item->condition->type != CONDITION_TYPE_UNDEFINED) {
        if (item->mouse.x < 20) {
            if (data.conditions.selected) {
                data.conditions.selected[item->index] ^= 1;
                update_conditions_selection_type();
                window_request_refresh();
            }
        } else {
            stop_input();
            window_editor_scenario_condition_edit_show(list_item->condition);
        }
    }
}

static void click_action_button(const grid_box_item *item)
{
    scenario_action_t *action = data.actions.list[item->index];
    if (action->type != ACTION_TYPE_UNDEFINED) {
        if (item->mouse.x < 20) {
            if (data.actions.selected) {
                data.actions.selected[item->index] ^= 1;
                update_actions_selection_type();
                window_request_refresh();
            }
        } else {
            stop_input();
            window_editor_scenario_action_edit_show(action);
        }
    }
}

static void handle_condition_tooltip(const grid_box_item *item, tooltip_context *c)
{
    if (item->mouse.x < 24) {
        return;
    }
    const condition_list_item *list_item = &data.conditions.list[item->index];
    if (!list_item->condition) {
        return;
    }
    static uint8_t text[MAX_TEXT_LENGTH * 2];
    scenario_events_parameter_data_get_display_string_for_condition(list_item->condition, text, MAX_TEXT_LENGTH * 2);
    if (text_get_width(text, FONT_NORMAL_BLACK) > item->width - 32) {
        c->precomposed_text = text;
        c->type = TOOLTIP_BUTTON;
    }
}

static void handle_action_tooltip(const grid_box_item *item, tooltip_context *c)
{
    const scenario_action_t *action = data.actions.list[item->index];
    if (action->type == ACTION_TYPE_UNDEFINED) {
        return;
    }
    static uint8_t text[MAX_TEXT_LENGTH * 2];
    scenario_events_parameter_data_get_display_string_for_action(action, text, MAX_TEXT_LENGTH * 2);
    if (text_get_width(text, FONT_NORMAL_BLACK) > item->width - 32) {
        c->precomposed_text = text;
        c->type = TOOLTIP_BUTTON;
    }
}

static void handle_check_all_none_tooltip(tooltip_context *c)
{
    if (data.focus_button.select_all_none == 1) {
        if (data.conditions.active > 0) {
            c->precomposed_text = lang_get_string(CUSTOM_TRANSLATION,
                data.conditions.selection_type == CHECKBOX_ALL_SELECTED ? TR_SELECT_NONE : TR_SELECT_ALL);
            c->type = TOOLTIP_BUTTON;
        }
    } else if (data.focus_button.select_all_none == 2) {
        if (data.actions.active > 0) {
            c->precomposed_text = lang_get_string(CUSTOM_TRANSLATION,
                data.actions.selection_type == CHECKBOX_ALL_SELECTED ? TR_SELECT_NONE : TR_SELECT_ALL);
            c->type = TOOLTIP_BUTTON;
        }
    }
}

static void get_tooltip(tooltip_context *c)
{
    handle_check_all_none_tooltip(c);
    if (c->type == TOOLTIP_NONE) {
        if (data.focus_button.top == 7) {
            c->text_group = CUSTOM_TRANSLATION;
            c->text_id = TR_EDITOR_SCENARIO_EVENTS_GROUPS_EXPLANATION_TOOLTIP;
            c->type = TOOLTIP_BUTTON;
        } else {
            grid_box_handle_tooltip(data.focused_grid_box, c);
        }
    }
}

static void on_return(window_id from)
{
    start_input();
}

void window_editor_scenario_event_details_show(int event_id)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_EVENT_DETAILS,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip,
        on_return
    };
    init(event_id);
    window_show(&window);
}
