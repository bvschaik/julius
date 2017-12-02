#include "loader.h"

#include "cityview.h"
#include "keyboardinput.h"
#include "terrain.h"
#include "ui/warning.h"
#include "core/random.h"

#include <data>
#include <scenario>


void Loader_GameState_init()
{
    Data_State.winState = WinState_None;
    Terrain_initDistanceRing();

    Data_Settings_Map.orientation = 0;
    CityView_calculateLookup();
    if (Data_State.sidebarCollapsed)
    {
        CityView_setViewportWithoutSidebar();
    }
    else
    {
        CityView_setViewportWithSidebar();
    }
    Data_Settings_Map.camera.x = 76;
    Data_Settings_Map.camera.y = 152;
    CityView_checkCameraBoundaries();

    random_generate_pool();

    Data_KeyboardInput.current = 0;
    KeyboardInput_initTextField(1, scenario.settings.player_name, 25, 200, 0, FONT_NORMAL_WHITE);
    KeyboardInput_initTextField(2, Data_FileList.selectedCity, 64, 280, 1, FONT_NORMAL_WHITE);

    UI_Warning_clearAll();
}
