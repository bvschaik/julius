#include "Grid.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/Grid.h"
#include "Data/Random.h"

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
	int changed = 1;
	Grid_andShortGrid(Data_Grid_terrain, ~(Terrain_FountainRange | Terrain_ReservoirRange));
	// TODO
}

void WaterManagement_updateFireSpreadDirection()
{
	spreadDirection = Data_Random.random1_7bit & 7;
}
