#include "custom_variables.h"

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
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/custom_variable.h"
#include "scenario/event/controller.h"
#include "scenario/message_media_text_blob.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"
#include "window/text_input.h"

#define CHECKBOX_ROW_WIDTH 25
#define ID_ROW_WIDTH 32
#define VALUE_ROW_WIDTH 120
#define BUTTONS_PADDING 4
#define NUM_ITEM_BUTTONS (sizeof(item_buttons) / sizeof(generic_button))
#define NUM_CONSTANT_BUTTONS (sizeof(constant_buttons) / sizeof(generic_button))

#define NO_SELECTION (unsigned int) -1

typedef enum {
    CHECKBOX_NO_SELECTION = 0,
    CHECKBOX_SOME_SELECTED = 1,
    CHECKBOX_ALL_SELECTED = 2
} checkbox_selection_type;

static void button_variable_checkbox(const generic_button *button);
static void button_edit_variable_name(const generic_button *button);
static void button_edit_variable_value(const generic_button *button);

static void button_select_all_none(const generic_button *button);
static void button_delete_selected(const generic_button *button);
static void button_new_variable(const generic_button *button);
static void button_ok(const generic_button *button);

static void variable_item_click(const grid_box_item *item);
static void draw_variable_item(const grid_box_item *item);

static struct {
    unsigned int constant_button_focus_id;
    unsigned int item_buttons_focus_id;

    unsigned int target_index;

    unsigned int *custom_variable_ids;
    unsigned int total_custom_variables;
    unsigned int custom_variables_in_use;

    uint8_t *selected;
    checkbox_selection_type selection_type;
    int do_not_ask_again_for_delete;

    void (*callback)(unsigned int id);
} data;

static generic_button item_buttons[] = {
    { 1, 2, 20, 20, button_variable_checkbox },
    { 0, 0, 0, 25, button_edit_variable_name },
    { 0, 0, VALUE_ROW_WIDTH, 25, button_edit_variable_value }
};

static generic_button constant_buttons[] = {
    { 32, 91, 20, 20, button_select_all_none },
    { 32, 454, 200, 30, button_delete_selected, 0 },
    { 237, 454, 200, 30, button_new_variable },
    { 442, 454, 200, 30, button_ok }
};

static grid_box_type variable_buttons = {
    .x = 26,
    .y = 116,
    .width = 38 * BLOCK_SIZE,
    .height = 21 * BLOCK_SIZE,
    .num_columns = 1,
    .item_height = 30,
    .item_margin.horizontal = 10,
    .item_margin.vertical = 5,
    .extend_to_hidden_scrollbar = 1,
    .on_click = variable_item_click,
    .draw_item = draw_variable_item
};

static void select_all_to(uint8_t value)
{
    if (data.selected) {
        memset(data.selected, value, data.custom_variables_in_use * sizeof(uint8_t));
    }
    data.selection_type = value ? CHECKBOX_ALL_SELECTED : CHECKBOX_NO_SELECTION;
}

static void populate_list(void)
{
    data.target_index = NO_SELECTION;

    unsigned int total_custom_variables = scenario_custom_variable_count();
    if (total_custom_variables > data.total_custom_variables) {
        free(data.custom_variable_ids);
        free(data.selected);
        data.custom_variable_ids = calloc(total_custom_variables, sizeof(unsigned int));
        if (!data.custom_variable_ids) {
            data.total_custom_variables = 0;
            data.custom_variables_in_use = 0;
            data.selected = 0;
            log_error("Failed to allocate memory for custom variable list", 0, 0);
            return;
        }
        data.total_custom_variables = total_custom_variables;
        data.selected = calloc(total_custom_variables, sizeof(uint8_t));
    }
    data.custom_variables_in_use = 0;
    for (unsigned int i = 0; i < data.total_custom_variables; i++) {
        if (!scenario_custom_variable_exists(i)) {
            continue;
        }
        data.custom_variable_ids[data.custom_variables_in_use] = i;
        data.custom_variables_in_use++;
    }

    select_all_to(0);
}

static void init(void (*callback)(unsigned int id))
{
    data.callback = callback;
    populate_list();
    grid_box_init(&variable_buttons, data.custom_variables_in_use);
}

static void update_item_buttons_positions(void)
{
    if (data.callback) {
        item_buttons[1].x = ID_ROW_WIDTH;
        item_buttons[1].width = variable_buttons.width - variable_buttons.item_margin.horizontal - ID_ROW_WIDTH;
        if (grid_box_has_scrollbar(&variable_buttons)) {
            item_buttons[1].width -= 2 * BLOCK_SIZE;
        }
        return;
    }
    item_buttons[1].x = CHECKBOX_ROW_WIDTH + ID_ROW_WIDTH;
    item_buttons[1].width = variable_buttons.width - variable_buttons.item_margin.horizontal - item_buttons[1].x -
        VALUE_ROW_WIDTH - BUTTONS_PADDING;
    if (grid_box_has_scrollbar(&variable_buttons)) {
        item_buttons[1].width -= 2 * BLOCK_SIZE;
    }

    item_buttons[2].x = item_buttons[1].x + item_buttons[1].width + BUTTONS_PADDING;
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    update_item_buttons_positions();

    graphics_in_dialog();

    outer_panel_draw(16, 16, 40, data.callback ? 28 : 30);

    text_draw_centered(translation_for(TR_EDITOR_CUSTOM_VARIABLES_TITLE), 0, 32, 640, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number_centered(translation_for(TR_EDITOR_CUSTOM_VARIABLES_COUNT), data.custom_variables_in_use,
        "", 0, 70, 640, FONT_NORMAL_BLACK, 0);

    int base_x_offset = variable_buttons.x + variable_buttons.item_margin.horizontal / 2;

    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_ID,
        variable_buttons.x + (data.callback ? 0 : CHECKBOX_ROW_WIDTH), 96, 40, FONT_SMALL_PLAIN);
    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_NAME, base_x_offset + item_buttons[1].x, 96,
        FONT_SMALL_PLAIN);

    grid_box_request_refresh(&variable_buttons);

    if (data.callback) {
        graphics_reset_dialog();
        return;
    }

    // Checkmarks for select all/none button
    int checkmark_id = assets_lookup_image_id(ASSET_UI_SELECTION_CHECKMARK);
    const image *img = image_get(checkmark_id);
    const generic_button *select_all_none_button = &constant_buttons[0];
    if (data.selection_type == CHECKBOX_SOME_SELECTED) {
        text_draw(string_from_ascii("-"), select_all_none_button->x + 8, select_all_none_button->y + 4,
            FONT_NORMAL_BLACK, 0);
    } else if (data.selection_type == CHECKBOX_ALL_SELECTED) {
        image_draw(checkmark_id, select_all_none_button->x + (20 - img->original.width) / 2,
             select_all_none_button->y + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    lang_text_draw(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_VALUE, base_x_offset + item_buttons[2].x, 96,
        FONT_SMALL_PLAIN);

    // Bottom buttons
    const generic_button *delete_selected_button = &constant_buttons[1];
    color_t color = data.selection_type == CHECKBOX_NO_SELECTION ? COLOR_FONT_LIGHT_GRAY : COLOR_RED;
    lang_text_draw_centered_colored(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED,
        delete_selected_button->x, delete_selected_button->y + 9, delete_selected_button->width,
        FONT_NORMAL_PLAIN, color);
    const generic_button *new_variable_button = &constant_buttons[2];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_NEW,
        new_variable_button->x, new_variable_button->y + 9, new_variable_button->width, FONT_NORMAL_BLACK);
    lang_text_draw_centered(18, 3, constant_buttons[3].x, constant_buttons[3].y + 9, constant_buttons[3].width,
        FONT_NORMAL_BLACK);

    graphics_reset_dialog();
}

static void draw_variable_item(const grid_box_item *item)
{
    unsigned int id = data.custom_variable_ids[item->index];
    const uint8_t *name = scenario_custom_variable_get_name(id);
    int value = scenario_custom_variable_get_value(id);

    // Variable ID
    text_draw_number_centered(id, item->x + (data.callback ? 0 : CHECKBOX_ROW_WIDTH), item->y + 8,
        32, FONT_NORMAL_BLACK);

    // Variable Name
    button_border_draw(item->x + item_buttons[1].x, item->y + item_buttons[1].y,
        item_buttons[1].width, item_buttons[1].height, item->is_focused && data.item_buttons_focus_id == 2);

    if (name && *name) {
        text_draw(name, item->x + item_buttons[1].x + 8, item->y + item_buttons[1].y + 8,
            FONT_NORMAL_BLACK, COLOR_MASK_NONE);
    }

    if (data.callback) {
        return;
    }

    // Checkbox
    button_border_draw(item->x + item_buttons[0].x, item->y + item_buttons[0].y, item_buttons[0].width,
        item_buttons[0].height, item->is_focused && data.item_buttons_focus_id == 1);

    if (data.selected && data.selected[item->index]) {
        int checkmark_id = assets_lookup_image_id(ASSET_UI_SELECTION_CHECKMARK);
        const image *img = image_get(checkmark_id);
        image_draw(checkmark_id, item->x + item_buttons[0].x + (20 - img->original.width) / 2,
            item->y + item_buttons[0].y + (20 - img->original.height) / 2, COLOR_MASK_NONE, SCALE_NONE);
    }

    // Variable Value
    button_border_draw(item->x + item_buttons[2].x, item->y + item_buttons[2].y, item_buttons[2].width,
        item_buttons[2].height, item->is_focused && data.item_buttons_focus_id == 3);

    text_draw_number(value, ' ', "", item->x + item_buttons[2].x + 8, item->y + item_buttons[2].y + 8,
        FONT_NORMAL_BLACK, COLOR_MASK_NONE);
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    grid_box_draw(&variable_buttons);

    if (data.callback) {
        graphics_reset_dialog();
        return;
    }

    for (unsigned int i = 0; i < NUM_CONSTANT_BUTTONS; i++) {
        int focus = data.constant_button_focus_id == i + 1;
        if ((i == 0 && data.custom_variables_in_use == 0) || (i == 1 && data.selection_type == CHECKBOX_NO_SELECTION)) {
            focus = 0;
        }
        button_border_draw(constant_buttons[i].x, constant_buttons[i].y, constant_buttons[i].width,
            constant_buttons[i].height, focus);
    }

    graphics_reset_dialog();
}

static void button_select_all_none(const generic_button *button)
{
    if (!data.custom_variables_in_use || data.callback) {
        return;
    }
    if (data.selection_type != CHECKBOX_ALL_SELECTED) {
        select_all_to(1);
    } else {
        select_all_to(0);
    }
    window_request_refresh();
}

static void update_selection_type(void)
{
    uint8_t some_selected = 0;
    uint8_t all_selected = 1;
    for (unsigned int i = 0; i < data.custom_variables_in_use; i++) {
        some_selected |= data.selected[i];
        all_selected &= data.selected[i];
        if (some_selected != all_selected) {
            data.selection_type = CHECKBOX_SOME_SELECTED;
            return;
        }
    }
    data.selection_type = some_selected ? CHECKBOX_ALL_SELECTED : CHECKBOX_NO_SELECTION;
}

static void button_variable_checkbox(const generic_button *button)
{
    if (!data.selected || data.callback) {
        return;
    }
    data.selected[data.target_index] ^= 1;
    update_selection_type();
    window_request_refresh();
}

static void create_new_variable(const uint8_t *name)
{
    unsigned int id = scenario_custom_variable_create(name, 0);
    if (!id) {
        log_error("There was an error creating the new variable - out of memory", 0, 0);
        return;
    }
    populate_list();
    grid_box_update_total_items(&variable_buttons, data.custom_variables_in_use);
    for (unsigned int i = 0; i < data.custom_variables_in_use; i++) {
        if (data.custom_variable_ids[i] == id) {
            grid_box_show_index(&variable_buttons, i);
            break;
        }
    }
    window_request_refresh();
}

static int check_valid_name(const uint8_t *name)
{
    if (!name || !*name) {
        return 0;
    }
    for (unsigned int i = 0; i < data.custom_variables_in_use; i++) {
        if (data.target_index == i) {
            continue;
        }
        if (string_equals(name, scenario_custom_variable_get_name(data.custom_variable_ids[i]))) {
            return 0;
        }
    }
    return 1;
}

static void set_variable_name(const uint8_t *name)
{
    if (!check_valid_name(name)) {
        window_plain_message_dialog_show(TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_SET_NAME_TITLE,
            TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_SET_NAME_TEXT, 1);
        return;
    }
    // New variable
    if (data.target_index == NO_SELECTION) {
        create_new_variable(name);
        return;
    }
    scenario_custom_variable_rename(data.custom_variable_ids[data.target_index], name);
    data.target_index = NO_SELECTION;
}

static void show_name_edit_popup(void)
{
    const uint8_t *title;
    const uint8_t *name = 0;
    if (data.target_index != NO_SELECTION) {
        static uint8_t text_input_title[100];
        uint8_t *cursor = string_copy(translation_for(TR_PARAMETER_TYPE_CUSTOM_VARIABLE), text_input_title, 100);
        cursor = string_copy(string_from_ascii(" "), cursor, 100 - (cursor - text_input_title));
        unsigned int id = data.custom_variable_ids[data.target_index];
        string_from_int(cursor, id, 0);
        title = text_input_title;
        name = scenario_custom_variable_get_name(id);
    } else {
        title = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_NEW);
    }

    window_text_input_show(title, 0, name, CUSTOM_VARIABLE_NAME_LENGTH, set_variable_name);
}

static void button_edit_variable_name(const generic_button *button)
{
    if (data.callback) {
        return;
    }
    show_name_edit_popup();
}

static void set_variable_value(int value)
{
    scenario_custom_variable_set_value(data.custom_variable_ids[data.target_index], value);
    data.target_index = NO_SELECTION;
}

static void button_edit_variable_value(const generic_button *button)
{
    if (data.callback) {
        return;
    }
    window_numeric_input_bound_show(variable_buttons.focused_item.x, variable_buttons.focused_item.y, button,
        9, -1000000000, 1000000000, set_variable_value);
}

static void variable_item_click(const grid_box_item *item)
{
    unsigned int id = data.custom_variable_ids[item->index];

    if (data.callback) {
        if (item->mouse.x >= ID_ROW_WIDTH) {
            data.callback(id);
            data.target_index = NO_SELECTION;
        }
        window_go_back();
        return;
    }
    data.target_index = item->index;
}

static void show_used_event_sigle_variable_popup_dialog(const scenario_event_t *event)
{
    static uint8_t event_id_text[50];
    uint8_t *cursor = string_copy(translation_for(TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_CHANGE_EVENT_ID),
        event_id_text, 50);
    string_from_int(cursor, event->id, 0);
    window_plain_message_dialog_show_with_extra(TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_CHANGE_TITLE,
        TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_CHANGE_TEXT, 0, event_id_text);
}

static void show_multiple_variables_in_use_popup_dialog(unsigned int *variables_in_use, unsigned int total)
{
    static uint8_t event_id_text[200];
    uint8_t *cursor = string_copy(translation_for(TR_EDITOR_CUSTOM_VARIABLES_IN_USE),
        event_id_text, 200);
    cursor += string_from_int(cursor, variables_in_use[0], 0);
    for (unsigned int i = 1; i < total; i++) {
        cursor = string_copy(string_from_ascii(", "), cursor, 200 - (cursor - event_id_text));
        if (cursor - event_id_text > 188) {
            cursor = string_copy(string_from_ascii("..."), cursor, 200 - (cursor - event_id_text));
            break;
        }
        cursor += string_from_int(cursor, variables_in_use[i], 0);
    }
    window_plain_message_dialog_show_with_extra(TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_CHANGE_TITLE,
        TR_EDITOR_CUSTOM_VARIABLE_UNABLE_TO_CHANGE_TEXT, 0, event_id_text);
}

static void delete_selected(int is_ok, int checked)
{
    if (!is_ok) {
        return;
    }
    if (checked) {
        data.do_not_ask_again_for_delete = 1;
    }

    for (unsigned int i = 0; i < data.custom_variables_in_use; i++) {
        if (!data.selected[i]) {
            continue;
        }
        unsigned int id = data.custom_variable_ids[i];
        if (scenario_custom_variable_exists(id)) {
            scenario_custom_variable_delete(id);
        }
    }
    populate_list();
    grid_box_update_total_items(&variable_buttons, data.custom_variables_in_use);
    window_request_refresh();
}

static void button_delete_selected(const generic_button *button)
{
    if (data.callback || !data.selected || data.selection_type == CHECKBOX_NO_SELECTION) {
        return;
    }

    unsigned int *variables_in_use = calloc(data.custom_variables_in_use, sizeof(unsigned int));
    if (!variables_in_use) {
        log_error("Failed to allocate memory for custom variable list", 0, 0);
        return;
    }
    unsigned int total_variables_in_use = 0;
    const scenario_event_t *event = 0;

    // Step 1: Check if any of the selected variables are used in events
    for (unsigned int i = 0; i < data.custom_variables_in_use; i++) {
        if (!data.selected[i]) {
            continue;
        }
        event = scenario_events_get_using_custom_variable(data.custom_variable_ids[i]);
        if (!event) {
            continue;
        }
        variables_in_use[total_variables_in_use] = data.custom_variable_ids[i];
        total_variables_in_use++;
    }

    // If any variables are in use, alert user and abort
    if (total_variables_in_use) {
        if (total_variables_in_use == 1) {
            show_used_event_sigle_variable_popup_dialog(event);
        } else {
            show_multiple_variables_in_use_popup_dialog(variables_in_use, total_variables_in_use);
        }
        free(variables_in_use);
        return;
    }

    free(variables_in_use);

    // Step 2: Request confirmation
    if (!data.do_not_ask_again_for_delete) {
        const uint8_t *title = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED_CONFIRM_TITLE);
        const uint8_t *text = lang_get_string(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_DELETE_SELECTED_CONFIRM_TEXT);
        const uint8_t *check_text = lang_get_string(CUSTOM_TRANSLATION, TR_SAVE_DIALOG_OVERWRITE_FILE_DO_NOT_ASK_AGAIN);
        window_popup_dialog_show_confirmation(title, text, check_text, delete_selected);
    } else {
        delete_selected(1, 1);
    }
}

static void button_new_variable(const generic_button *button)
{
    if (data.callback) {
        return;
    }
    data.target_index = NO_SELECTION;
    show_name_edit_popup();
}

static void button_ok(const generic_button *button)
{
    if (data.callback) {
        return;
    }
    window_go_back();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (grid_box_handle_input(&variable_buttons, m_dialog, 1)) {
        if (data.callback) {
            return;
        }
    }
    int x = 0, y = 0;
    if (variable_buttons.focused_item.is_focused) {
        x = variable_buttons.focused_item.x;
        y = variable_buttons.focused_item.y;
    }
    if (generic_buttons_handle_mouse(m_dialog, x, y, item_buttons, NUM_ITEM_BUTTONS, &data.item_buttons_focus_id) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, constant_buttons, NUM_CONSTANT_BUTTONS,
            &data.constant_button_focus_id)) {
        return;
    }

    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void get_tooltip(tooltip_context *c)
{
    if (data.callback || !data.selected || !data.custom_variables_in_use || data.constant_button_focus_id != 1) {
        return;
    }
    c->precomposed_text = lang_get_string(CUSTOM_TRANSLATION,
        data.selection_type == CHECKBOX_ALL_SELECTED ? TR_SELECT_NONE : TR_SELECT_ALL);
    c->type = TOOLTIP_BUTTON;
}

void window_editor_custom_variables_show(void (*callback)(unsigned int id))
{
    window_type window = {
        WINDOW_EDITOR_CUSTOM_VARIABLES,
        draw_background,
        draw_foreground,
        handle_input,
        get_tooltip
    };
    init(callback);
    window_show(&window);
}
