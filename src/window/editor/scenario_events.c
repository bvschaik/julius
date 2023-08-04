#include "scenario_events.h"

#include "core/string.h"
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
#include "scenario/editor_events.h"
#include "scenario/property.h"
#include "scenario/scenario_events_controller.h"
#include "scenario/scenario_events_parameter_data.h"
#include "window/city.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/editor/custom_variables.h"
#include "window/editor/scenario_event_details.h"
#include "window/file_dialog.h"
#include "window/numeric_input.h"

#define EVENTS_Y_OFFSET 146
#define EVENTS_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 8
#define BUTTON_WIDTH 320


static void on_scroll(void);
static void button_click(int type, int param2);
static void button_event(int button_index, int param2);
static void button_open_variables(int param1, int param2);
static void populate_list(int offset);
static void add_new_event(void);

static scrollbar_type scrollbar = {
    368, EVENTS_Y_OFFSET, EVENTS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH - 16, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {48, EVENTS_Y_OFFSET + (0 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 1, 0},
    {48, EVENTS_Y_OFFSET + (1 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 2, 0},
    {48, EVENTS_Y_OFFSET + (2 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 3, 0},
    {48, EVENTS_Y_OFFSET + (3 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 4, 0},
    {48, EVENTS_Y_OFFSET + (4 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 5, 0},
    {48, EVENTS_Y_OFFSET + (5 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 6, 0},
    {48, EVENTS_Y_OFFSET + (6 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 7, 0},
    {48, EVENTS_Y_OFFSET + (7 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, button_none, 8, 0},
    {48, EVENTS_Y_OFFSET + (9 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, button_none, 9, 0},
    {48, EVENTS_Y_OFFSET + (11 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, button_none, 10, 0},
    {48, EVENTS_Y_OFFSET + (12 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, button_none, 11, 0},
    {48, EVENTS_Y_OFFSET + (13 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, button_none, 12, 0},
    {210, 100, BUTTON_WIDTH / 2, EVENTS_ROW_HEIGHT, button_open_variables, button_none, 0, 0}
};
#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    int focus_button_id;

    int total_events;
    scenario_event_t *list[MAX_VISIBLE_ROWS];

    int from_editor;
} data;

static void init_list(void)
{
    data.total_events = scenario_events_get_count();
    populate_list(0);
    scrollbar_init(&scrollbar, 0, data.total_events);
}

static void init(int from_editor)
{
    scenario_events_parameter_data_sort_alphabetically();
    data.from_editor = from_editor;
    init_list();
}

static void populate_list(int offset)
{
    // Ensure we dont offset past the end or beginning of the list.
    if (data.total_events - offset < MAX_VISIBLE_ROWS) {
        offset = data.total_events - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        int target_id = i + offset;
        if (target_id < data.total_events) {
            data.list[i] = scenario_event_get(target_id);
        } else {
            data.list[i] = 0;
        }
    }
}

static void add_new_event(void)
{
    scenario_event_create(0, 0, 0);
    init_list();
    window_request_refresh();
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static int color_from_state(event_state state)
{
    if (!data.from_editor) {
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

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 26, 38);

    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENTS_TITLE), 48, 58, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_COUNT), data.total_events, "", 48, 106, FONT_NORMAL_PLAIN, COLOR_BLACK);

    for (int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i]) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);

            if (data.list[i]->state != EVENT_STATE_UNDEFINED) {
                text_draw_label_and_number(0, data.list[i]->id, "", buttons[i].x, buttons[i].y + 8, FONT_NORMAL_GREEN, color_from_state(data.list[i]->state));
                text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_CONDITIONS), data.list[i]->conditions.size, "", 100, buttons[i].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
                text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_ACTIONS), data.list[i]->actions.size, "", 250, buttons[i].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);

                if (data.focus_button_id == i + 1) {
                    button_border_draw(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, 1);
                }
            } else {
                text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_DELETED), 48, buttons[i].y + 8, BUTTON_WIDTH, FONT_NORMAL_GREEN, 0);
            }
        }
    }

    for (size_t i = 8; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
        if (data.focus_button_id == i + 1) {
            button_border_draw(buttons[i].x, buttons[i].y , buttons[i].width, buttons[i].height, 1);
        }
    }

    generic_button *btn = &buttons[8];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_ADD, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[9];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_IMPORT, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[10];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_EXPORT, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[11];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_CLEAR, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[12];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_TITLE, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    lang_text_draw_centered(13, 3, 48, 600, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void button_event(int button_index, int param2)
{
    int target_index = button_index - 1;
    if (!data.list[target_index]) {
        return;
    }
    if (data.list[target_index]->state != EVENT_STATE_UNDEFINED) {
        window_editor_scenario_event_details_show(data.list[target_index]->id, data.from_editor);
    }
}

static void on_scroll(void)
{
    window_request_refresh();
}

static void close_window(void)
{
    if (data.from_editor) {
        window_editor_attributes_show();
    } else {
        window_city_show();
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog, 1) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        close_window();
    }
    populate_list(scrollbar.scroll_position);
}

static void button_click(int type, int param2)
{
    if (type == 9) {
        add_new_event();
    } else if (type == 10) {
        window_file_dialog_show(FILE_TYPE_SCENARIO_EVENTS, FILE_DIALOG_LOAD);
    } else if (type == 11) {
        window_file_dialog_show(FILE_TYPE_SCENARIO_EVENTS, FILE_DIALOG_SAVE);
    } else if (type == 12) {
        scenario_events_clear();
        data.total_events = scenario_events_get_count();
    }
}

static void button_open_variables(int param1, int param2)
{
    window_editor_custom_variables_show(data.from_editor);
}

void window_editor_scenario_events_show(int from_editor)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_EVENTS,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(from_editor);
    window_show(&window);
}
