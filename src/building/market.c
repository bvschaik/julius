#include "market.h"

#include "building/monument.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

#define MAX_DISTANCE 40
#define BASELINE_STOCK 50

static const int INVENTORY_SEARCH_ORDER[INVENTORY_MAX] = {
    INVENTORY_WHEAT, INVENTORY_VEGETABLES, INVENTORY_FRUIT, INVENTORY_MEAT, INVENTORY_POTTERY, INVENTORY_FURNITURE, INVENTORY_OIL, INVENTORY_WINE
};

typedef struct {
    int building_id;
    int min_distance;
} resource_data;

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

static void update_food_resource(resource_data *data, resource_type resource, const building *b, int distance)
{
    if (distance < data->min_distance && b->data.granary.resource_stored[resource]) {
        data->min_distance = distance;
        data->building_id = b->id;
    }
}

static void update_good_resource(resource_data *data, resource_type resource, building *b, int distance)
{
    if (distance < data->min_distance && !city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        data->min_distance = distance;
        data->building_id = b->id;
    }
}

int building_venus_temple_get_wine_destination(building* temple, building* grand_temple)
{
    if (temple->data.market.inventory[INVENTORY_WINE] < 50 && grand_temple->loads_stored > 0) {
        temple->data.market.fetch_inventory_id = INVENTORY_WINE;
        return grand_temple->id;
    }
    return 0;  
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

int is_good_accepted(inventory_type resource, building *market)
{
    int goods_bit = 1 << resource;
    return !(market->subtype.market_goods & goods_bit);
}

void toggle_good_accepted(inventory_type resource, building *market)
{
    int goods_bit = 1 << resource;
    market->subtype.market_goods ^= goods_bit;
}

void unaccept_all_goods(building *market)
{
    market->subtype.market_goods = 0xffff;
}

static int inventory_is_type_needed(int inventory, int type)
{
    return (inventory >> type) & 1;
}

static void inventory_add_needed_type(int *inventory, int type)
{
    *inventory |= 1 << type; 
}

int building_market_get_needed_inventory(building *market)
{
    int needed = 0;
    if (!scenario_property_rome_supplies_wheat()) {
        if (is_good_accepted(INVENTORY_WHEAT, market)) {
            inventory_add_needed_type(&needed, INVENTORY_WHEAT);
        }
        if (is_good_accepted(INVENTORY_VEGETABLES, market)) {
            inventory_add_needed_type(&needed, INVENTORY_VEGETABLES);
        }
        if (is_good_accepted(INVENTORY_FRUIT, market)) {
            inventory_add_needed_type(&needed, INVENTORY_FRUIT);
        }
        if (is_good_accepted(INVENTORY_MEAT, market)) {
            inventory_add_needed_type(&needed, INVENTORY_MEAT);
        }
    }
    if (market->data.market.pottery_demand && is_good_accepted(INVENTORY_POTTERY, market)) {
        inventory_add_needed_type(&needed, INVENTORY_POTTERY);
    }
    if (market->data.market.furniture_demand && is_good_accepted(INVENTORY_FURNITURE, market)) {
        inventory_add_needed_type(&needed, INVENTORY_FURNITURE);
    }
    if (market->data.market.oil_demand && is_good_accepted(INVENTORY_OIL, market)) {
        inventory_add_needed_type(&needed, INVENTORY_OIL);
    }
    if (market->data.market.wine_demand && is_good_accepted(INVENTORY_WINE, market)) {
        inventory_add_needed_type(&needed, INVENTORY_WINE);
    }
    return needed;
}

static void get_closest_buildings_for_resources(resource_data *resources, building *market)
{
    int permission; 
    if (market->type == BUILDING_MESS_HALL) {
        permission = BUILDING_STORAGE_PERMISSION_QUARTERMASTER;
    } else {
        permission = BUILDING_STORAGE_PERMISSION_MARKET;
    }

    for (int i = 1; i < building_count(); i++) {
        building *b = building_get(i);

        if (b->state != BUILDING_STATE_IN_USE ||
            (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE) ||
            !b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != market->road_network_id ||
            !building_storage_get_permission(permission, b)) {
            continue;
        }
        int distance = calc_maximum_distance(market->x, market->y, b->x, b->y);
        if (b->type == BUILDING_GRANARY) {
            update_food_resource(&resources[INVENTORY_WHEAT], RESOURCE_WHEAT, b, distance);
            update_food_resource(&resources[INVENTORY_VEGETABLES], RESOURCE_VEGETABLES, b, distance);
            update_food_resource(&resources[INVENTORY_FRUIT], RESOURCE_FRUIT, b, distance);
            update_food_resource(&resources[INVENTORY_MEAT], RESOURCE_MEAT, b, distance);
        } else if (b->type == BUILDING_WAREHOUSE) {
            update_good_resource(&resources[INVENTORY_WINE], RESOURCE_WINE, b, distance);
            update_good_resource(&resources[INVENTORY_OIL], RESOURCE_OIL, b, distance);
            update_good_resource(&resources[INVENTORY_POTTERY], RESOURCE_POTTERY, b, distance);
            update_good_resource(&resources[INVENTORY_FURNITURE], RESOURCE_FURNITURE, b, distance);
        }
    }
}

static int pick_inventory_to_fetch(building *market, resource_data *resources, int min_stock, int pick_first_found, int pick_goods)
{
    int inventory = INVENTORY_NONE;
    if (!min_stock) {
        min_stock = 1;
    }

    for (int i = 0; i < INVENTORY_MAX; i++) {
        int current_inventory = INVENTORY_SEARCH_ORDER[i];
        if (current_inventory >= INVENTORY_MIN_GOOD && !pick_goods) {
            return inventory;
        }
        if (resources[current_inventory].building_id && market->data.market.inventory[current_inventory] < min_stock) {
            if (pick_first_found) {
                return current_inventory;
            }
            min_stock = market->data.market.inventory[current_inventory];
            inventory = current_inventory;
        }
    }
    return inventory;
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

    resource_data resources[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].min_distance = inventory_is_type_needed(needed_inventory, i) ? MAX_DISTANCE : 0;
    }

    get_closest_buildings_for_resources(resources, market);

    int can_go = 0;

    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].building_id) {
            can_go = 1;
            break;
        }
    }
    if (!can_go) {
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
        if (resources[inventory].building_id && market->data.market.inventory[inventory] < food_threshhold(market)) {
            market->data.market.fetch_inventory_id = inventory;
            return resources[inventory].building_id;
        }
        if (resources[INVENTORY_OIL].building_id && market->data.market.inventory[INVENTORY_OIL] < BASELINE_STOCK) {
            market->data.market.fetch_inventory_id = INVENTORY_OIL;
            return resources[INVENTORY_OIL].building_id;
        }
        return 0;
    }

    // Tavern
    if (market->type == BUILDING_TAVERN) {
        if (resources[INVENTORY_WINE].building_id && market->data.market.inventory[INVENTORY_WINE] < BASELINE_STOCK) {
            market->data.market.fetch_inventory_id = INVENTORY_WINE;
            return resources[INVENTORY_WINE].building_id;
        }
        if (market->data.market.inventory[INVENTORY_WINE] >= BASELINE_STOCK && 
            resources[INVENTORY_MEAT].building_id && market->data.market.inventory[INVENTORY_MEAT] < food_threshhold(market)) {
            market->data.market.fetch_inventory_id = INVENTORY_MEAT;
            return resources[INVENTORY_MEAT].building_id;
        }
        return 0;
    }

    // prefer whichever good we don't have
    int fetch_inventory = pick_inventory_to_fetch(market, resources, 0, 1, 1);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return resources[fetch_inventory].building_id;
    }

    // then prefer smallest stock below baseline stock
    fetch_inventory = pick_inventory_to_fetch(market, resources, BASELINE_STOCK, 0, 1);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return resources[fetch_inventory].building_id;
    }    
;
    // all items well stocked: pick food below threshold
    fetch_inventory = pick_inventory_to_fetch(market, resources, food_threshhold(market), 0, 0);
    if (fetch_inventory != INVENTORY_NONE) {
        market->data.market.fetch_inventory_id = fetch_inventory;
        return resources[fetch_inventory].building_id;
    }

    return 0;
}

int building_mars_temple_food_to_deliver(building *temple, int mess_hall_id)
{
    int most_stocked_food_id = -1;
    int next;
    building* mess_hall = building_get(mess_hall_id);
    for (int i = 0; i < INVENTORY_MAX_FOOD; i++) {
        next = temple->data.market.inventory[i];
        if (next > most_stocked_food_id && next >= 100 && mess_hall->data.market.inventory[i] <= 1600) {
            most_stocked_food_id = i;
        }
    }

    return most_stocked_food_id;
}
