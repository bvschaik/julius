#include "caravanserai.h"

#include "building/distribution.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/trade.h"
#include "city/resource.h"

#define INFINITE 10000

static void get_needed_food(building *caravanserai, int needed_food[RESOURCE_MAX])
{
    needed_food[RESOURCE_NONE] = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        needed_food[r] = resource_is_food(r) && resource_get_data(r)->is_inventory &&
            building_distribution_is_good_accepted(r, caravanserai);
    }
}

int building_caravanserai_enough_foods(building *caravanserai)
{
    int food_required_monthly = trade_caravan_count() * FOOD_PER_TRADER_MONTHLY;
    int total_food_in_caravanserai = 0;

    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        total_food_in_caravanserai += caravanserai->resources[r];            
    }

    return total_food_in_caravanserai >= food_required_monthly;
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

int building_caravanserai_get_storage_destination(building *caravanserai)
{
    int food_needed[RESOURCE_MAX];
    get_needed_food(caravanserai, food_needed);

    if (!has_inventory_needs(food_needed)) {
        return 0;
    }
    inventory_storage_info data[RESOURCE_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, caravanserai, INFINITE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(caravanserai, data, 0, 1, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(caravanserai, data, BASELINE_STOCK, 0, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(caravanserai, data, MAX_FOOD, 0, food_needed);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    return 0;
}

int building_caravanserai_is_fully_functional(void)
{
    if (!building_monument_working(BUILDING_CARAVANSERAI)) {
        return 0;
    }

    building *b = building_get(city_buildings_get_caravanserai());

    if (building_caravanserai_enough_foods(b)) {
        return 1;
    } else {
        return 0;
    }
}
