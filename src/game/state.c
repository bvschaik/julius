#include "state.h"

#include "city/view.h"
#include "city/warning.h"
#include "core/random.h"
#include "map/ring.h"

#include "Data/State.h"
#include "../CityView.h"

static struct {
    int paused;
    int current_overlay;
    int previous_overlay;
} data = {0, OVERLAY_NONE, OVERLAY_NONE};

void game_state_init()
{
    Data_State.winState = WinState_None;
    map_ring_init();

    Data_State.map.orientation = 0;
    city_view_calculate_lookup();
    if (Data_State.sidebarCollapsed) {
        CityView_setViewportWithoutSidebar();
    } else {
        CityView_setViewportWithSidebar();
    }
    Data_State.map.camera.x = 76;
    Data_State.map.camera.y = 152;
    city_view_check_camera_boundaries();

    random_generate_pool();

    city_warning_clear_all();
}

int game_state_is_paused()
{
    return data.paused;
}

void game_state_unpause()
{
    data.paused = 0;
}

void game_state_toggle_paused()
{
    data.paused = data.paused ? 0 : 1;
}

int game_state_overlay()
{
    return data.current_overlay;
}

void game_state_reset_overlay()
{
    data.current_overlay = OVERLAY_NONE;
    data.previous_overlay = OVERLAY_NONE;
}

void game_state_toggle_overlay()
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
