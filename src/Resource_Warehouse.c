#include "Resource.h"

#include "Building.h"
#include "Terrain.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/count.h"
#include "building/model.h"
#include "building/storage.h"
#include "city/finance.h"
#include "core/calc.h"
#include "empire/trade_prices.h"
#include "graphics/image.h"
#include "game/tutorial.h"
#include "map/image.h"
#include "scenario/property.h"

static int granaryGettingResource[7];
static int granaryAcceptingResource[7];

void Resource_setWarehouseSpaceGraphic(building *space, int resource)
{
	int image_id;
	if (space->loadsStored <= 0) {
		image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
	} else {
		image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) +
			4 * (resource - 1) + Resource_getGraphicIdOffset(resource, 0) +
			       space->loadsStored - 1;
	}
	map_image_set(space->gridOffset, image_id);
}

void Resource_addToCityWarehouses(int resource, int amount)
{
	int buildingId = Data_CityInfo.resourceLastTargetWarehouse;
	for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
		buildingId++;
		if (buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		building *b = building_get(buildingId);
		if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
			Data_CityInfo.resourceLastTargetWarehouse = buildingId;
			while (amount && Resource_addToWarehouse(buildingId, resource)) {
				amount--;
			}
		}
	}
}

int Resource_removeFromCityWarehouses(int resource, int amount)
{
	int amountLeft = amount;
	int buildingId = Data_CityInfo.resourceLastTargetWarehouse;
	// first go for non-getting warehouses
	for (int i = 1; i < MAX_BUILDINGS && amountLeft > 0; i++) {
		buildingId++;
		if (buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		building *b = building_get(buildingId);
		if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
			if (building_storage_get(b->storage_id)->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
				Data_CityInfo.resourceLastTargetWarehouse = buildingId;
				amountLeft = Resource_removeFromWarehouse(buildingId, resource, amountLeft);
			}
		}
	}
	// if that doesn't work, take it anyway
	for (int i = 1; i < MAX_BUILDINGS && amountLeft > 0; i++) {
		buildingId++;
		if (buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		building *b = building_get(buildingId);
		if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
			Data_CityInfo.resourceLastTargetWarehouse = buildingId;
			amountLeft = Resource_removeFromWarehouse(buildingId, resource, amountLeft);
		}
	}
	return amount - amountLeft;
}

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
		int main_building_id = Building_getMainBuildingId(i);
		if (srcBuildingId == main_building_id) {
			continue;
		}
		building *dst = building_get(main_building_id);
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
	int resultBuildingId = Building_getMainBuildingId(minBuildingId);
	building *b = building_get(resultBuildingId);
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
	int minBuildingId = 0;
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
				minBuildingId = i;
			}
		}
	}
	if (minBuildingId > 0) {
        building *min = building_get(minBuildingId);
		*xDst = min->roadAccessX;
		*yDst = min->roadAccessY;
		return minBuildingId;
	} else {
		return 0;
	}
}

int Resource_addToWarehouse(int buildingId, int resource)
{
	if (buildingId <= 0) {
		return 0;
	}
	// check building itself
	int findSpace = 0;
	building *b = building_get(buildingId);
	if (b->subtype.warehouseResourceId && b->subtype.warehouseResourceId != resource) {
		findSpace = 1;
	} else if (b->loadsStored >= 4) {
		findSpace = 1;
	} else if (b->type == BUILDING_WAREHOUSE) {
		findSpace = 1;
	}
	if (findSpace) {
		int spaceFound = 0;
		buildingId = Building_getMainBuildingId(buildingId);
		building *space = building_get(buildingId);
		for (int i = 0; i < 8; i++) {
			space = building_next(space);
			if (!space->id) {
				return 0;
			}
			if (!space->subtype.warehouseResourceId || space->subtype.warehouseResourceId == resource) {
				if (space->loadsStored < 4) {
					spaceFound = 1;
                    b = space;
					break;
				}
			}
		}
		if (!spaceFound) {
			return 0;
		}
	}
	Data_CityInfo.resourceSpaceInWarehouses[resource]--;
	Data_CityInfo.resourceStored[resource]++;
	b->subtype.warehouseResourceId = resource;
	b->loadsStored++;
	tutorial_on_add_to_warehouse();
	Resource_setWarehouseSpaceGraphic(b, resource);
	return 1;
}

int Resource_removeFromWarehouse(int buildingId, int resource, int amount)
{
	// returns amount still needing removal
	building *warehouse = building_get(buildingId);
	if (warehouse->type != BUILDING_WAREHOUSE) {
		return amount;
	}
	building *space = warehouse;
	for (int i = 0; i < 8; i++) {
		if (amount <= 0) {
			return 0;
		}
		space = building_next(space);
		if (space->id <= 0) {
			continue;
		}
		if (space->subtype.warehouseResourceId != resource || space->loadsStored <= 0) {
			continue;
		}
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
		Resource_setWarehouseSpaceGraphic(space, resource);
	}
	return amount;
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
		Resource_setWarehouseSpaceGraphic(space, resource);
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
	
	Resource_setWarehouseSpaceGraphic(space, resourceId);
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
	
	Resource_setWarehouseSpaceGraphic(space, resourceId);
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
