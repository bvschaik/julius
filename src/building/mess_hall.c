#include "mess_hall.h"

#include "building/distribution.h"
#include "game/resource.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 1200

static int get_needed_food(building *mess_hall)
{
    int food_needed = INVENTORY_FLAG_NONE;

    if (building_distribution_is_good_accepted(INVENTORY_WHEAT, mess_hall)) {
        inventory_set(&food_needed, INVENTORY_WHEAT);
    }
    if (building_distribution_is_good_accepted(INVENTORY_VEGETABLES, mess_hall)) {
        inventory_set(&food_needed, INVENTORY_VEGETABLES);
    }
    if (building_distribution_is_good_accepted(INVENTORY_FRUIT, mess_hall)) {
        inventory_set(&food_needed, INVENTORY_FRUIT);
    }
    if (building_distribution_is_good_accepted(INVENTORY_MEAT, mess_hall)) {
        inventory_set(&food_needed, INVENTORY_MEAT);
    }

    return food_needed;
}

int building_mess_hall_get_storage_destination(building *mess_hall)
{
    int food_needed = get_needed_food(mess_hall);

    if (food_needed == INVENTORY_FLAG_NONE) {
        return 0;
    }
    inventory_storage_info data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, mess_hall, MAX_DISTANCE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(mess_hall, data, 0, 1, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(mess_hall, data, BASELINE_STOCK, 0, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }    
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(mess_hall, data, MAX_FOOD, 0, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    return 0;
}
