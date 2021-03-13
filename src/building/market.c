#include "market.h"
#include "building/distribution.h"
#include "building/monument.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/random.h"
#include "game/resource.h"
#include "scenario/property.h"

#define MAX_DISTANCE 40
#define MAX_FOOD 600

static const int LOOT_PREFERENCE[] = {
    RESOURCE_WINE,
    RESOURCE_WHEAT,
    RESOURCE_MEAT,
    RESOURCE_OIL,
    RESOURCE_FURNITURE,
    RESOURCE_FRUIT,
    RESOURCE_POTTERY,
    RESOURCE_VEGETABLES
};

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

int building_market_get_needed_inventory(building *market)
{
    int needed = INVENTORY_FLAG_NONE;
    if (!scenario_property_rome_supplies_wheat()) {
        if (building_distribution_is_good_accepted(INVENTORY_WHEAT, market)) {
            inventory_set(&needed, INVENTORY_WHEAT);
        }

        if (building_distribution_is_good_accepted(INVENTORY_VEGETABLES, market)) {
            inventory_set(&needed, INVENTORY_VEGETABLES);
    }
}

static void update_good_resource(struct resource_data *data, resource_type resource, building *b, int distance)
{
    if (!city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

static void update_good_resource_generic(struct resource_data* data, resource_type resource, building* b, int distance)
{
    if (!city_resource_is_stockpiled(resource) && b->data.market.inventory[resource] > 100) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
    }
}

int is_good_accepted(inventory_type resource, building *market) {
    int goods_bit = 1 << resource;
    return !(market->subtype.market_goods & goods_bit);
}

void toggle_good_accepted(inventory_type resource, building *market) {
    int goods_bit = 1 << resource;
    market->subtype.market_goods ^= goods_bit;
}

void unaccept_all_goods(building *market) {
    market->subtype.market_goods=0xFFFF;
}

int building_venus_temple_get_wine_destination(building* temple, building* grand_temple) {
    if (temple->data.market.inventory[INVENTORY_WINE] < 50 && grand_temple->loads_stored > 0)
    {
        temple->data.market.fetch_inventory_id = INVENTORY_WINE;
        return grand_temple->id;
    }
    return 0;  
}

static int food_threshhold(building *b) {
    return b->data.market.is_mess_hall ? 1200 : 600;
}

// Looters
int building_market_remove_resource(int building_id, int resource, int amount)
{
    int inventory = 0;
    switch (resource) {
    case RESOURCE_WHEAT: inventory = INVENTORY_WHEAT; break;
    case RESOURCE_VEGETABLES: inventory = INVENTORY_VEGETABLES; break;
    case RESOURCE_FRUIT: inventory = INVENTORY_FRUIT; break;
    case RESOURCE_MEAT: inventory = INVENTORY_MEAT; break;
    case RESOURCE_POTTERY: inventory = INVENTORY_POTTERY; break;
    case RESOURCE_FURNITURE: inventory = INVENTORY_FURNITURE; break;
    case RESOURCE_OIL: inventory = INVENTORY_OIL; break;
    case RESOURCE_WINE: inventory = INVENTORY_WINE; break;
    }

    building* b = building_get(building_id);
    if (amount >= b->data.market.inventory[inventory]) {
        b->data.market.inventory[inventory] = 0;
    }
    else {
        b->data.market.inventory[inventory] -= amount;
    }

    return 1;
}

int building_market_get_destination_for_looting(int x, int y, int *x_tile, int *y_tile, int *resource) {
    struct resource_data resources[RESOURCE_MAX];
    for (int i = 0; i < RESOURCE_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].num_buildings = 0;
        resources[i].distance = 40;
    }

    for (int i = 1; i < building_count(); i++) {
        building* b = building_get(i);

        if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE && ! b->type != BUILDING_MARKET) {
            continue;
        }

        int distance = calc_maximum_distance(x, y, b->x, b->y);
        if (distance >= 120) {
            continue;
        }

        if (b->type == BUILDING_GRANARY || b->type == BUILDING_MARKET) {
            update_food_resource(&resources[RESOURCE_WHEAT], RESOURCE_WHEAT, b, distance);
            update_food_resource(&resources[RESOURCE_VEGETABLES], RESOURCE_VEGETABLES, b, distance);
            update_food_resource(&resources[RESOURCE_FRUIT], RESOURCE_FRUIT, b, distance);
            update_food_resource(&resources[RESOURCE_MEAT], RESOURCE_MEAT, b, distance);
        }
        
        if (b->type == BUILDING_WAREHOUSE || b->type == BUILDING_MARKET) {
            update_good_resource(&resources[RESOURCE_WINE], RESOURCE_WINE, b, distance);
            update_good_resource(&resources[RESOURCE_OIL], RESOURCE_OIL, b, distance);
            update_good_resource(&resources[RESOURCE_POTTERY], RESOURCE_POTTERY, b, distance);
            update_good_resource(&resources[RESOURCE_FURNITURE], RESOURCE_FURNITURE, b, distance);
        }
    }

    // pick random from these targets instead of always following preference
    for (int i = 0; i < 8; i++) {        
        if (resources[LOOT_PREFERENCE[i]].building_id) {
            building* b = building_get(resources[LOOT_PREFERENCE[i]].building_id);
            *x_tile = b->x + 1;
            *y_tile = b->y;
            *resource = LOOT_PREFERENCE[i];
            return resources[LOOT_PREFERENCE[i]].building_id;
        }
    }

    return 0;
}

int building_market_get_storage_destination(building *market)
{
    struct resource_data resources[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].num_buildings = 0;
        resources[i].distance = 40;
    }
    int permission; 
    if (market->type == BUILDING_MESS_HALL) {
        permission = BUILDING_STORAGE_PERMISSION_QUARTERMASTER;
    } else {
        permission = BUILDING_STORAGE_PERMISSION_MARKET;
    }
    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);

        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
>>>>>>> strikes
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

int building_market_fetch_inventory(building *market, inventory_storage_info *info, int needed_inventory)
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

int building_market_get_storage_destination(building *market)
{
    int needed_inventory = building_market_get_needed_inventory(market);
    if (needed_inventory == INVENTORY_FLAG_NONE) {
        return 0;
    }
    inventory_storage_info info[INVENTORY_MAX];
    if (!building_distribution_get_inventory_storages(info, BUILDING_MARKET,
            market->road_network_id, market->road_access_x, market->road_access_y, MAX_DISTANCE)) {
        return 0;
    }
    int fetch_inventory = building_market_fetch_inventory(market, info, needed_inventory);
    if (fetch_inventory == INVENTORY_NONE) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    return info[fetch_inventory].building_id;
}
