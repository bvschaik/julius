#include "distribution.h"

#include "building/properties.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/calc.h"
#include "empire/city.h"

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
    for (resource_type resource = RESOURCE_MIN_NON_FOOD; resource < RESOURCE_MAX_NON_FOOD; resource++) {
        if (resource_is_inventory(resource) && b->accepted_goods[resource] > 1) {
            b->accepted_goods[resource]--;
        }
    }
}

int building_distribution_resource_is_handled(resource_type resource, building_type type)
{
    if (!empire_can_produce_resource_potentially(resource) && !empire_can_import_resource_potentially(resource)) {
        return 0;
    }
    if (type == BUILDING_DOCK) {
        return 1;
    }
    if (!resource_is_inventory(resource)) {
        return 0;
    }
    switch (type) {
        case BUILDING_MARKET:
            return 1;
        case BUILDING_CARAVANSERAI:
        case BUILDING_MESS_HALL:
            return resource_is_food(resource);
        case BUILDING_TAVERN:
            return resource == RESOURCE_WINE || resource == RESOURCE_MEAT || resource == RESOURCE_FISH;
        case BUILDING_LIGHTHOUSE:
            return resource == RESOURCE_TIMBER;
        case BUILDING_SMALL_TEMPLE_VENUS:
        case BUILDING_LARGE_TEMPLE_VENUS:
            return resource == RESOURCE_WINE;
        case BUILDING_SMALL_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_CERES:
            return resource == city_resource_ceres_temple_food() || resource == RESOURCE_OIL;
        default:
            return 0;
    }
}

int building_distribution_get_handled_resources_for_building(const building *b,
    resource_storage_info info[RESOURCE_MAX])
{
    int needed = 0;
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        info[r].needed = building_distribution_resource_is_handled(r, b->type) &&
            building_distribution_is_good_accepted(r, b);
        if (!needed && info[r].needed) {
            needed = 1;
        }
    }
    return needed;
}

resource_type building_distribution_fetch(const building *b, const resource_storage_info info[RESOURCE_MAX],
    int min_stock, int pick_first)
{
    resource_type resource = RESOURCE_NONE;
    if (!min_stock) {
        min_stock = 1;
    }
    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (info[r].needed && info[r].building_id && b->resources[r] < min_stock) {
            if (pick_first) {
                return r;
            }
            min_stock = b->resources[r];
            resource = r;
        }
    }
    return resource;
}

static void update_food_resource(resource_storage_info *info, resource_type resource, const building *b, int distance)
{
    if (distance < info[resource].min_distance && b->resources[resource]) {
        info[resource].min_distance = distance;
        info[resource].building_id = b->id;
    }
}

static void update_good_resource(resource_storage_info *info, resource_type resource, building *b, int distance)
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

static int is_food_needed(const resource_storage_info info[RESOURCE_MAX])
{
    for (resource_type r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
        if (info[r].needed) {
            return 1;
        }
    }
    return 0;
}

static int building_distribution_get_resource_storages(resource_storage_info info[RESOURCE_MAX],
    building_type type, int road_network, int x, int y, int w, int h, int max_distance)
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

    if (is_food_needed(info)) {
        for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
            // Looter walkers have no type
            if (type && is_invalid_destination(b, permission, road_network)) {
                continue;
            }
            int distance = building_dist(x, y, w, h, b);

            for (int r = RESOURCE_MIN_FOOD; r < RESOURCE_MAX_FOOD; r++) {
                if (info[r].needed) {
                    update_food_resource(info, r, b, distance);
                }
            }
        }
    }
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (type && is_invalid_destination(b, permission, road_network)) {
            continue;
        }
        int distance = building_dist(x, y, w, h, b);

        for (resource_type r = RESOURCE_MIN_NON_FOOD; r < RESOURCE_MAX_NON_FOOD; r++) {
            if (info[r].needed) {
                update_good_resource(info, r, b, distance);
            }
        }
    }

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (info[r].building_id) {
            return 1;
        }
    }
    return 0;
}

int building_distribution_get_resource_storages_for_building(resource_storage_info info[RESOURCE_MAX], building *start, int max_distance)
{
    int size = building_properties_for_type(start->type)->size;
    return building_distribution_get_resource_storages(info, start->type, start->road_network_id,
        start->x, start->y, size, size, max_distance);
}

int building_distribution_get_resource_storages_for_figure(resource_storage_info info[RESOURCE_MAX], building_type type,
    int road_network, figure *start, int max_distance)
{
    return building_distribution_get_resource_storages(info, type, road_network,
        start->x, start->y, 1, 1, max_distance);
}
