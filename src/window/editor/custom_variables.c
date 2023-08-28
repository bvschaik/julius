#include "custom_variables.h"

#include "core/string.h"
#include "editor/editor.h"
#include "graphics/button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/message_media_text_blob.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/numeric_input.h"
#include "window/text_input.h"

#define BUTTONS_X_OFFSET_NAME 48
#define BUTTONS_Y_OFFSET 146
#define BUTTONS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 8
#define BUTTON_WIDTH 560
#define BUTTON_WIDTH_VALUE 96
#define BUTTON_WIDTH_NAME (BUTTON_WIDTH - BUTTON_WIDTH_VALUE)
#define BUTTONS_X_OFFSET_VALUE (BUTTONS_X_OFFSET_NAME + BUTTON_WIDTH_NAME)


static void on_scroll(void);
static void button_variable(int button_index, int param2);
static void button_name_click(int button_index, int param2);
static void button_delete_variable(int button_index, int param2);
static void populate_list(int offset);

static scrollbar_type scrollbar = {
    608, BUTTONS_Y_OFFSET, BUTTONS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH - 16, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (0 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 0, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (0 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 0, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (1 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 1, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (1 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 1, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (2 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 2, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (2 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 2, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (3 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 3, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (3 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 3, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (4 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 4, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (4 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 4, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (5 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 5, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (5 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 5, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (6 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 6, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (6 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 6, 0},
    {BUTTONS_X_OFFSET_NAME, BUTTONS_Y_OFFSET + (7 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_NAME, BUTTONS_ROW_HEIGHT - 2, button_name_click, button_delete_variable, 7, 1},
    {BUTTONS_X_OFFSET_VALUE, BUTTONS_Y_OFFSET + (7 * BUTTONS_ROW_HEIGHT), BUTTON_WIDTH_VALUE, BUTTONS_ROW_HEIGHT - 2, button_variable, button_none, 7, 0}
};

#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    int focus_button_id;

    int target_variable;
    custom_variable_t *list[MAX_VISIBLE_ROWS];

    void (*callback)(custom_variable_t *);
    int select_only;
} data;

static void init(int select_only)
{
    data.select_only = select_only;
    populate_list(0);
    scrollbar_init(&scrollbar, 0, MAX_CUSTOM_VARIABLES);
}

static void populate_list(int offset)
{
    // Ensure we dont offset past the end or beginning of the list.
    if (offset + MAX_VISIBLE_ROWS > MAX_CUSTOM_VARIABLES) {
        offset = MAX_CUSTOM_VARIABLES - MAX_VISIBLE_ROWS;
    }
    if (offset < 1) {
        offset = 1;
    }
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        int target_id = i + offset;
        if (target_id <= MAX_CUSTOM_VARIABLES) {
            data.list[i] = scenario_get_custom_variable(target_id);
        } else {
            data.list[i] = 0;
        }
    }
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 40, 30);

    text_draw_centered(translation_for(TR_EDITOR_CUSTOM_VARIABLES_TITLE), 48, 58, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_CUSTOM_VARIABLES_COUNT), MAX_CUSTOM_VARIABLES, "", 48, 106, FONT_NORMAL_PLAIN, COLOR_BLACK);

    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        int j = (i * 2);
        if (data.list[i]) {
            large_label_draw(buttons[j].x, buttons[j].y, buttons[j].width / BLOCK_SIZE,
                data.focus_button_id == j + 1 && editor_is_active() ? 1 : 0);

            text_draw_label_and_number(0, data.list[i]->id, "", buttons[j].x, buttons[j].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
            if (data.list[i] && data.list[i]->linked_uid && data.list[i]->linked_uid->text) {
                text_draw_centered(data.list[i]->linked_uid->text, buttons[j].x + 52, buttons[j].y + 8, 150, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
            }
            
            if (data.list[i]->in_use && !data.select_only) {
                large_label_draw(buttons[j+1].x, buttons[j+1].y, buttons[j+1].width / BLOCK_SIZE,
                    data.focus_button_id == j + 2 ? 1 : 0);

                text_draw_number(data.list[i]->value, ' ', "", buttons[j+1].x, buttons[j+1].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
            }
        }
    }

    lang_text_draw_centered(13, 3, 48, 450, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void set_variable_value(int value)
{
    scenario_set_custom_variable_value(data.target_variable, value);
}

static void button_variable(int button_index, int param2)
{
    if (data.select_only) {
        return;
    }
    if (!data.list[button_index]) {
        return;
    };
    data.target_variable = data.list[button_index]->id;
    window_numeric_input_bound_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, 9, -1000000000, 1000000000, set_variable_value);
}

static void set_variable_name(char *value)
{
    scenario_custom_variable_rename(data.target_variable, string_from_ascii(value));
}

static void button_name_click(int button_index, int param2)
{
    if (!editor_is_active()) {
        return;
    }
    if (!data.list[button_index]) {
        return;
    };
    int has_name = data.list[button_index]->linked_uid && data.list[button_index]->linked_uid->in_use;
    if (data.select_only) {
        if (!has_name) {
            return;
        }
        data.callback(data.list[button_index]);
        window_go_back();
    } else {
        data.target_variable = data.list[button_index]->id;
        window_text_input_show(screen_dialog_offset_x() + 60, screen_dialog_offset_y() + 50, set_variable_name);
    }
}

static void button_delete_variable(int button_index, int param2)
{
    if (!editor_is_active()) {
        return;
    }
    if (data.select_only) {
        return;
    }
    if (!data.list[button_index]) {
        return;
    };
    const uint8_t empty_name[2] = "";
    scenario_custom_variable_rename(data.list[button_index]->id, empty_name);
    scenario_custom_toggle_in_use(data.list[button_index]->id);
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

void window_editor_custom_variables_show(void)
{
    window_type window = {
        WINDOW_EDITOR_CUSTOM_VARIABLES,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(0);
    window_show(&window);
}

void window_editor_custom_variables_select_show(void (*callback)(custom_variable_t *))
{
    window_type window = {
        WINDOW_EDITOR_CUSTOM_VARIABLES,
        draw_background,
        draw_foreground,
        handle_input
    };
    data.callback = callback;
    init(1);
    window_show(&window);
}
