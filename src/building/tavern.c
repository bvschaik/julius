#include "tavern.h"

#include "building/distribution.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600

int building_tavern_get_storage_destination(building *tavern)
{
    if (!building_distribution_is_good_accepted(RESOURCE_WINE, tavern) &&
        !building_distribution_is_good_accepted(RESOURCE_MEAT, tavern)) {
        return 0;
    }
    inventory_storage_info data[RESOURCE_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, tavern, INFINITE)) {
        return 0;
    }
    if (building_distribution_is_good_accepted(RESOURCE_WINE, tavern) &&
        data[RESOURCE_WINE].building_id && tavern->resources[RESOURCE_WINE] < BASELINE_STOCK) {
        tavern->data.market.fetch_inventory_id = RESOURCE_WINE;
        return data[RESOURCE_WINE].building_id;
    }
    if (tavern->resources[RESOURCE_WINE] >= BASELINE_STOCK &&
        building_distribution_is_good_accepted(RESOURCE_MEAT, tavern) &&
        data[RESOURCE_MEAT].building_id && tavern->resources[RESOURCE_MEAT] < MAX_FOOD) {
        tavern->data.market.fetch_inventory_id = RESOURCE_MEAT;
        return data[RESOURCE_MEAT].building_id;
    }
    return 0;
}
