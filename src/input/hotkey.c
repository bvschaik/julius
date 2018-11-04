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
#include "window/popup_dialog.h"
#include "window/city.h"

static struct {
    int ctrl_down;
    int alt_down;
    int shift_down;
    int is_cheating;
} data = {0, 0, 0, 0};

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
        for (int i = 1; i <= MAX_LEGIONS; i++) {
            legion_id++;
            if (legion_id > MAX_LEGIONS) {
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

void hotkey_character(int c)
{
    if (data.alt_down) {
        switch (c) {
            case 'X': case 'x':
                hotkey_esc();
                break;
            case 'K': case 'k':
                cheat_init_or_invasion();
            case 'C': case 'c':
                cheat_money();
                break;
            case 'V': case 'v':
                cheat_victory();
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
        case 'P': case 'p':
            toggle_pause();
            break;
        case 'F': case 'f':
            show_overlay(OVERLAY_FIRE);
            break;
        case 'D': case 'd':
            show_overlay(OVERLAY_DAMAGE);
            break;
        case 'C': case 'c':
            show_overlay(OVERLAY_CRIME);
            break;
        case 'T': case 't':
            show_overlay(OVERLAY_PROBLEMS);
            break;
        case 'W': case 'w':
            show_overlay(OVERLAY_WATER);
            break;
        case 'L': case 'l':
            cycle_legion();
            break;
        case '1':
            show_advisor(ADVISOR_LABOR);
            break;
        case '2':
            show_advisor(ADVISOR_MILITARY);
            break;
        case '3':
            show_advisor(ADVISOR_IMPERIAL);
            break;
        case '4':
            show_advisor(ADVISOR_RATINGS);
            break;
        case '5':
            show_advisor(ADVISOR_TRADE);
            break;
        case '6':
            show_advisor(ADVISOR_POPULATION);
            break;
        case '7':
            show_advisor(ADVISOR_HEALTH);
            break;
        case '8':
            show_advisor(ADVISOR_EDUCATION);
            break;
        case '9':
            show_advisor(ADVISOR_ENTERTAINMENT);
            break;
        case '0':
            show_advisor(ADVISOR_RELIGION);
            break;
        case '-':
            show_advisor(ADVISOR_FINANCIAL);
            break;
        case '=':
            show_advisor(ADVISOR_CHIEF);
            break;
    }
}

void hotkey_left(void)
{
    scroll_arrow_left();
}

void hotkey_right(void)
{
    scroll_arrow_right();
}

void hotkey_up(void)
{
    scroll_arrow_up();
}

void hotkey_down(void)
{
    scroll_arrow_down();
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

static void go_to_bookmark(int number)
{
    if (map_bookmark_go_to(number)) {
        window_invalidate();
    }
}

static void handle_bookmark(int number)
{
    exit_military_command();
    if (window_is(WINDOW_CITY)) {
        if (data.ctrl_down || data.shift_down) {
            map_bookmark_save(number);
        } else {
            go_to_bookmark(number);
        }
    }
}

static void take_screenshot(void)
{
    graphics_save_screenshot();
}

void hotkey_func(int f_number)
{
    switch (f_number) {
        case 1:
        case 2:
        case 3:
        case 4:
            handle_bookmark(f_number - 1);
            break;
        case 5: /* center window not implemented */; break;
        case 6: system_set_fullscreen(!setting_fullscreen()); break;
        case 7: system_resize(640, 480); break;
        case 8: system_resize(800, 600); break;
        case 9: system_resize(1024, 768); break;
        case 12: take_screenshot(); break;
    }
}

void hotkey_ctrl(int is_down)
{
    data.ctrl_down = is_down;
}

void hotkey_alt(int is_down)
{
    data.alt_down = is_down;
}

void hotkey_shift(int is_down)
{
    data.shift_down = is_down;
}

void hotkey_reset_state(void)
{
    data.ctrl_down = 0;
    data.alt_down = 0;
    data.shift_down = 0;
}
