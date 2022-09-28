#include "map_editor_pause_menu.h"

#include "game/file.h"
#include "game/game.h"
#include "game/undo.h"
#include "game/state.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/text.h"
#include "graphics/panel.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "translation/translation.h"
#include "widget/top_menu_editor.h"
#include "window/editor/attributes.h"
#include "window/editor/map.h"
#include "window/file_dialog.h"
#include "window/popup_dialog.h"
#include "window/city.h"
#include "window/main_menu.h"
#include "window/mission_briefing.h"

#define MAX_BUTTONS 6

static void menu_file_confirm_exit(int accepted, int checked)
{
    if (accepted) {
        game_exit_editor();
    } else {
        window_editor_map_show();
    }
}

static void button_click(int type, int param2);

static int focus_button_id;

static generic_button buttons[] = {
        {192, 100, 192, 25, button_click, button_none, 1, 0},
        {192, 140, 192, 25, button_click, button_none, 2, 0},
        {192, 180, 192, 25, button_click, button_none, 3, 0},
        {192, 220, 192, 25, button_click, button_none, 4, 0},
        {192, 260, 192, 25, button_click, button_none, 5, 0},
        {192, 300, 192, 25, button_click, button_none, 6, 0},
};

static void draw_foreground(void)
{
    graphics_in_dialog();

    outer_panel_draw(160, 44, 16, 19);

    for (int i = 0; i < MAX_BUTTONS; i++) {
        large_label_draw(buttons[i].x, buttons[i].y, buttons[i].width / 16, focus_button_id == i + 1 ? 1 : 0);
    }

    text_draw_centered(translation_for(TR_LABEL_PAUSE_MENU), 192, 58, 192, FONT_LARGE_BLACK, 0);
    lang_text_draw_centered(13, 5, 192, 106, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(7, 1, 192, 146, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(7, 2, 192, 186, 192, FONT_NORMAL_GREEN);
    lang_text_draw_centered(7, 3, 192, 226, 192, FONT_NORMAL_GREEN);
    text_draw_centered(translation_for(TR_MAP_EDITOR_OPTIONS), 192, 266, 192, FONT_NORMAL_GREEN, 0);
    lang_text_draw_centered(7, 4, 192, 306, 192, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, MAX_BUTTONS, &focus_button_id)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    }
}

static void main_menu_confirmed(int confirmed, int checked)
{
    if (confirmed) {
        if (scenario_is_saved()) {
            game_exit_editor();
        } else {
            window_popup_dialog_show(POPUP_DIALOG_EDITOR_QUIT_WITHOUT_SAVING, menu_file_confirm_exit, 1);
        }
    }
}

static void button_click(int type, int param2)
{
    if (type == 1) {
        window_go_back();
    } else if (type == 2) {
        menu_file_new_map(1);
    } else if (type == 3) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_LOAD);
    } else if (type == 4) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_SAVE);
    } else if (type == 5) {
        window_editor_attributes_show();
    } else if (type == 6) {
        window_popup_dialog_show_confirmation(translation_for(TR_BUTTON_BACK_TO_MAIN_MENU), 0, 0, main_menu_confirmed);
    }
}

void window_map_editor_pause_menu_show(void)
{
    window_type window = {
            WINDOW_EDITOR_MAP,
            window_draw_underlying_window,
            draw_foreground,
            handle_input
    };
    window_show(&window);
}
