#include "popup_dialog.h"
#include "core/image_group.h"

#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"

#define GROUP 5

#define PROCEED_GROUP 43
#define PROCEED_TEXT 5

static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);
static void confirm(void);

static image_button buttons[] = {
    {192, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
    {256, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_cancel, button_none, 0, 0, 1},
};

union text_holder {
    struct {
        int custom_text_group;
        int custom_text_id;
    };
    const uint8_t *str;
};

static struct {
    popup_dialog_type type;
    union text_holder custom_text;
    int ok_clicked;
    void (*close_func)(int accepted);
    int has_buttons;
} data;

static int init(popup_dialog_type type, union text_holder custom_text,
        void (*close_func)(int accepted), int has_ok_cancel_buttons)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.type = type;
    data.custom_text = custom_text;
    data.ok_clicked = 0;
    data.close_func = close_func;
    data.has_buttons = has_ok_cancel_buttons;
    return 1;
}

static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 80, 30, 10);
    if (data.type >= 0) {
        lang_text_draw_centered(GROUP, data.type, 80, 100, 480, FONT_LARGE_BLACK);
        if (lang_text_get_width(GROUP, data.type + 1, FONT_NORMAL_BLACK) >= 420) {
            lang_text_draw_multiline(GROUP, data.type + 1, 110, 140, 420, FONT_NORMAL_BLACK);
        } else {
            lang_text_draw_centered(GROUP, data.type + 1, 80, 140, 480, FONT_NORMAL_BLACK);
        }
    } else if (data.type == POPUP_DIALOG_NONE) {
        lang_text_draw_centered(data.custom_text.custom_text_group, data.custom_text.custom_text_id, 80, 100, 480, FONT_LARGE_BLACK);
        lang_text_draw_centered(PROCEED_GROUP, PROCEED_TEXT, 80, 140, 480, FONT_NORMAL_BLACK);
    } else {
        text_draw_centered(data.custom_text.str, 80, 100, 480, FONT_LARGE_BLACK, 0);
        lang_text_draw_centered(PROCEED_GROUP, PROCEED_TEXT, 80, 140, 480, FONT_NORMAL_BLACK);
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (data.has_buttons) {
        image_buttons_draw(80, 80, buttons, 2);
    } else {
        lang_text_draw_centered(13, 1, 80, 208, 480, FONT_NORMAL_BLACK);
    }
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (data.has_buttons && image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 2, 0)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        data.close_func(0);
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
    data.close_func(0);
}

static void confirm(void)
{
    window_go_back();
    data.close_func(1);
}

void window_popup_dialog_show(popup_dialog_type type,
        void (*close_func)(int accepted), int has_ok_cancel_buttons)
{
    union text_holder custom_text = {
            .custom_text_group = 0,
            .custom_text_id = 0
    };
    if (init(type, custom_text, close_func, has_ok_cancel_buttons)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_popup_dialog_show_confirmation(int text_group, int text_id,
        void (*close_func)(int accepted))
{
    union text_holder custom_text = {
            .custom_text_group = text_group,
            .custom_text_id = text_id
    };
    if (init(POPUP_DIALOG_NONE, custom_text, close_func, 1)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_input
        };
        window_show(&window);
    }
}

void window_text_popup_dialog_show(const uint8_t *str,
        void (*close_func)(int accepted), int has_ok_cancel_buttons)
{
    union text_holder custom_text = { .str = str };
    if (init(POPUP_DIALOG_CUSTOM, custom_text, close_func, 1)) {
        window_type window = {
                WINDOW_POPUP_DIALOG,
                draw_background,
                draw_foreground,
                handle_input
        };
        window_show(&window);
    }
}
