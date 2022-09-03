#include "dock.h"

#include "building/market.h"
#include "city/buildings.h"
#include "city/resource.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "figure/figure.h"
#include "figure/trader.h"
#include "figuretype/trader.h"
#include "game/resource.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/routing_data.h"
#include "map/terrain.h"
#include "scenario/map.h"

#include <string.h>

#define MAX_DISTANCE_FOR_REROUTING 50

typedef struct handled_good {
    unsigned char road_network_id;
    int goods[RESOURCE_MAX - 1];
} handled_good;

int building_dock_count_idle_dockers(const building *dock)
{
    int num_idle = 0;
    for (int i = 0; i < 3; i++) {
        if (dock->data.dock.docker_ids[i]) {
            figure *f = figure_get(dock->data.dock.docker_ids[i]);
            if (f->action_state == FIGURE_ACTION_132_DOCKER_IDLING ||
                f->action_state == FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE) {
                num_idle++;
            }
        }
    }
    return num_idle;
}

void building_dock_update_open_water_access(void)
{
    map_point river_entry = scenario_map_river_entry();
    map_routing_calculate_distances_water_boat(river_entry.x, river_entry.y);
    for (building *b = building_first_of_type(BUILDING_DOCK); b; b = b->next_of_type) {
        if (b->state == BUILDING_STATE_IN_USE && !b->house_size) {
            if (map_terrain_is_adjacent_to_open_water(b->x, b->y, 3)) {
                b->has_water_access = 1;
            } else {
                b->has_water_access = 0;
            }
        }
    }
}

int building_dock_is_connected_to_open_water(int x, int y)
{
    map_point river_entry = scenario_map_river_entry();
    map_routing_calculate_distances_water_boat(river_entry.x, river_entry.y);
    if (map_terrain_is_adjacent_to_open_water(x, y, 3)) {
        return 1;
    } else {
        return 0;
    }
}

int building_dock_accepts_ship(int ship_id, int dock_id)
{
    building *dock = building_get(dock_id);
    figure *f = figure_get(ship_id);
    empire_city *city = empire_city_get(f->empire_city_id);
    if (!building_dock_can_trade_with_route(city->route_id, dock_id)) {
        return 0;
    }
    for (int resource = RESOURCE_WHEAT; resource < RESOURCE_MAX; resource++) {
        if (city->sells_resource[resource] || city->buys_resource[resource]) {
            if (building_distribution_is_good_accepted(resource - 1, dock)) {
                return 1;
            }
        }
    }
    return 0;
}

int building_dock_can_import_from_ship(building *dock, int ship_id)
{
    figure *ship = figure_get(ship_id);
    if (trader_has_sold_max(ship->trader_id)) {
        return 0;
    }

    // dock has plague, trading is disabled
    if (dock->has_plague) {
        return 0;
    }

    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (building_distribution_is_good_accepted(r - 1, dock)) {
            return 1;
        }
    }
    return 0;
}

int building_dock_can_export_to_ship(building *dock, int ship_id)
{
    figure *ship = figure_get(ship_id);
    if (trader_has_bought_max(ship->trader_id)) {
        return 0;
    }

    // dock has plague, trading is disabled
    if (dock->has_plague) {
        return 0;
    }

    for (resource_type r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (building_distribution_is_good_accepted(r - 1, dock)) {
            return 1;
        }
    }
    return 0;
}

// returns a list of goods that have been "handled" (i.e. the dock allowed for it to be traded) for each road network a ship has visited
static void get_already_handled_goods(handled_good *handled_goods, int ship_id)
{
    memset(handled_goods, 0, sizeof(handled_good) * MAX_DOCKS);
    figure *ship = figure_get(ship_id);

    // loop through the docks
    for (int i = 0; i < MAX_DOCKS; i++) {
        // check and see if the ship has visited this dock
        if (!figure_trader_ship_already_docked_at(ship, i)) {
            continue;
        }

        // get the actual dock
        int dock_id = city_buildings_get_working_dock(i);
        if (!dock_id) {
            continue;
        }
        building *dock = building_get(dock_id);

        // find the handled_good that is on this road network or find the next one that hasn't been assigned to a road network yet
        handled_good *current_handled_good;
        for (int j = 0; j < MAX_DOCKS; j++) {
            current_handled_good = &handled_goods[j];
            if (!current_handled_good->road_network_id || current_handled_good->road_network_id == dock->road_network_id) {
                break;
            }
        }

        // assign the road network (in case this is a new one) and add the goods this dock handles
        current_handled_good->road_network_id = dock->road_network_id;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (building_distribution_is_good_accepted(r - 1, dock)) {
                current_handled_good->goods[r - 1] = 1;
            }
        }
    }
}

static int all_dock_goods_already_handled(handled_good *handled_goods, building *dock, figure *ship)
{
    for (int i = 0; i < MAX_DOCKS; i++) {
        handled_good *handled_good = &handled_goods[i];
        if (handled_good->road_network_id != dock->road_network_id) {
            continue;
        }
        // we've visited docks on this road network
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (!empire_can_import_resource_from_city(ship->empire_city_id, r) && !empire_can_export_resource_to_city(ship->empire_city_id, r)) {
                // the ship doesn't buy or sell this good
                continue;
            }
            if (building_distribution_is_good_accepted(r - 1, dock) && !handled_good->goods[r - 1]) {
                // this dock accepts a good that all previous docks on this road network did not accept
                return 0;
            }
        }
        // all goods at this dock have already been handled on this road network
        return 1;
    }
    // no matching road networks, assume unhandled
    return 0;
}

static int get_free_destination(int ship_id, int exclude_dock_id, map_point *tile, handled_good *handled_goods)
{
    figure *ship = figure_get(ship_id);
    int importing_dock_id = 0;
    int exporting_dock_id = 0;
    int dock_id = 0;

    for (int i = 0; i < MAX_DOCKS; i++) {
        dock_id = city_buildings_get_working_dock(i);
        if (!dock_id ||
            dock_id == exclude_dock_id ||
            figure_trader_ship_docked_once_at_dock(ship, dock_id) ||
            !building_dock_accepts_ship(ship_id, dock_id)) {
            continue;
        }

        building *dock = building_get(dock_id);
        if (dock->data.dock.trade_ship_id) {
            continue;
        }

        if (all_dock_goods_already_handled(handled_goods, dock, ship)) {
            continue;
        }

        if (building_dock_can_import_from_ship(dock, ship_id)) {
            importing_dock_id = dock_id;
            // prioritize imports
            break;
        } else if (building_dock_can_export_to_ship(dock, ship_id)) {
            exporting_dock_id = dock_id;
        }
    }
    dock_id = importing_dock_id ? importing_dock_id : exporting_dock_id;
    if (!dock_id) {
        return 0;
    }
    building *dock = building_get(dock_id);
    building_dock_get_ship_request_tile(dock, SHIP_DOCK_REQUEST_2_FIRST_QUEUE, tile);
    return dock_id;
}


static int get_queue_destination(int ship_id, int exclude_dock_id, ship_dock_request_type request_type, map_point *tile, handled_good *handled_goods)
{
    figure *ship = figure_get(ship_id);
    int importing_dock_id = 0;
    int exporting_dock_id = 0;

    for (int i = 0; i < MAX_DOCKS; i++) {
        int dock_id = city_buildings_get_working_dock(i);
        if (!dock_id ||
            dock_id == exclude_dock_id ||
            figure_trader_ship_docked_once_at_dock(ship, dock_id) ||
            !building_dock_accepts_ship(ship_id, dock_id)) {
            continue;
        }
        building *dock = building_get(dock_id);
        if (all_dock_goods_already_handled(handled_goods, dock, ship)) {
            continue;
        }

        map_point requested_tile;
        building_dock_get_ship_request_tile(dock, request_type, &requested_tile);

        int figure_at_offset = map_figure_at(map_grid_offset(requested_tile.x, requested_tile.y));
        if (figure_at_offset && figure_at_offset != ship_id) {
            figure *ship_at_offset = figure_get(figure_at_offset);
            if (ship_at_offset->action_state == FIGURE_ACTION_114_TRADE_SHIP_ANCHORED) {
                continue;
            }
        }

        if (figure_trader_ship_can_queue_for_import(ship) && building_dock_can_import_from_ship(dock, ship_id)) {
            importing_dock_id = dock_id;
            map_point_store_result(requested_tile.x, requested_tile.y, tile);
            break;  // prioritize imports
        } else if (figure_trader_ship_can_queue_for_export(ship) && building_dock_can_export_to_ship(dock, ship_id)) {
            map_point_store_result(requested_tile.x, requested_tile.y, tile);
            exporting_dock_id = dock_id;
        }
    }

    return importing_dock_id ? importing_dock_id : exporting_dock_id;
}

static int destination_dock_ready_for_ship(figure *ship)
{
    building *destination_dock = building_get(ship->destination_building_id);
    if (destination_dock->data.dock.trade_ship_id &&
        destination_dock->data.dock.trade_ship_id != ship->id) {
        return 0;
    }

    if (!building_dock_is_working(destination_dock->id) ||
        !building_dock_accepts_ship(ship->id, destination_dock->id)) {
        return 0;
    }

    if (!building_dock_can_import_from_ship(destination_dock, ship->id) &&
        !building_dock_can_export_to_ship(destination_dock, ship->id)) {
        return 0;
    }
    return 1;
}

int building_dock_get_destination(int ship_id, int exclude_dock_id, map_point *tile)
{
    if (!city_buildings_has_working_dock()) {
        return 0;
    }

    handled_good handled_goods[MAX_DOCKS];
    get_already_handled_goods(handled_goods, ship_id);

    int dock_id = 0;
    if ((dock_id = get_free_destination(ship_id, exclude_dock_id, tile, handled_goods))) {
        return dock_id;
    } else if ((dock_id = get_queue_destination(ship_id, exclude_dock_id, SHIP_DOCK_REQUEST_2_FIRST_QUEUE, tile, handled_goods))) {
        return dock_id;
    } else {
        return get_queue_destination(ship_id, exclude_dock_id, SHIP_DOCK_REQUEST_4_SECOND_QUEUE, tile, handled_goods);
    }
}

int building_dock_get_closer_free_destination(int ship_id, ship_dock_request_type request_type, map_point *tile)
{
    figure *ship = figure_get(ship_id);
    int distance_to_destination = figure_trader_ship_get_distance_to_dock(ship, ship->destination_building_id);
    int min_distance_import = -1, min_distance_export = -1;
    int nearest_import_dock_id = 0, nearest_export_dock_id = 0;
    for (int i = 0; i < MAX_DOCKS; i++) {
        int dock_id = city_buildings_get_working_dock(i);
        if (!dock_id) {
            continue;
        }

        building *dock = building_get(dock_id);
        if (dock->data.dock.trade_ship_id ||
            dock_id == ship->destination_building_id ||
            figure_trader_ship_docked_once_at_dock(ship, dock_id) ||
            !building_dock_accepts_ship(ship_id, dock_id)) {
            continue;
        }

        int distance_to_dock = figure_trader_ship_get_distance_to_dock(ship, dock_id);
        if (distance_to_dock > MAX_DISTANCE_FOR_REROUTING ||
            (figure_trader_ship_other_ship_closer_to_dock(ship_id, dock_id, distance_to_dock))) {
            continue;
        }

        if (ship->action_state == FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE &&
            destination_dock_ready_for_ship(ship) &&
            distance_to_destination < distance_to_dock) {
            continue;
        }

        if (building_dock_can_import_from_ship(dock, ship_id)) {
            if (min_distance_import == -1 || distance_to_dock < min_distance_import) {
                nearest_import_dock_id = dock_id;
                min_distance_import = distance_to_dock;
            }
        }

        if (building_dock_can_export_to_ship(dock, ship_id)) {
            if (min_distance_export == -1 || distance_to_dock < min_distance_export) {
                nearest_export_dock_id = dock_id;
                min_distance_export = distance_to_dock;
            }
        }
    }

    int dock_id = 0;
    if (nearest_import_dock_id) {
        if (nearest_export_dock_id && min_distance_export < min_distance_import + MAX_DISTANCE_FOR_REROUTING) {
            dock_id = nearest_export_dock_id;
        } else {
            dock_id = nearest_import_dock_id;
        }
    } else if (nearest_export_dock_id) {
        dock_id = nearest_export_dock_id;
    }

    if (dock_id) {
        building_dock_get_ship_request_tile(building_get(dock_id), request_type, tile);
    }

    return dock_id;
}

int building_dock_can_trade_with_route(int route_id, int dock_id)
{
    building *dock = building_get(dock_id);
    if (!dock->data.dock.has_accepted_route_ids) {
        return 1;
    }
    return dock->data.dock.accepted_route_ids & (1 << route_id);
}

void building_dock_set_can_trade_with_route(int route_id, int dock_id, int can_trade)
{
    building *dock = building_get(dock_id);
    if (!dock->data.dock.has_accepted_route_ids) {
        dock->data.dock.has_accepted_route_ids = 1;
        dock->data.dock.accepted_route_ids = 0xffffffff;
    }
    int mask = 1 << route_id;
    if (can_trade) {
        dock->data.dock.accepted_route_ids |= mask;
    } else {
        dock->data.dock.accepted_route_ids &= ~mask;
    }
}

int building_dock_request_docking(int ship_id, int dock_id, map_point *tile)
{
    building *dock = building_get(dock_id);
    if ((!dock->data.dock.trade_ship_id || dock->data.dock.trade_ship_id == ship_id)) {
        building_dock_get_ship_request_tile(dock, SHIP_DOCK_REQUEST_1_DOCKING, tile);
        return 1;
    }
    return 0;
}

void building_dock_get_ship_request_tile(const building *dock, ship_dock_request_type request_type, map_point *tile)
{
    int dx, dy, grid_offset;
    switch (request_type) {
        case SHIP_DOCK_REQUEST_1_DOCKING:
            switch (dock->data.dock.orientation) {
                case 0: dx = 1; dy = -1; break;
                case 1: dx = 3; dy = 1; break;
                case 2: dx = 1; dy = 3; break;
                default: dx = -1; dy = 1; break;
            }
            break;
        case SHIP_DOCK_REQUEST_2_FIRST_QUEUE:
            switch (dock->data.dock.orientation) {
                case 0: dx = 2; dy = -2; break;
                case 1: dx = 4; dy = 2; break;
                case 2: dx = 2; dy = 4; break;
                default: dx = -2; dy = 2; break;
            }
            grid_offset = map_grid_offset(dock->x + dx, dock->y + dy);
            if (!map_terrain_is(grid_offset, TERRAIN_WATER) || terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
                switch (dock->data.dock.orientation) {
                    case 0: dx = 0; dy = -1; break;
                    case 1: dx = 3; dy = 0; break;
                    case 2: dx = 2; dy = 3; break;
                    default: dx = -1; dy = 2; break;
                }
            }
            if (!map_terrain_is(grid_offset, TERRAIN_WATER) || terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
                switch (dock->data.dock.orientation) {
                    case 0: dx = 1; dy = 0; break;
                    case 1: dx = 2; dy = 1; break;
                    case 2: dx = 1; dy = 4; break;
                    default: dx = -2; dy = 1; break;
                }
            }
            break;
        case SHIP_DOCK_REQUEST_4_SECOND_QUEUE:
        default:
            switch (dock->data.dock.orientation) {
                case 0: dx = 2; dy = -3; break;
                case 1: dx = 5; dy = 2; break;
                case 2: dx = 2; dy = 5; break;
                default: dx = -3; dy = 2; break;
            }
            grid_offset = map_grid_offset(dock->x + dx, dock->y + dy);
            if (!map_terrain_is(grid_offset, TERRAIN_WATER) || terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
                switch (dock->data.dock.orientation) {
                    case 0: dx = 2; dy = -1; break;
                    case 1: dx = 3; dy = 2; break;
                    case 2: dx = 0; dy = 3; break;
                    default: dx = -1; dy = 0; break;
                }
            }
            if (!map_terrain_is(grid_offset, TERRAIN_WATER) || terrain_water.items[grid_offset] == WATER_N1_BLOCKED) {
                switch (dock->data.dock.orientation) {
                    case 0: dx = 0; dy = -3; break;
                    case 1: dx = 5; dy = 0; break;
                    case 2: dx = 0; dy = 4; break;
                    default: dx = -1; dy = 0; break;
                }
            }
            break;
    }
    map_point_store_result(dock->x + dx, dock->y + dy, tile);
}

int building_dock_is_working(int dock_id)
{
    building *b = building_get(dock_id);
    return b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_DOCK && b->num_workers > 0 && !b->has_plague;
}

int building_dock_reposition_anchored_ship(int ship_id, map_point *tile)
{
    figure *ship = figure_get(ship_id);
    building *dock = building_get(ship->destination_building_id);
    map_point tile_first_queue;
    map_point tile_second_queue;
    building_dock_get_ship_request_tile(dock, SHIP_DOCK_REQUEST_2_FIRST_QUEUE, &tile_first_queue);
    building_dock_get_ship_request_tile(dock, SHIP_DOCK_REQUEST_4_SECOND_QUEUE, &tile_second_queue);
    if (map_figure_at(ship->grid_offset) != ship->id) {
        if (ship->grid_offset == map_grid_offset(tile_first_queue.x, tile_first_queue.y) && !map_has_figure_at(map_grid_offset(tile_second_queue.x, tile_second_queue.y))) {
            map_point_store_result(tile_second_queue.x, tile_second_queue.y, tile);
            return ship->destination_building_id;
        } else if (ship->grid_offset == map_grid_offset(tile_second_queue.x, tile_second_queue.y) && !map_has_figure_at(map_grid_offset(tile_first_queue.x, tile_first_queue.y))) {
            map_point_store_result(tile_first_queue.x, tile_first_queue.y, tile);
            return ship->destination_building_id;
        } else {    // prefer emptier dock queue, free or not, as this one has more than two waiting
            return building_dock_get_destination(ship_id, ship->destination_building_id, tile);
        }
    }
    return 0;
}
