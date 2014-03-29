#include "Terrain.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Building.h"

static int tilesAroundBuildingGridOffsets[][20] = {
	{0},
	{-162, 1, 162, -1, 0},
	{-162, -161, 2, 164, 325, 324, 161, -1, 0},
	{-162, -161, -160, 3, 165, 327, 488, 487, 486, 323, 161, -1, 0},
	{-162, -161, -160, -159, 4, 166, 328, 490, 651, 650, 649, 628, 485, 323, 161, -1, 0},
	{-162, -161, -160, -159, -158, 5, 167, 329, 491, 653, 814, 813, 812, 811, 810, 647, 485, 323, 161, -1},
};

#define FOR_XY_ADJACENT(block) \
	int baseOffset = Data_Settings_Map.gridStartOffset + 162 * y + x;\
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
	int gridOffset = Data_Settings_Map.gridStartOffset + 162 * yMin + xMin;\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}

#define STORE_XY_RADIUS(xTile,yTile) \
	*(xTile) = xx; *(yTile) = yy;

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

