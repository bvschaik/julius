#include "map.h"

#include "editor/editor.h"
#include "game/game.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "widget/top_menu_editor.h"
#include "window/popup_dialog.h"
#include "window/editor/attributes.h"

static void button_click(int type, int param2);

static int focus_button_id;

static generic_button buttons[] = {
    {192, 100, 448, 125, GB_IMMEDIATE, button_click, button_none, 1, 0},
    {192, 140, 448, 165, GB_IMMEDIATE, button_click, button_none, 2, 0},
};

static void draw_background(void)
{
    graphics_clear_screen();
    widget_top_menu_editor_draw();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    large_label_draw(192, 100, 16, focus_button_id == 1 ? 1 : 0);
    lang_text_draw_centered(7, 4, 192, 106, 256, FONT_NORMAL_GREEN);

    large_label_draw(192, 140, 16, focus_button_id == 2 ? 1 : 0);
    text_draw_centered((const uint8_t*)"attributes", 192, 146, 256, FONT_NORMAL_GREEN, 0);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    if (!generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 2, &focus_button_id)) {
        widget_top_menu_editor_handle_mouse(m);
    }
}

static void confirm_exit(int accepted)
{
    if (accepted) {
        game_exit_editor();
    }
}

static void button_click(int type, int param2)
{
    if (type == 1) {
        window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
    } else if (type == 2) {
        window_editor_attributes_show();
    }
}

void window_editor_map_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_editor_map_show(void)
{
    window_type window = {
        WINDOW_EDITOR_MAP,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
