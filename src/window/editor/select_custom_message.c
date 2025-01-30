#include "select_custom_message.h"

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
#include "scenario/property.h"
#include "scenario/custom_media.h"
#include "scenario/custom_messages.h"
#include "scenario/message_media_text_blob.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/file_dialog.h"
#include "window/numeric_input.h"

#define MESSAGES_Y_OFFSET 146
#define MESSAGES_ROW_HEIGHT 32
#define MAX_VISIBLE_ROWS 8
#define BUTTON_WIDTH 320


static void on_scroll(void);
static void button_click(const generic_button *button);
static void populate_list(int offset);

static scrollbar_type scrollbar = {
    368, MESSAGES_Y_OFFSET, MESSAGES_ROW_HEIGHT * MAX_VISIBLE_ROWS, BUTTON_WIDTH - 16, MAX_VISIBLE_ROWS, on_scroll, 0, 4
};

static generic_button buttons[] = {
    {48, MESSAGES_Y_OFFSET + (0 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 0},
    {48, MESSAGES_Y_OFFSET + (1 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 1},
    {48, MESSAGES_Y_OFFSET + (2 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 2},
    {48, MESSAGES_Y_OFFSET + (3 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 3},
    {48, MESSAGES_Y_OFFSET + (4 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 4},
    {48, MESSAGES_Y_OFFSET + (5 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 5},
    {48, MESSAGES_Y_OFFSET + (6 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 6},
    {48, MESSAGES_Y_OFFSET + (7 * MESSAGES_ROW_HEIGHT), BUTTON_WIDTH, MESSAGES_ROW_HEIGHT - 2, button_click, 0, 7},
};

#define MAX_BUTTONS (sizeof(buttons) / sizeof(generic_button))

static struct {
    unsigned int focus_button_id;

    unsigned int total_messages;
    void (*callback)(int);
    custom_message_t *list[MAX_VISIBLE_ROWS];
} data;

static void init(void (*callback)(int))
{
    data.callback = callback;
    data.total_messages = custom_messages_count();
    populate_list(0);
    scrollbar_init(&scrollbar, 0, data.total_messages);
}

static void populate_list(int offset)
{
    //Ensure we dont offset past the end or beginning of the list.
    if (data.total_messages - offset < MAX_VISIBLE_ROWS) {
        offset = data.total_messages - MAX_VISIBLE_ROWS;
    }
    if (offset < 0) {
        offset = 0;
    }
    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        unsigned int target_id = i + offset + 1; // Skip entry zero custom message
        if (target_id <= data.total_messages) {
            data.list[i] = custom_messages_get(target_id);
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

    outer_panel_draw(16, 16, 26, 30);

    text_draw_centered(translation_for(TR_EDITOR_CUSTOM_MESSAGES_TITLE), 48, 58, BUTTON_WIDTH, FONT_LARGE_BLACK, 0);
    text_draw_label_and_number(translation_for(TR_EDITOR_CUSTOM_MESSAGES_COUNT), data.total_messages, "", 48, 106, FONT_NORMAL_PLAIN, COLOR_BLACK);

    int y_offset = MESSAGES_Y_OFFSET;
    for (unsigned int i = 0; i < MAX_VISIBLE_ROWS; i++) {
        if (data.list[i]) {
            large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, data.focus_button_id == i ? 1 : 0);

            text_draw_label_and_number(0, data.list[i]->id, "", 48, y_offset + 8, FONT_NORMAL_PLAIN, COLOR_BLACK);
            text_draw_centered(data.list[i]->linked_uid->text, 100, y_offset + 8, 250, FONT_NORMAL_PLAIN, COLOR_BLACK);

            if (data.focus_button_id == (i + 1)) {
                button_border_draw(48, y_offset, BUTTON_WIDTH, MESSAGES_ROW_HEIGHT, 1);
            }
        }

        y_offset += MESSAGES_ROW_HEIGHT;
    }

    y_offset += MESSAGES_ROW_HEIGHT;
    lang_text_draw_centered(13, 3, 48, y_offset, BUTTON_WIDTH, FONT_NORMAL_BLACK);

    scrollbar_draw(&scrollbar);
    graphics_reset_dialog();
}

static void button_click(const generic_button *button)
{
    int button_index = button->parameter1;
    if (!data.list[button_index]) {
        return;
    };
    data.callback(data.list[button_index]->id);
    window_go_back();
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

void window_editor_select_custom_message_show(void (*callback)(int))
{
    window_type window = {
        WINDOW_EDITOR_SELECT_CUSTOM_MESSAGE,
        draw_background,
        draw_foreground,
        handle_input
    };
    init(callback);
    window_show(&window);
}
