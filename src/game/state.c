#include "state.h"

#include "city/victory.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/speed.h"
#include "core/random.h"
#include "map/ring.h"

#include <math.h>

static struct {
    int paused;
    int current_overlay;
    int previous_overlay;
    speed_type game_speed;
} data;

#define GAME_SPEED_MULTIPLIER 1.46

void game_state_init(int speed)
{
    city_victory_reset();
    map_ring_init();

    game_state_set_speed(speed, 1);

    city_view_reset_orientation();
    city_view_set_camera(76, 152);

    random_generate_pool();

    city_warning_clear_all();
}

int game_state_is_paused(void)
{
    return data.paused;
}

void game_state_unpause(void)
{
    data.paused = 0;
}

void game_state_toggle_paused(void)
{
    data.paused = data.paused ? 0 : 1;
}

void game_state_set_speed(int speed, int adjust_for_time)
{
    int game_speed_index = (100 - speed) / 10;
    if (game_speed_index >= 10) {
        game_speed_index = 9;
    } else if (game_speed_index < 0) {
        game_speed_index /= 10;
    }
    speed_set_target(&data.game_speed, pow(GAME_SPEED_MULTIPLIER, -game_speed_index), SPEED_CHANGE_IMMEDIATE, adjust_for_time);
}

int game_state_get_ticks(void)
{
    return speed_get_delta(&data.game_speed);
}

int game_state_overlay(void)
{
    return data.current_overlay;
}

void game_state_reset_overlay(void)
{
    data.current_overlay = OVERLAY_NONE;
    data.previous_overlay = OVERLAY_NONE;
}

void game_state_toggle_overlay(void)
{
    int tmp = data.previous_overlay;
    data.previous_overlay = data.current_overlay;
    data.current_overlay = tmp;
}

void game_state_set_overlay(int overlay)
{
    if (overlay == OVERLAY_NONE) {
        data.previous_overlay = data.current_overlay;
    } else {
        data.previous_overlay = OVERLAY_NONE;
    }
    data.current_overlay = overlay;
}
