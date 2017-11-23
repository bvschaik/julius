#include "Loader.h"

#include "CityView.h"
#include "Terrain.h"
#include "UI/Warning.h"

#include "Data/FileList.h"
#include "Data/Screen.h"
#include "Data/State.h"

#include "core/random.h"
#include "map/ring.h"

void Loader_GameState_init()
{
	Data_State.winState = WinState_None;
	map_ring_init();

	Data_State.map.orientation = 0;
	CityView_calculateLookup();
	if (Data_State.sidebarCollapsed) {
		CityView_setViewportWithoutSidebar();
	} else {
		CityView_setViewportWithSidebar();
	}
	Data_State.map.camera.x = 76;
	Data_State.map.camera.y = 152;
	CityView_checkCameraBoundaries();

	random_generate_pool();

	UI_Warning_clearAll();
}
