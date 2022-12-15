#include "market.h"

#include "building/distribution.h"
#include "scenario/property.h"

#define MAX_FOOD 600

int building_market_get_max_food_stock(building *market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (!resource_get_data(r)->is_inventory) {
                continue;
            }
            int stock = market->resources[r];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

int building_market_get_max_goods_stock(building *market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (resource_type r = RESOURCE_MIN_GOOD; r < RESOURCE_MAX_GOOD; r++) {
            if (!resource_get_data(r)->is_inventory) {
                continue;
            }
            int stock = market->resources[r];
            if (stock > max_stock) {
                max_stock = stock;
            }
        }
    }
    return max_stock;
}

static int is_good_wanted(building *market, resource_type resource)
{
    return market->accepted_goods[resource] > 1;
}

void building_market_get_needed_inventory(building *market, int needed[RESOURCE_MAX])
{
    needed[RESOURCE_NONE] = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX_FOOD; r++) {
        needed[r] = !scenario_property_rome_supplies_wheat() &&
            resource_get_data(r)->is_inventory && building_distribution_is_good_accepted(r, market);
    }
    for (resource_type r = RESOURCE_MIN_GOOD; r < RESOURCE_MAX_GOOD; r++) {
        needed[r] = resource_get_data(r)->is_inventory && is_good_wanted(market, r);
    }
}

resource_type building_market_fetch_inventory(building *market, inventory_storage_info *info,
    const int needed[RESOURCE_MAX])
{
    // Prefer whichever good we don't have
    resource_type fetch_inventory = building_distribution_fetch(market, info, 0, 1, needed);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(market, info, BASELINE_STOCK, 0, needed);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }

    int needed_foods[RESOURCE_MAX] = { 0 };
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        needed_foods[r] = needed[r];
    }

    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(market, info, MAX_FOOD, 0, needed_foods);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }

    return RESOURCE_NONE;
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

int building_market_get_storage_destination(building *market)
{
    int needed_inventory[RESOURCE_MAX];
    building_market_get_needed_inventory(market, needed_inventory);
    if (!has_inventory_needs(needed_inventory)) {
        return 0;
    }
    inventory_storage_info info[RESOURCE_MAX];
    if (!building_distribution_get_inventory_storages_for_building(info, market, MARKET_MAX_DISTANCE)) {
        return 0;
    }
    int fetch_inventory = building_market_fetch_inventory(market, info, needed_inventory);
    if (fetch_inventory == RESOURCE_NONE) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    return info[fetch_inventory].building_id;
}
