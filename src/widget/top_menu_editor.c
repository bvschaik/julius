#include "top_menu_editor.h"

#include "empire/empire.h"
#include "game/file_editor.h"
#include "game/game.h"
#include "game/system.h"
#include "graphics/image.h"
#include "graphics/menu.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "scenario/editor.h"
#include "scenario/editor_map.h"
#include "scenario/empire.h"
#include "scenario/scenario.h"
#include "translation/translation.h"
#include "window/config.h"
#include "window/file_dialog.h"
#include "window/hotkey_config.h"
#include "window/message_dialog.h"
#include "window/popup_dialog.h"
#include "window/select_list.h"
#include "window/editor/empire.h"
#include "window/editor/map.h"

void menu_file_new_map(int param);
static void menu_file_load_map(int param);
static void menu_file_save_map(int param);
static void menu_file_exit_to_menu(int param);
static void menu_file_exit_game(int param);

static void menu_options_general(int param);
static void menu_options_user_interface(int param);
static void menu_options_hotkeys(int param);

static void menu_help_help(int param);
static void menu_help_about(int param);

static void menu_resets_herds(int param);
static void menu_resets_fish(int param);
static void menu_resets_invasions(int param);

static void menu_empire_choose(int param);
static void menu_empire_custom(int param);

static menu_item menu_file[] = {
    {7, 1, menu_file_new_map, 0},
    {7, 2, menu_file_load_map, 0},
    {7, 3, menu_file_save_map, 0},
    {CUSTOM_TRANSLATION, TR_BUTTON_BACK_TO_MAIN_MENU, menu_file_exit_to_menu, 0},
    {1, 5, menu_file_exit_game, 0}
};

static menu_item menu_options[] = {
    {CUSTOM_TRANSLATION, TR_CONFIG_HEADER_GENERAL, menu_options_general, 0},
    {CUSTOM_TRANSLATION, TR_CONFIG_HEADER_UI_CHANGES, menu_options_user_interface, 0},
    {CUSTOM_TRANSLATION, TR_BUTTON_CONFIGURE_HOTKEYS, menu_options_hotkeys, 0}
};

static menu_item menu_help[] = {
    {3, 1, menu_help_help, 0},
    {3, 7, menu_help_about, 0},
};

static menu_item menu_resets[] = {
    {10, 1, menu_resets_herds, 0},
    {10, 2, menu_resets_fish, 0},
    {10, 3, menu_resets_invasions, 0},
};

static menu_item menu_empire[] = {
    {149, 1, menu_empire_choose, 0},
    {CUSTOM_TRANSLATION, TR_EDITOR_CHOOSE_CUSTOM_EMPIRE, menu_empire_custom, 0},
};

static menu_bar_item menu[] = {
    {7, menu_file, 5},
    {2, menu_options, 3},
    {3, menu_help, 2},
    {10, menu_resets, 3},
    {149, menu_empire, 2},
};

#define INDEX_OPTIONS 1

static struct {
    int open_sub_menu;
    int focus_menu_id;
    int focus_sub_menu_id;
} data;

static void clear_state(void)
{
    data.open_sub_menu = 0;
    data.focus_menu_id = 0;
    data.focus_sub_menu_id = 0;
}

static void draw_foreground(void)
{
    if (!data.open_sub_menu) {
        return;
    }
    menu_draw(&menu[data.open_sub_menu - 1], data.focus_sub_menu_id);
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    widget_top_menu_editor_handle_input(m, h);
}

static void top_menu_window_show(void)
{
    window_type window = {
        WINDOW_EDITOR_TOP_MENU,
        window_editor_map_draw_all,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}

void widget_top_menu_editor_draw(void)
{
    int block_width = 24;
    int image_base = image_group(GROUP_TOP_MENU);
    int s_width = screen_width();
    for (int i = 0; i * block_width < s_width; i++) {
        image_draw(image_base + i % 8, i * block_width, 0, COLOR_MASK_NONE, SCALE_NONE);
    }
    menu_bar_draw(menu, 5, s_width);
}

static int handle_input_submenu(const mouse *m, const hotkeys *h)
{
    if (m->right.went_up || h->escape_pressed) {
        clear_state();
        window_go_back();
        return 1;
    }
    int menu_id = menu_bar_handle_mouse(m, menu, 5, &data.focus_menu_id);
    if (menu_id && menu_id != data.open_sub_menu) {
        window_request_refresh();
        data.open_sub_menu = menu_id;
    }
    if (!menu_handle_mouse(m, &menu[data.open_sub_menu - 1], &data.focus_sub_menu_id)) {
        if (m->left.went_up) {
            clear_state();
            window_go_back();
            return 1;
        }
    }
    return 0;
}

static int handle_mouse_menu(const mouse *m)
{
    int menu_id = menu_bar_handle_mouse(m, menu, 5, &data.focus_menu_id);
    if (menu_id && m->left.went_up) {
        data.open_sub_menu = menu_id;
        top_menu_window_show();
        return 1;
    }
    return 0;
}

int widget_top_menu_editor_handle_input(const mouse *m, const hotkeys *h)
{
    if (data.open_sub_menu) {
        return handle_input_submenu(m, h);
    } else {
        return handle_mouse_menu(m);
    }
}

static void map_size_selected(int size)
{
    clear_state();
    if (size >= 0 && size <= 5) {
        game_file_editor_create_scenario(size);
        window_editor_map_show();
    } else {
        window_go_back();
    }
}

void menu_file_new_map(int centered)
{
    int x = 50;
    int y = 50;
    if (centered) {
        x += 325;
        y += 200;
    }
    window_select_list_show(x, y, 33, 7, map_size_selected);
}

static void menu_file_load_map(int param)
{
    clear_state();
    window_editor_map_show();
    window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_LOAD);
}

static void menu_file_save_map(int param)
{
    clear_state();
    window_editor_map_show();
    window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_SAVE);
}

static void menu_file_confirm_exit_to_menu(int accepted, int checked)
{
    if (accepted) {
        game_exit_editor();
    } else {
        window_editor_map_show();
    }
}

static void menu_file_exit_to_menu(int param)
{
    clear_state();
    window_editor_map_show();
    if (scenario_is_saved()) {
        game_exit_editor();
    } else {
        window_popup_dialog_show(POPUP_DIALOG_EDITOR_QUIT_WITHOUT_SAVING, menu_file_confirm_exit_to_menu, 1);
    }
}

static void menu_file_confirm_exit_game(int accepted, int checked)
{
    if (accepted) {
        system_exit();
    } else {
        window_editor_map_show();
    }
}

static void menu_file_exit_game(int param)
{
    clear_state();
    window_popup_dialog_show(POPUP_DIALOG_QUIT, menu_file_confirm_exit_game, 1);
}

static void menu_options_general(int param)
{
    clear_state();
    window_editor_map_show();
    window_config_show(CONFIG_PAGE_GENERAL, 0);
}

static void menu_options_user_interface(int param)
{
    clear_state();
    window_go_back();
    window_config_show(CONFIG_PAGE_UI_CHANGES, 0);
}

static void menu_options_hotkeys(int param)
{
    clear_state();
    window_go_back();
    window_hotkey_config_show();
}

static void menu_help_help(int param)
{
    clear_state();
    window_go_back();
    window_message_dialog_show(MESSAGE_DIALOG_EDITOR_HELP, window_editor_map_draw_all);
}

static void menu_help_about(int param)
{
    clear_state();
    window_go_back();
    window_message_dialog_show(MESSAGE_DIALOG_EDITOR_ABOUT, window_editor_map_draw_all);
}

static void menu_resets_herds(int param)
{
    scenario_editor_clear_herd_points();
    clear_state();
    window_go_back();
}

static void menu_resets_fish(int param)
{
    scenario_editor_clear_fishing_points();
    clear_state();
    window_go_back();
}

static void menu_resets_invasions(int param)
{
    scenario_editor_clear_invasion_points();
    clear_state();
    window_go_back();
}

static void menu_empire_choose(int param)
{
    clear_state();
    window_go_back();
    scenario_editor_unset_custom_empire();
    empire_load(1, scenario_empire_id());
    window_editor_empire_show();
}

static void menu_empire_custom(int param)
{
    clear_state();
    window_go_back();
    window_file_dialog_show(FILE_TYPE_EMPIRE, FILE_DIALOG_LOAD);
}
