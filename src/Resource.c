#include "Resource.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/count.h"
#include "core/calc.h"
#include "game/tutorial.h"
#include "scenario/building.h"

int Resource_getBarracksForWeapon(int xUnused, int yUnused, int resource, int roadNetworkId, int *xDst, int *yDst)
{
	if (resource != RESOURCE_WEAPONS) {
		return 0;
	}
	if (Data_CityInfo.resourceStockpiled[RESOURCE_WEAPONS]) {
		return 0;
	}
	if (building_count_active(BUILDING_BARRACKS) <= 0) {
		return 0;
	}
	building *b = building_get(Data_CityInfo.buildingBarracksBuildingId);
	if (b->loadsStored < 5 && Data_CityInfo.militaryLegionaryLegions > 0) {
		if (Terrain_hasRoadAccess(b->x, b->y, b->size, xDst, yDst) && b->roadNetworkId == roadNetworkId) {
			return Data_CityInfo.buildingBarracksBuildingId;
		}
	}
	return 0;
}

void Resource_addWeaponToBarracks(building *barracks)
{
	if (barracks->id > 0) {
		barracks->loadsStored++;
	}
}
