#include "Resource.h"

#include "core/calc.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"

#include "building/model.h"
#include "building/storage.h"

static struct {
	int buildingIds[100];
	int numItems;
	int totalStorageWheat;
	int totalStorageVegetables;
	int totalStorageFruit;
	int totalStorageMeat;
} nonGettingGranaries;

static int isFood(int resource) {
	switch (resource) {
		case Resource_Wheat:
		case Resource_Vegetables:
		case Resource_Fruit:
		case Resource_Meat:
			return 1;
		default:
			return 0;
	}
}

void Resource_gatherGranaryGettingInfo()
{
	nonGettingGranaries.numItems = 0;
	for (int i = 0; i < 100; i++) {
		nonGettingGranaries.buildingIds[i] = 0;
	}
	nonGettingGranaries.totalStorageWheat = 0;
	nonGettingGranaries.totalStorageVegetables = 0;
	nonGettingGranaries.totalStorageFruit = 0;
	nonGettingGranaries.totalStorageMeat = 0;

	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		int totalNonGetting = 0;
		if (s->resource_state[Resource_Wheat] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Wheat];
			nonGettingGranaries.totalStorageWheat += b->data.storage.resourceStored[Resource_Wheat];
		}
		if (s->resource_state[Resource_Vegetables] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Vegetables];
			nonGettingGranaries.totalStorageVegetables += b->data.storage.resourceStored[Resource_Vegetables];
		}
		if (s->resource_state[Resource_Fruit] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Fruit];
			nonGettingGranaries.totalStorageFruit += b->data.storage.resourceStored[Resource_Fruit];
		}
		if (s->resource_state[Resource_Meat] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[Resource_Meat];
			nonGettingGranaries.totalStorageMeat += b->data.storage.resourceStored[Resource_Meat];
		}
		if (totalNonGetting > 100) {
			nonGettingGranaries.buildingIds[nonGettingGranaries.numItems] = i;
			if (nonGettingGranaries.numItems < 98) {
				nonGettingGranaries.numItems++;
			}
		}
	}
}

int Resource_getGranaryForStoringFood(
	int forceOnStockpile, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst)
{
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	if (Data_CityInfo.resourceStockpiled[resource] && !forceOnStockpile) {
		return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
		if (pctWorkers < 100) {
			if (understaffed) {
				*understaffed += 1;
			}
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		if (s->resource_state[resource] == BUILDING_STORAGE_STATE_NOT_ACCEPTING || s->empty_all) {
			continue;
		}
		if (b->data.storage.resourceStored[RESOURCE_NONE] >= 100) {
			// there is room
			int dist = Resource_getDistance(b->x + 1, b->y + 1, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	// deliver to center of granary
	*xDst = Data_Buildings[minBuildingId].x + 1;
	*yDst = Data_Buildings[minBuildingId].y + 1;
	return minBuildingId;
}

int Resource_getGettingGranaryForStoringFood(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst)
{
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	if (Data_CityInfo.resourceStockpiled[resource]) {
		return 0;
	}
	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (!BuildingIsInUse(i) || b->type != BUILDING_GRANARY) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != roadNetworkId) {
			continue;
		}
		int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
		if (pctWorkers < 100) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		if (s->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING || s->empty_all) {
			continue;
		}
		if (b->data.storage.resourceStored[Resource_None] > 100) {
			// there is room
			int dist = Resource_getDistance(b->x + 1, b->y + 1, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].x + 1;
	*yDst = Data_Buildings[minBuildingId].y + 1;
	return minBuildingId;
}

int Resource_getGranaryForGettingFood(int srcBuildingId, int *xDst, int *yDst)
{
	struct Data_Building *bSrc = &Data_Buildings[srcBuildingId];
	const building_storage *sSrc = building_storage_get(bSrc->storage_id);
	if (sSrc->empty_all) {
		return 0;
	}
	if (Data_Scenario.romeSuppliesWheat) {
		return 0;
	}
	int numGetting = 0;
	if (sSrc->resource_state[Resource_Wheat] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[Resource_Vegetables] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[Resource_Fruit] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[Resource_Meat] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (numGetting <= 0) {
		return 0;
	}

	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 0; i < nonGettingGranaries.numItems; i++) {
		int buildingId = nonGettingGranaries.buildingIds[i];
		struct Data_Building *b = &Data_Buildings[buildingId];
		if (b->roadNetworkId != bSrc->roadNetworkId) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		int amountGettable = 0;
		if (sSrc->resource_state[Resource_Wheat] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[Resource_Wheat] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[Resource_Wheat];
		}
		if (sSrc->resource_state[Resource_Vegetables] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[Resource_Vegetables] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[Resource_Vegetables];
		}
		if (sSrc->resource_state[Resource_Fruit] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[Resource_Fruit] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[Resource_Fruit];
		}
		if (sSrc->resource_state[Resource_Meat] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[Resource_Meat] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[Resource_Meat];
		}
		if (amountGettable > 0) {
			int dist = Resource_getDistance(
				b->x + 1, b->y + 1,
				bSrc->x + 1, bSrc->y + 1,
				bSrc->distanceFromEntry, b->distanceFromEntry);
			if (amountGettable <= 400) {
				dist *= 2; // penalty for less food
			}
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = buildingId;
			}
		}
	}
	*xDst = Data_Buildings[minBuildingId].x + 1;
	*yDst = Data_Buildings[minBuildingId].y + 1;
	return minBuildingId;
}

int Resource_getAmountStoredInGranary(int buildingId, int resource)
{
	if (buildingId <= 0) {
		return 0;
	}
	if (!isFood(resource)) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != BUILDING_GRANARY) {
		return 0;
	}
	return b->data.storage.resourceStored[resource];
}

int Resource_addToGranary(int buildingId, int resource, int countAsProduced)
{
	if (buildingId <= 0) {
		return 1;
	}
	if (!isFood(resource)) {
		return 0;
	}
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->type != BUILDING_GRANARY) {
		return 0;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 0) {
		return 0; // no space
	}
	if (countAsProduced) {
		Data_CityInfo.foodInfoFoodStoredSoFarThisMonth += 100;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 100) {
		b->data.storage.resourceStored[resource] += b->data.storage.resourceStored[Resource_None];
		b->data.storage.resourceStored[Resource_None] = 0;
	} else {
		b->data.storage.resourceStored[resource] += 100;
		b->data.storage.resourceStored[Resource_None] -= 100;
	}
	return 1;
}

int Resource_removeFromGranary(int buildingId, int resource, int amount)
{
	if (amount <= 0) {
		return 0;
	}
	int toRemove;
	struct Data_Building *b = &Data_Buildings[buildingId];
	if (b->data.storage.resourceStored[resource] >= amount) {
		Data_CityInfo.resourceGranaryFoodStored[resource] -= amount;
		b->data.storage.resourceStored[resource] -= amount;
		b->data.storage.resourceStored[Resource_None] += amount;
		toRemove = 0;
	} else {
		int removed = b->data.storage.resourceStored[resource];
		Data_CityInfo.resourceGranaryFoodStored[resource] -= removed;
		b->data.storage.resourceStored[resource] = 0;
		b->data.storage.resourceStored[Resource_None] += removed;
		toRemove = amount - removed;
	}
	return toRemove;
}

int Resource_determineGranaryWorkerTask(int buildingId)
{
	struct Data_Building *b = &Data_Buildings[buildingId];
	int pctWorkers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
	if (pctWorkers < 50) {
		return -1;
	}
	const building_storage *s = building_storage_get(b->storage_id);
	if (s->empty_all) {
		// bring food to another granary
		for (int i = Resource_MinFood; i < Resource_MaxFood; i++) {
			if (b->data.storage.resourceStored[i]) {
				return i;
			}
		}
		return -1;
	}
	if (b->data.storage.resourceStored[Resource_None] <= 0) {
		return -1; // granary full, nothing to get
	}
	if (s->resource_state[Resource_Wheat] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageWheat > 100) {
		return 0;
	}
	if (s->resource_state[Resource_Vegetables] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageVegetables > 100) {
		return 0;
	}
	if (s->resource_state[Resource_Fruit] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageFruit > 100) {
		return 0;
	}
	if (s->resource_state[Resource_Meat] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageMeat > 100) {
		return 0;
	}
	return -1;
}

int Resource_takeFoodFromGranaryForGettingDeliveryman(int dstBuildingId, int srcBuildingId, int *resource)
{
	struct Data_Building *bSrc = &Data_Buildings[srcBuildingId];
	const building_storage *sSrc = building_storage_get(bSrc->storage_id);
	struct Data_Building *bDst = &Data_Buildings[dstBuildingId];
	const building_storage *sDst = building_storage_get(bDst->storage_id);
	
	int maxAmount = 0;
	int maxResource = 0;
	if (sDst->resource_state[Resource_Wheat] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[Resource_Wheat] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[Resource_Wheat] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Wheat];
			maxResource = Resource_Wheat;
		}
	}
	if (sDst->resource_state[Resource_Vegetables] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[Resource_Vegetables] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[Resource_Vegetables] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Vegetables];
			maxResource = Resource_Vegetables;
		}
	}
	if (sDst->resource_state[Resource_Fruit] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[Resource_Fruit] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[Resource_Fruit] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Fruit];
			maxResource = Resource_Fruit;
		}
	}
	if (sDst->resource_state[Resource_Meat] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[Resource_Meat] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[Resource_Meat] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[Resource_Meat];
			maxResource = Resource_Meat;
		}
	}
	
	if (maxAmount > 800) {
		maxAmount = 800;
	}
	if (maxAmount > bDst->data.storage.resourceStored[Resource_None]) {
		maxAmount = bDst->data.storage.resourceStored[Resource_None];
	}
	Resource_removeFromGranary(srcBuildingId, maxResource, maxAmount);
	*resource = maxResource;
	return maxAmount / 100;
}
