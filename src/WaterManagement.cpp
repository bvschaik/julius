#include "Grid.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/Random.h"

#include <string.h>

static int spreadDirection;

void WaterManagement_updateHouseWaterAccess()
{
	Data_BuildingList.small.numItems = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1) {
			continue;
		}
		if (Data_Buildings[i].type == Building_Well) {
			DATA_BUILDINGLIST_SMALL_ENQUEUE(i);
		} else if (Data_Buildings[i].houseSize) {
			Data_Buildings[i].hasWaterAccess = 0;
			Data_Buildings[i].hasWellAccess = 0;
			if (Terrain_existsTileWithinAreaWithType(
				Data_Buildings[i].x, Data_Buildings[i].y,
				Data_Buildings[i].size, Terrain_FountainRange)) {
				Data_Buildings[i].hasWaterAccess = 1;
			}
		}
	}
	DATA_BUILDINGLIST_SMALL_FOREACH(
		Terrain_markBuildingsWithinWellRadius(item, 2);
	);
}

void WaterManagement_updateReservoirFountain()
{
	Grid_andShortGrid(Data_Grid_terrain, ~(Terrain_FountainRange | Terrain_ReservoirRange));
	TerrainGraphics_setAllAqueductsToNoWater();
	memset(Data_BuildingList.buildingIds, 0, 4000);
	Data_BuildingList.size = 0;
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].type == Building_Reservoir) {
			Data_BuildingList.buildingIds[Data_BuildingList.size++] = i;
			if (Terrain_existsTileWithinAreaWithType(
				Data_Buildings[i].x - 1, Data_Buildings[i].y - 1, 5, Terrain_Water)) {
				Data_Buildings[i].hasWaterAccess = 2;
			} else {
				Data_Buildings[i].hasWaterAccess = 0;
			}
		}
	}
	int changed = 1;
	while (changed == 1) {
		for (int i = 0; i < Data_BuildingList.size; i++) {
			int buildingId = Data_BuildingList.buildingIds[i];
			if (Data_Buildings[buildingId].hasWaterAccess == 2) {
				Data_Buildings[buildingId].hasWaterAccess = 1;
				changed = 1;
				// TODO
			}
		}
	}
	// TODO
}

void WaterManagement_updateFireSpreadDirection()
{
	spreadDirection = Data_Random.random1_7bit & 7;
}
