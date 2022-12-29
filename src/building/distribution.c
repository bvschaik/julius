#include "distribution.h"

#include "building/properties.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"

#include <string.h>

int building_distribution_is_good_accepted(resource_type resource, const building *b)
{
    return b->accepted_goods[resource] > 0;
}

void building_distribution_toggle_good_accepted(resource_type resource, building *b)
{
    if (b->accepted_goods[resource] == 0) {
        b->accepted_goods[resource] = 1;
    } else {
        b->accepted_goods[resource] = 0;
    }
}

void building_distribution_unaccept_all_goods(building *b)
{
    memset(b->accepted_goods, 0, sizeof(b->accepted_goods));
}

void building_distribution_update_demands(building *b)
{
    for (resource_type resource = RESOURCE_MIN; resource < RESOURCE_MAX; resource++) {
        if (!resource_is_good(resource) || !resource_get_data(resource)->is_inventory) {
            continue;
        }
        if (b->accepted_goods[resource] > 1) {
            b->accepted_goods[resource]--;
        }
    }
}

resource_type building_distribution_fetch(const building *b, inventory_storage_info *info,
    int min_stock, int pick_first, const int allowed[RESOURCE_MAX])
{
    resource_type resource = RESOURCE_NONE;
    if (!min_stock) {
        min_stock = 1;
    }
    for (resource_type current_resource = RESOURCE_MIN; current_resource < RESOURCE_MAX; current_resource++) {
        if (!resource_get_data(current_resource)->is_inventory) {
            continue;
        }
        if (allowed[current_resource] &&
            info[current_resource].building_id && b->resources[current_resource] < min_stock) {
            if (pick_first) {
                return current_resource;
            }
            min_stock = b->resources[current_resource];
            resource = current_resource;
        }
    }
    return resource;
}

static void update_food_resource(inventory_storage_info *info, resource_type resource, const building *b, int distance)
{
    if (distance < info[resource].min_distance && b->resources[resource]) {
        info[resource].min_distance = distance;
        info[resource].building_id = b->id;
    }
}

static void update_good_resource(inventory_storage_info *info, resource_type resource, building *b, int distance)
{
    if (distance < info[resource].min_distance &&
        !city_resource_is_stockpiled(resource) && building_warehouse_get_amount(b, resource) > 0) {
        info[resource].min_distance = distance;
        info[resource].building_id = b->id;
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
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        info[r].min_distance = max_distance;
        info[r].building_id = 0;
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

        for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
            if (resource_get_data(r)->is_inventory) {
                update_food_resource(info, r, b, distance);
            }
        }
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        for (int r = RESOURCE_MIN_GOOD; r < RESOURCE_MAX_GOOD; r++) {
            if (resource_get_data(r)->is_inventory) {
                update_good_resource(info, r, b, distance);
            }
        }
    }

    for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
        if (info[i].building_id) {
            return 1;
        }
    }
    return 0;
}

static int building_distribution_get_raw_material_storages(inventory_storage_info *info, building_type type,
    int road_network, int x, int y, int w, int h, int max_distance)
{
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        info[r].min_distance = max_distance;
        info[r].building_id = 0;
    }

    int permission = BUILDING_STORAGE_PERMISSION_MARKET;

    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        for (resource_type r = RESOURCE_MAX_FOOD; r < RESOURCE_MAX; r++) {
            if (resource_is_raw_material(r)) {
                update_good_resource(info, r, b, distance);
            }
        }
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
    return building_distribution_get_inventory_storages(info, start->type, start->road_network_id,
        start->x, start->y, size, size, max_distance);
}

int building_distribution_get_raw_material_storages_for_building(inventory_storage_info *info, building *start, int max_distance)
{
    int size = building_properties_for_type(start->type)->size;
    return building_distribution_get_raw_material_storages(info, start->type, start->road_network_id,
        start->x, start->y, size, size, max_distance);
}

int building_distribution_get_inventory_storages_for_figure(inventory_storage_info *info, building_type type,
    int road_network, figure *start, int max_distance)
{
    return building_distribution_get_inventory_storages(info, type, road_network,
        start->x, start->y, 1, 1, max_distance);
}

int building_distribution_get_raw_material_storages_for_figure(inventory_storage_info *info, building_type type,
    int road_network, figure *start, int max_distance)
{
    return building_distribution_get_raw_material_storages(info, type, road_network,
        start->x, start->y, 1, 1, max_distance);
}
