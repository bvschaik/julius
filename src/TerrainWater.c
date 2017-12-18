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

void Terrain_addWatersideBuildingToGrids(int buildingId, int x, int y, int size, int graphicId)
{
	if (!map_grid_is_inside(x, y, size)) {
		return;
	}
	int xLeftmost;
	int yLeftmost;
	switch (Data_State.map.orientation) {
		case DIR_0_TOP:
			xLeftmost = 0;
			yLeftmost = size - 1;
			break;
		case DIR_2_RIGHT:
			xLeftmost = yLeftmost = 0;
			break;
		case DIR_4_BOTTOM:
			xLeftmost = size - 1;
			yLeftmost = 0;
			break;
		case DIR_6_LEFT:
			xLeftmost = yLeftmost = size - 1;
			break;
		default:
			return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			map_terrain_add(gridOffset, TERRAIN_BUILDING);
			if (!map_terrain_is(gridOffset, TERRAIN_WATER)) {
				map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
				map_terrain_add(gridOffset, TERRAIN_BUILDING);
			}
			map_building_set(gridOffset, buildingId);
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, size);
			map_image_set(gridOffset, graphicId);
			map_property_set_multi_tile_xy(gridOffset, dx, dy,
			    dx == xLeftmost && dy == yLeftmost);
		}
	}
}

static int blocked_land_terrain()
{
    return
        TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER |
        TERRAIN_BUILDING | TERRAIN_SCRUB | TERRAIN_GARDEN |
        TERRAIN_ROAD | TERRAIN_ELEVATION | TERRAIN_RUBBLE;
}

int Terrain_determineOrientationWatersideSize2(int x, int y, int adjustXY,
	int *orientationAbsolute, int *orientationRelative)
{
	if (adjustXY == 1) {
		switch (Data_State.map.orientation) {
			case DIR_0_TOP: break;
			case DIR_2_RIGHT: x--; break;
			case DIR_6_LEFT: y--; break;
			case DIR_4_BOTTOM: x--; y--; break;
		}
	}
	if (!map_grid_is_inside(x, y, 2)) {
		return 999;
	}

	int baseOffset = map_grid_offset(x, y);
	int tileOffsets[] = {0, 1, 162, 163};
	const int shouldBeWater[4][4] = {{1, 1, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 1}, {1, 0, 1, 0}};
	for (int dir = 0; dir < 4; dir++) {
		int okTiles = 0;
		int blockedTiles = 0;
		for (int i = 0; i < 4; i++) {
			int gridOffset = baseOffset + tileOffsets[i];
			if (shouldBeWater[dir][i]) {
				if (!map_terrain_is(gridOffset, TERRAIN_WATER)) {
					break;
				}
				okTiles++;
				if (map_terrain_is(gridOffset, TERRAIN_ROCK | TERRAIN_ROAD)) {
					// bridge or map edge
					blockedTiles++;
				}
			} else {
				if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
					break;
				}
				okTiles++;
				if (map_terrain_is(gridOffset, blocked_land_terrain())) {
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
			if (!map_terrain_is(baseOffset + tilesToCheck[dir][i], TERRAIN_WATER)) {
				okTiles = 0;
			}
		}
		if (okTiles == 4) {
			// water/land is OK in this orientation
			if (orientationAbsolute) {
				*orientationAbsolute = dir;
			}
			if (orientationRelative) {
				*orientationRelative = (4 + dir - Data_State.map.orientation / 2) % 4;
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
		switch (Data_State.map.orientation) {
			case DIR_0_TOP: break;
			case DIR_2_RIGHT: x -= 2; break;
			case DIR_6_LEFT: y -= 2; break;
			case DIR_4_BOTTOM: x -= 2; y -= 2; break;
		}
	}
	if (!map_grid_is_inside(x, y, 3)) {
		return 999;
	}

	int baseOffset = map_grid_offset(x, y);
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
				if (!map_terrain_is(gridOffset, TERRAIN_WATER)) {
					break;
				}
				okTiles++;
				if (map_terrain_is(gridOffset, TERRAIN_ROCK | TERRAIN_ROAD)) {
					// bridge or map edge
					blockedTiles++;
				}
			} else {
				if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
					break;
				}
				okTiles++;
				if (map_terrain_is(gridOffset, blocked_land_terrain())) {
					blockedTiles++;
				}
			}
		}
		// check two water tiles at the side
		int tilesToCheck[4][2] = {{-1, 3}, {-160, 488}, {327, 323}, {-162, 486}};
		for (int i = 0; i < 2; i++) {
			if (!map_terrain_is(baseOffset + tilesToCheck[dir][i], TERRAIN_WATER)) {
				okTiles = 0;
			}
		}
		if (okTiles == 9) {
			// water/land is OK in this orientation
			if (orientationAbsolute) {
				*orientationAbsolute = dir;
			}
			if (orientationRelative) {
				*orientationRelative = (4 + dir - Data_State.map.orientation / 2) % 4;
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
        building *b = building_get(i);
		if (BuildingIsInUse(i) && b->type == BUILDING_WHARF) {
			int wharfBoatId = b->data.other.boatFigureId;
			if (!wharfBoatId || wharfBoatId == figureId) {
				wharfId = i;
				break;
			}
		}
	}
	if (wharfId <= 0) {
		return 0;
	}
	building *wharf = building_get(wharfId);
	*xTile = wharf->x;
	*yTile = wharf->y;
	switch (wharf->data.other.dockOrientation) {
		case 0: *xTile += 1; *yTile -= 1; break;
		case 1: *xTile += 2; *yTile += 1; break;
		case 2: *xTile += 1; *yTile += 2; break;
		default: *xTile -= 1; *yTile += 1; break;
	}
	return wharfId;
}

int Terrain_Water_getNearestFishTile(int figureId, int *xTile, int *yTile)
{
    figure *f = figure_get(figureId);
    return scenario_map_closest_fishing_point(f->x, f->y, xTile, yTile);
}

int Terrain_Water_findAlternativeTileForFishingBoat(int figureId, int *xTile, int *yTile)
{
    figure *f = figure_get(figureId);
	if (map_figure_at(f->gridOffset) == figureId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int xMin = f->x - radius;
		int yMin = f->y - radius;
		int xMax = f->x + radius;
		int yMax = f->y + radius;
		map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
		FOREACH_REGION({
			if (!map_has_figure_at(gridOffset) && map_terrain_is(gridOffset, TERRAIN_WATER)) {
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
    figure *f = figure_get(figureId);
	if (map_terrain_is(f->gridOffset, TERRAIN_WATER) && map_figure_at(f->gridOffset) == figureId) {
		return 0;
	}
	for (int radius = 1; radius <= 5; radius++) {
		int xMin = f->x - radius;
		int yMin = f->y - radius;
		int xMax = f->x + radius;
		int yMax = f->y + radius;
		map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);
		FOREACH_REGION({
			if (!map_has_figure_at(gridOffset) || map_figure_at(gridOffset) == figureId) {
				if (map_terrain_is(gridOffset, TERRAIN_WATER) &&
					map_terrain_is(map_grid_offset(xx, yy - 2), TERRAIN_WATER) &&
					map_terrain_is(map_grid_offset(xx, yy + 2), TERRAIN_WATER) &&
					map_terrain_is(map_grid_offset(xx - 2, yy), TERRAIN_WATER) &&
					map_terrain_is(map_grid_offset(xx + 2, yy), TERRAIN_WATER)) {
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

