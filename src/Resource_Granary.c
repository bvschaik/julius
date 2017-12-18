#include "Resource.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/model.h"
#include "building/storage.h"
#include "core/calc.h"
#include "scenario/property.h"

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
		case RESOURCE_WHEAT:
		case RESOURCE_VEGETABLES:
		case RESOURCE_FRUIT:
		case RESOURCE_MEAT:
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
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
			continue;
		}
		if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		int totalNonGetting = 0;
		if (s->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[RESOURCE_WHEAT];
			nonGettingGranaries.totalStorageWheat += b->data.storage.resourceStored[RESOURCE_WHEAT];
		}
		if (s->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
			nonGettingGranaries.totalStorageVegetables += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
		}
		if (s->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[RESOURCE_FRUIT];
			nonGettingGranaries.totalStorageFruit += b->data.storage.resourceStored[RESOURCE_FRUIT];
		}
		if (s->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
			totalNonGetting += b->data.storage.resourceStored[RESOURCE_MEAT];
			nonGettingGranaries.totalStorageMeat += b->data.storage.resourceStored[RESOURCE_MEAT];
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
	if (scenario_property_rome_supplies_wheat()) {
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
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
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
	building *min = building_get(minBuildingId);
	*xDst = min->x + 1;
	*yDst = min->y + 1;
	return minBuildingId;
}

int Resource_getGettingGranaryForStoringFood(
	int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *xDst, int *yDst)
{
	if (scenario_property_rome_supplies_wheat()) {
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
		building *b = building_get(i);
		if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
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
		if (b->data.storage.resourceStored[RESOURCE_NONE] > 100) {
			// there is room
			int dist = Resource_getDistance(b->x + 1, b->y + 1, x, y, distanceFromEntry, b->distanceFromEntry);
			if (dist < minDist) {
				minDist = dist;
				minBuildingId = i;
			}
		}
	}
	building *min = building_get(minBuildingId);
	*xDst = min->x + 1;
	*yDst = min->y + 1;
	return minBuildingId;
}

int Resource_getGranaryForGettingFood(int srcBuildingId, int *xDst, int *yDst)
{
	building *bSrc = building_get(srcBuildingId);
	const building_storage *sSrc = building_storage_get(bSrc->storage_id);
	if (sSrc->empty_all) {
		return 0;
	}
	if (scenario_property_rome_supplies_wheat()) {
		return 0;
	}
	int numGetting = 0;
	if (sSrc->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (sSrc->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING) {
		numGetting++;
	}
	if (numGetting <= 0) {
		return 0;
	}

	int minDist = 10000;
	int minBuildingId = 0;
	for (int i = 0; i < nonGettingGranaries.numItems; i++) {
		int buildingId = nonGettingGranaries.buildingIds[i];
		building *b = building_get(buildingId);
		if (b->roadNetworkId != bSrc->roadNetworkId) {
			continue;
		}
		const building_storage *s = building_storage_get(b->storage_id);
		int amountGettable = 0;
		if (sSrc->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[RESOURCE_WHEAT];
		}
		if (sSrc->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
		}
		if (sSrc->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[RESOURCE_FRUIT];
		}
		if (sSrc->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING &&
			s->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
			amountGettable += b->data.storage.resourceStored[RESOURCE_MEAT];
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
	building *min = building_get(minBuildingId);
	*xDst = min->x + 1;
	*yDst = min->y + 1;
	return minBuildingId;
}

int Resource_getAmountStoredInGranary(building *granary, int resource)
{
	if (!isFood(resource)) {
		return 0;
	}
	if (granary->type != BUILDING_GRANARY) {
		return 0;
	}
	return granary->data.storage.resourceStored[resource];
}

int Resource_addToGranary(int buildingId, int resource, int countAsProduced)
{
	if (buildingId <= 0) {
		return 1;
	}
	if (!isFood(resource)) {
		return 0;
	}
	building *b = building_get(buildingId);
	if (b->type != BUILDING_GRANARY) {
		return 0;
	}
	if (b->data.storage.resourceStored[RESOURCE_NONE] <= 0) {
		return 0; // no space
	}
	if (countAsProduced) {
		Data_CityInfo.foodInfoFoodStoredSoFarThisMonth += 100;
	}
	if (b->data.storage.resourceStored[RESOURCE_NONE] <= 100) {
		b->data.storage.resourceStored[resource] += b->data.storage.resourceStored[RESOURCE_NONE];
		b->data.storage.resourceStored[RESOURCE_NONE] = 0;
	} else {
		b->data.storage.resourceStored[resource] += 100;
		b->data.storage.resourceStored[RESOURCE_NONE] -= 100;
	}
	return 1;
}

int Resource_removeFromGranary(int buildingId, int resource, int amount)
{
	if (amount <= 0) {
		return 0;
	}
	int toRemove;
	building *b = building_get(buildingId);
	if (b->data.storage.resourceStored[resource] >= amount) {
		Data_CityInfo.resourceGranaryFoodStored[resource] -= amount;
		b->data.storage.resourceStored[resource] -= amount;
		b->data.storage.resourceStored[RESOURCE_NONE] += amount;
		toRemove = 0;
	} else {
		int removed = b->data.storage.resourceStored[resource];
		Data_CityInfo.resourceGranaryFoodStored[resource] -= removed;
		b->data.storage.resourceStored[resource] = 0;
		b->data.storage.resourceStored[RESOURCE_NONE] += removed;
		toRemove = amount - removed;
	}
	return toRemove;
}

int Resource_determineGranaryWorkerTask(building *granary)
{
	int pctWorkers = calc_percentage(granary->numWorkers, model_get_building(granary->type)->laborers);
	if (pctWorkers < 50) {
		return -1;
	}
	const building_storage *s = building_storage_get(granary->storage_id);
	if (s->empty_all) {
		// bring food to another granary
		for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
			if (granary->data.storage.resourceStored[i]) {
				return i;
			}
		}
		return -1;
	}
	if (granary->data.storage.resourceStored[RESOURCE_NONE] <= 0) {
		return -1; // granary full, nothing to get
	}
	if (s->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageWheat > 100) {
		return 0;
	}
	if (s->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageVegetables > 100) {
		return 0;
	}
	if (s->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageFruit > 100) {
		return 0;
	}
	if (s->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageMeat > 100) {
		return 0;
	}
	return -1;
}

int Resource_takeFoodFromGranaryForGettingDeliveryman(int dstBuildingId, int srcBuildingId, int *resource)
{
	building *bSrc = building_get(srcBuildingId);
	const building_storage *sSrc = building_storage_get(bSrc->storage_id);
	building *bDst = building_get(dstBuildingId);
	const building_storage *sDst = building_storage_get(bDst->storage_id);
	
	int maxAmount = 0;
	int maxResource = 0;
	if (sDst->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[RESOURCE_WHEAT] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[RESOURCE_WHEAT];
			maxResource = RESOURCE_WHEAT;
		}
	}
	if (sDst->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[RESOURCE_VEGETABLES] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[RESOURCE_VEGETABLES];
			maxResource = RESOURCE_VEGETABLES;
		}
	}
	if (sDst->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[RESOURCE_FRUIT] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[RESOURCE_FRUIT];
			maxResource = RESOURCE_FRUIT;
		}
	}
	if (sDst->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING &&
		sSrc->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
		if (bSrc->data.storage.resourceStored[RESOURCE_MEAT] > maxAmount) {
			maxAmount = bSrc->data.storage.resourceStored[RESOURCE_MEAT];
			maxResource = RESOURCE_MEAT;
		}
	}
	
	if (maxAmount > 800) {
		maxAmount = 800;
	}
	if (maxAmount > bDst->data.storage.resourceStored[RESOURCE_NONE]) {
		maxAmount = bDst->data.storage.resourceStored[RESOURCE_NONE];
	}
	Resource_removeFromGranary(srcBuildingId, maxResource, maxAmount);
	*resource = maxResource;
	return maxAmount / 100;
}
