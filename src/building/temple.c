
#include "temple.h"

#include "building/distribution.h"
#include "building/monument.h"
#include "city/resource.h"
#include "game/resource.h"

#define INFINITE 10000
#define MAX_FOOD 600
#define MAX_MESS_HALL_FOOD 1600

int building_temple_get_storage_destination(building *temple)
{
    if (building_is_venus_temple(temple->type)) {
        building *grand_temple = building_get(building_monument_get_venus_gt());
        if (grand_temple->id != 0 && grand_temple->road_network_id == temple->road_network_id &&
            temple->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK && grand_temple->loads_stored > 0) {
            temple->data.market.fetch_inventory_id = INVENTORY_WINE;
            return grand_temple->id;
        }
        return 0;  
    }

    if (!building_is_ceres_temple(temple->type)) { // Ceres module 2
        return 0;
    }
    int allowed_inventory = INVENTORY_FLAG_ALL_FOODS;
    inventory_set(&allowed_inventory, INVENTORY_WINE);
    
    inventory_data *data = building_distribution_get_inventory_data(temple, allowed_inventory, INFINITE);
    if (!data) {
        return 0;
    }
    int inventory;
    int ceres_food = city_resource_ceres_temple_food();
    switch (ceres_food) {
        case RESOURCE_WHEAT: inventory = INVENTORY_WHEAT; break;
        case RESOURCE_VEGETABLES: inventory = INVENTORY_VEGETABLES; break;
        case RESOURCE_FRUIT: inventory = INVENTORY_FRUIT; break;
        case RESOURCE_MEAT: inventory = INVENTORY_MEAT; break;
        case RESOURCE_WINE: inventory = INVENTORY_WINE; break;
        default: return 0;
    }
    if (data[inventory].building_id && temple->data.market.inventory[inventory] < MAX_FOOD) {
        temple->data.market.fetch_inventory_id = inventory;
        return data[inventory].building_id;
    }
    if (data[INVENTORY_OIL].building_id && temple->data.market.inventory[INVENTORY_OIL] < BASELINE_STOCK) {
        temple->data.market.fetch_inventory_id = INVENTORY_OIL;
        return data[INVENTORY_OIL].building_id;
    }
    return 0;
}

int building_temple_mars_food_to_deliver(building *temple, int mess_hall_id)
{
    int most_stocked_food_id = -1;
    int next;
    building *mess_hall = building_get(mess_hall_id);
    for (int i = 0; i < INVENTORY_MAX_FOOD; i++) {
        next = temple->data.market.inventory[i];
        if (next > most_stocked_food_id && next >= 100 && mess_hall->data.market.inventory[i] <= MAX_MESS_HALL_FOOD) {
            most_stocked_food_id = i;
        }
    }
    return most_stocked_food_id;
}
