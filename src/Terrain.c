#include "Terrain.h"

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
