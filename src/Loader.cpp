#include "Loader.hpp"

#include "CityView.hpp"
#include "KeyboardInput.hpp"
#include "Terrain.hpp"
#include "UI/Warning.hpp"

#include "Data/KeyboardInput.hpp"
#include "Data/FileList.hpp"
#include "Data/Screen.hpp"
#include "Data/Settings.hpp"
#include "Data/State.hpp"

#include "core/random.hpp"

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
    KeyboardInput_initTextField(1, Data_Settings.playerName, 25, 200, 0, FONT_NORMAL_WHITE);
    KeyboardInput_initTextField(2, Data_FileList.selectedCity, 64, 280, 1, FONT_NORMAL_WHITE);

    UI_Warning_clearAll();
}
