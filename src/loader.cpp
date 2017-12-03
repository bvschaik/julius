#include "loader.h"

#include "cityview.h"
#include "terrain.h"
#include "ui/warning.h"
#include "core/random.h"

#include <data>
#include <scenario>
#include <game>

void Loader_GameState_init()
{
    Data_State.winState = WinState_None;
    map_ring_init();

    Data_State.map.orientation = 0;
    CityView_calculateLookup();
    if (Data_State.sidebarCollapsed)
    {
        CityView_setViewportWithoutSidebar();
    }
    else
    {
        CityView_setViewportWithSidebar();
    }
    Data_State.map.camera = { 76, 152 };
    CityView_checkCameraBoundaries();

    random_generate_pool();

    UI_Warning_clearAll();
}
