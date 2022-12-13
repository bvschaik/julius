#include "distribution.h"

#include "building/properties.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"

#include <string.h>

static const int INVENTORY_SEARCH_ORDER[INVENTORY_MAX] = {
    INVENTORY_WHEAT, INVENTORY_VEGETABLES, INVENTORY_FRUIT, INVENTORY_MEAT,
    INVENTORY_POTTERY, INVENTORY_FURNITURE, INVENTORY_OIL, INVENTORY_WINE
};

int building_distribution_is_good_accepted(inventory_type resource, building *b)
{
    int goods_bit = 1 << resource;
    return !(b->subtype.market_goods & goods_bit);
}

void building_distribution_toggle_good_accepted(inventory_type resource, building *b)
{
    int goods_bit = 1 << resource;
    b->subtype.market_goods ^= goods_bit;
}

void building_distribution_unaccept_all_goods(building *b)
{
    b->subtype.market_goods = 0xffff;
}

void building_distribution_update_demands(building *b)
{
    if (b->data.market.pottery_demand) {
        b->data.market.pottery_demand--;
    }
    if (b->data.market.furniture_demand) {
        b->data.market.furniture_demand--;
    }
    if (b->data.market.oil_demand) {
        b->data.market.oil_demand--;
    }
    if (b->data.market.wine_demand) {
        b->data.market.wine_demand--;
    }
}

int building_distribution_fetch(const building *b, inventory_storage_info *info,
    int min_stock, int pick_first, int allowed)
{
    int inventory = INVENTORY_NONE;
    if (!min_stock) {
        min_stock = 1;
    }
    for (int i = 0; i < INVENTORY_MAX; i++) {
        int current_inventory = INVENTORY_SEARCH_ORDER[i];
        if (inventory_is_set(allowed, current_inventory) &&
            info[current_inventory].building_id && b->data.market.inventory[current_inventory] < min_stock) {
            if (pick_first) {
                return current_inventory;
            }
            min_stock = b->data.market.inventory[current_inventory];
            inventory = current_inventory;
        }
    }
    return inventory;
}

static void update_food_resource(inventory_storage_info *info, resource_type resource, const building *b, int distance)
{
    if (distance < info->min_distance && b->data.granary.resource_stored[resource]) {
        info->min_distance = distance;
        info->building_id = b->id;
    }
}

static void update_good_resource(inventory_storage_info *info, resource_type resource, building *b, int distance)
{
    if (distance < info->min_distance &&
        !city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        info->min_distance = distance;
        info->building_id = b->id;
    }
}

static int is_invalid_destination(building *b, int permission, int road_network)
{
    return (b->state != BUILDING_STATE_IN_USE ||
        !b->has_road_access || b->distance_from_entry <= 0 ||
        b->road_network_id != road_network ||
        !building_storage_get_permission(permission, b));
}

static int building_distribution_get_inventory_storages(inventory_storage_info *info, building_type type,
    int road_network, int x, int y, int w, int h, int max_distance)
{
    for (int i = 0; i < INVENTORY_MAX; i++) {
        info[i].min_distance = max_distance;
        info[i].building_id = 0;
    }

    int permission;

    if (type == BUILDING_MESS_HALL) {
        permission = BUILDING_STORAGE_PERMISSION_QUARTERMASTER;
    } else {
        permission = BUILDING_STORAGE_PERMISSION_MARKET;
    }

    for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
        // Looter walkers have no type
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        update_food_resource(&info[INVENTORY_WHEAT], RESOURCE_WHEAT, b, distance);
        update_food_resource(&info[INVENTORY_VEGETABLES], RESOURCE_VEGETABLES, b, distance);
        update_food_resource(&info[INVENTORY_FRUIT], RESOURCE_FRUIT, b, distance);
        update_food_resource(&info[INVENTORY_MEAT], RESOURCE_MEAT, b, distance);
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        update_good_resource(&info[INVENTORY_WINE], RESOURCE_WINE, b, distance);
        update_good_resource(&info[INVENTORY_OIL], RESOURCE_OIL, b, distance);
        update_good_resource(&info[INVENTORY_POTTERY], RESOURCE_POTTERY, b, distance);
        update_good_resource(&info[INVENTORY_FURNITURE], RESOURCE_FURNITURE, b, distance);
    }

    for (int i = 0; i < INVENTORY_MAX; i++) {
        if (info[i].building_id) {
            return 1;
        }
    }
    return 0;
}

static int building_distribution_get_raw_material_storages(inventory_storage_info *info, building_type type,
    int road_network, int x, int y, int w, int h, int max_distance)
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        info[i].min_distance = max_distance;
        info[i].building_id = 0;
    }

    int permission = BUILDING_STORAGE_PERMISSION_MARKET;

    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        update_good_resource(&info[RESOURCE_IRON], RESOURCE_IRON, b, distance);
        update_good_resource(&info[RESOURCE_TIMBER], RESOURCE_TIMBER, b, distance);
        update_good_resource(&info[RESOURCE_CLAY], RESOURCE_CLAY, b, distance);
        update_good_resource(&info[RESOURCE_MARBLE], RESOURCE_MARBLE, b, distance);
    }

    for (int i = 0; i < RESOURCE_MAX; i++) {
        if (info[i].building_id) {
            return 1;
        }
    }
    return 0;
}

int building_distribution_get_inventory_storages_for_building(inventory_storage_info *info, building *start, int max_distance) 
{
    int size = building_properties_for_type(start->type)->size;
    return building_distribution_get_inventory_storages(info, start->type, start->road_network_id, start->x, start->y, size, size, max_distance);
}

int building_distribution_get_raw_material_storages_for_building(inventory_storage_info *info, building *start, int max_distance)
{
    int size = building_properties_for_type(start->type)->size;
    return building_distribution_get_raw_material_storages(info, start->type, start->road_network_id, start->x, start->y, size, size, max_distance);
}

int building_distribution_get_inventory_storages_for_figure(inventory_storage_info *info, building_type type, int road_network, figure *start, int max_distance)
{
    return building_distribution_get_inventory_storages(info, type, road_network, start->x, start->y, 1, 1, max_distance);
}

int building_distribution_get_raw_material_storages_for_figure(inventory_storage_info *info, building_type type, int road_network, figure *start, int max_distance)
{
    return building_distribution_get_raw_material_storages(info, type, road_network, start->x, start->y, 1, 1, max_distance);
}