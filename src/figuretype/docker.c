#include "docker.h"

#include "building/building.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/buildings.h"
#include "city/trade.h"
#include "core/calc.h"
#include "core/image.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_route.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "map/road_access.h"

static int try_import_resource(int building_id, int resource, int city_id)
{
    building *warehouse = building_get(building_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }

    if (building_warehouse_is_not_accepting(resource,warehouse)) {
        return 0;
    }

    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, warehouse)) {
        return 0;
    }
    
    int route_id = empire_city_get_route_id(city_id);
    // try existing storage bay with the same resource
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->loads_stored && space->loads_stored < 4 && space->subtype.warehouse_resource_id == resource) {
                trade_route_increase_traded(route_id, resource);
                building_warehouse_space_add_import(space, resource);
                return 1;
            }
        }
    }
    // try unused storage bay
    space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->subtype.warehouse_resource_id == RESOURCE_NONE) {
                trade_route_increase_traded(route_id, resource);
                building_warehouse_space_add_import(space, resource);
                return 1;
            }
        }
    }
    return 0;
}

static int try_export_resource(int building_id, int resource, int city_id)
{
    building *warehouse = building_get(building_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }

    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, warehouse)) {
        return 0;
    }
    
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->loads_stored && space->subtype.warehouse_resource_id == resource) {
                trade_route_increase_traded(empire_city_get_route_id(city_id), resource);
                building_warehouse_space_remove_export(space, resource);
                return 1;
            }
        }
    }
    return 0;
}

static int get_closest_warehouse_for_import(int x, int y, int city_id, int distance_from_entry, int road_network_id,
                                            map_point *warehouse, int *import_resource)
{
    int importable[16];
    importable[RESOURCE_NONE] = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        importable[r] = empire_can_import_resource_from_city(city_id, r);
    }
    int resource = city_trade_next_docker_import_resource();
    for (int i = RESOURCE_MIN; i < RESOURCE_MAX && !importable[resource]; i++) {
        resource = city_trade_next_docker_import_resource();
    }
    if (!importable[resource]) {
        return 0;
    }
    int min_distance = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (b->road_network_id != road_network_id) {
            continue;
        }
        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, b)) {
            continue;
        }
        const building_storage *storage = building_storage_get(b->storage_id);
        if (!building_warehouse_is_not_accepting(resource,b) && !storage->empty_all) {
            int distance_penalty = 32;
            building *space = b;
            for (int s = 0; s < 8; s++) {
                space = building_next(space);
                if (space->id && space->subtype.warehouse_resource_id == RESOURCE_NONE) {
                    distance_penalty -= 8;
                }
                if (space->id && space->subtype.warehouse_resource_id == resource && space->loads_stored < 4) {
                    distance_penalty -= 4;
                }
            }
            if (distance_penalty < 32) {
                int distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
                // prefer emptier warehouse
                distance += distance_penalty;
                if (distance < min_distance) {
                    min_distance = distance;
                    min_building_id = i;
                }
            }
        }
    }
    if (!min_building_id) {
        return 0;
    }
    building *min = building_get(min_building_id);
    if (min->has_road_access == 1) {
        map_point_store_result(min->x, min->y, warehouse);
    } else if (!map_has_road_access(min->x, min->y, 3, warehouse)) {
        return 0;
    }
    *import_resource = resource;
    return min_building_id;
}

static int get_closest_warehouse_for_export(int x, int y, int city_id, int distance_from_entry, int road_network_id,
                                            map_point *warehouse, int *export_resource)
{
    int exportable[16];
    exportable[RESOURCE_NONE] = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        exportable[r] = empire_can_export_resource_to_city(city_id, r);
    }
    int resource = city_trade_next_docker_export_resource();
    for (int i = RESOURCE_MIN; i < RESOURCE_MAX && !exportable[resource]; i++) {
        resource = city_trade_next_docker_export_resource();
    }
    if (!exportable[resource]) {
        return 0;
    }
    int min_distance = 10000;
    int min_building_id = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (b->road_network_id != road_network_id) {
            continue;
        }
        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, b)) {
            continue;
        }
        int distance_penalty = 32;
        building *space = b;
        for (int s = 0; s < 8; s++) {
            space = building_next(space);
            if (space->id && space->subtype.warehouse_resource_id == resource && space->loads_stored > 0) {
                distance_penalty--;
            }
        }
        if (distance_penalty < 32) {
            int distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            // prefer fuller warehouse
            distance += distance_penalty;
            if (distance < min_distance) {
                min_distance = distance;
                min_building_id = i;
            }
        }
    }
    if (!min_building_id) {
        return 0;
    }
    building *min = building_get(min_building_id);
    if (min->has_road_access == 1) {
        map_point_store_result(min->x, min->y, warehouse);
    } else if (!map_has_road_access(min->x, min->y, 3, warehouse)) {
        return 0;
    }
    *export_resource = resource;
    return min_building_id;
}

static void get_trade_center_location(const figure *f, int *x, int *y)
{
    int trade_center_id = city_buildings_get_trade_center();
    if (trade_center_id) {
        building *trade_center = building_get(trade_center_id);
        *x = trade_center->x;
        *y = trade_center->y;
    } else {
        *x = f->x;
        *y = f->y;
    }
}

static int deliver_import_resource(figure *f, building *dock)
{
    int ship_id = dock->data.dock.trade_ship_id;
    if (!ship_id) {
        return 0;
    }
    figure *ship = figure_get(ship_id);
    if (ship->action_state != FIGURE_ACTION_112_TRADE_SHIP_MOORED || ship->loads_sold_or_carrying <= 0) {
        return 0;
    }
    int x, y;
    get_trade_center_location(f, &x, &y);
    map_point tile;
    int resource;
    int warehouse_id = get_closest_warehouse_for_import(x, y, ship->empire_city_id,
                      dock->distance_from_entry, dock->road_network_id, &tile, &resource);
    if (!warehouse_id) {
        return 0;
    }
    ship->loads_sold_or_carrying--;
    f->destination_building_id = warehouse_id;
    f->wait_ticks = 0;
    f->action_state = FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE;
    f->destination_x = tile.x;
    f->destination_y = tile.y;
    f->resource_id = resource;
    return 1;
}

static int fetch_export_resource(figure *f, building *dock)
{
    int ship_id = dock->data.dock.trade_ship_id;
    if (!ship_id) {
        return 0;
    }
    figure *ship = figure_get(ship_id);
    if (ship->action_state != FIGURE_ACTION_112_TRADE_SHIP_MOORED || ship->trader_amount_bought >= 12) {
        return 0;
    }
    int x, y;
    get_trade_center_location(f, &x, &y);
    map_point tile;
    int resource;
    int warehouse_id = get_closest_warehouse_for_export(x, y, ship->empire_city_id,
        dock->distance_from_entry, dock->road_network_id, &tile, &resource);
    if (!warehouse_id) {
        return 0;
    }
    ship->trader_amount_bought++;
    f->destination_building_id = warehouse_id;
    f->action_state = FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_WAREHOUSE;
    f->wait_ticks = 0;
    f->destination_x = tile.x;
    f->destination_y = tile.y;
    f->resource_id = resource;
    return 1;
}

static void set_cart_graphic(figure *f)
{
    f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART) + 8 * f->resource_id;
    f->cart_image_id += resource_image_offset(f->resource_id, RESOURCE_IMAGE_CART);
}

void figure_docker_action(figure *f)
{
    building *b = building_get(f->building_id);
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    if (b->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (b->type != BUILDING_DOCK && b->type != BUILDING_WHARF) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (b->data.dock.num_ships) {
        b->data.dock.num_ships--;
    }
    if (b->data.dock.trade_ship_id) {
        figure *ship = figure_get(b->data.dock.trade_ship_id);
        if (ship->state != FIGURE_STATE_ALIVE || ship->type != FIGURE_TRADE_SHIP) {
            b->data.dock.trade_ship_id = 0;
        } else if (trader_has_traded_max(ship->trader_id)) {
            b->data.dock.trade_ship_id = 0;
        } else if (ship->action_state == FIGURE_ACTION_115_TRADE_SHIP_LEAVING) {
            b->data.dock.trade_ship_id = 0;
        }
    }
    f->terrain_usage = TERRAIN_USAGE_ROADS;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_132_DOCKER_IDLING:
            f->resource_id = 0;
            f->cart_image_id = 0;
            if (!deliver_import_resource(f, b)) {
                fetch_export_resource(f, b);
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE:
            f->cart_image_id = 0;
            f->image_offset = 0;
            if (b->data.dock.queued_docker_id <= 0) {
                b->data.dock.queued_docker_id = f->id;
                f->wait_ticks = 0;
            }
            if (b->data.dock.queued_docker_id == f->id) {
                b->data.dock.num_ships = 120;
                f->wait_ticks++;
                if (f->wait_ticks >= 80) {
                    f->action_state = FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_WAREHOUSE;
                    f->wait_ticks = 0;
                    set_cart_graphic(f);
                    b->data.dock.queued_docker_id = 0;
                }
            } else {
                int has_queued_docker = 0;
                for (int i = 0; i < 3; i++) {
                    if (b->data.dock.docker_ids[i]) {
                        figure *docker = figure_get(b->data.dock.docker_ids[i]);
                        if (docker->id == b->data.dock.queued_docker_id && docker->state == FIGURE_STATE_ALIVE) {
                            if (docker->action_state == FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE ||
                                docker->action_state == FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE) {
                                has_queued_docker = 1;
                            }
                        }
                    }
                }
                if (!has_queued_docker) {
                    b->data.dock.queued_docker_id = 0;
                }
            }
            break;
        case FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE:
            set_cart_graphic(f);
            if (b->data.dock.queued_docker_id <= 0) {
                b->data.dock.queued_docker_id = f->id;
                f->wait_ticks = 0;
            }
            if (b->data.dock.queued_docker_id == f->id) {
                b->data.dock.num_ships = 120;
                f->wait_ticks++;
                if (f->wait_ticks >= 80) {
                    f->action_state = FIGURE_ACTION_132_DOCKER_IDLING;
                    f->wait_ticks = 0;
                    f->image_id = 0;
                    f->cart_image_id = 0;
                    b->data.dock.queued_docker_id = 0;
                }
            }
            f->wait_ticks++;
            if (f->wait_ticks >= 20) {
                f->action_state = FIGURE_ACTION_132_DOCKER_IDLING;
                f->wait_ticks = 0;
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_WAREHOUSE:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_139_DOCKER_IMPORT_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_WAREHOUSE:
            f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_140_DOCKER_EXPORT_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE;
                f->wait_ticks = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_132_DOCKER_IDLING;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_139_DOCKER_IMPORT_AT_WAREHOUSE:
            set_cart_graphic(f);
            f->wait_ticks++;
            if (f->wait_ticks > 10) {
                int trade_city_id;
                if (b->data.dock.trade_ship_id) {
                    trade_city_id = figure_get(b->data.dock.trade_ship_id)->empire_city_id;
                } else {
                    trade_city_id = 0;
                }
                if (try_import_resource(f->destination_building_id, f->resource_id, trade_city_id)) {
                    int trader_id = figure_get(b->data.dock.trade_ship_id)->trader_id;
                    trader_record_sold_resource(trader_id, f->resource_id);
                    f->action_state = FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING;
                    f->wait_ticks = 0;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                    f->resource_id = 0;
                    fetch_export_resource(f, b);
                } else {
                    f->action_state = FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                }
                f->wait_ticks = 0;
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_140_DOCKER_EXPORT_AT_WAREHOUSE:
            f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            f->wait_ticks++;
            if (f->wait_ticks > 10) {
                int trade_city_id;
                if (b->data.dock.trade_ship_id) {
                    trade_city_id = figure_get(b->data.dock.trade_ship_id)->empire_city_id;
                } else {
                    trade_city_id = 0;
                }
                f->action_state = FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
                f->wait_ticks = 0;
                if (try_export_resource(f->destination_building_id, f->resource_id, trade_city_id)) {
                    int trader_id = figure_get(b->data.dock.trade_ship_id)->trader_id;
                    trader_record_bought_resource(trader_id, f->resource_id);
                    f->action_state = FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING;
                } else {
                    fetch_export_resource(f, b);
                }
            }
            f->image_offset = 0;
            break;
    }

    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);

    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CARTPUSHER) + figure_image_corpse_offset(f) + 96;
        f->cart_image_id = 0;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->image_offset;
    }
    if (f->cart_image_id) {
        f->cart_image_id += dir;
        figure_image_set_cart_offset(f, dir);
    } else {
        f->image_id = 0;
    }
}
