#include "Terrain.h"
#include "Data/Grid.h"
#include "Data/Settings.h"
#include "Data/Building.h"

#define SET_XY_MINMAX() \
	int xMin = x - radius;\
	int yMin = y - radius;\
	int xMax = x + size + radius - 1;\
	int yMax = y + size + radius - 1;\
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;

#define FOR_XY_MINMAX(block) \
	int gridOffset = Data_Settings_Map.gridStartOffset + 162 * yMin + xMin;\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	};

void Terrain_markNativeLand(int x, int y, int size, int radius)
{
	SET_XY_MINMAX();
	FOR_XY_MINMAX({
		Data_Grid_edge[gridOffset] |= 0x80;
	});
}

int Terrain_hasBuildingOnNativeLand(int x, int y, int size, int radius)
{
	SET_XY_MINMAX();
	FOR_XY_MINMAX({
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

