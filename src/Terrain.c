#include "Terrain.h"

#include "Building.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "core/calc.h"
#include "figuretype/animal.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/ring.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/map.h"

static void add_road(int grid_offset)
{
    if (!map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        map_terrain_add(grid_offset, TERRAIN_ROAD);
    }
}

void Terrain_addRoadsForGatehouse(int x, int y, int orientation)
{
	// roads under gatehouse
	map_terrain_add(map_grid_offset(x,y), TERRAIN_ROAD);
	map_terrain_add(map_grid_offset(x+1,y), TERRAIN_ROAD);
	map_terrain_add(map_grid_offset(x,y+1), TERRAIN_ROAD);
	map_terrain_add(map_grid_offset(x+1,y+1), TERRAIN_ROAD);

	// free roads before/after gate
	if (orientation == 1) {
		add_road(map_grid_offset(x, y-1));
		add_road(map_grid_offset(x+1, y-1));
		add_road(map_grid_offset(x, y+2));
		add_road(map_grid_offset(x+1, y+2));
	} else if (orientation == 2) {
		add_road(map_grid_offset(x-1, y));
		add_road(map_grid_offset(x-1, y+1));
		add_road(map_grid_offset(x+2, y));
		add_road(map_grid_offset(x+2, y+1));
	}
}

void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation)
{
	if (orientation == 1) {
		// road in the middle
		map_terrain_add(map_grid_offset(x+1,y), TERRAIN_ROAD);
		map_terrain_add(map_grid_offset(x+1,y+1), TERRAIN_ROAD);
		map_terrain_add(map_grid_offset(x+1,y+2), TERRAIN_ROAD);
		// no roads on other tiles
		map_terrain_remove(map_grid_offset(x,y), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x,y+1), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x,y+2), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+2,y), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+2,y+1), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+2,y+2), TERRAIN_ROAD);
	} else if (orientation == 2) {
		// road in the middle
		map_terrain_add(map_grid_offset(x,y+1), TERRAIN_ROAD);
		map_terrain_add(map_grid_offset(x+1,y+1), TERRAIN_ROAD);
		map_terrain_add(map_grid_offset(x+2,y+1), TERRAIN_ROAD);
		// no roads on other tiles
		map_terrain_remove(map_grid_offset(x,y), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+1,y), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+2,y), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x,y+2), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+1,y+2), TERRAIN_ROAD);
		map_terrain_remove(map_grid_offset(x+2,y+2), TERRAIN_ROAD);
	}
}

int Terrain_getOrientationGatehouse(int x, int y)
{
	switch (Data_State.map.orientation) {
		case DIR_2_RIGHT: x--; break;
		case DIR_4_BOTTOM: x--; y--; break;
		case DIR_6_LEFT: y--; break;
	}
	int gridOffset = map_grid_offset(x, y);
	int numRoadTilesWithin = 0;
	int roadTilesWithin = 0;
	// tiles within gate, flags:
	// 1  2
	// 4  8
	if (map_terrain_is(map_grid_offset(x, y), TERRAIN_ROAD)) {
		roadTilesWithin |= 1;
		numRoadTilesWithin++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(1, 0), TERRAIN_ROAD)) {
		roadTilesWithin |= 2;
		numRoadTilesWithin++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(0, 1), TERRAIN_ROAD)) {
		roadTilesWithin |= 4;
		numRoadTilesWithin++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(1, 1), TERRAIN_ROAD)) {
		roadTilesWithin |= 8;
		numRoadTilesWithin++;
	}

	if (numRoadTilesWithin != 2 && numRoadTilesWithin != 4) {
		return 0;
	}
	if (numRoadTilesWithin == 2) {
		if (roadTilesWithin == 6 || roadTilesWithin == 9) { // diagonals
			return 0;
		}
		if (roadTilesWithin == 5 || roadTilesWithin == 10) { // top to bottom
			return 1;
		}
		if (roadTilesWithin == 3 || roadTilesWithin == 12) { // left to right
			return 2;
		}
		return 0;
	}
	// all 4 tiles are road: check adjacent roads
	int numRoadTilesTop = 0;
	int numRoadTilesRight = 0;
	int numRoadTilesBottom = 0;
	int numRoadTilesLeft = 0;
	// top
	if (map_terrain_is(gridOffset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
		numRoadTilesTop++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(1, -1), TERRAIN_ROAD)) {
		numRoadTilesTop++;
	}
	// bottom
	if (map_terrain_is(gridOffset + map_grid_delta(0, 2), TERRAIN_ROAD)) {
		numRoadTilesBottom++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(1, 2), TERRAIN_ROAD)) {
		numRoadTilesBottom++;
	}
	// left
	if (map_terrain_is(gridOffset + map_grid_delta(-1, 0), TERRAIN_ROAD)) {
		numRoadTilesLeft++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(-1, 1), TERRAIN_ROAD)) {
		numRoadTilesLeft++;
	}
	// right
	if (map_terrain_is(gridOffset + map_grid_delta(2, 0), TERRAIN_ROAD)) {
		numRoadTilesRight++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(2, 1), TERRAIN_ROAD)) {
		numRoadTilesRight++;
	}
	// determine direction
	if (numRoadTilesTop || numRoadTilesBottom) {
		if (numRoadTilesLeft || numRoadTilesRight) {
			return 0;
		}
		return 1;
	} else if (numRoadTilesLeft || numRoadTilesRight) {
		return 2;
	}
	return 0;
}

int Terrain_getOrientationTriumphalArch(int x, int y)
{
	switch (Data_State.map.orientation) {
		case DIR_2_RIGHT: x -= 2; break;
		case DIR_4_BOTTOM: x -= 2; y -= 2; break;
		case DIR_6_LEFT: y -= 2; break;
	}
	int numRoadTilesTopBottom = 0;
	int numRoadTilesLeftRight = 0;
	int numBlockedTiles = 0;

	int gridOffset = map_grid_offset(x, y);
	// check corner tiles
	if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(2, 0), TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(0, 2), TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	if (map_terrain_is(gridOffset + map_grid_delta(2, 2), TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	// road tiles top to bottom
	int topOffset = gridOffset + map_grid_delta(1, 0);
	if ((map_terrain_get(topOffset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
		numRoadTilesTopBottom++;
	} else if (map_terrain_is(topOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	int bottomOffset = gridOffset + map_grid_delta(1, 2);
	if ((map_terrain_get(bottomOffset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
		numRoadTilesTopBottom++;
	} else if (map_terrain_is(bottomOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	// road tiles left to right
	int leftOffset = gridOffset + map_grid_delta(0, 1);
	if ((map_terrain_get(leftOffset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
		numRoadTilesLeftRight++;
	} else if (map_terrain_is(leftOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	int rightOffset = gridOffset + map_grid_delta(2, 1);
	if ((map_terrain_get(rightOffset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
		numRoadTilesLeftRight++;
	} else if (map_terrain_is(rightOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	// center tile
	int centerOffset = gridOffset + map_grid_delta(2, 1);
	if ((map_terrain_get(centerOffset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
		// do nothing
	} else if (map_terrain_is(centerOffset, TERRAIN_NOT_CLEAR)) {
		numBlockedTiles++;
	}
	// judgement time
	if (numBlockedTiles) {
		return 0;
	}
	if (!numRoadTilesLeftRight && !numRoadTilesTopBottom) {
		return 0; // no road: can't determine direction
	}
	if (numRoadTilesTopBottom == 2 && !numRoadTilesLeftRight) {
		return 1;
	}
	if (numRoadTilesLeftRight == 2 && !numRoadTilesTopBottom) {
		return 2;
	}
	return 0;
}

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet)
{
	if (!map_grid_is_inside(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR & disallowedTerrain)) {
				return 0;
			} else if (map_has_figure_at(gridOffset)) {
				return 0;
			} else if (graphicSet && map_image_at(gridOffset)) {
				return 0;
			}
		}
	}
	return 1;
}

void Terrain_updateEntryExitFlags(int remove)
{
	if (remove) {
		int gridOffsetEntry = map_grid_offset(Data_CityInfo_Extra.entryPointFlag.x, Data_CityInfo_Extra.entryPointFlag.y);
		map_terrain_remove(gridOffsetEntry, TERRAIN_ROCK);
		int gridOffsetExit = map_grid_offset(Data_CityInfo_Extra.exitPointFlag.x, Data_CityInfo_Extra.exitPointFlag.y);
		map_terrain_remove(gridOffsetExit, TERRAIN_ROCK);
		return;
	}
	int entryOrientation;
    map_point entry_point = scenario_map_entry();
	if (entry_point.x == 0) {
		entryOrientation = DIR_2_RIGHT;
	} else if (entry_point.x == Data_State.map.width - 1) {
		entryOrientation = DIR_6_LEFT;
	} else if (entry_point.y == 0) {
		entryOrientation = DIR_0_TOP;
	} else if (entry_point.y == Data_State.map.height - 1) {
		entryOrientation = DIR_4_BOTTOM;
	} else {
		entryOrientation = -1;
	}
	int exitOrientation;
    map_point exit_point = scenario_map_exit();
	if (exit_point.x == 0) {
		exitOrientation = DIR_2_RIGHT;
	} else if (exit_point.x == Data_State.map.width - 1) {
		exitOrientation = DIR_6_LEFT;
	} else if (exit_point.y == 0) {
		exitOrientation = DIR_0_TOP;
	} else if (exit_point.y == Data_State.map.height - 1) {
		exitOrientation = DIR_4_BOTTOM;
	} else {
		exitOrientation = -1;
	}
	if (entryOrientation >= 0) {
		int gridOffset = map_grid_offset(entry_point.x, entry_point.y);
		int xTile, yTile;
		for (int i = 1; i < 10; i++) {
			if (map_terrain_exists_clear_tile_in_radius(
					    entry_point.x, entry_point.y,
					1, i, gridOffset, &xTile, &yTile)) {
				break;
			}
		}
		int gridOffsetFlag = map_grid_offset(xTile, yTile);
		Data_CityInfo_Extra.entryPointFlag.x = xTile;
		Data_CityInfo_Extra.entryPointFlag.y = yTile;
		Data_CityInfo_Extra.entryPointFlag.gridOffset = gridOffsetFlag;
		map_terrain_add(gridOffsetFlag, TERRAIN_ROCK);
		int orientation = (Data_State.map.orientation + entryOrientation) % 8;
		map_image_set(gridOffsetFlag, image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + orientation / 2);
	}
	if (exitOrientation >= 0) {
		int gridOffset = map_grid_offset(exit_point.x, exit_point.y);
		int xTile, yTile;
		for (int i = 1; i < 10; i++) {
			if (map_terrain_exists_clear_tile_in_radius(
					    exit_point.x, exit_point.y,
					1, i, gridOffset, &xTile, &yTile)) {
				break;
			}
		}
		int gridOffsetFlag = map_grid_offset(xTile, yTile);
		Data_CityInfo_Extra.exitPointFlag.x = xTile;
		Data_CityInfo_Extra.exitPointFlag.y = yTile;
		Data_CityInfo_Extra.exitPointFlag.gridOffset = gridOffsetFlag;
		map_terrain_add(gridOffsetFlag, TERRAIN_ROCK);
		int orientation = (Data_State.map.orientation + exitOrientation) % 8;
		map_image_set(gridOffsetFlag, image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + 4 + orientation / 2);
	}
}

static void determineLeftmostTile()
{
	for (int y = 0; y < Data_State.map.height; y++) {
		for (int x = 0; x < Data_State.map.width; x++) {
			int gridOffset = map_grid_offset(x, y);
			int size = map_property_multi_tile_size(gridOffset);
			if (size == 1) {
				map_property_mark_draw_tile(gridOffset);
				continue;
			}
			map_property_clear_draw_tile(gridOffset);
			int orientation = Data_State.map.orientation;
			int dx = orientation == DIR_4_BOTTOM || orientation == DIR_6_LEFT ? size - 1 : 0;
			int dy = orientation == DIR_0_TOP || orientation == DIR_6_LEFT ? size - 1 : 0;
			if (map_property_is_multi_tile_xy(gridOffset, dx, dy)) {
				map_property_mark_draw_tile(gridOffset);
			}
		}
	}
}

void Terrain_rotateMap(int ccw)
{
	Terrain_updateEntryExitFlags(1);
	game_undo_disable();
	determineLeftmostTile();

	TerrainGraphics_updateRegionElevation(0, 0, Data_State.map.width - 2, Data_State.map.height - 2);
	TerrainGraphics_updateAllWater();
	TerrainGraphics_updateAllEarthquake();
	map_tiles_update_all_rocks();
	map_tiles_update_all_gardens();

	Terrain_updateEntryExitFlags(0);

	TerrainGraphics_updateRegionEmptyLand(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionMeadow(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionRubble(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	map_tiles_update_all_roads();
	map_tiles_update_all_plazas();
	map_tiles_update_all_walls();
	TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);

	Building_determineGraphicIdsForOrientedBuildings();
	map_bridge_update_after_rotate(ccw);
	map_routing_update_walls();

	figure_tower_sentry_reroute();
	figure_hippodrome_horse_reroute();
}
