#include "Terrain.h"
#include "Terrain_private.h"

#include "Building.h"
#include "FigureAction.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Grid.h"
#include "Data/State.h"

#include "core/calc.h"
#include "graphics/image.h"
#include "map/bridge.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/random.h"
#include "map/ring.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "scenario/map.h"

static const int tilesAroundBuildingGridOffsets[][20] = {
	{0},
	{-162, 1, 162, -1, 0},
	{-162, -161, 2, 164, 325, 324, 161, -1, 0},
	{-162, -161, -160, 3, 165, 327, 488, 487, 486, 323, 161, -1, 0},
	{-162, -161, -160, -159, 4, 166, 328, 490, 651, 650, 649, 648, 485, 323, 161, -1, 0},
	{-162, -161, -160, -159, -158, 5, 167, 329, 491, 653, 814, 813, 812, 811, 810, 647, 485, 323, 161, -1},
};

static const int tileEdgeSizeOffsets[5][5] = {
	{0, 1, 2, 3, 4},
	{8, 9, 10, 11, 12},
	{16, 17, 18, 19, 20},
	{24, 25, 26, 27, 28},
	{32, 33, 34, 35, 36},
};

#define FOR_XY_ADJACENT \
	{int baseOffset = map_grid_offset(x, y);\
	for (int i = 0; i < 20; i++) {\
		if (!tilesAroundBuildingGridOffsets[size][i]) break;\
		int gridOffset = baseOffset + tilesAroundBuildingGridOffsets[size][i];

#define END_FOR_XY_ADJACENT }}

#define STORE_XY_ADJACENT(xTile,yTile) \
	*(xTile) = x + (tilesAroundBuildingGridOffsets[size][i] + 172) % 162 - 10;\
	*(yTile) = y + (tilesAroundBuildingGridOffsets[size][i] + 162) / 161 - 1;

#define FOR_XY_RADIUS \
	int xMin = x - radius;\
	int yMin = y - radius;\
	int xMax = x + size + radius - 1;\
	int yMax = y + size + radius - 1;\
	map_grid_bound_area(&xMin, &yMin, &xMax, &yMax);\
	int gridOffset = map_grid_offset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {

#define END_FOR_XY_RADIUS \
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

#define STORE_XY_RADIUS(xTile,yTile) \
	*(xTile) = xx; *(yTile) = yy;

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain)
{
	if (IsOutsideMap(x, y, size)) {
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
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= terrain;
			Data_Grid_buildingIds[gridOffset] = buildingId;
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, size);
			Data_Grid_graphicIds[gridOffset] = graphicId;
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
	if (IsOutsideMap(x, y, 1)) {
		return;
	}
	int size;
	int baseGridOffset = getNorthTileGridOffset(x, y, &size);
	x = map_grid_offset_to_x(baseGridOffset);
	y = map_grid_offset_to_y(baseGridOffset);
	if (Data_Grid_terrain[baseGridOffset] == Terrain_Rock) {
		return;
	}
	if (buildingId && BuildingIsFarm(Data_Buildings[buildingId].type)) {
		size = 3;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (buildingId && Data_Grid_buildingIds[gridOffset] != buildingId) {
				continue;
			}
			if (buildingId && Data_Buildings[buildingId].type != BUILDING_BURNING_RUIN) {
				Data_Grid_rubbleBuildingType[gridOffset] = Data_Buildings[buildingId].type;
			}
			map_property_clear_constructing(gridOffset);
			map_property_set_multi_tile_size(gridOffset, 1);
			map_property_clear_multi_tile_xy(gridOffset);
			map_property_mark_draw_tile(gridOffset);
			Data_Grid_aqueducts[gridOffset] = 0;
			Data_Grid_buildingIds[gridOffset] = 0;
			Data_Grid_buildingDamage[gridOffset] = 0;
			Data_Grid_spriteOffsets[gridOffset] = 0;
			if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
				Data_Grid_terrain[gridOffset] &= Terrain_Water;
				TerrainGraphics_setTileWater(x + dx, y + dy);
			} else {
				Data_Grid_graphicIds[gridOffset] =
					image_group(GROUP_TERRAIN_UGLY_GRASS) +
					(map_random_get(gridOffset) & 7);
				Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			}
		}
	}
	TerrainGraphics_updateRegionEmptyLand(x, y, x + size, y + size);
	TerrainGraphics_updateRegionMeadow(x, y, x + size, y + size);
	TerrainGraphics_updateRegionRubble(x, y, x + size, y + size);
}

#define ADD_ROAD(g) \
	if (!(Data_Grid_terrain[g] & Terrain_NotClear)) \
		Data_Grid_terrain[g] |= Terrain_Road;

void Terrain_addRoadsForGatehouse(int x, int y, int orientation)
{
	// roads under gatehouse
	Data_Grid_terrain[map_grid_offset(x,y)] |= Terrain_Road;
	Data_Grid_terrain[map_grid_offset(x+1,y)] |= Terrain_Road;
	Data_Grid_terrain[map_grid_offset(x,y+1)] |= Terrain_Road;
	Data_Grid_terrain[map_grid_offset(x+1,y+1)] |= Terrain_Road;

	// free roads before/after gate
	if (orientation == 1) {
		ADD_ROAD(map_grid_offset(x, y-1));
		ADD_ROAD(map_grid_offset(x+1, y-1));
		ADD_ROAD(map_grid_offset(x, y+2));
		ADD_ROAD(map_grid_offset(x+1, y+2));
	} else if (orientation == 2) {
		ADD_ROAD(map_grid_offset(x-1, y));
		ADD_ROAD(map_grid_offset(x-1, y+1));
		ADD_ROAD(map_grid_offset(x+2, y));
		ADD_ROAD(map_grid_offset(x+2, y+1));
	}
}

void Terrain_addRoadsForTriumphalArch(int x, int y, int orientation)
{
	if (orientation == 1) {
		// road in the middle
		Data_Grid_terrain[map_grid_offset(x+1,y)] |= Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+1,y+1)] |= Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+1,y+2)] |= Terrain_Road;
		// no roads on other tiles
		Data_Grid_terrain[map_grid_offset(x,y)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x,y+1)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y+1)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y+2)] &= ~Terrain_Road;
	} else if (orientation == 2) {
		// road in the middle
		Data_Grid_terrain[map_grid_offset(x,y+1)] |= Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+1,y+1)] |= Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y+1)] |= Terrain_Road;
		// no roads on other tiles
		Data_Grid_terrain[map_grid_offset(x,y)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+1,y)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+1,y+2)] &= ~Terrain_Road;
		Data_Grid_terrain[map_grid_offset(x+2,y+2)] &= ~Terrain_Road;
	}
}

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY)
{
	int minValue = 12;
	int minGridOffset = map_grid_offset(x, y);
	FOR_XY_ADJACENT {
		if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) ||
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != BUILDING_GATEHOUSE) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
		if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) ||
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != BUILDING_GATEHOUSE) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
		if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) ||
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != BUILDING_GATEHOUSE) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
		if (!(Data_Grid_terrain[gridOffset] & Terrain_Building) ||
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].type != BUILDING_GATEHOUSE) {
			if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
	if (Data_Grid_terrain[map_grid_offset(x + 1, y - 1)] & Terrain_Road) {
		xx = x + 1;
		yy = y - 1;
	} else if (Data_Grid_terrain[map_grid_offset(x + 3, y + 1)] & Terrain_Road) {
		xx = x + 3;
		yy = y + 1;
	} else if (Data_Grid_terrain[map_grid_offset(x + 1, y + 3)] & Terrain_Road) {
		xx = x + 1;
		yy = y + 3;
	} else if (Data_Grid_terrain[map_grid_offset(x - 1, y + 1)] & Terrain_Road) {
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
	if (Data_Grid_terrain[map_grid_offset(x, y)] & Terrain_Road) {
		roadTilesWithin |= 1;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_Road) {
		roadTilesWithin |= 2;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_Road) {
		roadTilesWithin |= 4;
		numRoadTilesWithin++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & Terrain_Road) {
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
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & Terrain_Road) {
		numRoadTilesTop++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & Terrain_Road) {
		numRoadTilesTop++;
	}
	// bottom
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 2)] & Terrain_Road) {
		numRoadTilesBottom++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 2)] & Terrain_Road) {
		numRoadTilesBottom++;
	}
	// left
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & Terrain_Road) {
		numRoadTilesLeft++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & Terrain_Road) {
		numRoadTilesLeft++;
	}
	// right
	if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 0)] & Terrain_Road) {
		numRoadTilesRight++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 1)] & Terrain_Road) {
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
	if (Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 0)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// road tiles top to bottom
	if ((Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesTopBottom++;
	} else if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if ((Data_Grid_terrain[gridOffset + map_grid_delta(1, 2)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesTopBottom++;
	} else if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 2)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// road tiles left to right
	if ((Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesLeftRight++;
	} else if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	if ((Data_Grid_terrain[gridOffset + map_grid_delta(2, 1)] & Terrain_NotClear) == Terrain_Road) {
		numRoadTilesLeftRight++;
	} else if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 1)] & Terrain_NotClear) {
		numBlockedTiles++;
	}
	// center tile
	if ((Data_Grid_terrain[gridOffset + map_grid_delta(2, 1)] & Terrain_NotClear) == Terrain_Road) {
		// do nothing
	} else if (Data_Grid_terrain[gridOffset + map_grid_delta(2, 1)] & Terrain_NotClear) {
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
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
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
		if (Data_Grid_terrain[gridOffset] & Terrain_Road && map_routing_distance(gridOffset) > 0) {
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
			if (Data_Grid_terrain[gridOffset] & Terrain_Road && map_routing_distance(gridOffset) > 0) {
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
	int isRoad = (Data_Grid_terrain[gridOffset] & Terrain_Road) ? 1 : 0;
	if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
		int type = Data_Buildings[Data_Grid_buildingIds[gridOffset]].type;
		if (type == BUILDING_GATEHOUSE) {
			if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].subtype.orientation == gateOrientation) {
				isRoad = 1;
			}
		} else if (type == BUILDING_GRANARY) {
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
		if (Data_Buildings[Data_Grid_buildingIds[gridOffset]].type == BUILDING_GRANARY) {
			roadTiles = 2;
		}
	}
	return roadTiles;
}

static int getAdjacentRoadTileForRoaming(int gridOffset)
{
	int isRoad = (Data_Grid_terrain[gridOffset] & (Terrain_Road | Terrain_AccessRamp)) ? 1 : 0;
	if (Data_Grid_terrain[gridOffset] & Terrain_Building) {
		int type = Data_Buildings[Data_Grid_buildingIds[gridOffset]].type;
		if (type == BUILDING_GATEHOUSE) {
			isRoad = 0;
		} else if (type == BUILDING_GRANARY) {
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
	roadTiles[1] = (Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & (Terrain_Road | Terrain_AccessRamp)) ? 1 : 0;
	roadTiles[3] = (Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & (Terrain_Road | Terrain_AccessRamp)) ? 1 : 0;
	roadTiles[5] = (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & (Terrain_Road | Terrain_AccessRamp)) ? 1 : 0;
	roadTiles[7] = (Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & (Terrain_Road | Terrain_AccessRamp)) ? 1 : 0;
	
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
	if (IsOutsideMap(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (Data_Grid_terrain[gridOffset] & Terrain_NotClear & disallowedTerrain) {
				return 0;
			} else if (map_has_figure_at(gridOffset)) {
				return 0;
			} else if (graphicSet && Data_Grid_graphicIds[gridOffset] != 0) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_canSpawnFishingBoatInWater(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT {
		if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
			if (!(Data_Grid_terrain[gridOffset] & Terrain_Building)) {
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
		if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_isAdjacentToWater(int x, int y, int size)
{
	FOR_XY_ADJACENT {
		if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_isAdjacentToOpenWater(int x, int y, int size)
{
	FOR_XY_ADJACENT {
		if ((Data_Grid_terrain[gridOffset] & Terrain_Water) &&
			map_routing_distance(gridOffset)) {
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT {
		if ((Data_Grid_terrain[gridOffset] & Terrain_Road) ||
			!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
			STORE_XY_ADJACENT(xTile, yTile);
			return 1;
		}
	} END_FOR_XY_ADJACENT;
	return 0;
}

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd)
{
	FOR_XY_RADIUS {
		Data_Grid_terrain[gridOffset] |= typeToAdd;
	} END_FOR_XY_RADIUS;
}

void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep)
{
	FOR_XY_RADIUS {
		Data_Grid_terrain[gridOffset] &= typeToKeep;
	} END_FOR_XY_RADIUS;
}

int Terrain_existsTileWithinAreaWithType(int x, int y, int size, unsigned short type)
{
	for (int yy = y; yy < y + size; yy++) {
		for (int xx = x; xx < x + size; xx++) {
			if (IsInsideMap(xx, yy) && (type & Data_Grid_terrain[map_grid_offset(xx, yy)])) {
				return 1;
			}
		}
	}
	return 0;
}

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS {
		if (type & Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset, int *xTile, int *yTile)
{
	FOR_XY_RADIUS {
		if (gridOffset != exceptGridOffset && !Data_Grid_terrain[gridOffset]) {
			STORE_XY_RADIUS(xTile, yTile);
			return 1;
		}
	} END_FOR_XY_RADIUS;
	*xTile = xMax;
	*yTile = yMax;
	return 0;
}

int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS {
		if (!(type & Data_Grid_terrain[gridOffset])) {
			return 0;
		}
	} END_FOR_XY_RADIUS;
	return 1;
}

void Terrain_markBuildingsWithinWellRadius(int wellId, int radius)
{
	int x = Data_Buildings[wellId].x;
	int y = Data_Buildings[wellId].y;
	int size = 1;
	FOR_XY_RADIUS {
		if (Data_Grid_buildingIds[gridOffset]) {
			Data_Buildings[Data_Grid_buildingIds[gridOffset]].hasWellAccess = 1;
		}
	} END_FOR_XY_RADIUS;
}

int Terrain_allHousesWithinWellRadiusHaveFountain(int wellId, int radius)
{
	int numHouses = 0;
	int x = Data_Buildings[wellId].x;
	int y = Data_Buildings[wellId].y;
	int size = 1;
	FOR_XY_RADIUS {
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId > 0 && Data_Buildings[buildingId].houseSize) {
			numHouses++;
			if (!(Data_Grid_terrain[gridOffset] & Terrain_FountainRange)) {
				return 0;
			}
		}
	} END_FOR_XY_RADIUS;
	return numHouses ? 1 : 2;
}

int Terrain_isReservoir(int gridOffset)
{
	int buildingId = Data_Grid_buildingIds[gridOffset];
	if (!buildingId || Data_Buildings[buildingId].type != BUILDING_RESERVOIR) {
		return 0;
	}
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			int tileOffset = gridOffset + map_grid_delta(x, y);
			if (Data_Grid_buildingIds[tileOffset] != buildingId) {
				return 0;
			}
		}
	}
	return 1;
}

void Terrain_markNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS {
		map_property_mark_native_land(gridOffset);
	} END_FOR_XY_RADIUS;
}

int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS {
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId > 0) {
			int type = Data_Buildings[buildingId].type;
			if (type != BUILDING_MISSION_POST &&
				type != BUILDING_NATIVE_HUT &&
				type != BUILDING_NATIVE_MEETING &&
				type != BUILDING_NATIVE_CROPS) {
				return 1;
			}
		}
	} END_FOR_XY_RADIUS;
	return 0;
}

int Terrain_isAllRockAndTreesAtDistanceRing(int x, int y, int distance)
{
	int start = map_ring_start(1, distance);
	int end = map_ring_end(1, distance);
	int baseOffset = map_grid_offset(x, y);
	for (int i = start; i < end; i++) {
        const ring_tile *tile = map_ring_tile(i);
		if (map_ring_is_inside_map(x + tile->x, y + tile->y)) {
			int terrain = Data_Grid_terrain[baseOffset + tile->grid_offset];
			if (!(terrain & Terrain_Rock) || !(terrain & Terrain_Tree)) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_isAllMeadowAtDistanceRing(int x, int y, int distance)
{
	int start = map_ring_start(1, distance);
	int end = map_ring_end(1, distance);
	int baseOffset = map_grid_offset(x, y);
	for (int i = start; i < end; i++) {
        const ring_tile *tile = map_ring_tile(i);
		if (map_ring_is_inside_map(x + tile->x, y + tile->y)) {
			int terrain = Data_Grid_terrain[baseOffset + tile->grid_offset];
			if (!(terrain & Terrain_Meadow)) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_countTerrainTypeDirectlyAdjacentTo(int gridOffset, int terrainMask)
{
	int count = 0;
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & terrainMask) {
		count++;
	}
	return count;
}

int Terrain_countTerrainTypeDiagonallyAdjacentTo(int gridOffset, int terrainMask)
{
	int count = 0;
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, -1)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(1, 1)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 1)] & terrainMask) {
		count++;
	}
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, -1)] & terrainMask) {
		count++;
	}
	return count;
}

int Terrain_hasTerrainTypeSameYAdjacentTo(int gridOffset, int terrainMask)
{
	if (Data_Grid_terrain[gridOffset + map_grid_delta(-1, 0)] & terrainMask ||
		Data_Grid_terrain[gridOffset + map_grid_delta(1, 0)] & terrainMask) {
		return 1;
	}
	return 0;
}

int Terrain_hasTerrainTypeSameXAdjacentTo(int gridOffset, int terrainMask)
{
	if (Data_Grid_terrain[gridOffset + map_grid_delta(0, -1)] & terrainMask ||
		Data_Grid_terrain[gridOffset + map_grid_delta(0, 1)] & terrainMask) {
		return 1;
	}
	return 0;
}

void Terrain_updateEntryExitFlags(int remove)
{
	if (remove) {
		int gridOffsetEntry = map_grid_offset(Data_CityInfo_Extra.entryPointFlag.x, Data_CityInfo_Extra.entryPointFlag.y);
		Data_Grid_terrain[gridOffsetEntry] &= ~Terrain_Rock;
		int gridOffsetExit = map_grid_offset(Data_CityInfo_Extra.exitPointFlag.x, Data_CityInfo_Extra.exitPointFlag.y);
		Data_Grid_terrain[gridOffsetExit] &= ~Terrain_Rock;
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
		Data_Grid_terrain[gridOffsetFlag] |= Terrain_Rock;
		int orientation = (Data_State.map.orientation + entryOrientation) % 8;
		Data_Grid_graphicIds[gridOffsetFlag] = image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + orientation / 2;
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
		Data_Grid_terrain[gridOffsetFlag] |= Terrain_Rock;
		int orientation = (Data_State.map.orientation + exitOrientation) % 8;
		Data_Grid_graphicIds[gridOffsetFlag] = image_group(GROUP_TERRAIN_ENTRY_EXIT_FLAGS) + 4 + orientation / 2;
	}
}

int Terrain_isClearToBuild(int size, int x, int y, int terrainMask)
{
	switch (Data_State.map.orientation) {
		case 2: x = x - size + 1; break;
		case 4: x = x - size + 1; // fall-through
		case 6: y = y - size + 1; break;
	}
	if (IsOutsideMap(x, y, size)) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if (terrainMask & Data_Grid_terrain[gridOffset] & Terrain_NotClear) {
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
	if (IsOutsideMap(x, y, size)) {
		return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = map_grid_offset(x + dx, y + dy);
			if ((terrainMask & Data_Grid_terrain[gridOffset] & Terrain_NotClear) ||
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
	Data_State.undoAvailable = 0;
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

	FigureAction_TowerSentry_reroute();
	FigureAction_HippodromeHorse_reroute();
}
