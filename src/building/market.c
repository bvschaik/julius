#include "market.h"

#include "building/distribution.h"
#include "building/monument.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

#define MAX_DISTANCE 40
#define BASELINE_STOCK 50

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
    if (building_is_venus_temple(market->type)) {
        return;
    }
    if (market->data.market.pottery_demand) {
        market->data.market.pottery_demand--;
    }
    if (market->data.market.furniture_demand) {
        market->data.market.furniture_demand--;
    }
    if (market->data.market.oil_demand) {
        market->data.market.oil_demand--;
    }
    if (market->type == BUILDING_TAVERN) {
        market->data.market.wine_demand = 1;
    } else if (market->data.market.wine_demand) {
        market->data.market.wine_demand--;
    }
}

static int food_threshhold(building *b)
{
    return b->data.market.is_mess_hall ? 1200 : 600;
}

int building_market_get_needed_inventory(building *market)
{
    int needed = 0;
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

int building_market_get_storage_destination(building *market, int needed_inventory)
{
    if (building_is_venus_temple(market->type)) {
        building *gt = building_get(building_monument_get_venus_gt());
        if (gt->id != 0 && gt->road_network_id == market->road_network_id) {
            return building_venus_temple_get_wine_destination(market, gt);
        }
        return 0;
    }

    inventory_data *data = building_distribution_get_inventory_data(market, needed_inventory, MAX_DISTANCE);
    if (!data) {
        return 0;
    }

    // Ceres module 2
    if (building_is_ceres_temple(market->type)) {
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
        if (data[inventory].building_id && market->data.market.inventory[inventory] < food_threshhold(market)) {
            market->data.market.fetch_inventory_id = inventory;
            return data[inventory].building_id;
        }
        if (data[INVENTORY_OIL].building_id && market->data.market.inventory[INVENTORY_OIL] < BASELINE_STOCK) {
            market->data.market.fetch_inventory_id = INVENTORY_OIL;
            return data[INVENTORY_OIL].building_id;
        }
        return 0;
    }

    // Tavern
    if (market->type == BUILDING_TAVERN) {
        if (data[INVENTORY_WINE].building_id && market->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK) {
            market->data.market.fetch_inventory_id = INVENTORY_WINE;
            return data[INVENTORY_WINE].building_id;
        }
        if (market->data.market.inventory[INVENTORY_WINE] >= BASELINE_STOCK && 
            data[INVENTORY_MEAT].building_id && market->data.market.inventory[INVENTORY_MEAT] < food_threshhold(market)) {
            market->data.market.fetch_inventory_id = INVENTORY_MEAT;
            return data[INVENTORY_MEAT].building_id;
        }
        return 0;
    }

    // prefer whichever good we don't have
    int fetch_inventory = building_distribution_pick_inventory_to_fetch(market, data, 0, 1, INVENTORY_FLAG_ALL);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }

    // then prefer smallest stock below baseline stock
    fetch_inventory = building_distribution_pick_inventory_to_fetch(market, data, BASELINE_STOCK, 0, INVENTORY_FLAG_ALL);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }    

    // all items well stocked: pick food below threshold
    fetch_inventory = building_distribution_pick_inventory_to_fetch(
        market, data, food_threshhold(market), 0, INVENTORY_FLAG_ALL_FOODS);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return data[fetch_inventory].building_id;
    }

    return 0;
}

int building_venus_temple_get_wine_destination(building *temple, building *grand_temple)
{
    if (temple->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK && grand_temple->loads_stored > 0) {
        temple->data.market.fetch_inventory_id = INVENTORY_WINE;
        return grand_temple->id;
    }
    return 0;  
}

int building_mars_temple_food_to_deliver(building *temple, int mess_hall_id)
{
    int most_stocked_food_id = -1;
    int next;
    building *mess_hall = building_get(mess_hall_id);
    for (int i = 0; i < INVENTORY_MAX_FOOD; i++) {
        next = temple->data.market.inventory[i];
        if (next > most_stocked_food_id && next >= 100 && mess_hall->data.market.inventory[i] <= 1600) {
            most_stocked_food_id = i;
        }
    }

    return most_stocked_food_id;
}
