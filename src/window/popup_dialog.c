#include "popup_dialog.h"

#include "core/image_group.h"
#include "core/lang.h"
#include "core/string.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "translation/translation.h"

#define GROUP 5

#define PROCEED_GROUP 43
#define PROCEED_TEXT 5
#define CHECKBOX_CHECK_SIZE 20

static void button_checkbox(int param1, int param2);
static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);
static void confirm(void);

static image_button buttons[] = {
    {192, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
    {256, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_cancel, button_none, 0, 0, 1},
};

static generic_button checkbox = { 160, 180, 360, 20, button_checkbox, button_none };

static struct {
    int ok_clicked;
    void (*close_func)(int accepted, int checked);
    int has_buttons;
    int translation_key;
    int checked;
    int has_focus;
    int checkbox_start_width;
    const uint8_t *custom_title;
    const uint8_t *custom_text;
    const uint8_t *checkbox_text;
} data;

static int init(const uint8_t *custom_title, const uint8_t *custom_text,
    const uint8_t *checkbox_text, void (*close_func)(int accepted, int checked), int has_ok_cancel_buttons)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.ok_clicked = 0;
    data.close_func = close_func;
    data.has_buttons = has_ok_cancel_buttons;
    data.custom_title = custom_title;
    data.custom_text = custom_text;
    data.checkbox_text = checkbox_text;
    data.checked = 0;
    if (!data.custom_text) {
        data.custom_text = lang_get_string(PROCEED_GROUP, PROCEED_TEXT);
    }
    if (data.checkbox_text) {
        data.checkbox_start_width = 80 + (480 - text_get_width(data.checkbox_text, FONT_NORMAL_BLACK) - 30) / 2;
    }
    return 1;
}

static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 80, 30, data.checkbox_text ? 11 : 10);
    if (data.custom_title) {
        text_draw_centered(data.custom_title, 80, 100, 480, FONT_LARGE_BLACK, 0);
    }
    if (text_get_width(data.custom_text, FONT_NORMAL_BLACK) >= 420) {
        text_draw_multiline(data.custom_text, 110, 140, 420, FONT_NORMAL_BLACK, 0);
    } else {
        text_draw_centered(data.custom_text, 80, 140, 480, FONT_NORMAL_BLACK, 0);
    }
    if (data.checkbox_text) {
        if (data.checked) {
            text_draw(string_from_ascii("x"), data.checkbox_start_width + 6, 183, FONT_NORMAL_BLACK, 0);
        }
        text_draw(data.checkbox_text, data.checkbox_start_width + 30, 184, FONT_NORMAL_BLACK, 0);
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (data.checkbox_text) {
        button_border_draw(data.checkbox_start_width, 180, CHECKBOX_CHECK_SIZE, CHECKBOX_CHECK_SIZE, data.has_focus);
    }
    if (data.has_buttons) {
        image_buttons_draw(80, data.checkbox_text ? 110 : 80, buttons, 2);
    } else {
        lang_text_draw_centered(13, 1, 80, 208, 480, FONT_NORMAL_BLACK);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (data.checkbox_text && generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, &checkbox, 1, &data.has_focus)) {
        return;
    }
    if (data.has_buttons && image_buttons_handle_mouse(mouse_in_dialog(m), 80,
        data.checkbox_text ? 110 : 80, buttons, 2, 0)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        data.close_func(0, 0);
        window_go_back();
    }
    if (h->enter_pressed) {
        confirm();
    }
}

static void button_ok(int param1, int param2)
{
    confirm();
}

static void button_cancel(int param1, int param2)
{
    window_go_back();
    data.close_func(0, 0);
}

static void button_checkbox(int param1, int param2)
{
    data.checked ^= 1;
    window_request_refresh();
}

static void confirm(void)
{
    window_go_back();
    data.close_func(1, data.checked);
}

void window_popup_dialog_show(popup_dialog_type type,
    void (*close_func)(int accepted, int checked), int has_ok_cancel_buttons)
{
    if (init(lang_get_string(GROUP, type), lang_get_string(GROUP, type + 1), 0, close_func, has_ok_cancel_buttons)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_popup_dialog_show_confirmation(const uint8_t *custom_title, const uint8_t *custom_text,
    const uint8_t *checkbox_text, void (*close_func)(int accepted, int checked))
{
    if (init(custom_title, custom_text, checkbox_text, close_func, 1)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}
