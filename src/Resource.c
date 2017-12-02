#include "Resource.h"

#include "Building.h"
#include "core/calc.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"

#include "building/count.h"
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
		struct Data_Building *b = &Data_Buildings[i];
		if (BuildingIsInUse(i) && b->type == BUILDING_WAREHOUSE) {
			b->hasRoadAccess = 0;
			if (Terrain_hasRoadAccess(b->x, b->y, b->size, 0, 0)) {
				b->hasRoadAccess = 1;
			} else if (Terrain_hasRoadAccess(b->x, b->y, 3, 0, 0)) {
				b->hasRoadAccess = 2;
			}
		}
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_WAREHOUSE_SPACE) {
			continue;
		}
		int warehouseId = Building_getMainBuildingId(i);
		if (Data_Buildings[warehouseId].hasRoadAccess) {
			b->hasRoadAccess = Data_Buildings[warehouseId].hasRoadAccess;
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

void Resource_calculateWorkshopStocks()
{
	for (int i = 0; i < 6; i++) {
		Data_CityInfo.resourceWorkshopRawMaterialStored[i] = 0;
		Data_CityInfo.resourceWorkshopRawMaterialSpace[i] = 0;
	}
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || !BuildingIsWorkshop(b->type)) {
			continue;
		}
		b->hasRoadAccess = 0;
		if (Terrain_hasRoadAccess(b->x, b->y, b->size, 0, 0)) {
			b->hasRoadAccess = 1;
			int room = 2 - b->loadsStored;
			if (room < 0) {
				room = 0;
			}
			int workshopResource = b->subtype.workshopType;
			Data_CityInfo.resourceWorkshopRawMaterialSpace[workshopResource] += room;
			Data_CityInfo.resourceWorkshopRawMaterialStored[workshopResource] += b->loadsStored;
		}
	}
}

int Resource_getWorkshopWithRoomForRawMaterial(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst)
{
	if (Data_CityInfo.resourceStockpiled[resource]) {
		return 0;
	}
	int outputType;
	switch (resource) {
		case RESOURCE_OLIVES: outputType = WORKSHOP_OLIVES_TO_OIL; break;
		case RESOURCE_VINES: outputType = WORKSHOP_VINES_TO_WINE; break;
		case RESOURCE_IRON: outputType = WORKSHOP_IRON_TO_WEAPONS; break;
		case RESOURCE_TIMBER: outputType = WORKSHOP_TIMBER_TO_FURNITURE; break;
		case RESOURCE_CLAY: outputType = WORKSHOP_CLAY_TO_POTTERY; break;
		default: return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || !BuildingIsWorkshop(b->type)) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		if (b->subtype.workshopType == outputType && b->roadNetworkId == roadNetworkId && b->loadsStored < 2) {
			int dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			if (b->loadsStored > 0) {
				dist += 20;
			}
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].roadAccessX;
	*yDst = Data_Buildings[minBuildingId].roadAccessY;
	return minBuildingId;
}

int Resource_getWorkshopForRawMaterial(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst)
{
	if (Data_CityInfo.resourceStockpiled[resource]) {
		return 0;
	}
	int outputType;
	switch (resource) {
		case RESOURCE_OLIVES: outputType = WORKSHOP_OLIVES_TO_OIL; break;
		case RESOURCE_VINES: outputType = WORKSHOP_VINES_TO_WINE; break;
		case RESOURCE_IRON: outputType = WORKSHOP_IRON_TO_WEAPONS; break;
		case RESOURCE_TIMBER: outputType = WORKSHOP_TIMBER_TO_FURNITURE; break;
		case RESOURCE_CLAY: outputType = WORKSHOP_CLAY_TO_POTTERY; break;
		default: return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || !BuildingIsWorkshop(b->type)) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		if (b->subtype.workshopType == outputType && b->roadNetworkId == roadNetworkId) {
			int dist = 10 * b->loadsStored +
				Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].roadAccessX;
	*yDst = Data_Buildings[minBuildingId].roadAccessY;
	return minBuildingId;
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
	struct Data_Building *b = &Data_Buildings[Data_CityInfo.buildingBarracksBuildingId];
	if (b->loadsStored < 5 && Data_CityInfo.militaryLegionaryLegions > 0) {
		if (Terrain_hasRoadAccess(b->x, b->y, b->size, xDst, yDst) && b->roadNetworkId == roadNetworkId) {
			return Data_CityInfo.buildingBarracksBuildingId;
		}
	}
	return 0;
}

void Resource_addRawMaterialToWorkshop(int buildingId)
{
	if (buildingId > 0 && BuildingIsWorkshop(Data_Buildings[buildingId].type)) {
		Data_Buildings[buildingId].loadsStored++; // BUG: any raw material accepted
	}
}

void Resource_addWeaponToBarracks(int buildingId)
{
	if (buildingId > 0) {
		Data_Buildings[buildingId].loadsStored++;
	}
}

int Resource_getDistance(int x1, int y1, int x2, int y2, int distToEntry1, int distToEntry2)
{
	int diff;
	if (distToEntry1 > distToEntry2) {
		diff = distToEntry1 - distToEntry2;
	} else {
		diff = distToEntry2 - distToEntry1;
	}
	if (distToEntry1 == -1) {
		diff = 0;
	}
	return diff + calc_maximum_distance(x1, y1, x2, y2);
}
