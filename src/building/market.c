#include "market.h"

#include "building/distribution.h"
#include "scenario/property.h"

#define MAX_FOOD 600

int building_market_get_max_food_stock(building *market)
{
    int max_stock = 0;
    if (market->id > 0 && market->type == BUILDING_MARKET) {
        for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (!resource_is_inventory(r)) {
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
        for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
            if (!resource_is_inventory(r)) {
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

int building_market_get_needed_inventory(building *market, resource_storage_info info[RESOURCE_MAX])
{
    int needed = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX_FOOD; r++) {
        info[r].needed = building_distribution_resource_is_handled(r, BUILDING_MARKET) &&
            !scenario_property_rome_supplies_wheat() && building_distribution_is_good_accepted(r, market);
        if (!needed && info[r].needed) {
            needed = 1;
        }
    }
    for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
        info[r].needed = building_distribution_resource_is_handled(r, BUILDING_MARKET) && is_good_wanted(market, r);
        if (!needed && info[r].needed) {
            needed = 1;
        }
    }
    return needed;
}

resource_type building_market_fetch_inventory(building *market, resource_storage_info info[RESOURCE_MAX])
{
    // Prefer whichever good we don't have
    resource_type fetch_inventory = building_distribution_fetch(market, info, 0, 1);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(market, info, BASELINE_STOCK, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }

    for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
        info[r].needed = 0;
    }

    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(market, info, MAX_FOOD, 0);
    if (fetch_inventory != RESOURCE_NONE) {
        return fetch_inventory;
    }

    return RESOURCE_NONE;
}

int building_market_get_storage_destination(building *market)
{
    resource_storage_info info[RESOURCE_MAX] = { 0 };
    if (!building_market_get_needed_inventory(market, info) ||
        !building_distribution_get_resource_storages_for_building(info, market, MARKET_MAX_DISTANCE)) {
        return 0;
    }
    int fetch_inventory = building_market_fetch_inventory(market, info);
    market->data.market.fetch_inventory_id = fetch_inventory;
    return info[fetch_inventory].building_id;
}
