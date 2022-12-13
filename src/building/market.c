#include "market.h"

#include "building/distribution.h"
#include "scenario/property.h"

#define MAX_FOOD 600

int building_market_get_max_food_stock(building* market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_FOOD; i < INVENTORY_MAX_FOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

int building_market_get_max_goods_stock(building* market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (int i = INVENTORY_MIN_GOOD; i < INVENTORY_MAX_GOOD; i++) {
            int stock = market->data.market.inventory[i];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

int building_market_get_needed_inventory(building* market)
{
    int needed = INVENTORY_FLAG_NONE;
    if (!scenario_property_rome_supplies_wheat()) {
        if (building_distribution_is_good_accepted(INVENTORY_WHEAT, market)) {
            inventory_set(&needed, INVENTORY_WHEAT);
        }
        if (building_distribution_is_good_accepted(INVENTORY_VEGETABLES, market)) {
            inventory_set(&needed, INVENTORY_VEGETABLES);
        }
        if (building_distribution_is_good_accepted(INVENTORY_FRUIT, market)) {
            inventory_set(&needed, INVENTORY_FRUIT);
        }
        if (building_distribution_is_good_accepted(INVENTORY_MEAT, market)) {
            inventory_set(&needed, INVENTORY_MEAT);
        }
    }
    if (market->data.market.pottery_demand && building_distribution_is_good_accepted(INVENTORY_POTTERY, market)) {
        inventory_set(&needed, INVENTORY_POTTERY);
    }
    if (market->data.market.furniture_demand && building_distribution_is_good_accepted(INVENTORY_FURNITURE, market)) {
        inventory_set(&needed, INVENTORY_FURNITURE);
    }
    if (market->data.market.oil_demand && building_distribution_is_good_accepted(INVENTORY_OIL, market)) {
        inventory_set(&needed, INVENTORY_OIL);
    }
    if (market->data.market.wine_demand && building_distribution_is_good_accepted(INVENTORY_WINE, market)) {
        inventory_set(&needed, INVENTORY_WINE);
    }
    return needed;
}

int building_market_fetch_inventory(building* market, inventory_storage_info* info, int needed_inventory)
{
    // Prefer whichever good we don't have
    int fetch_inventory = building_distribution_fetch(market, info, 0, 1, needed_inventory);
    if (fetch_inventory != INVENTORY_NONE) {
        return fetch_inventory;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(market, info, BASELINE_STOCK, 0, needed_inventory);
    if (fetch_inventory != INVENTORY_NONE) {
        return fetch_inventory;
    }
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(market, info, MAX_FOOD, 0,
        needed_inventory & INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        return fetch_inventory;
    }
    return INVENTORY_NONE;
}

int building_market_get_storage_destination(building* market)
{
    int needed_inventory = building_market_get_needed_inventory(market);
    if (needed_inventory == INVENTORY_FLAG_NONE) {
        return 0;
    }
    inventory_storage_info info[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages_for_building(info, market, MARKET_MAX_DISTANCE)) {
        return 0;
    }
    int fetch_inventory = building_market_fetch_inventory(market, info, needed_inventory);
    if (fetch_inventory == INVENTORY_NONE) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    return info[fetch_inventory].building_id;
}


