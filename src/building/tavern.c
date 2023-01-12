#include "tavern.h"

#include "building/distribution.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600

int building_tavern_get_storage_destination(building *tavern)
{
    resource_storage_info info[RESOURCE_MAX] = { 0 };

    if (!building_distribution_get_handled_resources_for_building(tavern, info) ||
        !building_distribution_get_resource_storages_for_building(info, tavern, INFINITE)) {
        return 0;
    }

    // Get wine if below baseline stock
    if (tavern->resources[RESOURCE_WINE] < BASELINE_STOCK) {
        tavern->data.market.fetch_inventory_id = RESOURCE_WINE;
        return info[RESOURCE_WINE].building_id;
    }

    info[RESOURCE_WINE].needed = 0;

    // Otherwise get allowed food depending on stock
    resource_type fetch_resource = building_distribution_fetch(tavern, info, MAX_FOOD, 0);
    tavern->data.market.fetch_inventory_id = fetch_resource;
    return info[fetch_resource].building_id;
}
