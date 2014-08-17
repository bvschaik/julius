#include "Terrain.h"
#include "Terrain_private.h"

#include "Calc.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Scenario.h"
#include "Data/Walker.h"

int Terrain_Water_getWharfTileForNewFishingBoat(int walkerId, int *xTile, int *yTile)
{
	int wharfId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].type == Building_Wharf) {
			int wharfBoatId = Data_Buildings[i].data.other.boatWalkerId;
			if (!wharfBoatId || wharfBoatId == walkerId) {
				wharfId = 1;
				break;
			}
		}
	}
	if (wharfId <= 0) {
		return 0;
	}
	*xTile = Data_Buildings[wharfId].x;
	*yTile = Data_Buildings[wharfId].y;
	switch (Data_Buildings[wharfId].data.other.dockOrientation) {
		case 0: *xTile += 1; *yTile -= 1; break;
		case 1: *xTile += 2; *yTile += 1; break;
		case 2: *xTile += 1; *yTile += 2; break;
		default: *xTile -= 1; *yTile += 1; break;
	}
	return wharfId;
}

int Terrain_Water_getNearestFishTile(int walkerId, int *xTile, int *yTile)
{
	int numFishingSpots = 0;
	for (int i = 0; i < 8; i++) {
		if (Data_Scenario.fishingPoints.x[i] > 0) {
			numFishingSpots++;
		}
	}
	if (numFishingSpots <= 0) {
		return 0;
	}
	int minDist = 10000;
	int minFishId = 0;
	for (int i = 0; i < 8; i++) {
		if (Data_Scenario.fishingPoints.x > 0) {
			int dist = Calc_distanceMaximum(
				Data_Walkers[walkerId].x, Data_Walkers[walkerId].y,
				Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i]);
			if (dist < minDist) {
				minDist = dist;
				minFishId = i;
			}
		}
	}
	if (minDist < 10000) {
		*xTile = Data_Scenario.fishingPoints.x[minFishId];
		*yTile = Data_Scenario.fishingPoints.y[minFishId];
		return 1;
	}
	return 0;
}

int Terrain_Water_findFreeTileForFishingBoat(int walkerId, int *xTile, int *yTile)
{
	int gridOffset = Data_Walkers[walkerId].gridOffset;
	if (Data_Grid_walkerIds[gridOffset] == walkerId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int wx = Data_Walkers[walkerId].x;
		int wy = Data_Walkers[walkerId].y;
		int xMin = wx - radius;
		int yMin = wy - radius;
		int xMax = wx + radius;
		int yMax = wy + radius;
		BOUND_REGION();
		FOREACH_REGION({
			if (!Data_Grid_walkerIds[gridOffset] && Data_Grid_terrain[gridOffset] & Terrain_Water) {
				return 1;
			}
		});
	}
	return 0;
}

int Terrain_Water_findOpenWaterForShipwreck(int walkerId, int *xTile, int *yTile)
{
	int gridOffset = Data_Walkers[walkerId].gridOffset;
	if (Data_Grid_terrain[gridOffset] & Terrain_Water && Data_Grid_walkerIds[gridOffset] == walkerId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int wx = Data_Walkers[walkerId].x;
		int wy = Data_Walkers[walkerId].y;
		int xMin = wx - radius;
		int yMin = wy - radius;
		int xMax = wx + radius;
		int yMax = wy + radius;
		BOUND_REGION();
		FOREACH_REGION({
			if (!Data_Grid_walkerIds[gridOffset] || Data_Grid_walkerIds[gridOffset] != walkerId) {
				if (Data_Grid_terrain[gridOffset] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx, yy - 2)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx, yy + 2)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx - 2, yy)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx + 2, yy)] & Terrain_Water) {
					return 1;
				}
			}
		});
	}
	return 0;
}

int Terrain_Water_getFreeDockDestination(int walkerId, int *xTile, int *yTile)
{
	if (Data_CityInfo.numWorkingDocks <= 0) {
		return 0;
	}
	int dockId;
	for (int i = 0; i < 10; i++) {
		dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
		if (!Data_Buildings[dockId].data.other.boatWalkerId ||
			Data_Buildings[dockId].data.other.boatWalkerId == walkerId) {
			break;
		}
	}
	// BUG: when 10 docks in city, always takes last one...
	if (dockId <= 0) {
		return 0;
	}
	*xTile = Data_Buildings[dockId].x;
	*yTile = Data_Buildings[dockId].y;
	switch (Data_Buildings[dockId].data.other.dockOrientation) {
		case 0: *xTile += 1; *yTile -= 1; break;
		case 1: *xTile += 3; *yTile += 1; break;
		case 2: *xTile += 1; *yTile += 3; break;
		default: *xTile -= 1; *yTile += 1; break;
	}
	Data_Buildings[dockId].data.other.boatWalkerId = walkerId;
	return dockId;
}

int Terrain_Water_getQueueDockDestination(int walkerId, int *xTile, int *yTile)
{
	if (Data_CityInfo.numWorkingDocks <= 0) {
		return 0;
	}
	int dockId;
	for (int i = 0; i < 10; i++) {
		dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
		*xTile = Data_Buildings[dockId].x;
		*yTile = Data_Buildings[dockId].y;
		switch (Data_Buildings[dockId].data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 2; break;
			case 1: *xTile += 4; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 4; break;
			default: *xTile -= 2; *yTile += 2; break;
		}
		if (!Data_Grid_walkerIds[GridOffset(*xTile, *yTile)]) {
			return dockId;
		}
	}
	for (int i = 0; i < 10; i++) {
		dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
		*xTile = Data_Buildings[dockId].x;
		*yTile = Data_Buildings[dockId].y;
		switch (Data_Buildings[dockId].data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 3; break;
			case 1: *xTile += 5; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 5; break;
			default: *xTile -= 3; *yTile += 2; break;
		}
		if (!Data_Grid_walkerIds[GridOffset(*xTile, *yTile)]) {
			return dockId;
		}
	}
	return 0;
}

