#include "granary.h"

#include "building/model.h"
#include "building/storage.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

#define MAX_GRANARIES 100

static struct {
    int building_ids[MAX_GRANARIES];
    int num_items;
    int total_storage_wheat;
    int total_storage_vegetables;
    int total_storage_fruit;
    int total_storage_meat;
} non_getting_granaries;

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

int building_granary_remove_for_getting_deliveryman(building *src, building *dst, int *resource)
{
    const building_storage *s_src = building_storage_get(src->storage_id);
    const building_storage *s_dst = building_storage_get(dst->storage_id);
    
    int max_amount = 0;
    int max_resource = 0;
    if (s_dst->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING &&
            s_src->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
        if (src->data.storage.resourceStored[RESOURCE_WHEAT] > max_amount) {
            max_amount = src->data.storage.resourceStored[RESOURCE_WHEAT];
            max_resource = RESOURCE_WHEAT;
        }
    }
    if (s_dst->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING &&
            s_src->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
        if (src->data.storage.resourceStored[RESOURCE_VEGETABLES] > max_amount) {
            max_amount = src->data.storage.resourceStored[RESOURCE_VEGETABLES];
            max_resource = RESOURCE_VEGETABLES;
        }
    }
    if (s_dst->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING &&
            s_src->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
        if (src->data.storage.resourceStored[RESOURCE_FRUIT] > max_amount) {
            max_amount = src->data.storage.resourceStored[RESOURCE_FRUIT];
            max_resource = RESOURCE_FRUIT;
        }
    }
    if (s_dst->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING &&
            s_src->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
        if (src->data.storage.resourceStored[RESOURCE_MEAT] > max_amount) {
            max_amount = src->data.storage.resourceStored[RESOURCE_MEAT];
            max_resource = RESOURCE_MEAT;
        }
    }
    
    if (max_amount > 800) {
        max_amount = 800;
    }
    if (max_amount > dst->data.storage.resourceStored[RESOURCE_NONE]) {
        max_amount = dst->data.storage.resourceStored[RESOURCE_NONE];
    }
    building_granary_remove_resource(src, max_resource, max_amount);
    *resource = max_resource;
    return max_amount / 100;
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
    if (s->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING && non_getting_granaries.total_storage_wheat > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING && non_getting_granaries.total_storage_vegetables > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING && non_getting_granaries.total_storage_fruit > 100) {
        return GRANARY_TASK_GETTING;
    }
    if (s->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING && non_getting_granaries.total_storage_meat > 100) {
        return GRANARY_TASK_GETTING;
    }
    return GRANARY_TASK_NONE;
}

void building_granaries_calculate_stocks()
{
    non_getting_granaries.num_items = 0;
    for (int i = 0; i < MAX_GRANARIES; i++) {
        non_getting_granaries.building_ids[i] = 0;
    }
    non_getting_granaries.total_storage_wheat = 0;
    non_getting_granaries.total_storage_vegetables = 0;
    non_getting_granaries.total_storage_fruit = 0;
    non_getting_granaries.total_storage_meat = 0;

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        int total_non_getting = 0;
        if (s->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
            total_non_getting += b->data.storage.resourceStored[RESOURCE_WHEAT];
            non_getting_granaries.total_storage_wheat += b->data.storage.resourceStored[RESOURCE_WHEAT];
        }
        if (s->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
            total_non_getting += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
            non_getting_granaries.total_storage_vegetables += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
        }
        if (s->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
            total_non_getting += b->data.storage.resourceStored[RESOURCE_FRUIT];
            non_getting_granaries.total_storage_fruit += b->data.storage.resourceStored[RESOURCE_FRUIT];
        }
        if (s->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
            total_non_getting += b->data.storage.resourceStored[RESOURCE_MEAT];
            non_getting_granaries.total_storage_meat += b->data.storage.resourceStored[RESOURCE_MEAT];
        }
        if (total_non_getting > MAX_GRANARIES) {
            non_getting_granaries.building_ids[non_getting_granaries.num_items] = i;
            if (non_getting_granaries.num_items < MAX_GRANARIES - 2) {
                non_getting_granaries.num_items++;
            }
        }
    }
}

int building_granary_for_storing(int x, int y, int resource, int distance_from_entry, int road_network_id,
                                 int force_on_stockpile, int *understaffed, int *x_dst, int *y_dst)
{
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    if (!resource_is_food(resource)) {
        return 0;
    }
    if (Data_CityInfo.resourceStockpiled[resource] && !force_on_stockpile) {
        return 0;
    }
    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != road_network_id) {
            continue;
        }
        int pct_workers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
        if (pct_workers < 100) {
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
            int dist = calc_distance_with_penalty(b->x + 1, b->y + 1, x, y, distance_from_entry, b->distanceFromEntry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building_id = i;
            }
        }
    }
    // deliver to center of granary
    building *min = building_get(min_building_id);
    *x_dst = min->x + 1;
    *y_dst = min->y + 1;
    return min_building_id;
}

int building_getting_granary_for_storing(int x, int y, int resource, int distance_from_entry, int road_network_id,
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
    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != road_network_id) {
            continue;
        }
        int pct_workers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
        if (pct_workers < 100) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        if (s->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING || s->empty_all) {
            continue;
        }
        if (b->data.storage.resourceStored[RESOURCE_NONE] > 100) {
            // there is room
            int dist = calc_distance_with_penalty(b->x + 1, b->y + 1, x, y, distance_from_entry, b->distanceFromEntry);
            if (dist < min_dist) {
                min_dist = dist;
                min_building_id = i;
            }
        }
    }
    building *min = building_get(min_building_id);
    *xDst = min->x + 1;
    *yDst = min->y + 1;
    return min_building_id;
}

int building_granary_for_getting(building *src, int *x_dst, int *y_dst)
{
    const building_storage *s_src = building_storage_get(src->storage_id);
    if (s_src->empty_all) {
        return 0;
    }
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    int is_getting = 0;
    if (s_src->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING ||
            s_src->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING ||
            s_src->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING ||
            s_src->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING) {
        is_getting = 1;
    }
    if (is_getting <= 0) {
        return 0;
    }

    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 0; i < non_getting_granaries.num_items; i++) {
        building *b = building_get(non_getting_granaries.building_ids[i]);
        if (b->roadNetworkId != src->roadNetworkId) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        int amount_gettable = 0;
        if (s_src->resource_state[RESOURCE_WHEAT] == BUILDING_STORAGE_STATE_GETTING &&
            s->resource_state[RESOURCE_WHEAT] != BUILDING_STORAGE_STATE_GETTING) {
            amount_gettable += b->data.storage.resourceStored[RESOURCE_WHEAT];
        }
        if (s_src->resource_state[RESOURCE_VEGETABLES] == BUILDING_STORAGE_STATE_GETTING &&
            s->resource_state[RESOURCE_VEGETABLES] != BUILDING_STORAGE_STATE_GETTING) {
            amount_gettable += b->data.storage.resourceStored[RESOURCE_VEGETABLES];
        }
        if (s_src->resource_state[RESOURCE_FRUIT] == BUILDING_STORAGE_STATE_GETTING &&
            s->resource_state[RESOURCE_FRUIT] != BUILDING_STORAGE_STATE_GETTING) {
            amount_gettable += b->data.storage.resourceStored[RESOURCE_FRUIT];
        }
        if (s_src->resource_state[RESOURCE_MEAT] == BUILDING_STORAGE_STATE_GETTING &&
            s->resource_state[RESOURCE_MEAT] != BUILDING_STORAGE_STATE_GETTING) {
            amount_gettable += b->data.storage.resourceStored[RESOURCE_MEAT];
        }
        if (amount_gettable > 0) {
            int dist = calc_distance_with_penalty(
                b->x + 1, b->y + 1,
                           src->x + 1, src->y + 1,
                           src->distanceFromEntry, b->distanceFromEntry);
            if (amount_gettable <= 400) {
                dist *= 2; // penalty for less food
            }
            if (dist < min_dist) {
                min_dist = dist;
                min_building_id = b->id;
            }
        }
    }
    building *min = building_get(min_building_id);
    *x_dst = min->x + 1;
    *y_dst = min->y + 1;
    return min_building_id;
}
