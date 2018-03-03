#include "main_menu.h"

#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "window/cck_selection.h"
#include "window/file_dialog.h"
#include "window/new_career.h"
#include "window/popup_dialog.h"

static void button_click(int type, int param2);

static int focus_button_id;

static generic_button buttons[] = {
    {192, 100, 448, 125, GB_IMMEDIATE, button_click, button_none, 1, 0},
    {192, 140, 448, 165, GB_IMMEDIATE, button_click, button_none, 2, 0},
    {192, 180, 448, 205, GB_IMMEDIATE, button_click, button_none, 3, 0},
    {192, 220, 448, 245, GB_IMMEDIATE, button_click, button_none, 4, 0},
};

static void draw_background()
{
    graphics_clear_screen();
    graphics_in_dialog();
    image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
}

static void draw_foreground()
{
    graphics_in_dialog();

    large_label_draw(192, 100, 16, focus_button_id == 1 ? 1 : 0);
    large_label_draw(192, 140, 16, focus_button_id == 2 ? 1 : 0);
    large_label_draw(192, 180, 16, focus_button_id == 3 ? 1 : 0);
    large_label_draw(192, 220, 16, focus_button_id == 4 ? 1 : 0);

    lang_text_draw_centered(30, 1, 192, 106, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 2, 192, 146, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 3, 192, 186, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 5, 192, 226, 256, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 4, &focus_button_id);
}

static void confirm_exit(int accepted)
{
    if (accepted) {
        system_exit();
    }
}

static void button_click(int type, int param2)
{
    if (type == 1) {
        window_new_career_show();
    } else if (type == 2) {
        window_file_dialog_show(FILE_DIALOG_LOAD);
    } else if (type == 3) {
        window_cck_selection_show();
    } else if (type == 4) {
        window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
    }
}

void window_main_menu_show()
{
    window_type window = {
        WINDOW_MAIN_MENU,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
