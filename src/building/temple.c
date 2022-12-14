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

    resource_type resource = city_resource_ceres_temple_food();
    if (resource == RESOURCE_NONE) {
        return 0;
    }
    if (!building_distribution_is_good_accepted(resource, temple) &&
        (!building_distribution_is_good_accepted(RESOURCE_OIL, temple) || temple->accepted_goods[RESOURCE_OIL] <= 1)) {
        return 0;
    }

    inventory_storage_info data[RESOURCE_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, temple, INFINITE)) {
        return 0;
    }

    if (building_distribution_is_good_accepted(resource, temple) &&
        data[resource].building_id && temple->resources[resource] < MAX_FOOD) {
        temple->data.market.fetch_inventory_id = resource;
        return data[resource].building_id;
    }
    if (building_distribution_is_good_accepted(RESOURCE_OIL, temple) && temple->accepted_goods[RESOURCE_OIL] > 1 &&
        data[RESOURCE_OIL].building_id && temple->resources[RESOURCE_OIL] < BASELINE_STOCK) {
        temple->data.market.fetch_inventory_id = RESOURCE_OIL;
        return data[RESOURCE_OIL].building_id;
    }
    return 0;
}

int building_temple_mars_food_to_deliver(building *temple, int mess_hall_id)
{
    int most_stocked_food_id = -1;
    int next;
    building *mess_hall = building_get(mess_hall_id);
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (!resource_is_food(r)) {
            continue;
        }
        next = temple->resources[r];
        if (next > most_stocked_food_id && next >= 100 && mess_hall->resources[r] <= MAX_MESS_HALL_FOOD) {
            most_stocked_food_id = r;
        }
    }
    return most_stocked_food_id;
}
