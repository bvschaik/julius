#include "granary.h"

#include "building/model.h"
#include "building/storage.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

#define MAX_GRANARIES 100

static struct {
    int buildingIds[MAX_GRANARIES];
    int numItems;
    int totalStorageWheat;
    int totalStorageVegetables;
    int totalStorageFruit;
    int totalStorageMeat;
} nonGettingGranaries;

int building_granary_get_amount(building *granary, int resource)
{
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (granary->type != BUILDING_GRANARY) {
        return 0;
    }
    return granary->data.storage.resourceStored[resource];
}

int building_granary_add_resource(building *granary, int resource, int is_produced)
{
    if (granary->id <= 0) {
        return 1;
    }
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (granary->type != BUILDING_GRANARY) {
        return 0;
    }
    if (granary->data.storage.resourceStored[RESOURCE_NONE] <= 0) {
        return 0; // no space
    }
    if (is_produced) {
        Data_CityInfo.foodInfoFoodStoredSoFarThisMonth += 100;
    }
    if (granary->data.storage.resourceStored[RESOURCE_NONE] <= 100) {
        granary->data.storage.resourceStored[resource] += granary->data.storage.resourceStored[RESOURCE_NONE];
        granary->data.storage.resourceStored[RESOURCE_NONE] = 0;
    } else {
        granary->data.storage.resourceStored[resource] += 100;
        granary->data.storage.resourceStored[RESOURCE_NONE] -= 100;
    }
    return 1;
}

int building_granary_remove_resource(building *granary, int resource, int amount)
{
    if (amount <= 0) {
        return 0;
    }
    int to_remove;
    if (granary->data.storage.resourceStored[resource] >= amount) {
        Data_CityInfo.resourceGranaryFoodStored[resource] -= amount;
        granary->data.storage.resourceStored[resource] -= amount;
        granary->data.storage.resourceStored[RESOURCE_NONE] += amount;
        to_remove = 0;
    } else {
        int removed = granary->data.storage.resourceStored[resource];
        Data_CityInfo.resourceGranaryFoodStored[resource] -= removed;
        granary->data.storage.resourceStored[resource] = 0;
        granary->data.storage.resourceStored[RESOURCE_NONE] += removed;
        to_remove = amount - removed;
    }
    return to_remove;
}

int building_granary_determine_worker_task(building *granary)
{
    int pct_workers = calc_percentage(granary->numWorkers, model_get_building(granary->type)->laborers);
    if (pct_workers < 50) {
        return GRANARY_TASK_NONE;
    }
    const building_storage *s = building_storage_get(granary->storage_id);
    if (s->empty_all) {
        // bring food to another granary
        for (int i = RESOURCE_MIN_FOOD; i < RESOURCE_MAX_FOOD; i++) {
            if (granary->data.storage.resourceStored[i]) {
                return i;
            }
        }
        return GRANARY_TASK_NONE;
    }
    if (granary->data.storage.resourceStored[RESOURCE_NONE] <= 0) {
        return GRANARY_TASK_NONE; // granary full, nothing to get
    }
    if (s->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageWheat > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageVegetables > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageFruit > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING && nonGettingGranaries.totalStorageMeat > 100) {
        return GRANARY_TASK_GETTING;
    }
    return GRANARY_TASK_NONE;
}

void building_granaries_calculate_stocks()
{
    nonGettingGranaries.numItems = 0;
    for (int i = 0; i < MAX_GRANARIES; i++) {
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
        if (totalNonGetting > MAX_GRANARIES) {
            nonGettingGranaries.buildingIds[nonGettingGranaries.numItems] = i;
            if (nonGettingGranaries.numItems < MAX_GRANARIES - 2) {
                nonGettingGranaries.numItems++;
            }
        }
    }
}

int building_granary_for_getting(building *src, int *x_dst, int *y_dst)
{
    const building_storage *sSrc = building_storage_get(src->storage_id);
    if (sSrc->empty_all) {
        return 0;
    }
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    int is_getting = 0;
    if (sSrc->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING ||
        sSrc->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING ||
        sSrc->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING ||
        sSrc->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING) {
        is_getting = 1;
    }
    if (is_getting <= 0) {
        return 0;
    }

    int minDist = 10000;
    int minBuildingId = 0;
    for (int i = 0; i < nonGettingGranaries.numItems; i++) {
        int buildingId = nonGettingGranaries.buildingIds[i];
        building *b = building_get(buildingId);
        if (b->roadNetworkId != src->roadNetworkId) {
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
            int dist = calc_distance_with_penalty(
                b->x + 1, b->y + 1,
                           src->x + 1, src->y + 1,
                           src->distanceFromEntry, b->distanceFromEntry);
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
    *x_dst = min->x + 1;
    *y_dst = min->y + 1;
    return minBuildingId;
}
