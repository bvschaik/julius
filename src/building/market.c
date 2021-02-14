#include "market.h"

#include "building/distribution.h"
#include "scenario/property.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 600

int building_market_get_max_food_stock(building *market)
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

int building_market_get_max_goods_stock(building *market)
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

int building_market_is_good_accepted(inventory_type resource, building *market)
{
    int goods_bit = 1 << resource;
    return !(market->subtype.market_goods & goods_bit);
}

void building_market_toggle_good_accepted(inventory_type resource, building *market)
{
    int goods_bit = 1 << resource;
    market->subtype.market_goods ^= goods_bit;
}

void building_market_unaccept_all_goods(building *market)
{
    market->subtype.market_goods = 0xffff;
}

void building_market_update_demands(building *market)
{
    if (market->data.market.pottery_demand) {
        market->data.market.pottery_demand--;
    }
    if (market->data.market.furniture_demand) {
        market->data.market.furniture_demand--;
    }
    if (market->data.market.oil_demand) {
        market->data.market.oil_demand--;
    }
    if (market->data.market.wine_demand) {
        market->data.market.wine_demand--;
    }
}

static int get_needed_inventory(building *market)
{
    int needed = INVENTORY_FLAG_NONE;
    if (!scenario_property_rome_supplies_wheat()) {
        if (building_market_is_good_accepted(INVENTORY_WHEAT, market)) {
            inventory_set(&needed, INVENTORY_WHEAT);
        }
        if (building_market_is_good_accepted(INVENTORY_VEGETABLES, market)) {
            inventory_set(&needed, INVENTORY_VEGETABLES);
        }
        if (building_market_is_good_accepted(INVENTORY_FRUIT, market)) {
            inventory_set(&needed, INVENTORY_FRUIT);
        }
        if (building_market_is_good_accepted(INVENTORY_MEAT, market)) {
            inventory_set(&needed, INVENTORY_MEAT);
        }
    }
    if (market->data.market.pottery_demand && building_market_is_good_accepted(INVENTORY_POTTERY, market)) {
        inventory_set(&needed, INVENTORY_POTTERY);
    }
    if (market->data.market.furniture_demand && building_market_is_good_accepted(INVENTORY_FURNITURE, market)) {
        inventory_set(&needed, INVENTORY_FURNITURE);
    }
    if (market->data.market.oil_demand && building_market_is_good_accepted(INVENTORY_OIL, market)) {
        inventory_set(&needed, INVENTORY_OIL);
    }
    if (market->data.market.wine_demand && building_market_is_good_accepted(INVENTORY_WINE, market)) {
        inventory_set(&needed, INVENTORY_WINE);
    }
    return needed;
}

int building_market_get_storage_destination(building *market)
{
    int needed_inventory = get_needed_inventory(market);
    if (needed_inventory == INVENTORY_FLAG_NONE) {
        return 0;
    }
    inventory_data data[INVENTORY_MAX];
    if (!building_distribution_get_inventory_data(data, market, MAX_DISTANCE)) {
        return 0;
    }
    // Prefer whichever good we don't have
    int fetch_inventory = building_distribution_fetch(market, data, 0, 1, needed_inventory);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    // Then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_fetch(market, data, BASELINE_STOCK, 0, needed_inventory);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }    
    // All items well stocked: pick food below threshold
    fetch_inventory = building_distribution_fetch(market, data, MAX_FOOD, 0, needed_inventory & INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }
    return 0;
}
