#include "warehouse.h"

#include "building/count.h"
#include "building/model.h"
#include "building/storage.h"
#include "city/finance.h"
#include "core/calc.h"
#include "empire/trade_prices.h"
#include "game/tutorial.h"
#include "graphics/image.h"
#include "map/image.h"
#include "map/road_access.h"
#include "scenario/property.h"

#include "Data/CityInfo.h"

int building_warehouse_get_space_info(building *warehouse)
{
    int total_loads = 0;
    int empty_spaces = 0;
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id <= 0) {
            return 0;
        }
        if (space->subtype.warehouseResourceId) {
            total_loads += space->loadsStored;
        } else {
            empty_spaces++;
        }
    }
    if (empty_spaces > 0) {
        return WAREHOUSE_ROOM;
    } else if (total_loads < 32) {
        return WAREHOUSE_SOME_ROOM;
    } else {
        return WAREHOUSE_FULL;
    }
}

int building_warehouse_get_amount(building *warehouse, int resource)
{
    int loads = 0;
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id <= 0) {
            return 0;
        }
        if (space->subtype.warehouseResourceId && space->subtype.warehouseResourceId == resource) {
            loads += space->loadsStored;
        }
    }
    return loads;
}

int building_warehouse_add_resource(building *b, int resource)
{
    if (b->id <= 0) {
        return 0;
    }
    // check building itself
    int find_space = 0;
    if (b->subtype.warehouseResourceId && b->subtype.warehouseResourceId != resource) {
        find_space = 1;
    } else if (b->loadsStored >= 4) {
        find_space = 1;
    } else if (b->type == BUILDING_WAREHOUSE) {
        find_space = 1;
    }
    if (find_space) {
        int space_found = 0;
        building *space = building_main(b);
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (!space->id) {
                return 0;
            }
            if (!space->subtype.warehouseResourceId || space->subtype.warehouseResourceId == resource) {
                if (space->loadsStored < 4) {
                    space_found = 1;
                    b = space;
                    break;
                }
            }
        }
        if (!space_found) {
            return 0;
        }
    }
    Data_CityInfo.resourceSpaceInWarehouses[resource]--;
    Data_CityInfo.resourceStored[resource]++;
    b->subtype.warehouseResourceId = resource;
    b->loadsStored++;
    tutorial_on_add_to_warehouse();
    building_warehouse_space_set_image(b, resource);
    return 1;
}

int building_warehouse_remove_resource(building *warehouse, int resource, int amount)
{
    // returns amount still needing removal
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return amount;
    }
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        if (amount <= 0) {
            return 0;
        }
        space = building_next(space);
        if (space->id <= 0) {
            continue;
        }
        if (space->subtype.warehouseResourceId != resource || space->loadsStored <= 0) {
            continue;
        }
        if (space->loadsStored > amount) {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
            Data_CityInfo.resourceStored[resource] -= amount;
            space->loadsStored -= amount;
            amount = 0;
        } else {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += space->loadsStored;
            Data_CityInfo.resourceStored[resource] -= space->loadsStored;
            amount -= space->loadsStored;
            space->loadsStored = 0;
            space->subtype.warehouseResourceId = RESOURCE_NONE;
        }
        building_warehouse_space_set_image(space, resource);
    }
    return amount;
}

void building_warehouse_remove_resource_curse(building *warehouse, int amount)
{
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return;
    }
    building *space = warehouse;
    for (int i = 0; i < 8 && amount > 0; i++) {
        space = building_next(space);
        if (space->id <= 0 || space->loadsStored <= 0) {
            continue;
        }
        int resource = space->subtype.warehouseResourceId;
        if (space->loadsStored > amount) {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += amount;
            Data_CityInfo.resourceStored[resource] -= amount;
            space->loadsStored -= amount;
            amount = 0;
        } else {
            Data_CityInfo.resourceSpaceInWarehouses[resource] += space->loadsStored;
            Data_CityInfo.resourceStored[resource] -= space->loadsStored;
            amount -= space->loadsStored;
            space->loadsStored = 0;
            space->subtype.warehouseResourceId = RESOURCE_NONE;
        }
        building_warehouse_space_set_image(space, resource);
    }
}

void building_warehouse_space_set_image(building *space, int resource)
{
    int image_id;
    if (space->loadsStored <= 0) {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
    } else {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) +
            4 * (resource - 1) + resource_image_offset(resource, RESOURCE_IMAGE_STORAGE) +
                   space->loadsStored - 1;
    }
    map_image_set(space->gridOffset, image_id);
}

void building_warehouse_space_add_import(building *space, int resource)
{
    Data_CityInfo.resourceSpaceInWarehouses[resource]--;
    Data_CityInfo.resourceStored[resource]++;
    space->loadsStored++;
    space->subtype.warehouseResourceId = resource;
    
    int price = trade_price_buy(resource);
    city_finance_process_import(price);
    
    building_warehouse_space_set_image(space, resource);
}

void building_warehouse_space_remove_export(building *space, int resource)
{
    Data_CityInfo.resourceSpaceInWarehouses[resource]++;
    Data_CityInfo.resourceStored[resource]--;
    space->loadsStored--;
    if (space->loadsStored <= 0) {
        space->subtype.warehouseResourceId = RESOURCE_NONE;
    }
    
    int price = trade_price_sell(resource);
    city_finance_process_export(price);
    
    building_warehouse_space_set_image(space, resource);
}



void building_warehouses_add_resource(int resource, int amount)
{
    int building_id = Data_CityInfo.resourceLastTargetWarehouse;
    for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            Data_CityInfo.resourceLastTargetWarehouse = building_id;
            while (amount && building_warehouse_add_resource(b, resource)) {
                amount--;
            }
        }
    }
}

int building_warehouses_remove_resource(int resource, int amount)
{
    int amount_left = amount;
    int building_id = Data_CityInfo.resourceLastTargetWarehouse;
    // first go for non-getting warehouses
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            if (building_storage_get(b->storage_id)->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
                Data_CityInfo.resourceLastTargetWarehouse = building_id;
                amount_left = building_warehouse_remove_resource(b, resource, amount_left);
            }
        }
    }
    // if that doesn't work, take it anyway
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            Data_CityInfo.resourceLastTargetWarehouse = building_id;
            amount_left = building_warehouse_remove_resource(b, resource, amount_left);
        }
    }
    return amount - amount_left;
}

void building_warehouses_calculate_stocks()
{
    for (int i = 0; i < RESOURCE_MAX; i++) {
        Data_CityInfo.resourceSpaceInWarehouses[i] = 0;
        Data_CityInfo.resourceStored[i] = 0;
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (BuildingIsInUse(b) && b->type == BUILDING_WAREHOUSE) {
            b->hasRoadAccess = 0;
            if (map_has_road_access(b->x, b->y, b->size, 0, 0)) {
                b->hasRoadAccess = 1;
            } else if (map_has_road_access(b->x, b->y, 3, 0, 0)) {
                b->hasRoadAccess = 2;
            }
        }
    }
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE_SPACE) {
            continue;
        }
        building *warehouse = building_main(b);
        if (warehouse->hasRoadAccess) {
            b->hasRoadAccess = warehouse->hasRoadAccess;
            if (b->subtype.warehouseResourceId) {
                int loads = b->loadsStored;
                int resource = b->subtype.warehouseResourceId;
                Data_CityInfo.resourceStored[resource] += loads;
                Data_CityInfo.resourceSpaceInWarehouses[resource] += 4 - loads;
            } else {
                Data_CityInfo.resourceSpaceInWarehouses[RESOURCE_NONE] += 4;
            }
        }
    }
}

int building_warehouse_for_storing(int src_building_id, int x, int y, int resource,
                                   int distance_from_entry, int road_network_id, int *understaffed,
                                   int *x_dst, int *y_dst)
{
    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE_SPACE) {
            continue;
        }
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0 || b->roadNetworkId != road_network_id) {
            continue;
        }
        building *dst = building_main(b);
        if (src_building_id == dst->id) {
            continue;
        }
        const building_storage *s = building_storage_get(dst->storage_id);
        if (s->resource_state[resource] == BUILDING_STORAGE_STATE_NOT_ACCEPTING || s->empty_all) {
            continue;
        }
        int pct_workers = calc_percentage(dst->numWorkers, model_get_building(dst->type)->laborers);
        if (pct_workers < 100) {
            if (understaffed) {
                *understaffed += 1;
            }
            continue;
        }
        int dist;
        if (b->subtype.warehouseResourceId == RESOURCE_NONE) { // empty warehouse space
            dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distanceFromEntry);
        } else if (b->subtype.warehouseResourceId == resource && b->loadsStored < 4) {
            dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distanceFromEntry);
        } else {
            dist = 0;
        }
        if (dist > 0 && dist < min_dist) {
            min_dist = dist;
            min_building_id = i;
        }
    }
    building *b = building_main(building_get(min_building_id));
    if (b->hasRoadAccess == 1) {
        *x_dst = b->x;
        *y_dst = b->y;
    } else if (!map_has_road_access(b->x, b->y, 3, x_dst, y_dst)) {
        return 0;
    }
    return min_building_id;
}

int building_warehouse_for_getting(building *src, int resource, int *x_dst, int *y_dst)
{
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (i == src->id) {
            continue;
        }
        int loads_stored = 0;
        building *space = b;
        const building_storage *s = building_storage_get(b->storage_id);
        for (int t = 0; t < 8; t++) {
            space = building_next(space);
            if (space->id > 0 && space->loadsStored > 0) {
                if (space->subtype.warehouseResourceId == resource) {
                    loads_stored += space->loadsStored;
                }
            }
        }
        if (loads_stored > 0 && s->resource_state[resource] != BUILDING_STORAGE_STATE_GETTING) {
            int dist = calc_distance_with_penalty(b->x, b->y, src->x, src->y,
                                                  src->distanceFromEntry, b->distanceFromEntry);
            dist -= 4 * loads_stored;
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        *x_dst = min_building->roadAccessX;
        *y_dst = min_building->roadAccessY;
        return min_building->id;
    } else {
        return 0;
    }
}

static int workshop_type_for_resource(int resource)
{
    switch (resource) {
        case RESOURCE_OLIVES:
            return WORKSHOP_OLIVES_TO_OIL;
        case RESOURCE_VINES:
            return WORKSHOP_VINES_TO_WINE;
        case RESOURCE_IRON:
            return WORKSHOP_IRON_TO_WEAPONS;
        case RESOURCE_TIMBER:
            return WORKSHOP_TIMBER_TO_FURNITURE;
        case RESOURCE_CLAY:
            return WORKSHOP_CLAY_TO_POTTERY;
        default:
            return WORKSHOP_NONE;
    }
}

static int determine_granary_accept_foods(int resources[RESOURCE_MAX_FOOD])
{
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        resources[i] = 0;
    }
    int can_accept = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
            continue;
        }
        int pct_workers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
        if (pct_workers >= 100 && b->data.storage.resourceStored[RESOURCE_NONE] >= 1200) {
            const building_storage *s = building_storage_get(b->storage_id);
            if (!s->empty_all) {
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
                        resources[r]++;
                        can_accept = 1;
                    }
                }
            }
        }
    }
    return can_accept;
}

static int determine_granary_get_foods(int resources[RESOURCE_MAX_FOOD])
{
    if (scenario_property_rome_supplies_wheat()) {
        return 0;
    }
    for (int i = 0; i < RESOURCE_MAX_FOOD; i++) {
        resources[i] = 0;
    }
    int can_get = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (!BuildingIsInUse(b) || b->type != BUILDING_GRANARY || !b->hasRoadAccess) {
            continue;
        }
        int pct_workers = calc_percentage(b->numWorkers, model_get_building(b->type)->laborers);
        if (pct_workers >= 100 && b->data.storage.resourceStored[RESOURCE_NONE] > 100) {
            const building_storage *s = building_storage_get(b->storage_id);
            if (!s->empty_all) {
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    if (s->resource_state[r] == BUILDING_STORAGE_STATE_GETTING) {
                        resources[r]++;
                        can_get = 1;
                    }
                }
            }
        }
    }
    return can_get;
}

static int contains_non_stockpiled_food(building *space, int *resources)
{
    if (space->id <= 0) {
        return 0;
    }
    if (space->loadsStored <= 0) {
        return 0;
    }
    int resource = space->subtype.warehouseResourceId;
    if (Data_CityInfo.resourceStockpiled[resource]) {
        return 0;
    }
    if (resource == RESOURCE_WHEAT || resource == RESOURCE_VEGETABLES ||
        resource == RESOURCE_FRUIT || resource == RESOURCE_MEAT) {
        if (resources[resource] > 0) {
            return 1;
        }
    }
    return 0;
}

// 0 = getting resource, >0 = resource to deliver
int building_warehouse_determine_worker_task(building *warehouse, int *resource)
{
    int pct_workers = calc_percentage(warehouse->numWorkers, model_get_building(warehouse->type)->laborers);
    if (pct_workers < 50) {
        return -1;
    }
    const building_storage *s = building_storage_get(warehouse->storage_id);
    building *space;
    // get resources
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (s->resource_state[r] != BUILDING_STORAGE_STATE_GETTING || Data_CityInfo.resourceStockpiled[r]) {
            continue;
        }
        int loads_stored = 0;
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0 && space->loadsStored > 0) {
                if (space->subtype.warehouseResourceId == r) {
                    loads_stored += space->loadsStored;
                }
            }
        }
        int room = 0;
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0) {
                if (space->loadsStored <= 0) {
                    room += 4;
                }
                if (space->subtype.warehouseResourceId == r) {
                    room += 4 - space->loadsStored;
                }
            }
        }
        if (room >= 8 && loads_stored <= 4 && Data_CityInfo.resourceStored[r] - loads_stored > 4) {
            *resource = r;
            return WAREHOUSE_TASK_GETTING;
        }
    }
    // deliver weapons to barracks
    if (building_count_active(BUILDING_BARRACKS) > 0 && Data_CityInfo.militaryLegionaryLegions > 0 &&
        !Data_CityInfo.resourceStockpiled[RESOURCE_WEAPONS]) {
        building *barracks = building_get(Data_CityInfo.buildingBarracksBuildingId);
        if (barracks->loadsStored < 4 &&
                warehouse->roadNetworkId == barracks->roadNetworkId) {
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = building_next(space);
                if (space->id > 0 && space->loadsStored > 0 &&
                    space->subtype.warehouseResourceId == RESOURCE_WEAPONS) {
                    *resource = RESOURCE_WEAPONS;
                    return WAREHOUSE_TASK_DELIVERING;
                }
            }
        }
    }
    // deliver raw materials to workshops
    space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && space->loadsStored > 0) {
            if (!Data_CityInfo.resourceStockpiled[space->subtype.warehouseResourceId]) {
                int workshop_type = workshop_type_for_resource(space->subtype.warehouseResourceId);
                if (workshop_type != WORKSHOP_NONE &&
                        Data_CityInfo.resourceWorkshopRawMaterialSpace[workshop_type] > 0) {
                    *resource = space->subtype.warehouseResourceId;
                    return WAREHOUSE_TASK_DELIVERING;
                }
            }
        }
    }
    // deliver food to getting granary
    int granary_resources[RESOURCE_MAX_FOOD];
    if (determine_granary_get_foods(granary_resources)) {
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (contains_non_stockpiled_food(space, granary_resources)) {
                *resource = space->subtype.warehouseResourceId;
                return WAREHOUSE_TASK_DELIVERING;
            }
        }
    }
    // deliver food to accepting granary
    if (determine_granary_accept_foods(granary_resources) && !scenario_property_rome_supplies_wheat()) {
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (contains_non_stockpiled_food(space, granary_resources)) {
                *resource = space->subtype.warehouseResourceId;
                return WAREHOUSE_TASK_DELIVERING;
            }
        }
    }
    // move goods to other warehouses
    if (s->empty_all) {
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0 && space->loadsStored > 0) {
                *resource = space->subtype.warehouseResourceId;
                return WAREHOUSE_TASK_DELIVERING;
            }
        }
    }
    return WAREHOUSE_TASK_NONE;
}
