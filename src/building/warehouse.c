#include "warehouse.h"

#include "building/count.h"
#include "building/granary.h"
#include "building/model.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/military.h"
#include "city/resource.h"
#include "core/calc.h"
#include "core/image.h"
#include "empire/trade_prices.h"
#include "game/tutorial.h"
#include "map/image.h"
#include "map/road_access.h"
#include "scenario/property.h"

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
        if (space->subtype.warehouse_resource_id) {
            total_loads += space->loads_stored;
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
        if (space->subtype.warehouse_resource_id && space->subtype.warehouse_resource_id == resource) {
            loads += space->loads_stored;
        }
    }
    return loads;
}

int building_warehouse_add_resource(building *b, int resource)
{
    if (b->id <= 0) {
        return 0;
    }
    building *main = building_main(b);    
    if (building_warehouse_is_not_accepting(resource,main)) {
        return 0;
    }
    // check building itself
    int find_space = 0;
    if (b->subtype.warehouse_resource_id && b->subtype.warehouse_resource_id != resource) {
        find_space = 1;
    } else if (b->loads_stored >= 4) {
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
            if (!space->subtype.warehouse_resource_id || space->subtype.warehouse_resource_id == resource) {
                if (space->loads_stored < 4) {
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
    city_resource_add_to_warehouse(resource, 1);
    b->subtype.warehouse_resource_id = resource;
    b->loads_stored++;
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
        if (space->subtype.warehouse_resource_id != resource || space->loads_stored <= 0) {
            continue;
        }
        if (space->loads_stored > amount) {
            city_resource_remove_from_warehouse(resource, amount);
            space->loads_stored -= amount;
            amount = 0;
        } else {
            city_resource_remove_from_warehouse(resource, space->loads_stored);
            amount -= space->loads_stored;
            space->loads_stored = 0;
            space->subtype.warehouse_resource_id = RESOURCE_NONE;
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
        if (space->id <= 0 || space->loads_stored <= 0) {
            continue;
        }
        int resource = space->subtype.warehouse_resource_id;
        if (space->loads_stored > amount) {
            city_resource_remove_from_warehouse(resource, amount);
            space->loads_stored -= amount;
            amount = 0;
        } else {
            city_resource_remove_from_warehouse(resource, space->loads_stored);
            amount -= space->loads_stored;
            space->loads_stored = 0;
            space->subtype.warehouse_resource_id = RESOURCE_NONE;
        }
        building_warehouse_space_set_image(space, resource);
    }
}

void building_warehouse_space_set_image(building *space, int resource)
{
    int image_id;
    if (space->loads_stored <= 0) {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY);
    } else {
        image_id = image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_FILLED) +
            4 * (resource - 1) + resource_image_offset(resource, RESOURCE_IMAGE_STORAGE) +
                   space->loads_stored - 1;
    }
    map_image_set(space->grid_offset, image_id);
}

void building_warehouse_space_add_import(building *space, int resource)
{
    city_resource_add_to_warehouse(resource, 1);
    space->loads_stored++;
    space->subtype.warehouse_resource_id = resource;

    int price = trade_price_buy(resource);
    city_finance_process_import(price);

    building_warehouse_space_set_image(space, resource);
}

void building_warehouse_space_remove_export(building *space, int resource)
{
    city_resource_remove_from_warehouse(resource, 1);
    space->loads_stored--;
    if (space->loads_stored <= 0) {
        space->subtype.warehouse_resource_id = RESOURCE_NONE;
    }

    int price = trade_price_sell(resource);
    city_finance_process_export(price);

    building_warehouse_space_set_image(space, resource);
}



void building_warehouses_add_resource(int resource, int amount)
{
    int building_id = city_resource_last_used_warehouse();
    for (int i = 1; i < MAX_BUILDINGS && amount > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WAREHOUSE) {
            city_resource_set_last_used_warehouse(building_id);
            while (amount && building_warehouse_add_resource(b, resource)) {
                amount--;
            }
        }
    }
}

int THREEQ_WAREHOUSE = 24;
int HALF_WAREHOUSE = 16;
int QUARTER_WAREHOUSE = 8;

int building_warehouse_is_accepting(int resource, building *b)
{
        const building_storage *s = building_storage_get(b->storage_id);
        int amount = building_warehouse_get_amount(b, resource);	
        if ((s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING) ||
            (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS && amount < THREEQ_WAREHOUSE) ||
	        (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_HALF && amount < HALF_WAREHOUSE) ||
            (s->resource_state[resource] == BUILDING_STORAGE_STATE_ACCEPTING_QUARTER && amount < QUARTER_WAREHOUSE)) {
		return 1;
	} else {
        	return 0;
	}	
}

int building_warehouse_is_getting(int resource, building *b)
{
        const building_storage *s = building_storage_get(b->storage_id);
        int amount = building_warehouse_get_amount(b, resource);	
        if ((s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING) ||
            (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_3QUARTERS && amount < THREEQ_WAREHOUSE) ||
	        (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_HALF && amount < HALF_WAREHOUSE) ||
            (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_QUARTER && amount < QUARTER_WAREHOUSE)) {
		return 1;
	} else {
	    return 0;
	}	
}

int building_warehouse_is_gettable(int resource, building *b)
{
        const building_storage *s = building_storage_get(b->storage_id);
        if ((s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING) ||
	    (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_HALF) ||
            (s->resource_state[resource] == BUILDING_STORAGE_STATE_GETTING_QUARTER)) {
		return 1;
	} else {
	    return 0;
	}	
}

int building_warehouse_is_not_accepting(int resource, building *b)
{
    return !((building_warehouse_is_accepting(resource,b) || building_warehouse_is_getting(resource,b)));
}

int building_warehouse_get_acceptable_quantity(int resource, building* b)
{
    const building_storage* s = building_storage_get(b->storage_id);
    switch (s->resource_state[resource]) {
    case BUILDING_STORAGE_STATE_ACCEPTING:
    case BUILDING_STORAGE_STATE_GETTING:
        return 32;
        break;
    case BUILDING_STORAGE_STATE_ACCEPTING_3QUARTERS:
    case BUILDING_STORAGE_STATE_GETTING_3QUARTERS:
        return THREEQ_WAREHOUSE;
        break;
    case BUILDING_STORAGE_STATE_ACCEPTING_HALF:
    case BUILDING_STORAGE_STATE_GETTING_HALF:
        return HALF_WAREHOUSE;
        break;
    case BUILDING_STORAGE_STATE_ACCEPTING_QUARTER:
    case BUILDING_STORAGE_STATE_GETTING_QUARTER:
        return QUARTER_WAREHOUSE;
        break;
    default:
        return 0;
    }
}


int building_warehouses_remove_resource(int resource, int amount)
{
    int amount_left = amount;
    int building_id = city_resource_last_used_warehouse();
    // first go for non-getting warehouses
    for (int i = 1; i < MAX_BUILDINGS && amount_left > 0; i++) {
        building_id++;
        if (building_id >= MAX_BUILDINGS) {
            building_id = 1;
        }
        building *b = building_get(building_id);
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WAREHOUSE) {
            if (!building_warehouse_is_getting(resource,b)) {
                city_resource_set_last_used_warehouse(building_id);
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
        if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_WAREHOUSE) {
            city_resource_set_last_used_warehouse(building_id);
            amount_left = building_warehouse_remove_resource(b, resource, amount_left);
        }
    }
    return amount - amount_left;
}

int building_warehouse_for_storing(int src_building_id, int x, int y, int resource,
                                   int distance_from_entry, int road_network_id, int *understaffed,
                                   map_point *dst)
{
    int min_dist = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE_SPACE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0 || b->road_network_id != road_network_id) {
            continue;
        }
        building *building_dst = building_main(b);
        if (src_building_id == building_dst->id) {
            continue;
        }
        const building_storage *s = building_storage_get(building_dst->storage_id);
        if (building_warehouse_is_not_accepting(resource,building_dst) || s->empty_all) {
            continue;
        }
        int pct_workers = calc_percentage(building_dst->num_workers, model_get_building(building_dst->type)->laborers);
        if (pct_workers < 100) {
            if (understaffed) {
                *understaffed += 1;
            }
            continue;
        }
        int dist;
        if (b->subtype.warehouse_resource_id == RESOURCE_NONE) { // empty warehouse space
            dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
        } else if (b->subtype.warehouse_resource_id == resource && b->loads_stored < 4) {
            dist = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
        } else {
            dist = 0;
        }
        if (dist > 0 && dist < min_dist) {
            min_dist = dist;
            min_building_id = i;
        }
    }
    building *b = building_main(building_get(min_building_id));
    if (b->has_road_access == 1) {
        map_point_store_result(b->x, b->y, dst);
    } else if (!map_has_road_access(b->x, b->y, 3, dst)) {
        return 0;
    }
    return min_building_id;
}

int building_warehouse_for_getting(building *src, int resource, map_point *dst)
{
    int min_dist = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE) {
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
            if (space->id > 0 && space->loads_stored > 0) {
                if (space->subtype.warehouse_resource_id == resource) {
                    loads_stored += space->loads_stored;
                }
            }
        }
        if (loads_stored > 0 && !building_warehouse_is_gettable(resource,b)) {
            int dist = calc_distance_with_penalty(b->x, b->y, src->x, src->y,
                                                  src->distance_from_entry, b->distance_from_entry);
            dist -= 4 * loads_stored;
            if (dist < min_dist) {
                min_dist = dist;
                min_building = b;
            }
        }
    }
    if (min_building) {
        map_point_store_result(min_building->road_access_x, min_building->road_access_y, dst);
        return min_building->id;
    } else {
        return 0;
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
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_GRANARY || !b->has_road_access) {
            continue;
        }
        int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
        if (pct_workers >= 100 && b->data.granary.resource_stored[RESOURCE_NONE] >= 1200) {
            const building_storage *s = building_storage_get(b->storage_id);
            if (!s->empty_all) {
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    if (!building_granary_is_not_accepting(r,b)) {
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
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_GRANARY || !b->has_road_access) {
            continue;
        }
        int pct_workers = calc_percentage(b->num_workers, model_get_building(b->type)->laborers);
        if (pct_workers >= 100 && b->data.granary.resource_stored[RESOURCE_NONE] > 100) {
            const building_storage *s = building_storage_get(b->storage_id);
            if (!s->empty_all) {
                for (int r = 0; r < RESOURCE_MAX_FOOD; r++) {
                    if (building_granary_is_getting(r,b)) {
                        resources[r]++;
                        can_get = 1;
                    }
                }
            }
        }
    }
    return can_get;
}

static int contains_non_stockpiled_food(building *space, const int *resources)
{
    if (space->id <= 0) {
        return 0;
    }
    if (space->loads_stored <= 0) {
        return 0;
    }
    int resource = space->subtype.warehouse_resource_id;
    if (city_resource_is_stockpiled(resource)) {
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

int building_warehouse_determine_worker_task(building *warehouse, int *resource)
{
    int pct_workers = calc_percentage(warehouse->num_workers, model_get_building(warehouse->type)->laborers);
    if (pct_workers < 50) {
        return WAREHOUSE_TASK_NONE;
    }
    const building_storage *s = building_storage_get(warehouse->storage_id);
    building *space;
    // get resources
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (!building_warehouse_is_getting(r,warehouse) || city_resource_is_stockpiled(r)) {
            continue;
        }
        int loads_stored = 0;
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0 && space->loads_stored > 0) {
                if (space->subtype.warehouse_resource_id == r) {
                    loads_stored += space->loads_stored;
                }
            }
        }
        int room = 0;
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0) {
                if (space->loads_stored <= 0) {
                    room += 4;
                }
                if (space->subtype.warehouse_resource_id == r) {
                    room += 4 - space->loads_stored;
                }
            }
        }
        if (room >= 8 && (loads_stored <= 4 || ((building_warehouse_get_acceptable_quantity(r,space) - loads_stored) >= 4)) && city_resource_count(r) - loads_stored >= 4) {
            *resource = r;
            return WAREHOUSE_TASK_GETTING;
        }
    }
    // deliver weapons to barracks
    if (building_count_active(BUILDING_BARRACKS) > 0 && city_military_has_legionary_legions() &&
        !city_resource_is_stockpiled(RESOURCE_WEAPONS)) {
        building *barracks = building_get(city_buildings_get_barracks());
        if (barracks->loads_stored < 4 &&
                warehouse->road_network_id == barracks->road_network_id) {
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = building_next(space);
                if (space->id > 0 && space->loads_stored > 0 &&
                    space->subtype.warehouse_resource_id == RESOURCE_WEAPONS) {
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
        if (space->id > 0 && space->loads_stored > 0) {
            if (!city_resource_is_stockpiled(space->subtype.warehouse_resource_id)) {
                int workshop_type = resource_to_workshop_type(space->subtype.warehouse_resource_id);
                if (workshop_type != WORKSHOP_NONE && city_resource_has_workshop_with_room(workshop_type)) {
                    *resource = space->subtype.warehouse_resource_id;
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
                *resource = space->subtype.warehouse_resource_id;
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
                *resource = space->subtype.warehouse_resource_id;
                return WAREHOUSE_TASK_DELIVERING;
            }
        }
    }
    // move goods to other warehouses
    if (s->empty_all) {
        space = warehouse;
        for (int i = 0; i < 8; i++) {
            space = building_next(space);
            if (space->id > 0 && space->loads_stored > 0) {
                *resource = space->subtype.warehouse_resource_id;
                return WAREHOUSE_TASK_DELIVERING;
            }
        }
    }
    return WAREHOUSE_TASK_NONE;
}
