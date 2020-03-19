#include "popup_dialog.h"

#include "core/image_group.h"
#include "graphics/graphics.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"

#define GROUP 5

#define PROCEED_GROUP 43
#define PROCEED_TEXT 5

static void button_ok(int param1, int param2);
static void button_cancel(int param1, int param2);

static image_button buttons[] = {
    {192, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 0, button_ok, button_none, 1, 0, 1},
    {256, 100, 39, 26, IB_NORMAL, GROUP_OK_CANCEL_SCROLL_BUTTONS, 4, button_cancel, button_none, 0, 0, 1},
};

static struct {
    popup_dialog_type type;
    int custom_text_group;
    int custom_text_id;
    int ok_clicked;
    void (*close_func)(int accepted);
    int has_buttons;
} data;

static int init(popup_dialog_type type, int custom_text_group, int custom_text_id,
        void (*close_func)(int accepted), int has_ok_cancel_buttons)
{
    if (window_is(WINDOW_POPUP_DIALOG)) {
        // don't show popup over popup
        return 0;
    }
    data.type = type;
    data.custom_text_group = custom_text_group;
    data.custom_text_id = custom_text_id;
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
    } else {
        lang_text_draw_centered(data.custom_text_group, data.custom_text_id, 80, 100, 480, FONT_LARGE_BLACK);
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

static void handle_mouse(const mouse *m)
{
    if (data.has_buttons) {
        image_buttons_handle_mouse(mouse_in_dialog(m), 80, 80, buttons, 2, 0);
    } else if (m->right.went_up || (m->is_touch && m->left.double_click)) {
        data.close_func(0);
        window_go_back();
    }
}

void button_ok(int param1, int param2)
{
    window_popup_dialog_confirm();
}

void button_cancel(int param1, int param2)
{
    window_go_back();
    data.close_func(0);
}

void window_popup_dialog_confirm(void)
{
    window_go_back();
    data.close_func(1);
}

void window_popup_dialog_show(popup_dialog_type type,
        void (*close_func)(int accepted), int has_ok_cancel_buttons)
{
    if (init(type, 0, 0, close_func, has_ok_cancel_buttons)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_mouse
        };
        window_show(&window);
    }
}

void window_popup_dialog_show_confirmation(int text_group, int text_id,
        void (*close_func)(int accepted))
{
    if (init(POPUP_DIALOG_NONE, text_group, text_id, close_func, 1)) {
        window_type window = {
            WINDOW_POPUP_DIALOG,
            draw_background,
            draw_foreground,
            handle_mouse
        };
        window_show(&window);
    }
}
