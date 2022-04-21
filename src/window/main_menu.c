#include "main_menu.h"

#include "assets/assets.h"
#include "core/calc.h"
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
#include "sound/music.h"
#include "window/cck_selection.h"
#include "window/config.h"
#include "window/file_dialog.h"
#include "window/new_career.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"

#define MAX_BUTTONS 6

static void button_click(int type, int param2);

static struct {
    int focus_button_id;
    int logo_image_id;
} data;

static generic_button buttons[] = {
    {192, 130, 256, 25, button_click, button_none, 1, 0},
    {192, 170, 256, 25, button_click, button_none, 2, 0},
    {192, 210, 256, 25, button_click, button_none, 3, 0},
    {192, 250, 256, 25, button_click, button_none, 4, 0},
    {192, 290, 256, 25, button_click, button_none, 5, 0},
    {192, 330, 256, 25, button_click, button_none, 6, 0},
};

static void draw_version_string(void)
{
    uint8_t version_string[100] = "Augustus v";
    int version_prefix_length = string_length(version_string);
    int text_y = screen_height() - 54;

    string_copy(string_from_ascii(system_version()), version_string + version_prefix_length, 99);

    int text_width = text_get_width(version_string, FONT_NORMAL_GREEN);
    int width = calc_value_in_step(text_width + 20, 16);

    inner_panel_draw(20, text_y, width / 16, 2);
    text_draw_centered(version_string, 20, text_y + 11, width, FONT_NORMAL_GREEN, 0);
}

static void draw_background(void)
{
    graphics_reset_dialog();
    graphics_reset_clip_rectangle();
    image_draw_fullscreen_background(image_group(GROUP_INTERMEZZO_BACKGROUND));

    if (!window_is(WINDOW_FILE_DIALOG)) {
        graphics_in_dialog();
        outer_panel_draw(162, 32, 20, 22);
        if (!data.logo_image_id) {
            data.logo_image_id = assets_get_image_id("UI", "Main Menu Banner");
        }
        image_draw(data.logo_image_id, 176, 50, COLOR_MASK_NONE, SCALE_NONE);
        graphics_reset_dialog();
        draw_version_string();
    }
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    for (int i = 0; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / BLOCK_SIZE, data.focus_button_id == i + 1 ? 1 : 0);
    }

    lang_text_draw_centered(30, 1, 192, 136, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 2, 192, 176, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 3, 192, 216, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(9, 8, 192, 256, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(2, 0, 192, 296, 256, FONT_NORMAL_GREEN);
    lang_text_draw_centered(30, 5, 192, 336, 256, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &data.focus_button_id)) {
        return;
    }
    if (h->escape_pressed) {
        hotkey_handle_escape();
    }
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
}

static void confirm_exit(int accepted, int checked)
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
                TR_NO_EDITOR_TITLE, TR_NO_EDITOR_MESSAGE, 1);
        } else {
            sound_music_play_editor();
        }
    } else if (type == 5) {
        window_config_show(CONFIG_FIRST_PAGE, 1);
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
        handle_input
    };
    window_show(&window);
}
