#include "Terrain.h"

#include "Building.h"
#include "FigureAction.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/building.h"
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
#include "scenario/map.h"

#define FOR_XY_ADJACENT \
	{int baseOffset = map_grid_offset(x, y);\
	const int *tile_delta = map_adjacent_offsets(size);\
	while (*tile_delta) {\
		int gridOffset = baseOffset + *tile_delta;

#define END_FOR_XY_ADJACENT tile_delta++;}}

#define STORE_XY_ADJACENT(xTile,yTile) \
	*(xTile) = x + (*tile_delta + 172) % 162 - 10;\
	*(yTile) = y + (*tile_delta + 162) / 161 - 1;

#define FOR_XY_RADIUS \
	int x_min, y_min, x_max, y_max;\
	map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);\
	int gridOffset = map_grid_offset(x_min, y_min);\
	for (int yy = y_min; yy <= y_max; yy++) {\
		for (int xx = x_min; xx <= x_max; xx++) {

#define END_FOR_XY_RADIUS \
			++gridOffset;\
		}\
		gridOffset += 162 - (x_max - x_min + 1);\
	}

#define STORE_XY_RADIUS(xTile,yTile) \
	*(xTile) = xx; *(yTile) = yy;

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain)
{
	if (!map_grid_is_inside(x, y, size)) {
		return;
	}
	int xLeftmost, yLeftmost;
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
			map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
			map_terrain_add(gridOffset, terrain);
			map_building_set(gridOffset, buildingId);
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, size);
			map_image_set(gridOffset, graphicId);
			map_property_set_multi_tile_xy(gridOffset, dx, dy,
			    dx == xLeftmost && dy == yLeftmost);
		}
	}
}

static int getNorthTileGridOffset(int x, int y, int *size)
{
	int gridOffset = map_grid_offset(x, y);
	*size = map_property_multi_tile_size(gridOffset);
	for (int i = 0; i < *size && map_property_multi_tile_x(gridOffset); i++) {
		gridOffset += map_grid_delta(-1, 0);
	}
	for (int i = 0; i < *size && map_property_multi_tile_y(gridOffset); i++) {
		gridOffset += map_grid_delta(0, -1);
	}
	return gridOffset;
}

void Terrain_removeBuildingFromGrids(int buildingId, int x, int y)
{
	if (!map_grid_is_inside(x, y, 1)) {
		return;
	}
	int size;
	int baseGridOffset = getNorthTileGridOffset(x, y, &size);
	x = map_grid_offset_to_x(baseGridOffset);
	y = map_grid_offset_to_y(baseGridOffset);
	if (map_terrain_get(baseGridOffset) == TERRAIN_ROCK) {
		return;
	}
	building *b = building_get(buildingId);
	if (buildingId && BuildingIsFarm(b->type)) {
		size = 3;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (buildingId && map_building_at(gridOffset) != buildingId) {
				continue;
			}
			if (buildingId && b->type != BUILDING_BURNING_RUIN) {
				map_set_rubble_building_type(gridOffset, b->type);
			}
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_clear_multi_tile_xy(gridOffset);
			map_property_mark_draw_tile(gridOffset);
			map_aqueduct_set(gridOffset, 0);
			map_building_set(gridOffset, 0);
			map_building_damage_clear(gridOffset);
			map_sprite_clear_tile(gridOffset);
			if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
				map_terrain_set(gridOffset, TERRAIN_WATER); // clear other flags
				TerrainGraphics_setTileWater(x + dx, y + dy);
			} else {
				map_image_set(gridOffset,
					image_group(GROUP_TERRAIN_UGLY_GRASS) +
					(map_random_get(gridOffset) & 7));
				map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
			}
		}
	}
	TerrainGraphics_updateRegionEmptyLand(x, y, x + size, y + size);
	TerrainGraphics_updateRegionMeadow(x, y, x + size, y + size);
	TerrainGraphics_updateRegionRubble(x, y, x + size, y + size);
}

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

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY)
{
	int minValue = 12;
	int minGridOffset = map_grid_offset(x, y);
	FOR_XY_ADJACENT {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING) ||
			building_get(map_building_at(gridOffset))->type != BUILDING_GATEHOUSE) {
			if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
				int roadIndex = 11;
				for (int n = 0; n < 10; n++) {
					if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
						roadIndex = n;
						break;
					}
				}
				if (roadIndex < minValue) {
					minValue = roadIndex;
					minGridOffset = gridOffset;
				}
			}
		}
	} END_FOR_XY_ADJACENT;
	if (minValue < 12) {
		if (roadX && roadY) {
			*roadX = map_grid_offset_to_x(minGridOffset);
			*roadY = map_grid_offset_to_y(minGridOffset);
		}
		return 1;
	}
	return 0;
}

int Terrain_hasRoadAccessHippodrome(int x, int y, int *roadX, int *roadY)
{
	int size = 5;
	int minValue = 12;
	int minGridOffset = map_grid_offset(x, y);
	FOR_XY_ADJACENT {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING) ||
			building_get(map_building_at(gridOffset))->type != BUILDING_GATEHOUSE) {
			if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
				int roadIndex = 11;
				for (int n = 0; n < 10; n++) {
					if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
						roadIndex = n;
						break;
					}
				}
				if (roadIndex < minValue) {
					minValue = roadIndex;
					minGridOffset = gridOffset;
				}
			}
		}
	} END_FOR_XY_ADJACENT;
	x += 5;
	FOR_XY_ADJACENT {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING) ||
			building_get(map_building_at(gridOffset))->type != BUILDING_GATEHOUSE) {
			if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
				int roadIndex = 11;
				for (int n = 0; n < 10; n++) {
					if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
						roadIndex = n;
						break;
					}
				}
				if (roadIndex < minValue) {
					minValue = roadIndex;
					minGridOffset = gridOffset;
				}
			}
		}
	} END_FOR_XY_ADJACENT;
	x += 5;
	FOR_XY_ADJACENT {
		if (!map_terrain_is(gridOffset, TERRAIN_BUILDING) ||
			building_get(map_building_at(gridOffset))->type != BUILDING_GATEHOUSE) {
			if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
				int roadIndex = 11;
				for (int n = 0; n < 10; n++) {
					if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
						roadIndex = n;
						break;
					}
				}
				if (roadIndex < minValue) {
					minValue = roadIndex;
					minGridOffset = gridOffset;
				}
			}
		}
	} END_FOR_XY_ADJACENT;
	if (minValue < 12) {
		if (roadX && roadY) {
			*roadX = map_grid_offset_to_x(minGridOffset);
			*roadY = map_grid_offset_to_y(minGridOffset);
		}
		return 1;
	}
	return 0;
}

int Terrain_hasRoadAccessGranary(int x, int y, int *roadX, int *roadY)
{
	int xx = -1, yy = -1;
	if (map_terrain_is(map_grid_offset(x + 1, y - 1), TERRAIN_ROAD)) {
		xx = x + 1;
		yy = y - 1;
	} else if (map_terrain_is(map_grid_offset(x + 3, y + 1), TERRAIN_ROAD)) {
		xx = x + 3;
		yy = y + 1;
	} else if (map_terrain_is(map_grid_offset(x + 1, y + 3), TERRAIN_ROAD)) {
		xx = x + 1;
		yy = y + 3;
	} else if (map_terrain_is(map_grid_offset(x - 1, y + 1), TERRAIN_ROAD)) {
		xx = x - 1;
		yy = y + 1;
	}
	if (xx > 0 && yy > 0) {
		if (roadX && roadY) {
			*roadX = xx;
			*roadY = yy;
		}
		return 1;
	}
	return 0;
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

static int getRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	FOR_XY_RADIUS {
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			if (xTile && yTile) {
				STORE_XY_RADIUS(xTile, yTile);
			}
			return 1;
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	for (int r = 1; r <= radius; r++) {
		if (getRoadWithinRadius(x, y, size, r, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

static int getReachableRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	FOR_XY_RADIUS {
		if (map_terrain_is(gridOffset, TERRAIN_ROAD)) {
			if (map_routing_distance(gridOffset) > 0) {
				if (xTile && yTile) {
					STORE_XY_RADIUS(xTile, yTile);
				}
				return 1;
			}
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_getClosestReachableRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	for (int r = 1; r <= radius; r++) {
		if (getReachableRoadWithinRadius(x, y, size, r, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

// NOTE: return value not 0 = no tile / gridOffset + 1; but: -1 = no tile / gridOffset
int Terrain_getRoadToLargestRoadNetwork(int x, int y, int size, int *xTile, int *yTile)
{
	int minIndex = 12;
	int minGridOffset = -1;
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_ROAD) && map_routing_distance(gridOffset) > 0) {
			int index = 11;
			for (int n = 0; n < 10; n++) {
				if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
					index = n;
					break;
				}
			}
			if (index < minIndex) {
				minIndex = index;
				minGridOffset = gridOffset;
			}
		}
	} END_FOR_XY_ADJACENT;
	if (minIndex < 12) {
		*xTile = map_grid_offset_to_x(minGridOffset);
		*yTile = map_grid_offset_to_y(minGridOffset);
		return minGridOffset;
	}
	int minDist = 100000;
	minGridOffset = -1;
	FOR_XY_ADJACENT {
		int dist = map_routing_distance(gridOffset);
		if (dist > 0 && dist < minDist) {
			minDist = dist;
			minGridOffset = gridOffset;
		}
	} END_FOR_XY_ADJACENT;
	if (minGridOffset >= 0) {
		*xTile = map_grid_offset_to_x(minGridOffset);
		*yTile = map_grid_offset_to_y(minGridOffset);
		return minGridOffset;
	}
	return -1;
}

// NOTE: return value not 0 = no tile / gridOffset + 1; but: -1 = no tile / gridOffset
int Terrain_getRoadToLargestRoadNetworkHippodrome(int x, int y, int size, int *xTile, int *yTile)
{
	int xBase = x;
	int minIndex = 12;
	int minGridOffset = -1;
	for (int xOffset = 0; xOffset <= 10; xOffset += 5) {
		x = xBase + xOffset;
		FOR_XY_ADJACENT {
			if (map_terrain_is(gridOffset, TERRAIN_ROAD) && map_routing_distance(gridOffset) > 0) {
				int index = 11;
				for (int n = 0; n < 10; n++) {
					if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(gridOffset)) {
						index = n;
						break;
					}
				}
				if (index < minIndex) {
					minIndex = index;
					minGridOffset = gridOffset;
				}
			}
		} END_FOR_XY_ADJACENT;
	}
	if (minIndex < 12) {
		*xTile = map_grid_offset_to_x(minGridOffset);
		*yTile = map_grid_offset_to_y(minGridOffset);
		return minGridOffset;
	}
	int minDist = 100000;
	minGridOffset = -1;
	for (int xOffset = 0; xOffset <= 10; xOffset += 5) {
		x = xBase + xOffset;
		FOR_XY_ADJACENT {
			int dist = map_routing_distance(gridOffset);
			if (dist > 0 && dist < minDist) {
				minDist = dist;
				minGridOffset = gridOffset;
			}
		} END_FOR_XY_ADJACENT;
	}
	if (minGridOffset >= 0) {
		*xTile = map_grid_offset_to_x(minGridOffset);
		*yTile = map_grid_offset_to_y(minGridOffset);
		return minGridOffset;
	}
	return -1;
}

static int getRoadTileForAqueduct(int gridOffset, int gateOrientation)
{
	int isRoad = map_terrain_is(gridOffset, TERRAIN_ROAD) ? 1 : 0;
	if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		building *b = building_get(map_building_at(gridOffset));
		if (b->type == BUILDING_GATEHOUSE) {
			if (b->subtype.orientation == gateOrientation) {
				isRoad = 1;
			}
		} else if (b->type == BUILDING_GRANARY) {
			if (map_routing_citizen_is_road(gridOffset)) {
				isRoad = 1;
			}
		}
	}
	return isRoad;
}

int Terrain_getAdjacentRoadTilesForAqueduct(int gridOffset)
{
	int roadTiles = 0;
	roadTiles += getRoadTileForAqueduct(gridOffset + map_grid_delta(0, -1), 1);
	roadTiles += getRoadTileForAqueduct(gridOffset + map_grid_delta(1, 0), 2);
	roadTiles += getRoadTileForAqueduct(gridOffset + map_grid_delta(0, 1), 1);
	roadTiles += getRoadTileForAqueduct(gridOffset + map_grid_delta(-1, 0), 2);
	if (roadTiles == 4) {
		if (building_get(map_building_at(gridOffset))->type == BUILDING_GRANARY) {
			roadTiles = 2;
		}
	}
	return roadTiles;
}

static int terrain_is_road_like(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_ROAD | TERRAIN_ACCESS_RAMP) ? 1 : 0;
}

static int getAdjacentRoadTileForRoaming(int gridOffset)
{
	int isRoad = terrain_is_road_like(gridOffset);
	if (map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
		building *b = building_get(map_building_at(gridOffset));
		if (b->type == BUILDING_GATEHOUSE) {
			isRoad = 0;
		} else if (b->type == BUILDING_GRANARY) {
			if (map_routing_citizen_is_road(gridOffset)) {
				isRoad = 1;
			}
		}
	}
	return isRoad;
}

int Terrain_getAdjacentRoadTilesForRoaming(int gridOffset, int *roadTiles)
{
	roadTiles[1] = roadTiles[3] = roadTiles[5] = roadTiles[7] = 0;

	roadTiles[0] = getAdjacentRoadTileForRoaming(gridOffset + map_grid_delta(0, -1));
	roadTiles[2] = getAdjacentRoadTileForRoaming(gridOffset + map_grid_delta(1, 0));
	roadTiles[4] = getAdjacentRoadTileForRoaming(gridOffset + map_grid_delta(0, 1));
	roadTiles[6] = getAdjacentRoadTileForRoaming(gridOffset + map_grid_delta(-1, 0));

	return roadTiles[0] + roadTiles[2] + roadTiles[4] + roadTiles[6];
}

int Terrain_getSurroundingRoadTilesForRoaming(int gridOffset, int *roadTiles)
{
	roadTiles[1] = terrain_is_road_like(gridOffset + map_grid_delta(1, -1));
	roadTiles[3] = terrain_is_road_like(gridOffset + map_grid_delta(1, 1));
	roadTiles[5] = terrain_is_road_like(gridOffset + map_grid_delta(-1, 1));
	roadTiles[7] = terrain_is_road_like(gridOffset + map_grid_delta(-1, -1));
	
	int maxStretch = 0;
	int stretch = 0;
	for (int i = 0; i < 16; i++) {
		if (roadTiles[i % 8]) {
			stretch++;
			if (stretch > maxStretch) {
				maxStretch = stretch;
			}
		} else {
			stretch = 0;
		}
	}
	return maxStretch;
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

int Terrain_canSpawnFishingBoatInWater(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
			if (!map_terrain_is(gridOffset, TERRAIN_BUILDING)) {
				if (TerrainGraphicsContext_getNumWaterTiles(gridOffset) >= 8) {
					STORE_XY_ADJACENT(xTile, yTile);
					return 1;
				}
			}
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_isAdjacentToWall(int x, int y, int size)
{
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_WALL)) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_isAdjacentToWater(int x, int y, int size)
{
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_isAdjacentToOpenWater(int x, int y, int size)
{
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_WATER) &&
			map_routing_distance(gridOffset)) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT {
		if (map_terrain_is(gridOffset, TERRAIN_ROAD) ||
			!map_terrain_is(gridOffset, TERRAIN_NOT_CLEAR)) {
			STORE_XY_ADJACENT(xTile, yTile);
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd)
{
	FOR_XY_RADIUS {
		map_terrain_add(gridOffset, typeToAdd);
	} END_FOR_XY_RADIUS;
}

void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToClear)
{
	FOR_XY_RADIUS {
		map_terrain_remove(gridOffset, typeToClear);
	} END_FOR_XY_RADIUS;
}

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS {
		if (map_terrain_is(gridOffset, type)) {
			return 1;
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset, int *xTile, int *yTile)
{
	FOR_XY_RADIUS {
		if (gridOffset != exceptGridOffset && !map_terrain_get(gridOffset)) {
			STORE_XY_RADIUS(xTile, yTile);
			return 1;
		}
	} END_FOR_XY_RADIUS;
	*xTile = x_max;
	*yTile = y_max;
	return 0;
}

int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS {
		if (!map_terrain_is(gridOffset, type)) {
			return 0;
		}
	} END_FOR_XY_RADIUS;
	return 1;
}

void Terrain_markBuildingsWithinWellRadius(int wellId, int radius)
{
    building *well = building_get(wellId);
	int x = well->x;
	int y = well->y;
	int size = 1;
	FOR_XY_RADIUS {
		if (map_building_at(gridOffset)) {
			building_get(map_building_at(gridOffset))->hasWellAccess = 1;
		}
	} END_FOR_XY_RADIUS;
}

int Terrain_allHousesWithinWellRadiusHaveFountain(int wellId, int radius)
{
    building *well = building_get(wellId);
	int numHouses = 0;
	int x = well->x;
	int y = well->y;
	int size = 1;
	FOR_XY_RADIUS {
		int buildingId = map_building_at(gridOffset);
		if (buildingId > 0 && building_get(buildingId)->houseSize) {
			numHouses++;
			if (!map_terrain_is(gridOffset, TERRAIN_FOUNTAIN_RANGE)) {
				return 0;
			}
		}
	} END_FOR_XY_RADIUS;
	return numHouses ? 1 : 2;
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
			if (Terrain_existsClearTileWithinRadius(
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
			if (Terrain_existsClearTileWithinRadius(
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

int Terrain_isClearToBuild(int size, int x, int y, int terrainMask)
{
	switch (Data_State.map.orientation) {
		case 2: x = x - size + 1; break;
		case 4: x = x - size + 1; // fall-through
		case 6: y = y - size + 1; break;
	}
	if (!map_grid_is_inside(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (map_terrain_is(gridOffset, terrainMask & TERRAIN_NOT_CLEAR)) {
				return 0;
			}
		}
	}
	return 1;
}

void Terrain_updateToPlaceBuildingToOverlay(int size, int x, int y, int terrainMask, int isAbsoluteXY)
{
	if (!isAbsoluteXY) {
		switch (Data_State.map.orientation) {
			case 2: x = x - size + 1; break;
			case 4: x = x - size + 1; // fall-through
			case 6: y = y - size + 1; break;
		}
	}
	if (!map_grid_is_inside(x, y, size)) {
		return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (map_terrain_is(gridOffset, terrainMask & TERRAIN_NOT_CLEAR) ||
				map_has_figure_at(gridOffset)) {
				return;
			}
		}
	}
	// mark as being constructed
	Data_State.selectedBuilding.drawAsConstructing = 1;
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			map_property_mark_constructing(gridOffset);
		}
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

static int getWallTileWithinRadius(int x, int y, int radius, int *xTile, int *yTile)
{
	int size = 1;
	FOR_XY_RADIUS {
		if (map_routing_is_wall_passable(gridOffset)) {
			*xTile = xx;
			*yTile = yy;
			return 1;
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_getWallTileWithinRadius(int x, int y, int radius, int *xTile, int *yTile)
{
	for (int i = 1; i <= radius; i++) {
		if (getWallTileWithinRadius(x, y, i, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

void Terrain_rotateMap(int ccw)
{
	Terrain_updateEntryExitFlags(1);
	game_undo_disable();
	determineLeftmostTile();

	TerrainGraphics_updateRegionElevation(0, 0, Data_State.map.width - 2, Data_State.map.height - 2);
	TerrainGraphics_updateRegionWater(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionEarthquake(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllRocks();
	TerrainGraphics_updateAllGardens();

	Terrain_updateEntryExitFlags(0);

	TerrainGraphics_updateRegionEmptyLand(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionMeadow(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionRubble(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllWalls();
	TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);

	Building_determineGraphicIdsForOrientedBuildings();
	map_bridge_update_after_rotate(ccw);
	map_routing_update_walls();

	figure_tower_sentry_reroute();
	figure_hippodrome_horse_reroute();
}
