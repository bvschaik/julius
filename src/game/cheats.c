#include "cheats.h"

#include "building/type.h"
#include "city/finance.h"
#include "city/victory.h"
#include "graphics/window.h"
#include "scenario/invasion.h"
#include "window/building_info.h"

static struct {
    int is_cheating;
} data;

void game_cheat_activate(void)
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

void game_cheat_money(void)
{
    if (data.is_cheating) {
        city_victory_force_win();
    }
}

void game_cheat_victory(void)
{
    if (data.is_cheating) {
        city_finance_process_cheat();
        window_invalidate();
    }
}
