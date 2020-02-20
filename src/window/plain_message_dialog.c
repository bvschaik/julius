#include "plain_message_dialog.h"

#include "core/image_group.h"
#include "core/string.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"

static void button_ok(int param1, int param2);

static image_button buttons[] = {
    {223, 140, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
};

static struct {
    const uint8_t *title;
    const uint8_t *message;
} data;

static int init(const char *title, const char *message)
{
    if (window_is(WINDOW_PLAIN_MESSAGE_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.title = string_from_ascii(title);
    data.message = string_from_ascii(message);
    return 1;
}

static void draw_background(void)
{
    graphics_in_dialog();
    outer_panel_draw(80, 80, 30, 12);
    text_draw_centered(data.title, 80, 100, 480, FONT_LARGE_BLACK, 0);
    text_draw_multiline(data.message, 100, 140, 450, FONT_NORMAL_BLACK);
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(80, 80, buttons, 1);
    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if(image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 1, 0)) {
        return;
    }
    if (m->right.went_up || (m->is_touch && m->left.double_click)) {
        window_plain_message_dialog_accept();
    }
}

void button_ok(int param1, int param2)
{
    window_plain_message_dialog_accept();
}

void window_plain_message_dialog_accept(void)
{
    window_go_back();
}

void window_plain_message_dialog_show(const char *title, const char *message)
{
    if (init(title, message)) {
        window_type window = {
            WINDOW_PLAIN_MESSAGE_DIALOG,
            draw_background,
            draw_foreground,
            handle_mouse
        };
        window_show(&window);
    }
}
