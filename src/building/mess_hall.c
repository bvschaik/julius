#include "mess_hall.h"

#include "building/distribution.h"
#include "game/resource.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 1200

int building_mess_hall_get_storage_destination(building *mess_hall)
{
    resource_storage_info info[RESOURCE_MAX] = { 0 };

    if (!building_distribution_get_handled_resources_for_building(mess_hall, info) ||
        !building_distribution_get_resource_storages_for_building(info, mess_hall, MAX_DISTANCE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(mess_hall, info, 0, 1);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(mess_hall, info, BASELINE_STOCK, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
    }    
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(mess_hall, info, MAX_FOOD, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
    }
    return 0;
}
