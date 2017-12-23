#include "granary.h"

#include "game/resource.h"

#include "Data/CityInfo.h"

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

