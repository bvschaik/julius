#include "Loader.h"

#include "CityView.h"
#include "KeyboardInput.h"
#include "Terrain.h"
#include "UI/Warning.h"

#include "Data/KeyboardInput.h"
#include "Data/FileList.h"
#include "Data/Screen.h"
#include "Data/Settings.h"
#include "Data/State.h"

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
	KeyboardInput_initTextField(1, Data_Settings.playerName, 25, 200, 0, Font_NormalWhite);
	KeyboardInput_initTextField(2, Data_FileList.selectedCity, 64, 280, 1, Font_NormalWhite);

	UI_Warning_clearAll();
}
