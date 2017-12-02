#include "loader.h"

#include "cityview.h"
#include "terrain.h"
#include "ui/warning.h"
#include "core/random.h"

#include <data>
#include <scenario>


void Loader_GameState_init()
{
    Data_State.winState = WinState_None;
    Terrain_initDistanceRing();

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
    Data_State.map.camera.x = 76;
    Data_State.map.camera.y = 152;
    CityView_checkCameraBoundaries();

    random_generate_pool();

    UI_Warning_clearAll();
}
