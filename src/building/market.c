#include "market.h"

#include "building/monument.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "game/resource.h"
#include "scenario/property.h"

struct resource_data {
    int building_id;
    int distance;
    int num_buildings;
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

static void update_food_resource(struct resource_data *data, resource_type resource, const building *b, int distance)
{
    if (b->data.granary.resource_stored[resource]) {
        data->num_buildings++;
        if (distance < data->distance) {
            data->distance = distance;
            data->building_id = b->id;
        }
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
    if (temple->data.market.inventory[INVENTORY_WINE] < 50 && grand_temple->loads_stored > 0) {
        temple->data.market.fetch_inventory_id = INVENTORY_WINE;
        return grand_temple->id;
    }
    return 0;  
}

static int food_threshhold(building *b) {
    return b->data.market.is_mess_hall ? 1200 : 600;
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

static int should_fetch_inventory(building *market, short inventory, struct resource_data *data, int min_amount)
{
    if (!min_amount) {
        min_amount = 1;
    }
    return
        market->data.market.inventory[inventory] < min_amount &&
        data[inventory].num_buildings &&
        is_good_accepted(inventory, market);
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
        }

        if (b->type == BUILDING_GRAND_TEMPLE_VENUS && building_is_venus_temple(market->type) && b->road_network_id == market->road_network_id) {
            building *gt = building_get(building_monument_get_venus_gt());
            return building_venus_temple_get_wine_destination(market, gt);
        }

        if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != market->road_network_id) {
            continue;
        }
        if (!building_storage_get_permission(permission, b)) {
            continue;
        }
        int distance = calc_maximum_distance(market->x, market->y, b->x, b->y);
        if (distance >= 40) {
            continue;
        }
        if (b->type == BUILDING_GRANARY) {
            if (scenario_property_rome_supplies_wheat()) {
                continue;
            }
            update_food_resource(&resources[INVENTORY_WHEAT], RESOURCE_WHEAT, b, distance);
            update_food_resource(&resources[INVENTORY_VEGETABLES], RESOURCE_VEGETABLES, b, distance);
            update_food_resource(&resources[INVENTORY_FRUIT], RESOURCE_FRUIT, b, distance);
            update_food_resource(&resources[INVENTORY_MEAT], RESOURCE_MEAT, b, distance);
        } else if (b->type == BUILDING_WAREHOUSE) {
            // goods
            update_good_resource(&resources[INVENTORY_WINE], RESOURCE_WINE, b, distance);
            update_good_resource(&resources[INVENTORY_OIL], RESOURCE_OIL, b, distance);
            update_good_resource(&resources[INVENTORY_POTTERY], RESOURCE_POTTERY, b, distance);
            update_good_resource(&resources[INVENTORY_FURNITURE], RESOURCE_FURNITURE, b, distance);
        }
    }

    if (!market->data.market.pottery_demand) {
        resources[INVENTORY_POTTERY].num_buildings = 0;
    }
    if (!market->data.market.furniture_demand) {
        resources[INVENTORY_FURNITURE].num_buildings = 0;
    }
    if (!market->data.market.oil_demand) {
        resources[INVENTORY_OIL].num_buildings = 0;
    }
    if (!market->data.market.wine_demand) {
        resources[INVENTORY_WINE].num_buildings = 0;
    }

    int can_go = 0;
    int min_stock = 50;

    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].num_buildings) {
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
        if (should_fetch_inventory(market, inventory, resources, food_threshhold(market))) {
            market->data.market.fetch_inventory_id = inventory;
            return resources[inventory].building_id;
        }
        if (should_fetch_inventory(market, INVENTORY_OIL, resources, min_stock)) {
            market->data.market.fetch_inventory_id = INVENTORY_OIL;
            return resources[INVENTORY_OIL].building_id;
        }
        return 0;
    }

    // Tavern
    if (market->type == BUILDING_TAVERN) {
        if (should_fetch_inventory(market, INVENTORY_WINE, resources, min_stock)) {
            market->data.market.fetch_inventory_id = INVENTORY_WINE;
            return resources[INVENTORY_WINE].building_id;
        }
        if (market->data.market.inventory[INVENTORY_WINE] >= min_stock && 
            should_fetch_inventory(market, INVENTORY_MEAT, resources, food_threshhold(market))) {
            market->data.market.fetch_inventory_id = INVENTORY_MEAT;
            return resources[INVENTORY_MEAT].building_id;
        }
        return 0;
    }

    // prefer food if we don't have it
    if (should_fetch_inventory(market, INVENTORY_WHEAT, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_WHEAT;
        return resources[INVENTORY_WHEAT].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_VEGETABLES, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_VEGETABLES;
        return resources[INVENTORY_VEGETABLES].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_FRUIT, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_FRUIT;
        return resources[INVENTORY_FRUIT].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_MEAT, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_MEAT;
        return resources[INVENTORY_MEAT].building_id;
    }
    // then prefer resource if we don't have it
    if (should_fetch_inventory(market, INVENTORY_POTTERY, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_POTTERY;
        return resources[INVENTORY_POTTERY].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_FURNITURE, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_FURNITURE;
        return resources[INVENTORY_FURNITURE].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_OIL, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_OIL;
        return resources[INVENTORY_OIL].building_id;
    } else if (should_fetch_inventory(market, INVENTORY_WINE, resources, 0)) {
        market->data.market.fetch_inventory_id = INVENTORY_WINE;
        return resources[INVENTORY_WINE].building_id;
    }
    // then prefer smallest stock below 50
    
    int fetch_inventory = -1;
    if (should_fetch_inventory(market, INVENTORY_WHEAT, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_WHEAT];
        fetch_inventory = INVENTORY_WHEAT;
    }
    if (should_fetch_inventory(market, INVENTORY_VEGETABLES, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_VEGETABLES];
        fetch_inventory = INVENTORY_VEGETABLES;
    }
    if (should_fetch_inventory(market, INVENTORY_FRUIT, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_FRUIT];
        fetch_inventory = INVENTORY_FRUIT;
    }
    if (should_fetch_inventory(market, INVENTORY_MEAT, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_MEAT];
        fetch_inventory = INVENTORY_MEAT;
    }
    if (should_fetch_inventory(market, INVENTORY_POTTERY, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_POTTERY];
        fetch_inventory = INVENTORY_POTTERY;
    }
    if (should_fetch_inventory(market, INVENTORY_FURNITURE, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_FURNITURE];
        fetch_inventory = INVENTORY_FURNITURE;
    }
    if (should_fetch_inventory(market, INVENTORY_OIL, resources, min_stock)) {
        min_stock = market->data.market.inventory[INVENTORY_OIL];
        fetch_inventory = INVENTORY_OIL;
    }
    if (should_fetch_inventory(market, INVENTORY_WINE, resources, min_stock)) {
        fetch_inventory = INVENTORY_WINE;
    }

    if (fetch_inventory == -1) {
        int food_limit = food_threshhold(market);
        // all items well stocked: pick food below threshold
        if (should_fetch_inventory(market, INVENTORY_WHEAT, resources, food_limit)) {
            fetch_inventory = INVENTORY_WHEAT;
        }
        if (should_fetch_inventory(market, INVENTORY_VEGETABLES, resources, food_limit)) {
            fetch_inventory = INVENTORY_VEGETABLES;
        }
        if (should_fetch_inventory(market, INVENTORY_FRUIT, resources, food_limit)) {
            fetch_inventory = INVENTORY_FRUIT;
        }
        if (should_fetch_inventory(market, INVENTORY_MEAT, resources, food_limit)) {
            fetch_inventory = INVENTORY_MEAT;
        }
    }
    if (fetch_inventory < 0) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    return resources[fetch_inventory].building_id;
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
