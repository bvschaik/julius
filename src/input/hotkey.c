#include "hotkey.h"

#include "building/type.h"
#include "city/finance.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "figure/formation.h"
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
#include "window/numeric_input.h"
#include "window/plain_message_dialog.h"
#include "window/popup_dialog.h"
#include "window/city.h"
#include "window/editor/empire.h"

static struct {
    int is_cheating;
} data;

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

static void toggle_overlay(void)
{
    exit_military_command();
    if (window_is(WINDOW_CITY)) {
        game_state_toggle_overlay();
        window_invalidate();
    }
}

static void show_overlay(int overlay)
{
    exit_military_command();
    if (window_is(WINDOW_CITY)) {
        if (game_state_overlay() == overlay) {
            game_state_set_overlay(OVERLAY_NONE);
        } else {
            game_state_set_overlay(overlay);
        }
        window_invalidate();
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

static void show_advisor(advisor_type advisor)
{
    exit_military_command();
    if (window_is(WINDOW_ADVISORS)) {
        if (window_advisors_get_advisor() == advisor) {
            window_city_show();
        } else {
            window_advisors_show_advisor(advisor);
        }
    } else if (window_is(WINDOW_CITY)) {
        window_advisors_show_advisor(advisor);
    }
}

static void cycle_legion(void)
{
    static int current_legion_id = 1;
    if (window_is(WINDOW_CITY)) {
        int legion_id = current_legion_id;
        current_legion_id = 0;
        for (int i = 1; i <= formation_get_max_legions(); i++) {
            legion_id++;
            if (legion_id > formation_get_max_legions()) {
                legion_id = 1;
            }
            const formation *m = formation_get(legion_id);
            if (m->in_use == 1 && !m->is_herd && m->is_legion) {
                if (current_legion_id == 0) {
                    current_legion_id = legion_id;
                    break;
                }
            }
        }
        if (current_legion_id > 0) {
            const formation *m = formation_get(current_legion_id);
            city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
            window_invalidate();
        }
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

static void input_number(int number)
{
    if (window_is(WINDOW_NUMERIC_INPUT)) {
        window_numeric_input_number(number);
    }
}

void hotkey_character(int c, int with_ctrl, int with_alt)
{
    if (with_ctrl && c == 'a') {
        editor_toggle_battle_info();
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
            toggle_overlay();
            break;
        case 'p':
            toggle_pause();
            break;
        case 'f':
            show_overlay(OVERLAY_FIRE);
            break;
        case 'd':
            show_overlay(OVERLAY_DAMAGE);
            break;
        case 'c':
            show_overlay(OVERLAY_CRIME);
            break;
        case 't':
            show_overlay(OVERLAY_PROBLEMS);
            break;
        case 'w':
            show_overlay(OVERLAY_WATER);
            break;
        case 'l':
            cycle_legion();
            break;
        case '1':
            show_advisor(ADVISOR_LABOR);
            input_number(1);
            break;
        case '2':
            show_advisor(ADVISOR_MILITARY);
            input_number(2);
            break;
        case '3':
            show_advisor(ADVISOR_IMPERIAL);
            input_number(3);
            break;
        case '4':
            show_advisor(ADVISOR_RATINGS);
            input_number(4);
            break;
        case '5':
            show_advisor(ADVISOR_TRADE);
            input_number(5);
            break;
        case '6':
            show_advisor(ADVISOR_POPULATION);
            input_number(6);
            break;
        case '7':
            show_advisor(ADVISOR_HEALTH);
            input_number(7);
            break;
        case '8':
            show_advisor(ADVISOR_EDUCATION);
            input_number(8);
            break;
        case '9':
            show_advisor(ADVISOR_ENTERTAINMENT);
            input_number(9);
            break;
        case '0':
            show_advisor(ADVISOR_RELIGION);
            input_number(0);
            break;
        case '-':
            show_advisor(ADVISOR_FINANCIAL);
            break;
        case '=':
            show_advisor(ADVISOR_CHIEF);
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

void hotkey_enter(void)
{
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
