#include "select_special_attribute_mapping.h"

#include "core/lang.h"
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
#include "window/editor/map.h"
#include "window/numeric_input.h"

#define MAX_BUTTONS 13
#define BUTTON_LEFT_PADDING 32
#define BUTTON_WIDTH 608
#define DETAILS_Y_OFFSET 64
#define DETAILS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 13

static void on_scroll(void);
static void button_click(int param1, int param2);

static scrollbar_type scrollbar = {
    640, DETAILS_Y_OFFSET, DETAILS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
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
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (10 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 10, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (11 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 11, 0},
    {BUTTON_LEFT_PADDING, DETAILS_Y_OFFSET + (12 * DETAILS_ROW_HEIGHT), BUTTON_WIDTH, DETAILS_ROW_HEIGHT - 2, button_click, button_none, 12, 0}
};

static struct {
    int focus_button_id;
    int list_size;
    int parameter_type;
    void (*callback)(int);

    int current_value;
    const uint8_t *heading_text;

    special_attribute_mapping_t *list[MAX_VISIBLE_ROWS];
} data;

static void populate_list(int offset)
{
    if (data.list_size - offset < MAX_VISIBLE_ROWS) {
        offset = data.list_size - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        int target_index = i + offset;
        if (target_index < data.list_size) {
            data.list[i] = scenario_events_parameter_data_get_attribute_mapping(data.parameter_type, target_index);
        }
    }
}

static const uint8_t *get_display_string(special_attribute_mapping_t *entry)
{
    switch (entry->type) {
        case PARAMETER_TYPE_BUILDING:
        case PARAMETER_TYPE_BUILDING_COUNTING:
            if (entry->key == TR_PARAMETER_VALUE_DYNAMIC_RESOLVE) {
                return lang_get_string(28, entry->value);
            } else {
                return translation_for(entry->key);
            }
            break;
        case PARAMETER_TYPE_ALLOWED_BUILDING:
            if (entry->key == TR_PARAMETER_VALUE_DYNAMIC_RESOLVE) {
                return lang_get_string(67, entry->value);
            } else {
                return translation_for(entry->key);
            }
            break;
        default:
            return translation_for(entry->key);
    }
}

static void set_heading_display_text(void)
{
    special_attribute_mapping_t *mapping = scenario_events_parameter_data_get_attribute_mapping_by_value(data.parameter_type, data.current_value);

    int length = string_length(get_display_string(mapping)) + 10;
    uint8_t *dst = malloc(sizeof(uint8_t) * length);
    if (!dst) {
        return;
    }
    int offset = string_from_int(dst, data.current_value, 0);
    dst[offset++] = ' ';
    dst[offset++] = '-';
    dst[offset++] = ' ';
    string_copy(get_display_string(mapping), &dst[offset], length - offset);
    data.heading_text = dst;
}

static void init(parameter_type type, void (*callback)(int), int current_value)
{
    data.parameter_type = type;
    data.callback = callback;
    data.current_value = current_value;
    data.list_size = scenario_events_parameter_data_get_mappings_size(type);

    if (!data.list_size) {
        window_go_back();
    }

    scrollbar_init(&scrollbar, 0, data.list_size);
    set_heading_display_text();
    populate_list(0);
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 42, 33);

    text_draw(data.heading_text, 48, 40, FONT_NORMAL_PLAIN, COLOR_BLACK);

    int y_offset = DETAILS_Y_OFFSET;
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (i < data.list_size) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
            if (data.focus_button_id == (i + 1)) {
                button_border_draw(BUTTON_LEFT_PADDING, y_offset, BUTTON_WIDTH, DETAILS_ROW_HEIGHT, 1);
            }

            text_draw(get_display_string(data.list[i]), 48, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
        }

        y_offset += DETAILS_ROW_HEIGHT;
    }

    lang_text_draw_centered(13, 3, 48, 32 + 16 * 30, BUTTON_WIDTH, FONT_NORMAL_BLACK);

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

static void button_click(int param1, int param2)
{
    if (param1 >= data.list_size) {
        return;
    }

    data.callback(data.list[param1]->value);
    window_go_back();
}

void window_editor_select_special_attribute_mapping_show(parameter_type type, void (*callback)(int), int current_value)
{
    window_type window = {
        WINDOW_EDITOR_SELECT_SPECIAL_ATTRIBUTE_MAPPING,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(type, callback, current_value);
    window_show(&window);
}
