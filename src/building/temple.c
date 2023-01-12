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
        if (!building_distribution_is_good_accepted(RESOURCE_WINE, temple) ||
            temple->accepted_goods[RESOURCE_WINE] <= 1) {
            return 0;
        }
        building *grand_temple = building_get(building_monument_get_venus_gt());
        if (grand_temple->id != 0 && grand_temple->road_network_id == temple->road_network_id &&
            temple->resources[RESOURCE_WINE] < BASELINE_STOCK && grand_temple->loads_stored > 0) {
            temple->data.market.fetch_inventory_id = RESOURCE_WINE;
            return grand_temple->id;
        }
        return 0;  
    }

    if (!building_is_ceres_temple(temple->type)) { // Ceres module 2
        return 0;
    }

    resource_type food = city_resource_ceres_temple_food();

    if (food == RESOURCE_NONE) {
        return 0;
    }

    resource_storage_info info[RESOURCE_MAX] = { 0 };
    if (!building_distribution_get_handled_resources_for_building(temple, info)) {
        return 0;
    }
    info[RESOURCE_OIL].needed = temple->accepted_goods[RESOURCE_OIL] > 1;

    if (!building_distribution_get_resource_storages_for_building(info, temple, INFINITE)) {
        return 0;
    }

    // Get food if below threshold
    if (info[food].building_id && temple->resources[food] < MAX_FOOD) {
        temple->data.market.fetch_inventory_id = food;
        return info[food].building_id;
    }

    // Otherwise get allowed oil depending on stock
    resource_type fetch_resource = building_distribution_fetch(temple, info, BASELINE_STOCK, 0);
    temple->data.market.fetch_inventory_id = fetch_resource;
    return info[fetch_resource].building_id;
}

int building_temple_mars_food_to_deliver(building *temple, int mess_hall_id)
{
    int most_stocked_food_id = -1;
    int next;
    building *mess_hall = building_get(mess_hall_id);
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        next = temple->resources[r];
        if (next > most_stocked_food_id && next >= 100 && mess_hall->resources[r] <= MAX_MESS_HALL_FOOD) {
            most_stocked_food_id = r;
        }
    }
    return most_stocked_food_id;
}
