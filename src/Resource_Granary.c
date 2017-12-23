#include "Resource.h"

#include "Data/CityInfo.h"

#include "building/building.h"
#include "building/granary.h"
#include "building/model.h"
#include "building/storage.h"
#include "core/calc.h"
#include "scenario/property.h"

int Resource_getGranaryForStoringFood(
	int forceOnStockpile, int x, int y, int resource, int distanceFromEntry, int roadNetworkId,
	int *understaffed, int *xDst, int *yDst)
{
	if (scenario_property_rome_supplies_wheat()) {
		return 0;
	}
	if (!resource_is_food(resource)) {
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
			int dist = calc_distance_with_penalty(b->x + 1, b->y + 1, x, y, distanceFromEntry, b->distanceFromEntry);
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
	if (!resource_is_food(resource)) {
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
			int dist = calc_distance_with_penalty(b->x + 1, b->y + 1, x, y, distanceFromEntry, b->distanceFromEntry);
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
	building_granary_remove_resource(bSrc, maxResource, maxAmount);
	*resource = maxResource;
	return maxAmount / 100;
}
