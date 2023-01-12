#include "caravanserai.h"

#include "building/distribution.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/trade.h"
#include "city/resource.h"

#define INFINITE 10000

int building_caravanserai_enough_foods(building *caravanserai)
{
    int food_required_monthly = trade_caravan_count() * FOOD_PER_TRADER_MONTHLY;
    int total_food_in_caravanserai = 0;

    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        total_food_in_caravanserai += caravanserai->resources[r];            
    }

    return total_food_in_caravanserai >= food_required_monthly;
}

int building_caravanserai_get_storage_destination(building *caravanserai)
{
    resource_storage_info info[RESOURCE_MAX] = { 0 };

    if (!building_distribution_get_handled_resources_for_building(caravanserai, info) ||
        !building_distribution_get_resource_storages_for_building(info, caravanserai, INFINITE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(caravanserai, info, 0, 1);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(caravanserai, info, BASELINE_STOCK, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
    }
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(caravanserai, info, MAX_FOOD, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return info[fetch_inventory].building_id;
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
