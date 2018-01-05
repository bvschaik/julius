#include "hotkey.h"

#include "Graphics.h"
#include "System.h"
#include "Video.h"

#include "UI/Advisors.h"
#include "UI/BuildingInfo.h"
#include "UI/PopupDialog.h"
#include "UI/Sidebar.h"
#include "UI/Window.h"

#include "Data/State.h"

#include "building/type.h"
#include "city/finance.h"
#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "figure/formation.h"
#include "game/settings.h"
#include "game/state.h"
#include "input/scroll.h"
#include "map/bookmark.h"
#include "map/grid.h"
#include "scenario/invasion.h"

static struct {
    int ctrl_down;
    int alt_down;
    int shift_down;
    int is_cheating;
} data = {0, 0, 0, 0};

static void change_game_speed(int is_down)
{
    if (UI_Window_getId() == Window_City) {
        if (is_down) {
            setting_decrease_game_speed();
        } else {
            setting_increase_game_speed();
        }
    }
}

static void exit_military_command()
{
    if (UI_Window_getId() == Window_CityMilitary) {
        UI_Window_goTo(Window_City);
    }
}

static void toggle_overlay()
{
    exit_military_command();
    if (UI_Window_getId() == Window_City) {
        game_state_toggle_overlay();
        UI_Window_requestRefresh();
    }
}

static void show_overlay(int overlay)
{
    exit_military_command();
    WindowId window = UI_Window_getId();
    if (window == Window_City) {
        if (game_state_overlay() == overlay) {
            game_state_set_overlay(OVERLAY_NONE);
        } else {
            game_state_set_overlay(overlay);
        }
        UI_Window_requestRefresh();
    }
}

static void toggle_pause()
{
    exit_military_command();
    if (UI_Window_getId() == Window_City) {
        game_state_toggle_paused();
        city_warning_clear_all();
    }
}

static void show_advisor(advisor_type advisor)
{
    exit_military_command();
    if (UI_Window_getId() == Window_Advisors) {
        if (UI_Advisors_getId() == advisor) {
            UI_Window_goTo(Window_City);
        } else {
            UI_Advisors_goToFromMessage(advisor);
        }
    } else if (UI_Window_getId() == Window_City) {
        UI_Advisors_goToFromMessage(advisor);
    }
}

static void cycle_legion()
{
    static int currentLegionId = 1;
    if (UI_Window_getId() == Window_City) {
        int legionId = currentLegionId;
        currentLegionId = 0;
        for (int i = 1; i <= MAX_LEGIONS; i++) {
            legionId++;
            if (legionId > MAX_LEGIONS) {
                legionId = 1;
            }
            const formation *m = formation_get(legionId);
            if (m->in_use == 1 && !m->is_herd && m->is_legion) {
                if (currentLegionId == 0) {
                    currentLegionId = legionId;
                    break;
                }
            }
        }
        if (currentLegionId > 0) {
            const formation *m = formation_get(currentLegionId);
            city_view_go_to_grid_offset(map_grid_offset(m->x_home, m->y_home));
            UI_Window_requestRefresh();
        }
    }
}

static void cheat_init_or_invasion()
{
    if (UI_Window_getId() == Window_BuildingInfo) {
        data.is_cheating = UI_BuildingInfo_getBuildingType() == BUILDING_WELL;
    } else if (data.is_cheating && UI_Window_getId() == Window_MessageDialog) {
        data.is_cheating = 2;
        scenario_invasion_start_from_cheat();
    } else {
        data.is_cheating = 0;
    }
}

static void cheat_victory()
{
    if (data.is_cheating) {
        city_victory_force_win();
    }
}

static void cheat_money()
{
    if (data.is_cheating) {
        city_finance_process_cheat();
        UI_Window_requestRefresh();
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

void hotkey_left()
{
    scroll_arrow_left();
}

void hotkey_right()
{
    scroll_arrow_right();
}

void hotkey_up()
{
    scroll_arrow_up();
}

void hotkey_down()
{
    scroll_arrow_down();
}

void hotkey_home()
{
    if (UI_Window_getId() == Window_City) {
        UI_Sidebar_rotateMap(0);
    }
}

void hotkey_end()
{
    if (UI_Window_getId() == Window_City) {
        UI_Sidebar_rotateMap(1);
    }
}

static void confirm_exit(int accepted)
{
    if (accepted) {
        System_exit();
    }
}

void hotkey_esc()
{
    Video_stop();
    UI_PopupDialog_show(PopupDialog_Quit, confirm_exit, 1);
}

static void go_to_bookmark(int number)
{
    if (map_bookmark_go_to(number)) {
        UI_Window_requestRefresh();
    }
}

static void handle_bookmark(int number)
{
    exit_military_command();
    if (UI_Window_getId() == Window_City) {
        if (data.ctrl_down || data.shift_down) {
            map_bookmark_save(number);
        } else {
            go_to_bookmark(number);
        }
    }
}

static void take_screenshot()
{
    Graphics_saveScreenshot("city.bmp");
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
        case 6: System_toggleFullscreen(); break;
        case 7: System_resize(640, 480); break;
        case 8: System_resize(800, 600); break;
        case 9: System_resize(1024, 768); break;
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

void hotkey_reset_state()
{
    data.ctrl_down = 0;
    data.alt_down = 0;
    data.shift_down = 0;
}
