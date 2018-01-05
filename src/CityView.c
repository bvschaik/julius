#include "CityView.h"

#include "Data/CityView.h"
#include "Data/Screen.h"
#include "Data/State.h"

#include "city/view.h"
#include "core/direction.h"
#include "map/grid.h"
#include "map/image.h"

static void setViewport(int xOffset, int yOffset, int widthInTiles, int heightInTiles);

void CityView_setViewport()
{
	if (Data_State.sidebarCollapsed) {
		CityView_setViewportWithoutSidebar();
	} else {
		CityView_setViewportWithSidebar();
	}
}

void CityView_setViewportWithoutSidebar()
{
	setViewport(0, 24,
		(Data_Screen.width - 40) / 60,
		(Data_Screen.height - 24) / 15);
}

void CityView_setViewportWithSidebar()
{
	setViewport(0, 24,
		(Data_Screen.width - 160) / 60,
		(Data_Screen.height - 24) / 15);
}

static void setViewport(int xOffset, int yOffset, int widthInTiles, int heightInTiles)
{
	Data_CityView.xOffsetInPixels = xOffset;
	Data_CityView.yOffsetInPixels = yOffset;
	Data_CityView.widthInPixels = widthInTiles * 60 - 2;
	Data_CityView.heightInPixels = heightInTiles * 15;
	Data_CityView.widthInTiles = widthInTiles;
	Data_CityView.heightInTiles = heightInTiles;
	Data_CityView.xInTiles = GRID_SIZE / 2;
	Data_CityView.yInTiles = GRID_SIZE;
}
