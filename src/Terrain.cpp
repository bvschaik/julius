#include "Terrain.h"
#include "Util.h"

#include "Data/Building.h"
#include "Data/Constants.h"
#include "Data/Grid.h"
#include "Data/Settings.h"

static const int tilesAroundBuildingGridOffsets[][20] = {
	{0},
	{-162, 1, 162, -1, 0},
	{-162, -161, 2, 164, 325, 324, 161, -1, 0},
	{-162, -161, -160, 3, 165, 327, 488, 487, 486, 323, 161, -1, 0},
	{-162, -161, -160, -159, 4, 166, 328, 490, 651, 650, 649, 628, 485, 323, 161, -1, 0},
	{-162, -161, -160, -159, -158, 5, 167, 329, 491, 653, 814, 813, 812, 811, 810, 647, 485, 323, 161, -1},
};

static const int tileEdgeSizeOffsets[5][5] = {
	{0, 1, 2, 3, 4},
	{8, 9, 10, 11, 12},
	{16, 17, 18, 19, 20},
	{24, 25, 26, 27, 28},
	{32, 33, 34, 35, 36},
};

struct RingTile {
	int x;
	int y;
	int gridOffset;
} ringTiles[1080];

// ringIndex[SIZE][DIST]
int ringIndex[6][7];

#define RING_SIZE(s,d) (4 * ((s) - 1) + 8 * (d))

#define FOR_XY_ADJACENT(block) \
	int baseOffset = GridOffset(x, y);\
	for (int i = 0; i < 20; i++) {\
		if (!tilesAroundBuildingGridOffsets[size][i]) break;\
		int gridOffset = baseOffset + tilesAroundBuildingGridOffsets[size][i];\
		block;\
	}

#define STORE_XY_ADJACENT(xTile,yTile) \
	*(xTile) = x + (tilesAroundBuildingGridOffsets[size][i] + 172) % 162 - 10;\
	*(yTile) = y + (tilesAroundBuildingGridOffsets[size][i] + 162) / 161 - 1;

#define FOR_XY_RADIUS(block) \
	int xMin = x - radius;\
	int yMin = y - radius;\
	int xMax = x + size + radius - 1;\
	int yMax = y + size + radius - 1;\
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;\
	int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

#define STORE_XY_RADIUS(xTile,yTile) \
	*(xTile) = xx; *(yTile) = yy;

void Terrain_addBuildingToGrids(int buildingId, int x, int y, int size, int graphicId, int terrain)
{
	if (x < 0 || x + size > Data_Settings_Map.width ||
		y < 0 || y + size > Data_Settings_Map.height) {
		return;
	}
	int xLeftmost, yLeftmost;
	switch (Data_Settings_Map.orientation) {
		case Direction_Top:
			xLeftmost = 0;
			yLeftmost = size - 1;
			break;
		case Direction_Right:
			xLeftmost = yLeftmost = 0;
			break;
		case Direction_Bottom:
			xLeftmost = size - 1;
			yLeftmost = 0;
			break;
		case Direction_Left:
			xLeftmost = yLeftmost = size - 1;
			break;
		default:
			return;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			Data_Grid_terrain[gridOffset] &= Terrain_2e80;
			Data_Grid_terrain[gridOffset] |= terrain;
			Data_Grid_buildingIds[gridOffset] = buildingId;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoOverlay;
			Data_Grid_bitfields[gridOffset] &= Bitfield_NoSizes;
			switch (size) {
				case 1: Data_Grid_bitfields[gridOffset] |= Bitfield_Size1; break;
				case 2: Data_Grid_bitfields[gridOffset] |= Bitfield_Size2; break;
				case 3: Data_Grid_bitfields[gridOffset] |= Bitfield_Size3; break;
				case 4: Data_Grid_bitfields[gridOffset] |= Bitfield_Size4; break;
				case 5: Data_Grid_bitfields[gridOffset] |= Bitfield_Size5; break;
			}
			Data_Grid_graphicIds[gridOffset] = graphicId;
			Data_Grid_edge[gridOffset] = tileEdgeSizeOffsets[dy][dx];
			if (dx == xLeftmost && dy == yLeftmost) {
				Data_Grid_edge[gridOffset] |= Edge_LeftmostTile;
			}
		}
	}
}
void Terrain_removeBuildingFromGrids(int buildingId, int x, int y)
{
	// TODO
}

int Terrain_hasRoadAccess(int x, int y, int size, int *roadX, int *roadY)
{
	// TODO
	return 0;
}

int Terrain_hasRoadAccessHippodrome(int x, int y, int *roadX, int *roadY)
{
	// TODO
	return 0;
}

int Terrain_hasRoadAccessGranary(int x, int y, int *roadX, int *roadY)
{
	// TODO
	return 0;
}

static int getRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile);
int Terrain_getClosestRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	for (int r = 1; r <= radius; r++) {
		if (getRoadWithinRadius(x, y, size, r, xTile, yTile)) {
			return 1;
		}
	}
	return 0;
}

static int getRoadWithinRadius(int x, int y, int size, int radius, int *xTile, int *yTile)
{
	FOR_XY_RADIUS(
		if (Data_Grid_terrain[gridOffset] & Terrain_Road) {
			STORE_XY_RADIUS(xTile, yTile);
			return 1;
		}
	);
	return 0;
}

int Terrain_isClear(int x, int y, int size, int disallowedTerrain, int graphicSet)
{
	if (x < 0 || x + size > Data_Settings_Map.width ||
		y < 0 || y + size > Data_Settings_Map.height) {
		return 0;
	}
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			int gridOffset = GridOffset(x + dx, y + dy);
			if (Data_Grid_terrain[gridOffset] & Terrain_NotClear & disallowedTerrain) {
				return 0;
			} else if (Data_Grid_walkerIds[gridOffset]) {
				return 0;
			} else if (graphicSet && Data_Grid_graphicIds[gridOffset] != 0) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_isAdjacentToWall(int x, int y, int size)
{
	FOR_XY_ADJACENT(
		if (Data_Grid_terrain[gridOffset] & Terrain_Wall) {
			return 1;
		}
	);
	return 0;
}

int Terrain_isAdjacentToWater(int x, int y, int size)
{
	FOR_XY_ADJACENT(
		if (Data_Grid_terrain[gridOffset] & Terrain_Water) {
			return 1;
		}
	);
	return 0;
}

int Terrain_getAdjacentRoadOrClearLand(int x, int y, int size, int *xTile, int *yTile)
{
	FOR_XY_ADJACENT(
		if ((Data_Grid_terrain[gridOffset] & Terrain_Road) ||
			!(Data_Grid_terrain[gridOffset] & Terrain_NotClear)) {
			STORE_XY_ADJACENT(xTile, yTile);
			return 1;
		}
	);
	return 0;
}

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd)
{
	FOR_XY_RADIUS(
		Data_Grid_terrain[gridOffset] |= typeToAdd;
	);
}

void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep)
{
	FOR_XY_RADIUS(
		Data_Grid_terrain[gridOffset] &= typeToKeep;
	);
}

int Terrain_existsTileWithinAreaWithType(int x, int y, int size, unsigned short type)
{
	return Terrain_existsTileWithinRadiusWithType(x, y, size, 0, type);
}

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS(
		if (type & Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	);
	return 0;
}

int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset)
{
	FOR_XY_RADIUS(
		if (gridOffset != exceptGridOffset && !Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	);
	return 0;
}

int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS(
		if (!(type & Data_Grid_terrain[gridOffset])) {
			return 0;
		}
	);
	return 1;
}

void Terrain_markNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS(
		Data_Grid_edge[gridOffset] |= 0x80;
	);
}

int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS(
		int buildingId = Data_Grid_buildingIds[gridOffset];
		if (buildingId > 0) {
			int type = Data_Buildings[buildingId].type;
			if (type != Building_MissionPost &&
				type != Building_NativeHut &&
				type != Building_NativeMeeting &&
				type != Building_NativeCrops) {
				return 1;
			}
		}
	);
	return 0;
}

void Terrain_initDistanceRing()
{
	int index = 0;
	int x, y;
	for (int s = 1; s <= 5; s++) {
		for (int d = 1; d <= 6; d++) {
			ringIndex[s][d] = index;
			// top row, from x=0
			for (y = -d, x = 0; x < s + d; x++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// right row down
			for (x = s + d - 1, y = -d + 1; y < s + d; y++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// bottom row to the left
			for (y = s + d - 1, x = s + d - 2; x >= -d; x--, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// left row up
			for (x = -d, y = s + d - 2; y >= -d; y--, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
			// top row up to x=0
			for (y = -d, x = -d + 1; x < 0; x++, index++) {
				ringTiles[index].x = x;
				ringTiles[index].y = y;
			}
		}
	}
	for (int i = 0; i < index; i++) {
		ringTiles[i].gridOffset = ringTiles[i].x + GRID_SIZE * ringTiles[i].y;
	}
}

static int isInsideMap(int x, int y)
{
	return x >= 0 && x < Data_Settings_Map.width &&
		y >= 0 && y < Data_Settings_Map.height;
}

int Terrain_isAllRockAndTreesAtDistanceRing(int x, int y, int distance)
{
	int start = ringIndex[1][distance];
	int end = start + RING_SIZE(1,distance);
	int baseOffset = GridOffset(x, y);
	for (int i = start; i < end; i++) {
		if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
			int terrain = Data_Grid_terrain[baseOffset + ringTiles[i].gridOffset];
			if (!(terrain & Terrain_Rock) || !(terrain & Terrain_Tree)) {
				return 0;
			}
		}
	}
	return 1;
}

int Terrain_isAllMeadowAtDistanceRing(int x, int y, int distance)
{
	int start = ringIndex[1][distance];
	int end = start + RING_SIZE(1,distance);
	int baseOffset = GridOffset(x, y);
	for (int i = start; i < end; i++) {
		if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
			int terrain = Data_Grid_terrain[baseOffset + ringTiles[i].gridOffset];
			if (!(terrain & Terrain_Meadow)) {
				return 0;
			}
		}
	}
	return 1;
}

static void Terrain_addDesirabilityDistanceRing(int x, int y, int size, int distance, int desirability)
{
	int isPartiallyOutsideMap = 0;
	if (x - distance < -1 || x + distance + size - 1 > Data_Settings_Map.width) {
		isPartiallyOutsideMap = 1;
	}
	if (y - distance < -1 || y + distance + size - 1 > Data_Settings_Map.height) {
		isPartiallyOutsideMap = 1;
	}
	int start = ringIndex[size][distance];
	int end = start + RING_SIZE(size,distance);
	int baseOffset = GridOffset(x, y);

	if (isPartiallyOutsideMap) {
		for (int i = start; i < end; i++) {
			if (isInsideMap(x + ringTiles[i].x, y + ringTiles[i].y)) {
				Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset] += desirability;
				BOUND(Data_Grid_desirability[baseOffset], -100, 100); // BUGFIX: bounding on wrong tile
			}
		}
	} else {
		for (int i = start; i < end; i++) {
			Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset] += desirability;
			BOUND(Data_Grid_desirability[baseOffset + ringTiles[i].gridOffset], -100, 100);
		}
	}
}

void Terrain_addDesirability(int x, int y, int size, int desBase, int desStep, int desStepSize, int desRange)
{
	if (size > 0) {
		if (desRange > 6) desRange = 6;
		int tilesWithinStep = 0;
		int distance = 1;
		while (desRange > 0) {
			Terrain_addDesirabilityDistanceRing(x, y, size, distance, desBase);
			distance++;
			desRange--;
			tilesWithinStep++;
			if (tilesWithinStep >= desStep) {
				desBase += desStepSize;
				tilesWithinStep = 0;
			}
		}
	}
}
