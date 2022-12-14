#include "mess_hall.h"

#include "building/distribution.h"
#include "game/resource.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 1200

static void get_needed_food(building *mess_hall, int needed_food[RESOURCE_MAX])
{
    needed_food[RESOURCE_NONE] = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        needed_food[r] = resource_is_food(r) && resource_get_data(r)->is_inventory &&
            building_distribution_is_good_accepted(r, mess_hall);
    }
}

static int has_inventory_needs(const int needed[RESOURCE_MAX])
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (needed[r]) {
            return 1;
        }
    }
    return 0;
}

int building_mess_hall_get_storage_destination(building *mess_hall)
{
    int food_needed[RESOURCE_MAX];
    get_needed_food(mess_hall, food_needed);

    if (!has_inventory_needs(food_needed)) {
        return 0;
    }
    inventory_storage_info data[RESOURCE_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, mess_hall, MAX_DISTANCE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(mess_hall, data, 0, 1, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(mess_hall, data, BASELINE_STOCK, 0, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }    
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(mess_hall, data, MAX_FOOD, 0, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        mess_hall->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    return 0;
}
