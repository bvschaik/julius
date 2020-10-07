#include "game/speed.h"

#include "building/construction.h"
#include "core/time.h"
#include "game/settings.h"
#include "game/state.h"
#include "graphics/window.h"
#include "input/scroll.h"

#define MAX_TICKS_PER_FRAME 20

static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {
    702, 502, 352, 242, 162, 112, 82, 57, 37, 22, 16
};
static const time_millis MILLIS_PER_HYPER_SPEED[] = {
    702, 16, 8, 5, 3, 2
};

static struct {
    int last_check_was_valid;
    time_millis last_update;
} data;

int game_speed_get_elapsed_ticks(void)
{
    int last_check_was_valid = data.last_check_was_valid;
    data.last_check_was_valid = 0;
    if (game_state_is_paused()) {
        return 0;
    }
    int millis_per_tick = 1;
    switch (window_get_id()) {
        default:
            return 0;
        case WINDOW_CITY:
        case WINDOW_CITY_MILITARY:
        case WINDOW_SLIDING_SIDEBAR:
        case WINDOW_OVERLAY_MENU:
        case WINDOW_BUILD_MENU: {
            int speed = setting_game_speed();
            if (speed < 10) {
                return 0;
            } else if (speed <= 100) {
                millis_per_tick = MILLIS_PER_TICK_PER_SPEED[speed / 10];
            } else {
                if (speed > 500) {
                    speed = 500;
                }
                millis_per_tick = MILLIS_PER_HYPER_SPEED[speed / 100];
            }
            break;
        }
        case WINDOW_EDITOR_MAP:
            millis_per_tick = MILLIS_PER_TICK_PER_SPEED[7]; // 70%, nice speed for flag animations
            break;
    }
    if (building_construction_in_progress()) {
        return 0;
    }
    if (scroll_in_progress() && !scroll_is_smooth()) {
        return 0;
    }

    time_millis now = time_get_millis();
    time_millis diff = now - data.last_update;
    data.last_check_was_valid = 1;
    if (!last_check_was_valid) {
        // returning to map from another window or pause: always force a tick
        data.last_update = now;
        return 1;
    }
    int ticks = diff / millis_per_tick;
    if (!ticks) {
        return 0;
    } else if (ticks <= MAX_TICKS_PER_FRAME) {
        data.last_update = now - (diff % millis_per_tick); // account for left-over millis in this frame
        return ticks;
    } else {
        data.last_update = now;
        return MAX_TICKS_PER_FRAME;
    }
}
