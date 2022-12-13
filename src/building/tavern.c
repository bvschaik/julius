#include "tavern.h"

#include "building/distribution.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600

int building_tavern_get_storage_destination(building *tavern)
{
    if (!building_distribution_is_good_accepted(INVENTORY_WINE, tavern) &&
        !building_distribution_is_good_accepted(INVENTORY_MEAT, tavern)) {
        return 0;
    }
    inventory_storage_info data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, tavern, INFINITE)) {
        return 0;
    }
    if (building_distribution_is_good_accepted(INVENTORY_WINE, tavern) &&
        data[INVENTORY_WINE].building_id && tavern->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK) {
        tavern->data.market.fetch_inventory_id = INVENTORY_WINE;
        return data[INVENTORY_WINE].building_id;
    }
    if (tavern->data.market.inventory[INVENTORY_WINE] >= BASELINE_STOCK &&
        building_distribution_is_good_accepted(INVENTORY_MEAT, tavern) &&
        data[INVENTORY_MEAT].building_id && tavern->data.market.inventory[INVENTORY_MEAT] < MAX_FOOD) {
        tavern->data.market.fetch_inventory_id = INVENTORY_MEAT;
        return data[INVENTORY_MEAT].building_id;
    }
    return 0;
}
