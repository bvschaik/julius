#include "Terrain.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "city/view.h"
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
