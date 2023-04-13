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
    {223, 160, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
};

static struct {
    const uint8_t *title;
    const uint8_t *message;
    int draw_underlying_window;
    const uint8_t *extra;
    const uint8_t *extra2;
} data;

static int init(translation_key title, translation_key message, int should_draw_underlying_window, const uint8_t *extra, const uint8_t *extra2)
{
    if (window_is(WINDOW_PLAIN_MESSAGE_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.title = translation_for(title);
    data.message = translation_for(message);
    data.draw_underlying_window = should_draw_underlying_window;
    data.extra = extra;
    data.extra2 = extra2;
    return 1;
}

static void draw_background(void)
{
    if (data.draw_underlying_window) {
        window_draw_underlying_window();
    }

    graphics_in_dialog();
    outer_panel_draw(80, 80, 30, 13);
    text_draw_centered(data.title, 80, 100, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(data.message, 100, 140, 450, FONT_NORMAL_BLACK, 0);
    if (data.extra) {
        text_draw_centered(data.extra, 100, 180, 450, FONT_NORMAL_BLACK, 0);
    }
    if (data.extra2) {
        text_draw_centered(data.extra2, 100, 210, 450, FONT_NORMAL_BLACK, 0);
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
    if (init(title, message, should_draw_underlying_window, 0, 0)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_plain_message_dialog_show_with_extra(translation_key title, translation_key message, const uint8_t *extra, const uint8_t *extra2)
{
    if (init(title, message, 1, extra, extra2)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}
