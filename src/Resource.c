#include "Resource.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/count.h"
#include "core/calc.h"
#include "game/tutorial.h"
#include "scenario/building.h"

int Resource_getGraphicIdOffset(int resource, int type)
{
	if (resource == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF)) {
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

void Resource_calculateWarehouseStocks()
{
	for (int i = 0; i < RESOURCE_MAX; i++) {
		Data_CityInfo.resourceSpaceInWarehouses[i] = 0;
		Data_CityInfo.resourceStored[i] = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
			b->hasRoadAccess = 0;
			if (Terrain_hasRoadAccess(b->x, b->y, b->size, 0, 0)) {
				b->hasRoadAccess = 1;
			} else if (Terrain_hasRoadAccess(b->x, b->y, 3, 0, 0)) {
				b->hasRoadAccess = 2;
			}
		}
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE_SPACE) {
			continue;
		}
		building *warehouse = building_main(b);
		if (warehouse->hasRoadAccess) {
			b->hasRoadAccess = warehouse->hasRoadAccess;
			if (b->subtype.warehouseResourceId) {
				int loads = b->loadsStored;
				int resource = b->subtype.warehouseResourceId;
				Data_CityInfo.resourceStored[resource] += loads;
				Data_CityInfo.resourceSpaceInWarehouses[resource] += 4 - loads;
			} else {
				Data_CityInfo.resourceSpaceInWarehouses[RESOURCE_NONE] += 4;
			}
		}
	}
}

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
