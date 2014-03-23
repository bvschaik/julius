#include "Resource.h"
#include "Building.h"
#include "Terrain.h"
#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"

void Resource_calculateWarehouseStocks()
{
	for (int i = 0; i < 16; i++) {
		Data_CityInfo.resourceSpaceInWarehouses[i] = 0;
		Data_CityInfo.resourceStored[i] = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse == 1 && Data_Buildings[i].type == Building_Warehouse) {
			Data_Buildings[i].hasRoadAccess = 0;
			if (Terrain_hasRoadAccess(Data_Buildings[i].x, Data_Buildings[i].y,
				Data_Buildings[i].size, 0, 0)) {
				Data_Buildings[i].hasRoadAccess = 1;
			} else if (Terrain_hasRoadAccess(Data_Buildings[i].x, Data_Buildings[i].y, 3, 0, 0)) {
				Data_Buildings[i].hasRoadAccess = 2;
			}
		}
	}
	for (int i = 0; i < MAX_BUILDINGS; i++) {
		if (Data_Buildings[i].inUse != 1 || Data_Buildings[i].type != Building_WarehouseSpace) {
			continue;
		}
		int warehouseId = Building_getMainBuildingId(i);
		if (Data_Buildings[warehouseId].hasRoadAccess) {
			Data_Buildings[i].hasRoadAccess = Data_Buildings[warehouseId].hasRoadAccess;
			if (Data_Buildings[i].subtype.warehouseResourceId) {
				int loads = Data_Buildings[i].loadsStored;
				int resource = Data_Buildings[i].subtype.warehouseResourceId;
				Data_CityInfo.resourceStored[resource] += loads;
				Data_CityInfo.resourceSpaceInWarehouses[resource] += 4 - loads;
			} else {
				Data_CityInfo.resourceSpaceInWarehouses[Resource_None] += 4;
			}
		}
	}
}

void Resource_removeFromCityStorage(int resource, int amount)
{
	// TODO
}

int Resource_getGraphicIdOffset(int resource, int type)
{
	if (resource == Resource_Meat && Data_Scenario.allowedBuildings.wharf) {
		switch (type) {
			case 0: return 40;
			case 1: return 648;
			case 2: return 8;
			case 3: return 11;
			default: return 0;
		}
	} else {
		return 0;
	}
}