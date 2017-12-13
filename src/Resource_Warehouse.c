#include "Resource.h"

#include "Building.h"
#include "core/calc.h"
#include "Terrain.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Grid.h"

#include "building/count.h"
#include "building/model.h"
#include "building/storage.h"
#include "city/finance.h"
#include "empire/trade_prices.h"
#include "graphics/image.h"
#include "game/tutorial.h"
#include "map/image.h"
#include "scenario/property.h"

static int granaryGettingResource[7];
static int granaryAcceptingResource[7];

void Resource_setWarehouseSpaceGraphic(int spaceId, int resource)
{
	int graphicId;
	if (Data_Buildings[spaceId].loadsStored <= 0) {
		graphicId = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
	} else {
		graphicId = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) +
			4 * (resource - 1) + Resource_getGraphicIdOffset(resource, 0) +
			Data_Buildings[spaceId].loadsStored - 1;
	}
	map_image_set(Data_Buildings[spaceId].gridOffset, graphicId);
}

void Resource_addToCityWarehouses(int resource, int amount)
{
	int buildingId = Data_CityInfo.resourceLastTargetWarehouse;
	for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
		buildingId++;
		if (buildingId >= MAX_BUILDINGS) {
			buildingId = 1;
		}
		if (BuildingIsInUse(buildingId) &&
			Data_Buildings[buildingId].type == BUILDING_WAREHOUSE) {
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
		if (BuildingIsInUse(buildingId) &&
			Data_Buildings[buildingId].type == BUILDING_WAREHOUSE) {
			int storageId = Data_Buildings[buildingId].storage_id;
			if (building_storage_get(storageId)->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
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
		if (BuildingIsInUse(buildingId) &&
			Data_Buildings[buildingId].type == BUILDING_WAREHOUSE) {
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
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_WAREHOUSE_SPACE) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int dstBuildingId = Building_getMainBuildingId(i);
		if (srcBuildingId == dstBuildingId) {
			continue;
		}
		const building_storage *s = building_storage_get(Data_Buildings[dstBuildingId].storage_id);
		if (s->resource_state[resource] == BUILDING_STORAGE_STATE_NOT_ACCEPTING || s->empty_all) {
			continue;
		}
		int pctWorkers = calc_percentage(
			Data_Buildings[dstBuildingId].numWorkers,
			model_get_building(Data_Buildings[dstBuildingId].type)->laborers);
		if (pctWorkers < 100) {
			if (understaffed) {
				*understaffed += 1;
			}
			continue;
		}
		int dist;
		if (b->subtype.warehouseResourceId == RESOURCE_NONE) { // empty warehouse space
			dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
		} else if (b->subtype.warehouseResourceId == resource && b->loadsStored < 4) {
			dist = Resource_getDistance(b->x, b->y, x, y, distanceFromEntry, b->distanceFromEntry);
		} else {
			dist = 0;
		}
		if (dist > 0 && dist < minDist) {
			minDist = dist;
			minBuildingId = i;
		}
	}
	int resultBuildingId = Building_getMainBuildingId(minBuildingId);
	struct Data_Building *b = &Data_Buildings[resultBuildingId];
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
	struct Data_Building *bSrc = &Data_Buildings[srcBuildingId];
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_WAREHOUSE) {
			continue;
		}
		if (i == srcBuildingId) {
			continue;
		}
		int loadsStored = 0;
		int spaceId = i;
		const building_storage *s = building_storage_get(b->storage_id);
		for (int t = 0; t < 8; t++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0) {
				if (Data_Buildings[spaceId].subtype.warehouseResourceId == resource) {
					loadsStored += Data_Buildings[spaceId].loadsStored;
				}
			}
		}
		if (loadsStored > 0 && s->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
			int dist = Resource_getDistance(b->x, b->y, bSrc->x, bSrc->y,
				bSrc->distanceFromEntry, b->distanceFromEntry);
			dist -= 4 * loadsStored;
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	if (minBuildingId > 0) {
		*xDst = Data_Buildings[minBuildingId].roadAccessX;
		*yDst = Data_Buildings[minBuildingId].roadAccessY;
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
	struct Data_Building *b = &Data_Buildings[buildingId];
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
		b = &Data_Buildings[buildingId];
		for (int i = 0; i < 8; i++) {
			buildingId = b->nextPartBuildingId;
			if (!buildingId) {
				return 0;
			}
			b = &Data_Buildings[buildingId];
			if (!b->subtype.warehouseResourceId || b->subtype.warehouseResourceId == resource) {
				if (b->loadsStored < 4) {
					spaceFound = 1;
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
	Resource_setWarehouseSpaceGraphic(buildingId, resource);
	return 1;
}

int Resource_removeFromWarehouse(int buildingId, int resource, int amount)
{
	// returns amount still needing removal
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != BUILDING_WAREHOUSE) {
		return amount;
	}
	for (int i = 0; i < 8; i++) {
		if (amount <= 0) {
			return 0;
		}
		buildingId = b->nextPartBuildingId;
		if (buildingId <= 0) {
			continue;
		}
		b = &Data_Buildings[buildingId];
		if (b->subtype.warehouseResourceId != resource || b->loadsStored <= 0) {
			continue;
		}
		if (b->loadsStored > amount) {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
			Data_CityInfo.resourceStored[resource] -= amount;
			b->loadsStored -= amount;
			amount = 0;
		} else {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += b->loadsStored;
			Data_CityInfo.resourceStored[resource] -= b->loadsStored;
			amount -= b->loadsStored;
			b->loadsStored = 0;
			b->subtype.warehouseResourceId = RESOURCE_NONE;
		}
		Resource_setWarehouseSpaceGraphic(buildingId, resource);
	}
	return amount;
}

void Resource_removeFromWarehouseForMercury(int buildingId, int amount)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != BUILDING_WAREHOUSE) {
		return;
	}
	for (int i = 0; i < 8 && amount > 0; i++) {
		buildingId = b->nextPartBuildingId;
		b = &Data_Buildings[buildingId];
		if (buildingId <= 0 || b->loadsStored <= 0) {
			continue;
		}
		int resource = b->subtype.warehouseResourceId;
		if (b->loadsStored > amount) {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
			Data_CityInfo.resourceStored[resource] -= amount;
			b->loadsStored -= amount;
			amount = 0;
		} else {
			Data_CityInfo.resourceSpaceInWarehouses[resource] += b->loadsStored;
			Data_CityInfo.resourceStored[resource] -= b->loadsStored;
			amount -= b->loadsStored;
			b->loadsStored = 0;
			b->subtype.warehouseResourceId = RESOURCE_NONE;
		}
		Resource_setWarehouseSpaceGraphic(buildingId, resource);
	}
}

int Resource_getAmountStoredInWarehouse(int buildingId, int resource)
{
	int loads = 0;
	for (int i = 0; i < 8; i++) {
		buildingId = Data_Buildings[buildingId].nextPartBuildingId;
		if (buildingId <= 0) {
			return 0;
		}
		if (Data_Buildings[buildingId].subtype.warehouseResourceId &&
			Data_Buildings[buildingId].subtype.warehouseResourceId == resource) {
			loads += Data_Buildings[buildingId].loadsStored;
		}
	}
	return loads;
}

int Resource_getWarehouseSpaceInfo(int buildingId)
{
	int totalLoads = 0;
	int emptySpaces = 0;
	for (int i = 0; i < 8; i++) {
		buildingId = Data_Buildings[buildingId].nextPartBuildingId;
		if (buildingId <= 0) {
			return 0;
		}
		if (Data_Buildings[buildingId].subtype.warehouseResourceId) {
			totalLoads += Data_Buildings[buildingId].loadsStored;
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

void Resource_addImportedResourceToWarehouseSpace(int spaceId, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]--;
	Data_CityInfo.resourceStored[resourceId]++;
	Data_Buildings[spaceId].loadsStored++;
	Data_Buildings[spaceId].subtype.warehouseResourceId = resourceId;
	
    int price = trade_price_buy(resourceId);
    city_finance_process_import(price);
	
	Resource_setWarehouseSpaceGraphic(spaceId, resourceId);
}

void Resource_removeExportedResourceFromWarehouseSpace(int spaceId, int resourceId)
{
	Data_CityInfo.resourceSpaceInWarehouses[resourceId]++;
	Data_CityInfo.resourceStored[resourceId]--;
	Data_Buildings[spaceId].loadsStored--;
	if (Data_Buildings[spaceId].loadsStored <= 0) {
		Data_Buildings[spaceId].subtype.warehouseResourceId = RESOURCE_NONE;
	}
	
	int price = trade_price_sell(resourceId);
    city_finance_process_export(price);
	
	Resource_setWarehouseSpaceGraphic(spaceId, resourceId);
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
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
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
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
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

static int storesNonStockpiledFood(int spaceId, int *granaryResources)
{
	if (spaceId <= 0) {
		return 0;
	}
	if (Data_Buildings[spaceId].loadsStored <= 0) {
		return 0;
	}
	int resource = Data_Buildings[spaceId].subtype.warehouseResourceId;
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
int Resource_determineWarehouseWorkerTask(int buildingId, int *resource)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
	if (pctWorkers < 50) {
		return -1;
	}
	const building_storage *s = building_storage_get(b->storage_id);
	int spaceId;
	// get resources
	for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
		if (s->resource_state[r] != BUILDING_STORAGE_STATE_GETTING || Data_CityInfo.resourceStockpiled[r]) {
			continue;
		}
		int loadsStored = 0;
		spaceId = buildingId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0) {
				if (Data_Buildings[spaceId].subtype.warehouseResourceId == r) {
					loadsStored += Data_Buildings[spaceId].loadsStored;
				}
			}
		}
		int room = 0;
		spaceId = buildingId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0) {
				if (Data_Buildings[spaceId].loadsStored <= 0) {
					room += 4;
				}
				if (Data_Buildings[spaceId].subtype.warehouseResourceId == r) {
					room += 4 - Data_Buildings[spaceId].loadsStored;
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
		int barracksId = Data_CityInfo.buildingBarracksBuildingId;
		if (Data_Buildings[barracksId].loadsStored < 4 &&
			b->roadNetworkId == Data_Buildings[barracksId].roadNetworkId) {
			spaceId = buildingId;
			for (int i = 0; i < 8; i++) {
				spaceId = Data_Buildings[spaceId].nextPartBuildingId;
				if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0 &&
					Data_Buildings[spaceId].subtype.warehouseResourceId == RESOURCE_WEAPONS) {
					return RESOURCE_WEAPONS;
				}
			}
		}
	}
	// deliver raw materials to workshops
	spaceId = buildingId;
	for (int i = 0; i < 8; i++) {
		spaceId = Data_Buildings[spaceId].nextPartBuildingId;
		if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0) {
			int resource = Data_Buildings[spaceId].subtype.warehouseResourceId;
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
		spaceId = buildingId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (storesNonStockpiledFood(spaceId, granaryGettingResource)) {
				return Data_Buildings[spaceId].subtype.warehouseResourceId;
			}
		}
	}
	// deliver food to accepting granary
	if (determineGranaryAcceptFoods() && !scenario_property_rome_supplies_wheat()) {
		spaceId = buildingId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (storesNonStockpiledFood(spaceId, granaryAcceptingResource)) {
				return Data_Buildings[spaceId].subtype.warehouseResourceId;
			}
		}
	}
	// move goods to other warehouses
	if (s->empty_all) {
		spaceId = buildingId;
		for (int i = 0; i < 8; i++) {
			spaceId = Data_Buildings[spaceId].nextPartBuildingId;
			if (spaceId > 0 && Data_Buildings[spaceId].loadsStored > 0) {
				return Data_Buildings[spaceId].subtype.warehouseResourceId;
			}
		}
	}
	return -1;
}
