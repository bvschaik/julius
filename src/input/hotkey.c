#include "hotkey.h"

#include "building/type.h"
#include "city/finance.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "game/orientation.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "graphics/screenshot.h"
#include "graphics/video.h"
#include "graphics/window.h"
#include "input/scroll.h"
#include "map/bookmark.h"
#include "map/grid.h"
#include "scenario/invasion.h"
#include "window/advisors.h"
#include "window/building_info.h"
#include "window/file_dialog.h"
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"
#include "window/city.h"
#include "window/editor/empire.h"

#include <string.h>

static struct {
    int is_cheating;
    hotkeys hotkey_state;
} data;

const hotkeys *hotkey_state(void)
{
    return &data.hotkey_state;
}

void hotkey_reset_state(void)
{
    memset(&data.hotkey_state, 0, sizeof(data.hotkey_state));
}

static void change_game_speed(int is_down)
{
    if (window_is(WINDOW_CITY)) {
        if (is_down) {
            setting_decrease_game_speed();
        } else {
            setting_increase_game_speed();
        }
    }
}

static void exit_military_command(void)
{
    if (window_is(WINDOW_CITY_MILITARY)) {
        window_city_show();
    }
}

static void toggle_pause(void)
{
    exit_military_command();
    if (window_is(WINDOW_CITY)) {
        game_state_toggle_paused();
        city_warning_clear_all();
    }
}

static void cheat_init_or_invasion(void)
{
    if (window_is(WINDOW_BUILDING_INFO)) {
        data.is_cheating = window_building_info_get_building_type() == BUILDING_WELL;
    } else if (data.is_cheating && window_is(WINDOW_MESSAGE_DIALOG)) {
        data.is_cheating = 2;
        scenario_invasion_start_from_cheat();
    } else {
        data.is_cheating = 0;
    }
}

static void cheat_victory(void)
{
    if (data.is_cheating) {
        city_victory_force_win();
    }
}

static void cheat_money(void)
{
    if (data.is_cheating) {
        city_finance_process_cheat();
        window_invalidate();
    }
}

static void editor_toggle_battle_info(void)
{
    if (window_is(WINDOW_EDITOR_EMPIRE)) {
        window_editor_empire_toggle_battle_info();
    }
}

static void load_file(void)
{
    if (window_is(WINDOW_EDITOR_MAP)) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_LOAD);
    } else if (window_is(WINDOW_CITY) || window_is(WINDOW_MAIN_MENU)) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_LOAD);
    }
}

static void save_file(void)
{
    if (window_is(WINDOW_EDITOR_MAP)) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_SAVE);
    } else if (window_is(WINDOW_CITY)) {
        window_file_dialog_show(FILE_TYPE_SAVED_GAME, FILE_DIALOG_SAVE);
    }
}

void hotkey_character(int c, int with_ctrl, int with_alt)
{
    if (with_ctrl) {
        switch (c) {
            case 'a':
                editor_toggle_battle_info();
                break;
            case 'o':
                load_file();
                break;
            case 's':
                save_file();
                break;
        }
        return;
    }
    if (with_alt) {
        switch (c) {
            case 'x':
                hotkey_esc();
                break;
            case 'k':
                cheat_init_or_invasion();
                break;
            case 'c':
                cheat_money();
                break;
            case 'v':
                cheat_victory();
                break;

            // Azerty keyboards need alt gr for these keys
            case '[': case '5':
                change_game_speed(1);
                break;
            case ']': case '-':
                change_game_speed(0);
                break;
        }
        return;
    }

    switch (c) {
        case '[':
            change_game_speed(1);
            break;
        case ']':
            change_game_speed(0);
            break;
        case ' ':
            data.hotkey_state.toggle_overlay = 1;
            break;
        case 'p':
            toggle_pause();
            break;
        case 'f':
            data.hotkey_state.show_overlay = OVERLAY_FIRE;
            break;
        case 'd':
            data.hotkey_state.show_overlay = OVERLAY_DAMAGE;
            break;
        case 'c':
            data.hotkey_state.show_overlay = OVERLAY_CRIME;
            break;
        case 't':
            data.hotkey_state.show_overlay = OVERLAY_PROBLEMS;
            break;
        case 'w':
            data.hotkey_state.show_overlay = OVERLAY_WATER;
            break;
        case 'l':
            data.hotkey_state.cycle_legion = 1;
            break;
        case '1':
            data.hotkey_state.show_advisor = ADVISOR_LABOR;
            break;
        case '2':
            data.hotkey_state.show_advisor = ADVISOR_MILITARY;
            break;
        case '3':
            data.hotkey_state.show_advisor = ADVISOR_IMPERIAL;
            break;
        case '4':
            data.hotkey_state.show_advisor = ADVISOR_RATINGS;
            break;
        case '5':
            data.hotkey_state.show_advisor = ADVISOR_TRADE;
            break;
        case '6':
            data.hotkey_state.show_advisor = ADVISOR_POPULATION;
            break;
        case '7':
            data.hotkey_state.show_advisor = ADVISOR_HEALTH;
            break;
        case '8':
            data.hotkey_state.show_advisor = ADVISOR_EDUCATION;
            break;
        case '9':
            data.hotkey_state.show_advisor = ADVISOR_ENTERTAINMENT;
            break;
        case '0':
            data.hotkey_state.show_advisor = ADVISOR_RELIGION;
            break;
        case '-':
            data.hotkey_state.show_advisor = ADVISOR_FINANCIAL;
            break;
        case '=':
        case '+':
            data.hotkey_state.show_advisor = ADVISOR_CHIEF;
            break;
    }
}

void hotkey_left_press(void)
{
    scroll_arrow_left(1);
}

void hotkey_right_press(void)
{
    scroll_arrow_right(1);
}

void hotkey_up_press(void)
{
    scroll_arrow_up(1);
}

void hotkey_down_press(void)
{
    scroll_arrow_down(1);
}

void hotkey_left_release(void)
{
    scroll_arrow_left(0);
}

void hotkey_right_release(void)
{
    scroll_arrow_right(0);
}

void hotkey_up_release(void)
{
    scroll_arrow_up(0);
}

void hotkey_down_release(void)
{
    scroll_arrow_down(0);
}

void hotkey_home(void)
{
    if (window_is(WINDOW_CITY)) {
        game_orientation_rotate_left();
        window_invalidate();
    }
}

void hotkey_end(void)
{
    if (window_is(WINDOW_CITY)) {
        game_orientation_rotate_right();
        window_invalidate();
    }
}

static void confirm_exit(int accepted)
{
    if (accepted) {
        system_exit();
    }
}

void hotkey_esc(void)
{
    video_stop();
    window_popup_dialog_show(POPUP_DIALOG_QUIT, confirm_exit, 1);
}

void hotkey_page_up(void)
{
    change_game_speed(0);
}

void hotkey_page_down(void)
{
    change_game_speed(1);
}

void hotkey_enter(int with_alt)
{
    if (with_alt) {
        system_set_fullscreen(!setting_fullscreen());
        return;
    }

    if (window_is(WINDOW_POPUP_DIALOG)) {
        window_popup_dialog_confirm();
    } else if (window_is(WINDOW_PLAIN_MESSAGE_DIALOG)) {
        window_plain_message_dialog_accept();
    } else if (window_is(WINDOW_NUMERIC_INPUT)) {
        window_numeric_input_accept();
    }
}

static void go_to_bookmark(int number)
{
    if (map_bookmark_go_to(number)) {
        window_invalidate();
    }
}

static void handle_bookmark(int number, int with_modifier)
{
    exit_military_command();
    if (window_is(WINDOW_CITY)) {
        if (with_modifier) {
            map_bookmark_save(number);
        } else {
            go_to_bookmark(number);
        }
    }
}

static void take_screenshot(int full_city)
{
    graphics_save_screenshot(full_city);
}

void hotkey_func(int f_number, int with_any_modifier, int with_ctrl)
{
    switch (f_number) {
        case 1:
        case 2:
        case 3:
        case 4:
            handle_bookmark(f_number - 1, with_any_modifier);
            break;
        case 5: system_center(); break;
        case 6: system_set_fullscreen(!setting_fullscreen()); break;
        case 7: system_resize(640, 480); break;
        case 8: system_resize(800, 600); break;
        case 9: system_resize(1024, 768); break;
        case 12: take_screenshot(with_ctrl); break;
    }
}
