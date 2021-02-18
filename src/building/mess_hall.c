#include "mess_hall.h"

#include "building/distribution.h"
#include "game/resource.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 1200

int building_mess_hall_get_storage_destination(building *mess_hall)
{
    inventory_storage_info data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages(data, BUILDING_MESS_HALL,
            mess_hall->road_network_id, mess_hall->road_access_x, mess_hall->road_access_y, MAX_DISTANCE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(mess_hall, data, 0, 1, INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(mess_hall, data, BASELINE_STOCK, 0, INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }    
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(mess_hall, data, MAX_FOOD, 0, INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    return 0;
}
