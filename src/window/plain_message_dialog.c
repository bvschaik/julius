#include "plain_message_dialog.h"

#include "core/image_group.h"
#include "core/string.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"

static void button_ok(int param1, int param2);

static image_button buttons[] = {
    {223, 0, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
};

static const uint8_t EMPTY_STRING[] = { 0 };
static const uint8_t *original_extra_messages[3];

static struct {
    const uint8_t *title;
    const uint8_t *message;
    int draw_underlying_window;
    int height;
    struct {
        const uint8_t **texts;
        unsigned int total_texts;
        int is_list;
    } extra;
} data;

static int init(translation_key title, translation_key message, int should_draw_underlying_window,
    const uint8_t **texts, unsigned int num_texts, int texts_as_list)
{
    if (window_is(WINDOW_PLAIN_MESSAGE_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.title = translation_for(title);
    data.message = translation_for(message);
    data.draw_underlying_window = should_draw_underlying_window;
    data.extra.texts = texts;
    data.extra.is_list = texts_as_list;
    data.extra.total_texts = num_texts;
    data.height = 13;
    if (num_texts) {
        if (num_texts > 2) {
            data.height += ((num_texts - 2) * 30 + BLOCK_SIZE - 1) / BLOCK_SIZE;
        }
    }
    buttons[0].y_offset = data.height * (BLOCK_SIZE - 1) - buttons[0].height;
    return 1;
}

static void draw_background(void)
{
    if (data.draw_underlying_window) {
        window_draw_underlying_window();
    }

    graphics_in_dialog();
    outer_panel_draw(80, 80, 30, data.height);
    text_draw_centered(data.title, 80, 100, 480, FONT_LARGE_BLACK, 0);
    int y_offset = 150 + text_draw_multiline(data.message, 100, 140, 450, 0, FONT_NORMAL_BLACK, 0);
    if (data.extra.total_texts) {
        for (unsigned int i = 0; i < data.extra.total_texts; i++) {
            if (!data.extra.is_list) {
                text_draw_centered(data.extra.texts[i], 100, y_offset, 450, FONT_NORMAL_BLACK, 0);
            } else {
                int width = text_draw(string_from_ascii("-"), 100, y_offset, FONT_NORMAL_BLACK, 0);
                text_draw(data.extra.texts[i], 100 + width, y_offset, FONT_NORMAL_BLACK, 0);
            }
            y_offset += 30;
        }
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(80, 80, buttons, 1);
    graphics_reset_dialog();
}

static void close(void)
{
    window_go_back();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 1, 0)) {
        return;
    }
    if (input_go_back_requested(m, h) || h->enter_pressed) {
        close();
    }
}

static void button_ok(int param1, int param2)
{
    close();
}

void window_plain_message_dialog_show(translation_key title, translation_key message, int should_draw_underlying_window)
{
    if (init(title, message, should_draw_underlying_window, 0, 0, 0)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_plain_message_dialog_show_with_extra(translation_key title, translation_key message,
    const uint8_t *extra, const uint8_t *extra2)
{
    int num_texts = 0;
    if (extra) {
        original_extra_messages[0] = extra;
        num_texts++;
    }
    if (extra2) {
        if (!num_texts) {
            original_extra_messages[0] = EMPTY_STRING;
        }
        original_extra_messages[1] = extra2;
        num_texts++;
    }
    if (init(title, message, 1, original_extra_messages, num_texts, 0)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_plain_message_dialog_show_text_list(translation_key title, translation_key message,
    const uint8_t **texts, unsigned int num_texts)
{
    if (init(title, message, 1, texts, num_texts, 1)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}
