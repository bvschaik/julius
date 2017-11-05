#include "Loader.h"

#include "CityView.h"
#include "KeyboardInput.h"
#include "Terrain.h"
#include "UI/Warning.h"

#include "data/KeyboardInput.h"
#include "data/FileList.h"
#include "data/Screen.h"
#include "data/Settings.h"
#include "data/State.h"

#include "core/random.h"

void Loader_GameState_init()
{
	Data_State.winState = WinState_None;
	Terrain_initDistanceRing();

	Data_Settings_Map.orientation = 0;
	CityView_calculateLookup();
	if (Data_State.sidebarCollapsed) {
		CityView_setViewportWithoutSidebar();
	} else {
		CityView_setViewportWithSidebar();
	}
	Data_Settings_Map.camera.x = 76;
	Data_Settings_Map.camera.y = 152;
	CityView_checkCameraBoundaries();

	random_generate_pool();

	Data_KeyboardInput.current = 0;
	KeyboardInput_initTextField(1, Data_Settings.playerName, 25, 200, 0, FONT_NORMAL_WHITE);
	KeyboardInput_initTextField(2, (uint8_t*)Data_FileList.selectedCity, 64, 280, 1, FONT_NORMAL_WHITE);

	UI_Warning_clearAll();
}
