#include "docker.h"

#include "building/building.h"
#include "building/granary.h"
#include "building/market.h"
#include "building/storage.h"
#include "building/warehouse.h"
#include "city/health.h"
#include "city/trade.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/image.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_route.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figuretype/trader.h"
#include "map/road_access.h"

#define INFINITE 10000

static int try_import_resource(int building_id, int resource, int city_id)
{
    building *b = building_get(building_id);
    if (b->type != BUILDING_WAREHOUSE &&
        !(resource_is_food(resource) && b->type == BUILDING_GRANARY)) {
        return 0;
    }

    if ((b->type == BUILDING_WAREHOUSE && building_warehouse_is_not_accepting(resource, b)) ||
        (b->type == BUILDING_GRANARY && building_granary_is_not_accepting(resource, b))) {
        return 0;
    }

    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, b)) {
        return 0;
    }

    int route_id = empire_city_get_route_id(city_id);

    if (b->type == BUILDING_GRANARY) {
        int result = building_granary_add_import(b, resource, 0);
        if (result) {
            trade_route_increase_traded(route_id, resource);
        }
        return result;
    }

    // try existing storage bay with the same resource
    building *space = b;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->loads_stored && space->loads_stored < 4 && space->subtype.warehouse_resource_id == resource) {
                trade_route_increase_traded(route_id, resource);
                building_warehouse_space_add_import(space, resource, 0);
                return 1;
            }
        }
    }
    // try unused storage bay
    space = b;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->subtype.warehouse_resource_id == RESOURCE_NONE) {
                trade_route_increase_traded(route_id, resource);
                building_warehouse_space_add_import(space, resource, 0);
                return 1;
            }
        }
    }
    return 0;
}

static int try_export_resource(int building_id, int resource, int city_id)
{
    building *b = building_get(building_id);
    if (b->type != BUILDING_WAREHOUSE && b->type != BUILDING_GRANARY) {
        return 0;
    }

    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, b)) {
        return 0;
    }

    if (b->type == BUILDING_GRANARY) {
        int result = building_granary_remove_export(b, resource, 0);
        if (result) {
            trade_route_increase_traded(empire_city_get_route_id(city_id), resource);
        }
        return result;
    }

    building *space = b;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0) {
            if (space->loads_stored && space->subtype.warehouse_resource_id == resource) {
                trade_route_increase_traded(empire_city_get_route_id(city_id), resource);
                building_warehouse_space_remove_export(space, resource, 0);
                return 1;
            }
        }
    }
    return 0;
}

static int store_destination_map_point(int building_id, map_point *dst)
{
    if (!building_id) {
        return 0;
    }
    building *b = building_get(building_id);
    if (b->type == BUILDING_GRANARY) {
        // go to center of granary
        map_point_store_result(b->x + 1, b->y + 1, dst);
    } else if (b->has_road_access == 1) {
        map_point_store_result(b->x, b->y, dst);
    } else if (!map_has_road_access_rotation(b->subtype.orientation, b->x, b->y, 3, dst)) {
        return 0;
    }
    return 1;
}

static int is_invalid_destination(building *b, building *dock)
{
    return b->state != BUILDING_STATE_IN_USE ||
        !b->has_road_access || b->distance_from_entry <= 0 ||
        b->road_network_id != dock->road_network_id ||
        !building_storage_get_permission(BUILDING_STORAGE_PERMISSION_DOCK, b);
}

static int get_closest_building_for_import(int x, int y, int city_id, building *dock,
    map_point *dst, int *import_resource)
{
    int resource = *import_resource;
    if (resource == RESOURCE_NONE) {
        int importable[16];
        importable[RESOURCE_NONE] = 0;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            importable[r] = building_distribution_is_good_accepted(r - 1, dock) &&
                empire_can_import_resource_from_city(city_id, r);
        }
        resource = city_trade_next_docker_import_resource();
        for (int i = RESOURCE_MIN; i < RESOURCE_MAX && !importable[resource]; i++) {
            resource = city_trade_next_docker_import_resource();
        }
        if (!importable[resource]) {
            return 0;
        }
    }
    int min_distance = INFINITE;
    int min_building_id = 0;
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (is_invalid_destination(b, dock) ||
            building_storage_get(b->storage_id)->empty_all ||
            building_warehouse_is_not_accepting(resource, b)) {
            continue;
        }
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
        if (distance_penalty == 32) {
            continue;
        }
        int distance = calc_maximum_distance(b->x, b->y, x, y);
        // prefer emptier warehouse
        distance += distance_penalty;
        if (distance < min_distance) {
            min_distance = distance;
            min_building_id = b->id;
        }
    }
    if (resource_is_food(resource)) {
        for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
            if (is_invalid_destination(b, dock) ||
                building_storage_get(b->storage_id)->empty_all ||
                building_granary_is_not_accepting(resource, b) ||
                building_granary_is_full(resource, b)) {
                continue;
            }
            // always prefer granary
            int distance = calc_maximum_distance(b->x, b->y, x, y);
            if (distance < min_distance) {
                min_distance = distance;
                min_building_id = b->id;
            }
        }
    }
    if (!store_destination_map_point(min_building_id, dst)) {
        return 0;
    }
    *import_resource = resource;
    return min_building_id;
}

static int get_closest_building_for_export(int x, int y, int city_id, building *dock,
    map_point *dst, int *export_resource)
{
    int resource = *export_resource;
    if (resource == RESOURCE_NONE) {
        int exportable[16];
        exportable[RESOURCE_NONE] = 0;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            exportable[r] = building_distribution_is_good_accepted(r - 1, dock) &&
                empire_can_export_resource_to_city(city_id, r);
        }
        resource = city_trade_next_docker_export_resource();
        for (int i = RESOURCE_MIN; i < RESOURCE_MAX && !exportable[resource]; i++) {
            resource = city_trade_next_docker_export_resource();
        }
        if (!exportable[resource]) {
            return 0;
        }
    }
    int min_distance = INFINITE;
    int min_building_id = 0;
    for (building *b = building_first_of_type(BUILDING_WAREHOUSE); b; b = b->next_of_type) {
        if (is_invalid_destination(b, dock)) {
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
        if (distance_penalty == 32) {
            continue;
        }
        int distance = calc_maximum_distance(b->x, b->y, x, y);
        // prefer fuller warehouse
        distance += distance_penalty;
        if (distance < min_distance) {
            min_distance = distance;
            min_building_id = b->id;
        }
    }
    if (resource_is_food(resource) && config_get(CONFIG_GP_CH_ALLOW_EXPORTING_FROM_GRANARIES)) {
        for (building *b = building_first_of_type(BUILDING_GRANARY); b; b = b->next_of_type) {
            if (is_invalid_destination(b, dock) ||
                !building_granary_resource_amount(resource, b)) {
                continue;
            }
            int distance = calc_maximum_distance(b->x, b->y, x, y);
            // avoid granaries
            distance += 31;
            if (distance < min_distance) {
                min_distance = distance;
                min_building_id = b->id;
            }
        }
    }
    if (!store_destination_map_point(min_building_id, dst)) {
        return 0;
    }
    *export_resource = resource;
    return min_building_id;
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
    map_point tile;
    int resource = f->resource_id;
    int destination_id = get_closest_building_for_import(f->x, f->y, ship->empire_city_id,
        dock, &tile, &resource);
    if (!destination_id) {
        return 0;
    }
    if (!f->destination_building_id) {
        ship->loads_sold_or_carrying--;
        f->action_state = FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE;
    } else {
        f->action_state = FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_STORAGE;
    }
    if (f->destination_building_id != destination_id) {
        figure_route_remove(f);
    }
    f->destination_building_id = destination_id;
    f->wait_ticks = 0;
    f->destination_x = tile.x;
    f->destination_y = tile.y;
    f->resource_id = resource;
    return 1;
}

static int fetch_export_resource(figure *f, building *dock, int add_to_bought)
{
    int ship_id = dock->data.dock.trade_ship_id;
    if (!ship_id) {
        return 0;
    }
    figure *ship = figure_get(ship_id);
    if (ship->action_state != FIGURE_ACTION_112_TRADE_SHIP_MOORED ||
        (add_to_bought && ship->trader_amount_bought >= figure_trade_sea_trade_units())) {
        return 0;
    }
    map_point tile;
    int resource = f->resource_id;
    int destination_id = get_closest_building_for_export(f->x, f->y, ship->empire_city_id,
        dock, &tile, &resource);
    if (!destination_id) {
        return 0;
    }
    if (add_to_bought) {
        ship->trader_amount_bought++;
    }
    if (f->destination_building_id != destination_id) {
        figure_route_remove(f);
    }
    f->destination_building_id = destination_id;
    f->action_state = FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_STORAGE;
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

static void set_docker_as_idle(figure *f)
{
    f->action_state = FIGURE_ACTION_132_DOCKER_IDLING;
    f->resource_id = RESOURCE_NONE;
    f->destination_building_id = 0;
    f->wait_ticks = 0;
}

void figure_docker_action(figure *f)
{
    building *b = building_get(f->building_id);

    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    if (b->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (b->type != BUILDING_DOCK) {
        f->state = FIGURE_STATE_DEAD;
    }
    if (b->data.dock.num_ships) {
        b->data.dock.num_ships--;
    }
    if (b->data.dock.trade_ship_id) {
        figure *ship = figure_get(b->data.dock.trade_ship_id);
        if (ship->state != FIGURE_STATE_ALIVE || ship->type != FIGURE_TRADE_SHIP) {
            b->data.dock.trade_ship_id = 0;
        } else if (ship->action_state == FIGURE_ACTION_115_TRADE_SHIP_LEAVING) {
            b->data.dock.trade_ship_id = 0;
        }
    }
    f->terrain_usage = TERRAIN_USAGE_ROADS_HIGHWAY;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_132_DOCKER_IDLING:
            f->cart_image_id = 0;
            if (!deliver_import_resource(f, b)) {
                fetch_export_resource(f, b, 1);
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
                    f->action_state = FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_STORAGE;
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
                    set_docker_as_idle(f);
                    f->image_id = 0;
                    f->cart_image_id = 0;
                    b->data.dock.queued_docker_id = 0;
                }
            }
            f->wait_ticks++;
            if (f->wait_ticks >= 20) {
                set_docker_as_idle(f);
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_STORAGE:
            set_cart_graphic(f);
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_139_DOCKER_IMPORT_AT_STORAGE;
                f->wait_ticks = 0;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->wait_ticks++ > FIGURE_REROUTE_DESTINATION_TICKS) {
                if (!deliver_import_resource(f, b)) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE &&
                !deliver_import_resource(f, b)) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_STORAGE:
            f->cart_image_id = image_group(GROUP_FIGURE_CARTPUSHER_CART); // empty
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_140_DOCKER_EXPORT_AT_STORAGE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->wait_ticks++ > FIGURE_REROUTE_DESTINATION_TICKS) {
                if (!fetch_export_resource(f, b, 0)) {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE &&
                !fetch_export_resource(f, b, 0)) {
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
                set_docker_as_idle(f);
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_139_DOCKER_IMPORT_AT_STORAGE:
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
                    city_health_update_sickness_level_in_building(b->id);
                    city_health_dispatch_sickness(f);
                    f->action_state = FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING;
                    f->wait_ticks = 0;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                    f->resource_id = 0;
                    fetch_export_resource(f, b, 1);
                } else {
                    f->action_state = FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING;
                    f->destination_x = f->source_x;
                    f->destination_y = f->source_y;
                }
                f->wait_ticks = 0;
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_140_DOCKER_EXPORT_AT_STORAGE:
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
                    city_health_update_sickness_level_in_building(b->id);
                    city_health_dispatch_sickness(f);
                    f->action_state = FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING;
                } else {
                    fetch_export_resource(f, b, 1);
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
