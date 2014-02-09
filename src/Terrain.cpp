#include "Terrain.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Building.h"

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
	};

void Terrain_setWithRadius(int x, int y, int size, int radius, unsigned short typeToAdd)
{
	FOR_XY_RADIUS({
		Data_Grid_terrain[gridOffset] |= typeToAdd;
	});
}

void Terrain_clearWithRadius(int x, int y, int size, int radius, unsigned short typeToKeep)
{
	FOR_XY_RADIUS({
		Data_Grid_terrain[gridOffset] &= typeToKeep;
	});
}

int Terrain_existsTileWithinRadiusWithType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS({
		if (type & Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	});
	return 0;
}

int Terrain_existsClearTileWithinRadius(int x, int y, int size, int radius, int exceptGridOffset)
{
	FOR_XY_RADIUS({
		if (gridOffset != exceptGridOffset && !Data_Grid_terrain[gridOffset]) {
			return 1;
		}
	});
	return 0;
}

int Terrain_allTilesWithinRadiusHaveType(int x, int y, int size, int radius, unsigned short type)
{
	FOR_XY_RADIUS({
		if (!(type & Data_Grid_terrain[gridOffset])) {
			return 0;
		}
	});
	return 1;
}

void Terrain_markNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS({
		Data_Grid_edge[gridOffset] |= 0x80;
	});
}

int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius)
{
	FOR_XY_RADIUS({
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
	});
	return 0;
}

