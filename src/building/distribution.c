#include "distribution.h"

#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"

#include <string.h>

static inventory_data data[INVENTORY_MAX];

static const int INVENTORY_SEARCH_ORDER[INVENTORY_MAX] = {
    INVENTORY_WHEAT, INVENTORY_VEGETABLES, INVENTORY_FRUIT, INVENTORY_MEAT,
    INVENTORY_POTTERY, INVENTORY_FURNITURE, INVENTORY_OIL, INVENTORY_WINE
};

int building_distribution_fetch(const building *b, inventory_data *data, int min_stock, int pick_first, int allowed)
{
    int inventory = INVENTORY_NONE;
    if (!min_stock) {
        min_stock = 1;
    }
    for (int i = 0; i < INVENTORY_MAX; i++) {
        int current_inventory = INVENTORY_SEARCH_ORDER[i];
        if (inventory_is_set(allowed, current_inventory) &&
            data[current_inventory].building_id && b->data.market.inventory[current_inventory] < min_stock) {
            if (pick_first) {
                return current_inventory;
            }
            min_stock = b->data.market.inventory[current_inventory];
            inventory = current_inventory;
        }
    }
    return inventory;
}

static void update_food_resource(inventory_data *data, resource_type resource, const building *b, int distance)
{
    if (distance < data->min_distance && b->data.granary.resource_stored[resource]) {
        data->min_distance = distance;
        data->building_id = b->id;
    }
}

static void update_good_resource(inventory_data *data, resource_type resource, building *b, int distance)
{
    if (distance < data->min_distance &&
        !city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        data->min_distance = distance;
        data->building_id = b->id;
    }
}

inventory_data *building_distribution_get_inventory_data(building *b, int needed_inventory, int max_distance)
{
    memset(data, 0, sizeof(data));

    int can_go;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (inventory_is_set(needed_inventory, i)) {
            data[i].min_distance = max_distance;
            can_go = 1;
        }
    }
    if (!can_go) {
        return 0;
    }

    int permission; 
    if (b->type == BUILDING_MESS_HALL) {
        permission = BUILDING_STORAGE_PERMISSION_QUARTERMASTER;
    } else {
        permission = BUILDING_STORAGE_PERMISSION_MARKET;
    }

    for (int i = 1; i < building_count(); i++) {
        building *b_dst = building_get(i);

        if (b_dst->state != BUILDING_STATE_IN_USE ||
            (b_dst->type != BUILDING_GRANARY && b_dst->type != BUILDING_WAREHOUSE) ||
            !b_dst->has_road_access || b_dst->distance_from_entry <= 0 ||
            b_dst->road_network_id != b->road_network_id ||
            !building_storage_get_permission(permission, b_dst)) {
            continue;
        }
        int distance = calc_maximum_distance(b->x, b->y, b_dst->x, b_dst->y);
        if (b_dst->type == BUILDING_GRANARY) {
            update_food_resource(&data[INVENTORY_WHEAT], RESOURCE_WHEAT, b_dst, distance);
            update_food_resource(&data[INVENTORY_VEGETABLES], RESOURCE_VEGETABLES, b_dst, distance);
            update_food_resource(&data[INVENTORY_FRUIT], RESOURCE_FRUIT, b_dst, distance);
            update_food_resource(&data[INVENTORY_MEAT], RESOURCE_MEAT, b_dst, distance);
        } else {
            update_good_resource(&data[INVENTORY_WINE], RESOURCE_WINE, b_dst, distance);
            update_good_resource(&data[INVENTORY_OIL], RESOURCE_OIL, b_dst, distance);
            update_good_resource(&data[INVENTORY_POTTERY], RESOURCE_POTTERY, b_dst, distance);
            update_good_resource(&data[INVENTORY_FURNITURE], RESOURCE_FURNITURE, b_dst, distance);
        }
    }

    can_go = 0;
    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (data[i].building_id) {
            can_go = 1;
            break;
        }
    }
    return can_go ? data : 0;
}
