#include "Terrain.h"
#include "Terrain_private.h"

#include "Calc.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"
#include "Data/Figure.h"

void Terrain_addWatersideBuildingToGrids(int buildingId, int x, int y, int size, int graphicId)
{
	if (IsOutsideMap(x, y, size)) {
		return;
	}
	int xLeftmost;
	int yLeftmost;
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top:
			xLeftmost = 0;
			yLeftmost = size - 1;
			break;
		case Dir_2_Right:
			xLeftmost = yLeftmost = 0;
			break;
		case Dir_4_Bottom:
			xLeftmost = size - 1;
			yLeftmost = 0;
			break;
		case Dir_6_Left:
			xLeftmost = yLeftmost = size - 1;
			break;
		default:
			return;
	}
	int sizeMask = (size == 3) ? Bitfield_Size3 : Bitfield_Size2;
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] |= Terrain_Building;
			if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
				Data_Grid_terrain[gridOffset] |= Terrain_Building;
			}
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] |= sizeMask;
			Data_Grid_graphicIds[gridOffset] = graphicId;
			Data_Grid_edge[gridOffset] = EdgeXY(dx, dy);
			if (dx == xLeftmost && dy == yLeftmost) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
}

int Terrain_determineOrientationWatersideSize2(int x, int y, int adjustXY,
	int *orientationAbsolute, int *orientationRelative)
{
	if (adjustXY == 1) {
		switch (Data_Settings_Map.orientation) {
			case Dir_0_Top: break;
			case Dir_2_Right: x--; break;
			case Dir_6_Left: y--; break;
			case Dir_4_Bottom: x--; y--; break;
		}
	}
	if (IsOutsideMap(x, y, 2)) {
		return 999;
	}

	int baseOffset = GridOffset(x, y);
	int tileOffsets[] = {0, 1, 162, 163};
	const int shouldBeWater[4][4] = {{1, 1, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}};
	for (int dir = 0; dir < 4; dir++) {
		int okTiles = 0;
		int blockedTiles = 0;
		for (int i = 0; i < 4; i++) {
			int gridOffset = baseOffset + tileOffsets[i];
			if (shouldBeWater[dir][i]) {
				if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
					break;
				}
				okTiles++;
				if (Data_Grid_terrain[gridOffset] & (Terrain_Rock | Terrain_Road)) {
					// bridge or map edge
					blockedTiles++;
				}
			} else {
				if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
					break;
				}
				okTiles++;
				if (Data_Grid_terrain[gridOffset] & Terrain_127f) {
					blockedTiles++;
				}
			}
		}
		// check six water tiles in front
		const int tilesToCheck[4][6] = {
			{-1, -163, -162, -161, -160, 2},
			{-161, -160, 2, 164, 326, 325},
			{164, 326, 325, 324, 323, 161},
			{324, 323, 161, -1, -163, -162},
		};
		for (int i = 0; i < 6; i++) {
			if (!(Data_Grid_terrain[baseOffset + tilesToCheck[dir][i]] & Terrain_Water)) {
				okTiles = 0;
			}
		}
		if (okTiles == 4) {
			// water/land is OK in this orientation
			if (orientationAbsolute) {
				*orientationAbsolute = dir;
			}
			if (orientationRelative) {
				*orientationRelative = (4 + dir - Data_Settings_Map.orientation / 2) % 4;
			}
			return blockedTiles;
		}
	}
	return 999;
}

int Terrain_determineOrientationWatersideSize3(int x, int y, int adjustXY,
	int *orientationAbsolute, int *orientationRelative)
{
	if (adjustXY == 1) {
		switch (Data_Settings_Map.orientation) {
			case Dir_0_Top: break;
			case Dir_2_Right: x -= 2; break;
			case Dir_6_Left: y -= 2; break;
			case Dir_4_Bottom: x -= 2; y -= 2; break;
		}
	}
	if (IsOutsideMap(x, y, 3)) {
		return 999;
	}

	int baseOffset = GridOffset(x, y);
	int tileOffsets[] = {0, 1, 2, 162, 163, 164, 324, 325, 326};
	int shouldBeWater[4][9] = {
		{1, 1, 1, 0, 0, 0, 0, 0, 0},
		{0, 0, 1, 0, 0, 1, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 1, 1, 1},
		{1, 0, 0, 1, 0, 0, 1, 0, 0}
	};
	for (int dir = 0; dir < 4; dir++) {
		int okTiles = 0;
		int blockedTiles = 0;
		for (int i = 0; i < 9; i++) {
			int gridOffset = baseOffset + tileOffsets[i];
			if (shouldBeWater[dir][i]) {
				if (!(Data_Grid_terrain[gridOffset] & Terrain_Water)) {
					break;
				}
				okTiles++;
				if (Data_Grid_terrain[gridOffset] & (Terrain_Rock | Terrain_Road)) {
					// bridge or map edge
					blockedTiles++;
				}
			} else {
				if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
					break;
				}
				okTiles++;
				if (Data_Grid_terrain[gridOffset] & Terrain_127f) {
					blockedTiles++;
				}
			}
		}
		// check two water tiles at the side
		int tilesToCheck[4][2] = {{-1, 3}, {-160, 488}, {327, 323}, {-162, 486}};
		for (int i = 0; i < 2; i++) {
			if (!(Data_Grid_terrain[baseOffset + tilesToCheck[dir][i]] & Terrain_Water)) {
				okTiles = 0;
			}
		}
		if (okTiles == 9) {
			// water/land is OK in this orientation
			if (orientationAbsolute) {
				*orientationAbsolute = dir;
			}
			if (orientationRelative) {
				*orientationRelative = (4 + dir - Data_Settings_Map.orientation / 2) % 4;
			}
			return blockedTiles;
		}
	}
	return 999;
}

int Terrain_Water_getWharfTileForNewFishingBoat(int figureId, int *xTile, int *yTile)
{
	int wharfId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (BuildingIsInUse(i) && Data_Buildings[i].type == Building_Wharf) {
			int wharfBoatId = Data_Buildings[i].data.other.boatFigureId;
			if (!wharfBoatId || wharfBoatId == figureId) {
				wharfId = i;
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

int Terrain_Water_getNearestFishTile(int figureId, int *xTile, int *yTile)
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
				Data_Figures[figureId].x, Data_Figures[figureId].y,
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

int Terrain_Water_findAlternativeTileForFishingBoat(int figureId, int *xTile, int *yTile)
{
	int gridOffset = Data_Figures[figureId].gridOffset;
	if (Data_Grid_figureIds[gridOffset] == figureId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int wx = Data_Figures[figureId].x;
		int wy = Data_Figures[figureId].y;
		int xMin = wx - radius;
		int yMin = wy - radius;
		int xMax = wx + radius;
		int yMax = wy + radius;
		BOUND_REGION();
		FOREACH_REGION({
			if (!Data_Grid_figureIds[gridOffset] && Data_Grid_terrain[gridOffset] & Terrain_Water) {
				*xTile = xx;
				*yTile = yy;
				return 1;
			}
		});
	}
	return 0;
}

int Terrain_Water_findOpenWaterForShipwreck(int figureId, int *xTile, int *yTile)
{
	int gridOffset = Data_Figures[figureId].gridOffset;
	if (Data_Grid_terrain[gridOffset] & Terrain_Water && Data_Grid_figureIds[gridOffset] == figureId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int wx = Data_Figures[figureId].x;
		int wy = Data_Figures[figureId].y;
		int xMin = wx - radius;
		int yMin = wy - radius;
		int xMax = wx + radius;
		int yMax = wy + radius;
		BOUND_REGION();
		FOREACH_REGION({
			if (!Data_Grid_figureIds[gridOffset] || Data_Grid_figureIds[gridOffset] == figureId) {
				if (Data_Grid_terrain[gridOffset] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx, yy - 2)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx, yy + 2)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx - 2, yy)] & Terrain_Water &&
					Data_Grid_terrain[GridOffset(xx + 2, yy)] & Terrain_Water) {
					*xTile = xx;
					*yTile = yy;
					return 1;
				}
			}
		});
	}
	return 0;
}

int Terrain_Water_getFreeDockDestination(int figureId, int *xTile, int *yTile)
{
	if (Data_CityInfo.numWorkingDocks <= 0) {
		return 0;
	}
	int dockId;
	for (int i = 0; i < 10; i++) {
		dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
		if (!Data_Buildings[dockId].data.other.boatFigureId ||
			Data_Buildings[dockId].data.other.boatFigureId == figureId) {
			break;
		}
	}
	// BUG: when 10 docks in city, always takes last one... regardless of whether it is free
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
	Data_Buildings[dockId].data.other.boatFigureId = figureId;
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
		*xTile = Data_Buildings[dockId].x;
		*yTile = Data_Buildings[dockId].y;
		switch (Data_Buildings[dockId].data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 2; break;
			case 1: *xTile += 4; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 4; break;
			default: *xTile -= 2; *yTile += 2; break;
		}
		if (!Data_Grid_figureIds[GridOffset(*xTile, *yTile)]) {
			return dockId;
		}
	}
	// second queue position
	for (int i = 0; i < 10; i++) {
		int dockId = Data_CityInfo.workingDockBuildingIds[i];
		if (!dockId) continue;
		*xTile = Data_Buildings[dockId].x;
		*yTile = Data_Buildings[dockId].y;
		switch (Data_Buildings[dockId].data.other.dockOrientation) {
			case 0: *xTile += 2; *yTile -= 3; break;
			case 1: *xTile += 5; *yTile += 2; break;
			case 2: *xTile += 2; *yTile += 5; break;
			default: *xTile -= 3; *yTile += 2; break;
		}
		if (!Data_Grid_figureIds[GridOffset(*xTile, *yTile)]) {
			return dockId;
		}
	}
	return 0;
}

