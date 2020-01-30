#include "main_menu.h"

#include "core/string.h"
#include "editor/editor.h"
#include "game/game.h"
#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "platform/version.h"
#include "sound/music.h"
#include "window/cck_selection.h"
#include "window/file_dialog.h"
#include "window/intro_video.h"
#include "window/new_career.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"

static void button_click(int type, int param2);
static void button_intro_movie(int param1, int param2);

static int focus_button_id;

static generic_button buttons[] = {
    {192, 100, 256, 25, button_click, button_none, 1, 0},
    {192, 140, 256, 25, button_click, button_none, 2, 0},
    {192, 180, 256, 25, button_click, button_none, 3, 0},
    {192, 220, 256, 25, button_click, button_none, 4, 0},
    {192, 260, 256, 25, button_click, button_none, 5, 0},
};

static image_button movie_button =
    {591, 442, 33, 22, IB_NORMAL, 89, 0, button_intro_movie, button_none, 0, 0, 1};

static void draw_version(void)
{
    char version_string[100] = "v";
    int version_length = string_length(JULIUS_VERSION);
    int text_y = screen_height() - 30;

    string_copy(JULIUS_VERSION, version_string + 1, 99);
    string_copy(JULIUS_VERSION_SUFFIX, version_string + 1 + version_length, 99 - version_length);

    int text_width = text_get_width(version_string, FONT_SMALL_PLAIN);

    graphics_draw_rect(10, text_y, text_width + 14, 20, COLOR_BLACK);
    graphics_fill_rect(11, text_y + 1, text_width + 12, 18, COLOR_WHITE);

    text_draw(version_string, 18, text_y + 6, FONT_SMALL_PLAIN, COLOR_BLACK);
}

static void draw_background(void)
{
    graphics_clear_screen();
    graphics_in_dialog();
    image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
    draw_version();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    large_label_draw(192, 100, 16, focus_button_id == 1 ? 1 : 0);
    large_label_draw(192, 140, 16, focus_button_id == 2 ? 1 : 0);
    large_label_draw(192, 180, 16, focus_button_id == 3 ? 1 : 0);
    large_label_draw(192, 220, 16, focus_button_id == 4 ? 1 : 0);
    large_label_draw(192, 260, 16, focus_button_id == 5 ? 1 : 0);

    lang_text_draw_centered(30, 1, 192, 106, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 2, 192, 146, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 3, 192, 186, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(9, 8, 192, 226, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 5, 192, 266, 256, FONT_NORMAL_GREEN);

    image_buttons_draw(0, 0, &movie_button, 1);

    graphics_reset_dialog();
}

static void handle_mouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    int dummy;
    if (!image_buttons_handle_mouse(m_dialog, 0, 0, &movie_button, 1, &dummy)) {
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 5, &focus_button_id);
    }
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
        window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
    }
}

static void button_intro_movie(int param1, int param2)
{
    window_intro_video_show();
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
