#include "main_menu.h"

#include "editor/editor.h"
#include "game/game.h"
#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "sound/music.h"
#include "window/cck_selection.h"
#include "window/config.h"
#include "window/file_dialog.h"
#include "window/new_career.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"

#define MAX_BUTTONS 6

static void button_click(int type, int param2);

static int focus_button_id;

static generic_button buttons[] = {
    {192, 100, 256, 25, button_click, button_none, 1, 0},
    {192, 140, 256, 25, button_click, button_none, 2, 0},
    {192, 180, 256, 25, button_click, button_none, 3, 0},
    {192, 220, 256, 25, button_click, button_none, 4, 0},
    {192, 260, 256, 25, button_click, button_none, 5, 0},
    {192, 300, 256, 25, button_click, button_none, 6, 0},
};

static void draw_background(void)
{
    graphics_clear_screen();
    graphics_in_dialog();
    image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, focus_button_id == i + 1 ? 1 : 0);
    }

    lang_text_draw_centered(30, 1, 192, 106, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 2, 192, 146, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 3, 192, 186, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(9, 8, 192, 226, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(2, 0, 192, 266, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 5, 192, 306, 256, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &focus_button_id);
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
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    } else if (type == 3) {
        window_cck_selection_show();
    } else if (type == 4) {
        if (!editor_is_present() || !game_init_editor()) {
            window_plain_message_dialog_show(
                "Editor not installed",
                "Your Caesar 3 installation does not contain the editor files. "
                "You can download them from:\n"
                "https://bintray.com/bvschaik/caesar3-editor"
            );
        } else {
            sound_music_play_editor();
        }
    } else if (type == 5) {
        window_config_show();
    } else if (type == 6) {
        window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
    }
}

void window_main_menu_show(int restart_music)
{
    if (restart_music) {
        sound_music_play_intro();
    }
    window_type window = {
        WINDOW_MAIN_MENU,
        draw_background,
        draw_foreground,
        handle_mouse
    };
    window_show(&window);
}
