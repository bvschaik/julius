#include "tavern.h"

#include "building/distribution.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600

int building_tavern_get_storage_destination(building *tavern)
{
    inventory_storage_info data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages(data, BUILDING_TAVERN, 
            tavern->road_network_id, tavern->road_access_x, tavern->road_access_y, INFINITE)) {
        return 0;
    }
    if (data[INVENTORY_WINE].building_id && tavern->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK) {
        tavern->data.market.fetch_inventory_id = INVENTORY_WINE;
        return data[INVENTORY_WINE].building_id;
    }
    if (tavern->data.market.inventory[INVENTORY_WINE] >= BASELINE_STOCK && 
        data[INVENTORY_MEAT].building_id && tavern->data.market.inventory[INVENTORY_MEAT] < MAX_FOOD) {
        tavern->data.market.fetch_inventory_id = INVENTORY_MEAT;
        return data[INVENTORY_MEAT].building_id;
    }
    return 0;
}
