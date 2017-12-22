#include "Resource.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/count.h"
#include "building/model.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/finance.h"
#include "core/calc.h"
#include "empire/trade_prices.h"
#include "graphics/image.h"
#include "game/tutorial.h"
#include "map/image.h"
#include "scenario/property.h"

static int granaryGettingResource[7];
static int granaryAcceptingResource[7];

int Resource_getWarehouseForStoringResource(
	int srcBuildingId, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst)
{
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE_SPACE) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		building *dst = building_main(b);
		if (srcBuildingId == dst->id) {
			continue;
		}
		const building_storage *s = building_storage_get(dst->storage_id);
		if (s->resource_state[resource] == BUILDING_STORAGE_STATE_NOT_ACCEPTING || s->empty_all) {
			continue;
		}
		int pctWorkers = calc_percentage(dst->numWorkers, model_get_building(dst->type)->laborers);
		if (pctWorkers < 100) {
			if (understaffed) {
				*understaffed += 1;
			}
			continue;
		}
		int dist;
		if (b->subtype.warehouseResourceId == RESOURCE_NONE) { // empty warehouse space
			dist = calc_distance_with_penalty(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
		} else if (b->subtype.warehouseResourceId == resource && b->loadsStored < 4) {
			dist = calc_distance_with_penalty(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
		} else {
			dist = 0;
		}
		if (dist > 0 && dist < minDist) {
			minDist = dist;
			minBuildingId = i;
		}
	}
	building *b = building_main(building_get(minBuildingId));
	if (b->hasRoadAccess == 1) {
		*xDst = b->x;
		*yDst = b->y;
	} else if (!Terrain_hasRoadAccess(b->x, b->y, 3, xDst, yDst)) {
		return 0;
	}
	return minBuildingId;
}

int Resource_getWarehouseForGettingResource(int srcBuildingId, int resource, int *xDst, int *yDst)
{
	building *bSrc = building_get(srcBuildingId);
	int minDist = 10000;
	building *minBuilding = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (i == srcBuildingId) {
			continue;
		}
		int loadsStored = 0;
		building *space = b;
		const building_storage *s = building_storage_get(b->storage_id);
		for (int t = 0; t < 8; t++) {
			space = building_next(space);
			if (space->id > 0 && space->loadsStored > 0) {
				if (space->subtype.warehouseResourceId == resource) {
					loadsStored += space->loadsStored;
				}
			}
		}
		if (loadsStored > 0 && s->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
			int dist = calc_distance_with_penalty(b->x, b->y, bSrc->x, bSrc->y,
				bSrc->distanceFromEntry, b->distanceFromEntry);
			dist -= 4 * loadsStored;
			if (dist < minDist) {
				minDist = dist;
				minBuilding = b;
			}
		}
	}
	if (minBuilding) {
		*xDst = minBuilding->roadAccessX;
		*yDst = minBuilding->roadAccessY;
		return minBuilding->id;
	} else {
		return 0;
	}
}

void Resource_removeFromWarehouseForMercury(int buildingId, int amount)
{
	building *warehouse = building_get(buildingId);
	if (warehouse->type != BUILDING_WAREHOUSE) {
		return;
	}
	building *space = warehouse;
	for (int i = 0; i < 8 && amount > 0; i++) {
		space = building_next(space);
		if (space->id <= 0 || space->loadsStored <= 0) {
			continue;
		}
		int resource = space->subtype.warehouseResourceId;
		if (space->loadsStored > amount) {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
			Data_CityInfo.resourceStored[resource] -= amount;
			space->loadsStored -= amount;
			amount = 0;
		} else {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += space->loadsStored;
			Data_CityInfo.resourceStored[resource] -= space->loadsStored;
			amount -= space->loadsStored;
			space->loadsStored = 0;
			space->subtype.warehouseResourceId = RESOURCE_NONE;
		}
		building_warehouse_space_set_image(space, resource);
	}
}

int Resource_getAmountStoredInWarehouse(int buildingId, int resource)
{
	int loads = 0;
    building *space = building_get(buildingId);
	for (int i = 0; i < 8; i++) {
		space = building_next(space);
		if (space->id <= 0) {
			return 0;
		}
		if (space->subtype.warehouseResourceId && space->subtype.warehouseResourceId == resource) {
			loads += space->loadsStored;
		}
	}
	return loads;
}

int Resource_getWarehouseSpaceInfo(building *warehouse)
{
	int totalLoads = 0;
	int emptySpaces = 0;
    building *space = warehouse;
	for (int i = 0; i < 8; i++) {
		space = building_next(space);
		if (space->id <= 0) {
			return 0;
		}
		if (space->subtype.warehouseResourceId) {
			totalLoads += space->loadsStored;
		} else {
			emptySpaces++;
		}
	}
	if (emptySpaces > 0) {
		return 0; // room available
	} else if (totalLoads < 32) {
		return 2; // some room available for existing goods
	} else {
		return 1; // full
	}
}

void Resource_addImportedResourceToWarehouseSpace(building *space, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]--;
	Data_CityInfo.resourceStored[resourceId]++;
	space->loadsStored++;
	space->subtype.warehouseResourceId = resourceId;
	
    int price = trade_price_buy(resourceId);
    city_finance_process_import(price);
	
	building_warehouse_space_set_image(space, resourceId);
}

void Resource_removeExportedResourceFromWarehouseSpace(building *space, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]++;
	Data_CityInfo.resourceStored[resourceId]--;
	space->loadsStored--;
	if (space->loadsStored <= 0) {
		space->subtype.warehouseResourceId = RESOURCE_NONE;
	}
	
	int price = trade_price_sell(resourceId);
    city_finance_process_export(price);
	
	building_warehouse_space_set_image(space, resourceId);
}

static int determineGranaryAcceptFoods()
{
	if (scenario_property_rome_supplies_wheat()) {
		return 0;
	}
	for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
		granaryAcceptingResource[i] = 0;
	}
	int canAccept = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
			continue;
		}
		int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
		if (pctWorkers >= 100 && b->data.storage.resourceStored[RESOURCE_NONE] >= 1200) {
			const building_storage *s = building_storage_get(b->storage_id);
			if (!s->empty_all) {
				for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
					if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
						granaryAcceptingResource[r]++;
						canAccept = 1;
					}
				}
			}
		}
	}
	return canAccept;
}

static int determineGranaryGetFoods()
{
	if (scenario_property_rome_supplies_wheat()) {
		return 0;
	}
	for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
		granaryGettingResource[i] = 0;
	}
	int canGet = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
			continue;
		}
		int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
		if (pctWorkers >= 100 && b->data.storage.resourceStored[RESOURCE_NONE] > 100) {
			const building_storage *s = building_storage_get(b->storage_id);
			if (!s->empty_all) {
				for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
					if (s->resource_state[r] == BUILDING_STORAGE_STATE_GETTING) {
						granaryGettingResource[r]++;
						canGet = 1;
					}
				}
			}
		}
	}
	return canGet;
}

static int storesNonStockpiledFood(building *space, int *granaryResources)
{
	if (space->id <= 0) {
		return 0;
	}
	if (space->loadsStored <= 0) {
		return 0;
	}
	int resource = space->subtype.warehouseResourceId;
	if (Data_CityInfo.resourceStockpiled[resource]) {
		return 0;
	}
	if (resource == RESOURCE_WHEAT || resource == RESOURCE_VEGETABLES ||
		resource == RESOURCE_FRUIT || resource == RESOURCE_MEAT) {
		if (granaryResources[resource] > 0) {
			return 1;
		}
	}
	return 0;
}

// 0 = getting resource, >0 = resource to deliver
int Resource_determineWarehouseWorkerTask(building *warehouse, int *resource)
{
	int pctWorkers = calc_percentage(warehouse->numWorkers, model_get_building(warehouse->type)->laborers);
	if (pctWorkers < 50) {
		return -1;
	}
	const building_storage *s = building_storage_get(warehouse->storage_id);
	building *space;
	// get resources
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		if (s->resource_state[r] != BUILDING_STORAGE_STATE_GETTING || Data_CityInfo.resourceStockpiled[r]) {
			continue;
		}
		int loadsStored = 0;
		space = warehouse;
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (space->id > 0 && space->loadsStored > 0) {
				if (space->subtype.warehouseResourceId == r) {
					loadsStored += space->loadsStored;
				}
			}
		}
		int room = 0;
		space = warehouse;
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (space->id > 0) {
				if (space->loadsStored <= 0) {
					room += 4;
				}
				if (space->subtype.warehouseResourceId == r) {
					room += 4 - space->loadsStored;
				}
			}
		}
		if (room >= 8 && loadsStored <= 4 && Data_CityInfo.resourceStored[r] - loadsStored > 4) {
			*resource = r;
			return StorageFigureTask_Getting;
		}
	}
	// deliver weapons to barracks
	if (building_count_active(BUILDING_BARRACKS) > 0 && Data_CityInfo.militaryLegionaryLegions > 0 &&
		!Data_CityInfo.resourceStockpiled[RESOURCE_WEAPONS]) {
		building *barracks = building_get(Data_CityInfo.buildingBarracksBuildingId);
		if (barracks->loadsStored < 4 &&
			    warehouse->roadNetworkId == barracks->roadNetworkId) {
			space = warehouse;
			for (int i = 0; i < 8; i++) {
				space = building_next(space);
				if (space->id > 0 && space->loadsStored > 0 &&
					space->subtype.warehouseResourceId == RESOURCE_WEAPONS) {
					return RESOURCE_WEAPONS;
				}
			}
		}
	}
	// deliver raw materials to workshops
	space = warehouse;
	for (int i = 0; i < 8; i++) {
		space = building_next(space);
		if (space->id > 0 && space->loadsStored > 0) {
			int resource = space->subtype.warehouseResourceId;
			if (!Data_CityInfo.resourceStockpiled[resource]) {
				int workshopType;
				switch (resource) {
					case RESOURCE_OLIVES:
						workshopType = WORKSHOP_OLIVES_TO_OIL;
						break;
					case RESOURCE_VINES:
						workshopType = WORKSHOP_VINES_TO_WINE;
						break;
					case RESOURCE_IRON:
						workshopType = WORKSHOP_IRON_TO_WEAPONS;
						break;
					case RESOURCE_TIMBER:
						workshopType = WORKSHOP_TIMBER_TO_FURNITURE;
						break;
					case RESOURCE_CLAY:
						workshopType = WORKSHOP_CLAY_TO_POTTERY;
						break;
					default:
						workshopType = 0;
						break;
				}
				if (workshopType > 0 &&
						Data_CityInfo.resourceWorkshopRawMaterialSpace[workshopType] > 0) {
					return resource;
				}
			}
		}
	}
	// deliver food to getting granary
	if (determineGranaryGetFoods()) {
		space = warehouse;
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (storesNonStockpiledFood(space, granaryGettingResource)) {
				return space->subtype.warehouseResourceId;
			}
		}
	}
	// deliver food to accepting granary
	if (determineGranaryAcceptFoods() && !scenario_property_rome_supplies_wheat()) {
		space = warehouse;
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (storesNonStockpiledFood(space, granaryAcceptingResource)) {
				return space->subtype.warehouseResourceId;
			}
		}
	}
	// move goods to other warehouses
	if (s->empty_all) {
		space = warehouse;
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (space->id > 0 && space->loadsStored > 0) {
				return space->subtype.warehouseResourceId;
			}
		}
	}
	return -1;
}
