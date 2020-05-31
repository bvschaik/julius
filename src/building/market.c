#include "market.h"

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

int building_market_get_storage_destination(building *market)
{
    struct resource_data resources[INVENTORY_MAX];
    for (int i = 0; i < INVENTORY_MAX; i++) {
        resources[i].building_id = 0;
        resources[i].num_buildings = 0;
        resources[i].distance = 40;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE) {
            continue;
        }
        if (b->type != BUILDING_GRANARY && b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0 ||
            b->road_network_id != market->road_network_id) {
            continue;
        }
        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_MARKET,b)) {
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

    // update demands
    if (market->data.market.pottery_demand) {
        market->data.market.pottery_demand--;
    } else {
        resources[INVENTORY_POTTERY].num_buildings = 0;
    }
    if (market->data.market.furniture_demand) {
        market->data.market.furniture_demand--;
    } else {
        resources[INVENTORY_FURNITURE].num_buildings = 0;
    }
    if (market->data.market.oil_demand) {
        market->data.market.oil_demand--;
    } else {
        resources[INVENTORY_OIL].num_buildings = 0;
    }
    if (market->data.market.wine_demand) {
        market->data.market.wine_demand--;
    } else {
        resources[INVENTORY_WINE].num_buildings = 0;
    }

    int can_go = 0;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (resources[i].num_buildings) {
            can_go = 1;
            break;
        }
    }
    if (!can_go) {
        return 0;
    }
    // prefer food if we don't have it
    if (!market->data.market.inventory[INVENTORY_WHEAT] && resources[INVENTORY_WHEAT].num_buildings && is_good_accepted(INVENTORY_WHEAT, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_WHEAT;
        return resources[INVENTORY_WHEAT].building_id;
    } else if (!market->data.market.inventory[INVENTORY_VEGETABLES] && resources[INVENTORY_VEGETABLES].num_buildings && is_good_accepted(INVENTORY_VEGETABLES, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_VEGETABLES;
        return resources[INVENTORY_VEGETABLES].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FRUIT] && resources[INVENTORY_FRUIT].num_buildings && is_good_accepted(INVENTORY_FRUIT, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FRUIT;
        return resources[INVENTORY_FRUIT].building_id;
    } else if (!market->data.market.inventory[INVENTORY_MEAT] && resources[INVENTORY_MEAT].num_buildings && is_good_accepted(INVENTORY_MEAT, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_MEAT;
        return resources[INVENTORY_MEAT].building_id;
    }
    // then prefer resource if we don't have it
    if (!market->data.market.inventory[INVENTORY_POTTERY] && resources[INVENTORY_POTTERY].num_buildings && is_good_accepted(INVENTORY_POTTERY, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_POTTERY;
        return resources[INVENTORY_POTTERY].building_id;
    } else if (!market->data.market.inventory[INVENTORY_FURNITURE] && resources[INVENTORY_FURNITURE].num_buildings && is_good_accepted(INVENTORY_FURNITURE, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_FURNITURE;
        return resources[INVENTORY_FURNITURE].building_id;
    } else if (!market->data.market.inventory[INVENTORY_OIL] && resources[INVENTORY_OIL].num_buildings && is_good_accepted(INVENTORY_OIL, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_OIL;
        return resources[INVENTORY_OIL].building_id;
    } else if (!market->data.market.inventory[INVENTORY_WINE] && resources[INVENTORY_WINE].num_buildings && is_good_accepted(INVENTORY_WINE, market)) {
        market->data.market.fetch_inventory_id = INVENTORY_WINE;
        return resources[INVENTORY_WINE].building_id;
    }
    // then prefer smallest stock below 50
    int min_stock = 50;
    int fetch_inventory = -1;
    if (resources[INVENTORY_WHEAT].num_buildings &&
        market->data.market.inventory[INVENTORY_WHEAT] < min_stock
	&& is_good_accepted(INVENTORY_WHEAT, market)) {
        min_stock = market->data.market.inventory[INVENTORY_WHEAT];
        fetch_inventory = INVENTORY_WHEAT;
    }
    if (resources[INVENTORY_VEGETABLES].num_buildings &&
        market->data.market.inventory[INVENTORY_VEGETABLES] < min_stock
	&& is_good_accepted(INVENTORY_VEGETABLES, market)) {
        min_stock = market->data.market.inventory[INVENTORY_VEGETABLES];
        fetch_inventory = INVENTORY_VEGETABLES;
    }
    if (resources[INVENTORY_FRUIT].num_buildings &&
        market->data.market.inventory[INVENTORY_FRUIT] < min_stock
        && is_good_accepted(INVENTORY_FRUIT, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FRUIT];
        fetch_inventory = INVENTORY_FRUIT;
    }
    if (resources[INVENTORY_MEAT].num_buildings &&
        market->data.market.inventory[INVENTORY_MEAT] < min_stock
	&& is_good_accepted(INVENTORY_MEAT, market)) {
        min_stock = market->data.market.inventory[INVENTORY_MEAT];
        fetch_inventory = INVENTORY_MEAT;
    }
    if (resources[INVENTORY_POTTERY].num_buildings &&
        market->data.market.inventory[INVENTORY_POTTERY] < min_stock
	&& is_good_accepted(INVENTORY_POTTERY, market)) {
        min_stock = market->data.market.inventory[INVENTORY_POTTERY];
        fetch_inventory = INVENTORY_POTTERY;
    }
    if (resources[INVENTORY_FURNITURE].num_buildings &&
        market->data.market.inventory[INVENTORY_FURNITURE] < min_stock
	&& is_good_accepted(INVENTORY_FURNITURE, market)) {
        min_stock = market->data.market.inventory[INVENTORY_FURNITURE];
        fetch_inventory = INVENTORY_FURNITURE;
    }
    if (resources[INVENTORY_OIL].num_buildings &&
        market->data.market.inventory[INVENTORY_OIL] < min_stock
	&& is_good_accepted(INVENTORY_OIL, market)) {
        min_stock = market->data.market.inventory[INVENTORY_OIL];
        fetch_inventory = INVENTORY_OIL;
    }
    if (resources[INVENTORY_WINE].num_buildings &&
        market->data.market.inventory[INVENTORY_WINE] < min_stock
	&& is_good_accepted(INVENTORY_WINE, market)) {
        fetch_inventory = INVENTORY_WINE;
    }

    if (fetch_inventory == -1) {
        // all items well stocked: pick food below threshold
        if (resources[INVENTORY_WHEAT].num_buildings &&
            market->data.market.inventory[INVENTORY_WHEAT] < 600
	    && is_good_accepted(INVENTORY_WHEAT, market)) {
            fetch_inventory = INVENTORY_WHEAT;
        }
        if (resources[INVENTORY_VEGETABLES].num_buildings &&
            market->data.market.inventory[INVENTORY_VEGETABLES] < 400
	    && is_good_accepted(INVENTORY_VEGETABLES, market)) {
            fetch_inventory = INVENTORY_VEGETABLES;
        }
        if (resources[INVENTORY_FRUIT].num_buildings &&
            market->data.market.inventory[INVENTORY_FRUIT] < 400
	    && is_good_accepted(INVENTORY_FRUIT, market)) {
            fetch_inventory = INVENTORY_FRUIT;
        }
        if (resources[INVENTORY_MEAT].num_buildings &&
            market->data.market.inventory[INVENTORY_MEAT] < 400
	    && is_good_accepted(INVENTORY_MEAT, market)) {
            fetch_inventory = INVENTORY_MEAT;
        }
    }
    if (fetch_inventory < 0) {
        return 0;
    }
    market->data.market.fetch_inventory_id = fetch_inventory;
    return resources[fetch_inventory].building_id;
}
