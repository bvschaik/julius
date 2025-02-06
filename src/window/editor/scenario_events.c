#include "scenario_events.h"

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
#include "scenario/editor_events.h"
#include "scenario/event/event.h"
#include "scenario/event/controller.h"
#include "scenario/event/parameter_data.h"
#include "scenario/property.h"
#include "window/city.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/editor/custom_variables.h"
#include "window/editor/scenario_event_details.h"
#include "window/file_dialog.h"
#include "window/numeric_input.h"

#define EVENTS_Y_OFFSET 100
#define EVENTS_ROW_HEIGHT 31
#define MAX_VISIBLE_ROWS 10
#define BUTTON_WIDTH 320


static void on_scroll(void);
static void button_click(const generic_button *button);
static void button_event(const generic_button *button);
static void button_open_variables(const generic_button *button);
static void populate_list(int offset);
static void add_new_event(void);

static scrollbar_type scrollbar = {
    375, EVENTS_Y_OFFSET, EVENTS_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH - 17, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {48, EVENTS_Y_OFFSET + (0 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 1},
    {48, EVENTS_Y_OFFSET + (1 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 2},
    {48, EVENTS_Y_OFFSET + (2 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 3},
    {48, EVENTS_Y_OFFSET + (3 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 4},
    {48, EVENTS_Y_OFFSET + (4 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 5},
    {48, EVENTS_Y_OFFSET + (5 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 6},
    {48, EVENTS_Y_OFFSET + (6 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 7},
    {48, EVENTS_Y_OFFSET + (7 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 8},
    {48, EVENTS_Y_OFFSET + (8 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 9},
    {48, EVENTS_Y_OFFSET + (9 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_event, 0, 10},
    {48, EVENTS_Y_OFFSET + (11 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, 0, 11}, // add new
    {48, EVENTS_Y_OFFSET + (13 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, 0, 12}, // import
    {48, EVENTS_Y_OFFSET + (14 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, 0, 13}, // export
    {48, EVENTS_Y_OFFSET + (15 * EVENTS_ROW_HEIGHT), BUTTON_WIDTH, EVENTS_ROW_HEIGHT, button_click, 0, 14}, // clear
    {255, 60, BUTTON_WIDTH / 2, EVENTS_ROW_HEIGHT, button_open_variables}                                   // variables
};
#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    unsigned int focus_button_id;
    unsigned int total_events;
    scenario_event_t *list[MAX_VISIBLE_ROWS];
} data;

static void init_list(void)
{
    data.total_events = scenario_events_get_count();
    populate_list(0);
    scrollbar_init(&scrollbar, 0, data.total_events);
}

static void init(void)
{
    scenario_events_parameter_data_sort_alphabetically();
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
        unsigned int target_id = i + offset;
        if (target_id < data.total_events) {
            data.list[i] = scenario_event_get(target_id);
        } else {
            data.list[i] = 0;
        }
    }
}

static void add_new_event(void)
{
    scenario_event_t *event = scenario_event_create(0, 0, 0);
    array_advance(event->condition_groups);

    init_list();
    window_request_refresh();
}

static void draw_background(void)
{
    window_editor_map_draw_all();
}

static color_t color_from_state(event_state state)
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

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(16, 16, 26, 38);

    text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENTS_TITLE), 48, 30, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_COUNT), data.total_events, "",
        30, 70, FONT_NORMAL_PLAIN, COLOR_BLACK);

    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i]) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
            color_t color = color_from_state(data.list[i]->state);

            if (data.list[i]->state != EVENT_STATE_UNDEFINED) {
                text_draw_number(data.list[i]->id, 0, "", buttons[i].x + 6, buttons[i].y + 8, FONT_NORMAL_GREEN, color);
                if (!*data.list[i]->name) {
                    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_CONDITIONS),
                        scenario_event_count_conditions(data.list[i]), "", 100, buttons[i].y + 8,
                        FONT_NORMAL_GREEN, COLOR_MASK_NONE);
                    text_draw_label_and_number(translation_for(TR_EDITOR_SCENARIO_EVENTS_ACTIONS),
                        data.list[i]->actions.size, "", 250, buttons[i].y + 8, FONT_NORMAL_GREEN, COLOR_MASK_NONE);
                } else {
                    text_draw(data.list[i]->name, 100, buttons[i].y + 8, FONT_NORMAL_GREEN, color);
                }
            } else {
                text_draw_centered(translation_for(TR_EDITOR_SCENARIO_EVENT_DELETED), 48, buttons[i].y + 8,
                    BUTTON_WIDTH, FONT_NORMAL_GREEN, 0);
            }
        }
    }

    for (size_t i = 10; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i + 1 ? 1 : 0);
    }

    generic_button *btn = &buttons[10];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_ADD, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[11];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_IMPORT, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[12];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_EXPORT, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[13];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_SCENARIO_EVENTS_CLEAR, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    btn = &buttons[14];
    lang_text_draw_centered(CUSTOM_TRANSLATION, TR_EDITOR_CUSTOM_VARIABLES_TITLE, btn->x, btn->y + 8, btn->width, FONT_NORMAL_GREEN);

    lang_text_draw_centered(13, 3, 48, 600, BUTTON_WIDTH, FONT_NORMAL_BLACK); // Right-click to Continue

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void button_event(const generic_button *button)
{
    int target_index = button->parameter1 - 1;
    if (!data.list[target_index]) {
        return;
    }
    if (data.list[target_index]->state != EVENT_STATE_UNDEFINED) {
        window_editor_scenario_event_details_show(data.list[target_index]->id);
    }
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
        window_editor_attributes_show();
    }
    populate_list(scrollbar.scroll_position);
}

static void button_click(const generic_button *button)
{
    int type = button->parameter1;
    if (type == 11) {
        add_new_event();
    } else if (type == 12) {
        window_file_dialog_show(FILE_TYPE_SCENARIO_EVENTS, FILE_DIALOG_LOAD);
    } else if (type == 13) {
        window_file_dialog_show(FILE_TYPE_SCENARIO_EVENTS, FILE_DIALOG_SAVE);
    } else if (type == 14) {
        scenario_events_clear();
        data.total_events = scenario_events_get_count();
    }
}

static void button_open_variables(const generic_button *button)
{
    window_editor_custom_variables_show(0);
}

void window_editor_scenario_events_show(void)
{
    window_type window = {
        WINDOW_EDITOR_SCENARIO_EVENTS,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}
