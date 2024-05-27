#include "text_input.h"

#include "core/array.h"
#include "core/image_group.h"
#include "core/log.h"
#include "core/string.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "widget/input_box.h"

static void button_accept(int param1, int param2);
static void button_cancel(int param1, int param2);

static input_box text_input = { 160, 0, 20, 2, FONT_NORMAL_WHITE, 1 };

static image_button buttons[] = {
    {288, 74, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_accept, button_none, 1, 0, 1},
    {333, 74, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_cancel, button_none, 0, 0, 1},
};

static struct {
    void (*callback)(const uint8_t *);

    const uint8_t *title;
    uint8_t *text;
    int max_length;
    int text_buffer_size;

    int x_offset;
    int y_offset;
    int height_blocks;

    unsigned int focus_button_id;
} data;

static void init(const uint8_t *title, const uint8_t *placeholder, const uint8_t *text, int max_length,
    void (*callback)(const uint8_t *))
{
    data.callback = callback;
    data.focus_button_id = 0;
    if (max_length > data.text_buffer_size) {
        uint8_t *new_text = realloc(data.text, max_length * sizeof(uint8_t));
        if (!new_text) {
            return;
        }
        data.text = new_text;
        data.text_buffer_size = max_length;
        text_input.text = data.text;
    }
    data.max_length = max_length;
    memset(data.text, 0, data.max_length);
    if (text) {
        string_copy(text, data.text, data.max_length);
    }
    text_input.placeholder = placeholder;
    text_input.text_length = data.max_length;
    data.title = title;

    data.x_offset = 128;
    data.y_offset = data.title ? 176 : 168;
    data.height_blocks = data.title ? 8 : 7;

    text_input.y = data.y_offset + (data.title ? 48 : 32);

    input_box_start(&text_input);
}

static void close(void)
{
    input_box_stop(&text_input);
    window_go_back();
}

static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(data.x_offset, data.y_offset, 24, data.height_blocks);
    if (data.title) {
        text_draw_centered(data.title, data.x_offset, data.y_offset + 16, 384, FONT_LARGE_BLACK, 0);
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    input_box_draw(&text_input);
    image_buttons_draw(data.x_offset, data.y_offset + (data.title ? 16 : 0), buttons, 2);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (input_box_handle_mouse(m_dialog, &text_input) ||
        image_buttons_handle_mouse(m_dialog, data.x_offset, data.y_offset + (data.title ? 16 : 0), buttons, 2,
            &data.focus_button_id)) {
        return;
    }
    if (input_box_is_accepted()) {
        button_accept(0, 0);
        return;
    }
    if (input_go_back_requested(m, h)) {
        button_cancel(0, 0);
    }
}

static void button_cancel(int param1, int param2)
{
    close();
}

static void button_accept(int param1, int param2)
{
    close();
    data.callback(data.text);
}

void window_text_input_show(const uint8_t *title, const uint8_t *placeholder, const uint8_t *text, int max_length,
    void (*callback)(const uint8_t *))
{
    window_type window = {
        WINDOW_TEXT_INPUT,
        draw_background,
        draw_foreground,
        handle_input,
    };
    init(title, placeholder, text, max_length, callback);
    window_show(&window);
}
