#include "Terrain.h"
#include "Terrain_private.h"

#include "building/building.h"
#include "core/calc.h"
#include "figure/figure.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/property.h"
#include "map/terrain.h"
#include "scenario/map.h"

#include "Data/CityInfo.h"

int Terrain_Water_getFreeDockDestination(int figureId, int *xTile, int *yTile)
{
	if (Data_CityInfo.numWorkingDocks <= 0) {
		return 0;
	}
	int dockId;
	for (int i = 0; i < 10; i++) {
		dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
        building *dock = building_get(dockId);
		if (!dock->data.other.boatFigureId || dock->data.other.boatFigureId == figureId) {
			break;
		}
	}
	// BUG: when 10 docks in city, always takes last one... regardless of whether it is free
	if (dockId <= 0) {
		return 0;
	}
	building *dock = building_get(dockId);
	*xTile = dock->x;
	*yTile = dock->y;
	switch (dock->data.other.dockOrientation) {
		case 0: *xTile += 1; *yTile -= 1; break;
		case 1: *xTile += 3; *yTile += 1; break;
		case 2: *xTile += 1; *yTile += 3; break;
		default: *xTile -= 1; *yTile += 1; break;
	}
	dock->data.other.boatFigureId = figureId;
	return dockId;
}

int Terrain_Water_getQueueDockDestination(int* xTile, int* yTile)
{
	if (Data_CityInfo.numWorkingDocks <= 0) {
		return 0;
	}
	// first queue position
	for (int i = 0; i < 10; i++) {
		int dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
        building *dock = building_get(dockId);
		*xTile = dock->x;
		*yTile = dock->y;
		switch (dock->data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 2; break;
			case 1: *xTile += 4; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 4; break;
			default: *xTile -= 2; *yTile += 2; break;
		}
		if (!map_has_figure_at(map_grid_offset(*xTile, *yTile))) {
			return dockId;
		}
	}
	// second queue position
	for (int i = 0; i < 10; i++) {
		int dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
        building *dock = building_get(dockId);
		*xTile = dock->x;
		*yTile = dock->y;
		switch (dock->data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 3; break;
			case 1: *xTile += 5; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 5; break;
			default: *xTile -= 3; *yTile += 2; break;
		}
		if (!map_has_figure_at(map_grid_offset(*xTile, *yTile))) {
			return dockId;
		}
	}
	return 0;
}

