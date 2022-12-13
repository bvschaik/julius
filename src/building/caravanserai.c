#include "caravanserai.h"

#include "building/distribution.h"
#include "building/monument.h"
#include "city/buildings.h"
#include "city/trade.h"
#include "city/resource.h"

#define INFINITE 10000

static int get_needed_food(building *caravanserai)
{
    int food_needed = INVENTORY_FLAG_NONE;

    if (building_distribution_is_good_accepted(INVENTORY_WHEAT, caravanserai)) {
        inventory_set(&food_needed, INVENTORY_WHEAT);
    }
    if (building_distribution_is_good_accepted(INVENTORY_VEGETABLES, caravanserai)) {
        inventory_set(&food_needed, INVENTORY_VEGETABLES);
    }
    if (building_distribution_is_good_accepted(INVENTORY_FRUIT, caravanserai)) {
        inventory_set(&food_needed, INVENTORY_FRUIT);
    }
    if (building_distribution_is_good_accepted(INVENTORY_MEAT, caravanserai)) {
        inventory_set(&food_needed, INVENTORY_MEAT);
    }

    return food_needed;
}

int building_caravanserai_enough_foods(building *caravanserai)
{
    int food_required_monthly = trade_caravan_count() * FOOD_PER_TRADER_MONTHLY;
    int total_food_in_caravanserai = 0;

    for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; ++i) {
        total_food_in_caravanserai += caravanserai->data.market.inventory[i];
    }

    return total_food_in_caravanserai >= food_required_monthly;
}

int building_caravanserai_get_storage_destination(building *caravanserai)
{
    int food_needed = get_needed_food(caravanserai);

    if (food_needed == INVENTORY_FLAG_NONE) {
        return 0;
    }
    inventory_storage_info data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages_for_building(data, caravanserai, INFINITE)) {
        return 0;
    }
    // Prefer whichever food we don't have
    int fetch_inventory = building_distribution_fetch(caravanserai, data, 0, 1, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(caravanserai, data, BASELINE_STOCK, 0, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
        caravanserai->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(caravanserai, data, MAX_FOOD, 0, food_needed);
    if (fetch_inventory != INVENTORY_NONE) {
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
