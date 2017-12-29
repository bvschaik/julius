#include "Loader.h"

#include "CityView.h"

#include "Data/State.h"

#include "city/warning.h"
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

	city_warning_clear_all();
}
