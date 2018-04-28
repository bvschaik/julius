#include "cartpusher.h"

#include "building/barracks.h"
#include "building/granary.h"
#include "building/industry.h"
#include "building/warehouse.h"
#include "city/resource.h"
#include "core/image.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "game/resource.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"

static const int CART_OFFSET_MULTIPLE_LOADS_FOOD[] = {0, 0, 8, 16, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const int CART_OFFSET_MULTIPLE_LOADS_NON_FOOD[] = {0, 0, 0, 0, 0, 8, 0, 16, 24, 32, 40, 48, 56, 64, 72, 80};
static const int CART_OFFSET_8_LOADS_FOOD[] = {0, 40, 48, 56, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void set_cart_graphic(figure *f)
{
    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART) +
        8 * f->resourceId + resource_image_offset(f->resourceId, RESOURCE_IMAGE_CART);
}

static void set_destination(figure *f, int action, int building_id, int x_dst, int y_dst)
{
    f->destinationBuildingId = building_id;
    f->actionState = action;
    f->waitTicks = 0;
    f->destinationX = x_dst;
    f->destinationY = y_dst;
}

static void determine_cartpusher_destination(figure *f, building *b, int road_network_id)
{
    int x_dst, y_dst;
    int understaffed_storages = 0;
    
    // priority 1: warehouse if resource is on stockpile
    int dst_building_id = building_warehouse_for_storing(0, f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id,
        &understaffed_storages, &x_dst, &y_dst);
    if (!city_resource_is_stockpiled(b->outputResourceId)) {
        dst_building_id = 0;
    }
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 2: accepting granary for food
    dst_building_id = building_granary_for_storing(f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id, 0,
        &understaffed_storages, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 3: workshop for raw material
    dst_building_id = building_get_workshop_for_raw_material_with_room(f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 4: warehouse
    dst_building_id = building_warehouse_for_storing(0, f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id,
        &understaffed_storages, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 5: granary forced when on stockpile
    dst_building_id = building_granary_for_storing(f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id, 1,
        &understaffed_storages, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY, dst_building_id, x_dst, y_dst);
        return;
    }
    // no one will accept
    f->waitTicks = 0;
    // set cartpusher text
    f->minMaxSeen = understaffed_storages ? 2 : 1;
}

static void determine_cartpusher_destination_food(figure *f, int road_network_id)
{
    building *b = building_get(f->buildingId);
    int x_dst, y_dst;
    // priority 1: accepting granary for food
    int dst_building_id = building_granary_for_storing(f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id, 0,
        0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 2: warehouse
    dst_building_id = building_warehouse_for_storing(0, f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id,
        0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE, dst_building_id, x_dst, y_dst);
        return;
    }
    // priority 3: granary
    dst_building_id = building_granary_for_storing(f->x, f->y,
        b->outputResourceId, b->distanceFromEntry, road_network_id, 1,
        0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY, dst_building_id, x_dst, y_dst);
        return;
    }
    // no one will accept, stand idle
    f->waitTicks = 0;
}

static void update_image(figure *f)
{
    int dir = figure_image_normalize_direction(
        f->direction < 8 ? f->direction : f->previousTileDirection);

    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + figure_image_corpse_offset(f) + 96;
        f->cartGraphicId = 0;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->graphicOffset;
    }
    if (f->cartGraphicId) {
        f->cartGraphicId += dir;
        figure_image_set_cart_offset(f, dir);
        if (f->loadsSoldOrCarrying >= 8) {
            f->yOffsetCart -= 40;
        }
    }
}

static void reroute_cartpusher(figure *f)
{
    figure_route_remove(f);
    if (!map_routing_citizen_is_passable_terrain(f->gridOffset)) {
        f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
    }
    f->waitTicks = 0;
}

void figure_cartpusher_action(figure *f)
{
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    int road_network_id = map_road_network_get(f->gridOffset);
    f->terrainUsage = TERRAIN_USAGE_ROADS;
    building *b = building_get(f->buildingId);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_20_CARTPUSHER_INITIAL:
            set_cart_graphic(f);
            if (!map_routing_citizen_is_passable(f->gridOffset)) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
                f->state = FIGURE_STATE_DEAD;
            }
            f->waitTicks++;
            if (f->waitTicks > 30) {
                determine_cartpusher_destination(f, b, road_network_id);
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_21_CARTPUSHER_DELIVERING_TO_WAREHOUSE:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                reroute_cartpusher(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (building_get(f->destinationBuildingId)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_22_CARTPUSHER_DELIVERING_TO_GRANARY:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                reroute_cartpusher(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
                f->waitTicks = 0;
            }
            if (building_get(f->destinationBuildingId)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_23_CARTPUSHER_DELIVERING_TO_WORKSHOP:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                reroute_cartpusher(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_24_CARTPUSHER_AT_WAREHOUSE:
            f->waitTicks++;
            if (f->waitTicks > 10) {
                if (building_warehouse_add_resource(building_get(f->destinationBuildingId), f->resourceId)) {
                    f->actionState = FIGURE_ACTION_27_CARTPUSHER_RETURNING;
                    f->waitTicks = 0;
                    f->destinationX = f->sourceX;
                    f->destinationY = f->sourceY;
                } else {
                    figure_route_remove(f);
                    f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
                    f->waitTicks = 0;
                }
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_25_CARTPUSHER_AT_GRANARY:
            f->waitTicks++;
            if (f->waitTicks > 5) {
                if (building_granary_add_resource(building_get(f->destinationBuildingId), f->resourceId, 1)) {
                    f->actionState = FIGURE_ACTION_27_CARTPUSHER_RETURNING;
                    f->waitTicks = 0;
                    f->destinationX = f->sourceX;
                    f->destinationY = f->sourceY;
                } else {
                    determine_cartpusher_destination_food(f, road_network_id);
                }
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_26_CARTPUSHER_AT_WORKSHOP:
            f->waitTicks++;
            if (f->waitTicks > 5) {
                building_workshop_add_raw_material(building_get(f->destinationBuildingId));
                f->actionState = FIGURE_ACTION_27_CARTPUSHER_RETURNING;
                f->waitTicks = 0;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_27_CARTPUSHER_RETURNING:
            f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_20_CARTPUSHER_INITIAL;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_image(f);
}

static void determine_granaryman_destination(figure *f, int road_network_id)
{
    int dst_building_id, x_dst, y_dst;
    building *granary = building_get(f->buildingId);
    if (!f->resourceId) {
        // getting granaryman
        dst_building_id = building_granary_for_getting(granary, &x_dst, &y_dst);
        if (dst_building_id) {
            f->loadsSoldOrCarrying = 0;
            set_destination(f, FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD, dst_building_id, x_dst, y_dst);
        } else {
            f->state = FIGURE_STATE_DEAD;
        }
        return;
    }
    // delivering resource
    // priority 1: another granary
    dst_building_id = building_granary_for_storing(f->x, f->y,
        f->resourceId, granary->distanceFromEntry, road_network_id, 0,
        0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        building_granary_remove_resource(granary, f->resourceId, 100);
        return;
    }
    // priority 2: warehouse
    dst_building_id = building_warehouse_for_storing(0, f->x, f->y,
        f->resourceId, granary->distanceFromEntry,
                      road_network_id, 0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        building_granary_remove_resource(granary, f->resourceId, 100);
        return;
    }
    // priority 3: granary even though resource is on stockpile
    dst_building_id = building_granary_for_storing(f->x, f->y,
        f->resourceId, granary->distanceFromEntry, road_network_id, 1,
        0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        building_granary_remove_resource(granary, f->resourceId, 100);
        return;
    }
    // nowhere to go to: kill figure
    f->state = FIGURE_STATE_DEAD;
}

static void remove_resource_from_warehouse(figure *f)
{
    if (f->state != FIGURE_STATE_DEAD) {
        int err = building_warehouse_remove_resource(building_get(f->buildingId), f->resourceId, 1);
        if (err) {
            f->state = FIGURE_STATE_DEAD;
        }
    }
}

static void determine_warehouseman_destination(figure *f, int road_network_id)
{
    int dst_building_id, x_dst, y_dst;
    if (!f->resourceId) {
        // getting warehouseman
        dst_building_id = building_warehouse_for_getting(
            building_get(f->buildingId), f->collectingItemId, &x_dst, &y_dst);
        if (dst_building_id) {
            f->loadsSoldOrCarrying = 0;
            set_destination(f, FIGURE_ACTION_57_WAREHOUSEMAN_GETTING_RESOURCE, dst_building_id, x_dst, y_dst);
            f->terrainUsage = TERRAIN_USAGE_PREFER_ROADS;
        } else {
            f->state = FIGURE_STATE_DEAD;
        }
        return;
    }
    building *warehouse = building_get(f->buildingId);
    // delivering resource
    // priority 1: weapons to barracks
    dst_building_id = building_get_barracks_for_weapon(f->resourceId, road_network_id, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        remove_resource_from_warehouse(f);
        return;
    }
    // priority 2: raw materials to workshop
    dst_building_id = building_get_workshop_for_raw_material_with_room(f->x, f->y, f->resourceId,
        warehouse->distanceFromEntry, road_network_id, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        remove_resource_from_warehouse(f);
        return;
    }
    // priority 3: food to granary
    dst_building_id = building_granary_for_storing(f->x, f->y, f->resourceId,
        warehouse->distanceFromEntry, road_network_id, 0, 0, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        remove_resource_from_warehouse(f);
        return;
    }
    // priority 4: food to getting granary
    dst_building_id = building_getting_granary_for_storing(f->x, f->y, f->resourceId,
        warehouse->distanceFromEntry, road_network_id, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        remove_resource_from_warehouse(f);
        return;
    }
    // priority 5: resource to other warehouse
    dst_building_id = building_warehouse_for_storing(f->buildingId, f->x, f->y, f->resourceId,
        warehouse->distanceFromEntry, road_network_id, 0, &x_dst, &y_dst);
    if (dst_building_id) {
        if (dst_building_id == f->buildingId) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
            remove_resource_from_warehouse(f);
        }
        return;
    }
    // priority 6: raw material to well-stocked workshop
    dst_building_id = building_get_workshop_for_raw_material(f->x, f->y, f->resourceId,
        warehouse->distanceFromEntry, road_network_id, &x_dst, &y_dst);
    if (dst_building_id) {
        set_destination(f, FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE, dst_building_id, x_dst, y_dst);
        remove_resource_from_warehouse(f);
        return;
    }
    // no destination: kill figure
    f->state = FIGURE_STATE_DEAD;
}

void figure_warehouseman_action(figure *f)
{
    f->terrainUsage = TERRAIN_USAGE_ROADS;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    int road_network_id = map_road_network_get(f->gridOffset);
    
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_50_WAREHOUSEMAN_CREATED: {
            building *b = building_get(f->buildingId);
            if (b->state != BUILDING_STATE_IN_USE || b->figureId != f->id) {
                f->state = FIGURE_STATE_DEAD;
            }
            f->waitTicks++;
            if (f->waitTicks > 2) {
                if (building_get(f->buildingId)->type == BUILDING_GRANARY) {
                    determine_granaryman_destination(f, road_network_id);
                } else {
                    determine_warehouseman_destination(f, road_network_id);
                }
            }
            f->graphicOffset = 0;
            break;
        }
        case FIGURE_ACTION_51_WAREHOUSEMAN_DELIVERING_RESOURCE:
            if (f->loadsSoldOrCarrying == 1) {
                f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
                    8 * f->resourceId - 8 + resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
            } else {
                set_cart_graphic(f);
            }
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_52_WAREHOUSEMAN_AT_DELIVERY_BUILDING;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_52_WAREHOUSEMAN_AT_DELIVERY_BUILDING:
            f->waitTicks++;
            if (f->waitTicks > 4) {
                building *b = building_get(f->destinationBuildingId);
                switch (b->type) {
                    case BUILDING_GRANARY:
                        building_granary_add_resource(b, f->resourceId, 0);
                        break;
                    case BUILDING_BARRACKS:
                        building_barracks_add_weapon(b);
                        break;
                    case BUILDING_WAREHOUSE:
                    case BUILDING_WAREHOUSE_SPACE:
                        building_warehouse_add_resource(b, f->resourceId);
                        break;
                    default: // workshop
                        building_workshop_add_raw_material(b);
                        break;
                }
                // BUG: what if warehouse/granary is full and returns false?
                f->actionState = FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY;
                f->waitTicks = 0;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_53_WAREHOUSEMAN_RETURNING_EMPTY:
            f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_54_WAREHOUSEMAN_GETTING_FOOD:
            f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_55_WAREHOUSEMAN_AT_GRANARY;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_55_WAREHOUSEMAN_AT_GRANARY:
            f->waitTicks++;
            if (f->waitTicks > 4) {
                int resource;
                f->loadsSoldOrCarrying = building_granary_remove_for_getting_deliveryman(
                    building_get(f->destinationBuildingId), building_get(f->buildingId), &resource);
                f->resourceId = resource;
                f->actionState = FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD;
                f->waitTicks = 0;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                figure_route_remove(f);
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_56_WAREHOUSEMAN_RETURNING_WITH_FOOD:
            // update graphic
            if (f->loadsSoldOrCarrying <= 0) {
                f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            } else if (f->loadsSoldOrCarrying == 1) {
                set_cart_graphic(f);
            } else {
                if (f->loadsSoldOrCarrying >= 8) {
                    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
                        CART_OFFSET_8_LOADS_FOOD[f->resourceId];
                } else {
                    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
                        CART_OFFSET_MULTIPLE_LOADS_FOOD[f->resourceId];
                }
                f->cartGraphicId += resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
            }
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                for (int i = 0; i < f->loadsSoldOrCarrying; i++) {
                    building_granary_add_resource(building_get(f->buildingId), f->resourceId, 0);
                }
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_57_WAREHOUSEMAN_GETTING_RESOURCE:
            f->terrainUsage = TERRAIN_USAGE_PREFER_ROADS;
            f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_58_WAREHOUSEMAN_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_58_WAREHOUSEMAN_AT_WAREHOUSE:
            f->terrainUsage = TERRAIN_USAGE_PREFER_ROADS;
            f->waitTicks++;
            if (f->waitTicks > 4) {
                f->loadsSoldOrCarrying = 0;
                while (f->loadsSoldOrCarrying < 4 && 0 == building_warehouse_remove_resource(
                    building_get(f->destinationBuildingId), f->collectingItemId, 1)) {
                    f->loadsSoldOrCarrying++;
                }
                f->resourceId = f->collectingItemId;
                f->actionState = FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE;
                f->waitTicks = 0;
                f->destinationX = f->sourceX;
                f->destinationY = f->sourceY;
                figure_route_remove(f);
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_59_WAREHOUSEMAN_RETURNING_WITH_RESOURCE:
            f->terrainUsage = TERRAIN_USAGE_PREFER_ROADS;
            // update graphic
            if (f->loadsSoldOrCarrying <= 0) {
                f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            } else if (f->loadsSoldOrCarrying == 1) {
                set_cart_graphic(f);
            } else {
                if (f->resourceId == RESOURCE_WHEAT || f->resourceId == RESOURCE_VEGETABLES ||
                    f->resourceId == RESOURCE_FRUIT || f->resourceId == RESOURCE_MEAT) {
                    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_FOOD) +
                        CART_OFFSET_MULTIPLE_LOADS_FOOD[f->resourceId];
                } else {
                    f->cartGraphicId = image_group(GROUP_FIGURE_CARTPUSHER_CART_MULTIPLE_RESOURCE) +
                        CART_OFFSET_MULTIPLE_LOADS_NON_FOOD[f->resourceId];
                }
                f->cartGraphicId += resource_image_offset(f->resourceId, RESOURCE_IMAGE_FOOD_CART);
            }
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                for (int i = 0; i < f->loadsSoldOrCarrying; i++) {
                    building_warehouse_add_resource(building_get(f->buildingId), f->resourceId);
                }
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    update_image(f);
}
