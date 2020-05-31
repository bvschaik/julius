#include "trader.h"

#include "building/building.h"
#include "building/dock.h"
#include "building/warehouse.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/map.h"
#include "city/message.h"
#include "city/resource.h"
#include "city/trade.h"
#include "core/calc.h"
#include "core/image.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "map/figure.h"
#include "map/road_access.h"
#include "scenario/map.h"

int figure_create_trade_caravan(int x, int y, int city_id)
{
    figure *caravan = figure_create(FIGURE_TRADE_CARAVAN, x, y, DIR_0_TOP);
    caravan->empire_city_id = city_id;
    caravan->action_state = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    caravan->wait_ticks = 10;
    // donkey 1
    figure *donkey1 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY, x, y, DIR_0_TOP);
    donkey1->action_state = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    donkey1->leading_figure_id = caravan->id;
    // donkey 2
    figure *donkey2 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY, x, y, DIR_0_TOP);
    donkey2->action_state = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    donkey2->leading_figure_id = donkey1->id;
    return caravan->id;
}

int figure_create_trade_ship(int x, int y, int city_id)
{
    figure *ship = figure_create(FIGURE_TRADE_SHIP, x, y, DIR_0_TOP);
    ship->empire_city_id = city_id;
    ship->action_state = FIGURE_ACTION_110_TRADE_SHIP_CREATED;
    ship->wait_ticks = 10;
    return ship->id;
}

int figure_trade_caravan_can_buy(figure *trader, int warehouse_id, int city_id)
{
    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }
    if (trader->trader_amount_bought >= 8) {
        return 0;
    }
    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_TRADERS, warehouse)) {
        return 0;
    }
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && space->loads_stored > 0 &&
            empire_can_export_resource_to_city(city_id, space->subtype.warehouse_resource_id)) {
            return 1;
        }
    }
    return 0;
}

int figure_trade_caravan_can_sell(figure *trader, int warehouse_id, int city_id)
{
    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }
    if (trader->loads_sold_or_carrying >= 8) {
        return 0;
    }
    if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_TRADERS, warehouse)) {
        return 0;
    }
    const building_storage *storage = building_storage_get(warehouse->storage_id);
    if (storage->empty_all) {
        return 0;
    }
    int num_importable = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (!building_warehouse_is_not_accepting(r,warehouse)) {
            if (empire_can_import_resource_from_city(city_id, r)) {
                num_importable++;
            }
        }
    }
    if (num_importable <= 0) {
        return 0;
    }
    int can_import = 0;
    int resource = city_trade_current_caravan_import_resource();
    if (!building_warehouse_is_not_accepting(resource,warehouse) &&
        empire_can_import_resource_from_city(city_id, resource)) {
        can_import = 1;
    } else {
        for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
            resource = city_trade_next_caravan_import_resource();
            if (!building_warehouse_is_not_accepting(resource,warehouse) &&
                    empire_can_import_resource_from_city(city_id, resource)) {
                can_import = 1;
                break;
            }
        }
    }
    if (can_import) {
        // at least one resource can be imported and accepted by this warehouse
        // check if warehouse can store any importable goods
        building *space = warehouse;
        for (int s = 0; s < 8; s++) {
            space = building_next(space);
            if (space->id > 0 && space->loads_stored < 4) {
                if (!space->loads_stored) {
                    // empty space
                    return 1;
                }
                if (empire_can_import_resource_from_city(city_id, space->subtype.warehouse_resource_id)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int trader_get_buy_resource(int warehouse_id, int city_id)
{
    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return RESOURCE_NONE;
    }
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id <= 0) {
            continue;
        }
        int resource = space->subtype.warehouse_resource_id;
        if (space->loads_stored > 0 && empire_can_export_resource_to_city(city_id, resource)) {
            // update stocks
            city_resource_remove_from_warehouse(resource, 1);
            space->loads_stored--;
            if (space->loads_stored <= 0) {
                space->subtype.warehouse_resource_id = RESOURCE_NONE;
            }
            // update finances
            city_finance_process_export(trade_price_sell(resource));

            // update graphics
            building_warehouse_space_set_image(space, resource);
            return resource;
        }
    }
    return 0;
}

static int trader_get_sell_resource(int warehouse_id, int city_id)
{
    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }
    int resource_to_import = city_trade_current_caravan_import_resource();
    int imp = RESOURCE_MIN;
    while (imp < RESOURCE_MAX && !empire_can_import_resource_from_city(city_id, resource_to_import)) {
        imp++;
        resource_to_import = city_trade_next_caravan_import_resource();
    }
    if (imp >= RESOURCE_MAX) {
        return 0;
    }
    // add to existing bay with room
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && space->loads_stored > 0 && space->loads_stored < 4 &&
            space->subtype.warehouse_resource_id == resource_to_import) {
            building_warehouse_space_add_import(space, resource_to_import);
            city_trade_next_caravan_import_resource();
            return resource_to_import;
        }
    }
    // add to empty bay
    space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && !space->loads_stored) {
            building_warehouse_space_add_import(space, resource_to_import);
            city_trade_next_caravan_import_resource();
            return resource_to_import;
        }
    }
    // find another importable resource that can be added to this warehouse
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        resource_to_import = city_trade_next_caravan_backup_import_resource();
        if (empire_can_import_resource_from_city(city_id, resource_to_import)) {
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = building_next(space);
                if (space->id > 0 && space->loads_stored < 4 && space->subtype.warehouse_resource_id == resource_to_import) {
                    building_warehouse_space_add_import(space, resource_to_import);
                    return resource_to_import;
                }
            }
        }
    }
    return 0;
}

static int get_closest_warehouse(const figure *f, int x, int y, int city_id, int distance_from_entry,
                                 map_point *warehouse)
{
    int exportable[RESOURCE_MAX];
    int importable[RESOURCE_MAX];
    exportable[RESOURCE_NONE] = 0;
    importable[RESOURCE_NONE] = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        exportable[r] = empire_can_export_resource_to_city(city_id, r);
        if (f->trader_amount_bought >= 8) {
            exportable[r] = 0;
        }
        if (city_id) {
            importable[r] = empire_can_import_resource_from_city(city_id, r);
        } else { // exclude own city (id=0), shouldn't happen, but still..
            importable[r] = 0;
        }
        if (f->loads_sold_or_carrying >= 8) {
            importable[r] = 0;
        }
    }
    int num_importable = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (importable[r]) {
            num_importable++;
        }
    }
    int min_distance = 10000;
    building *min_building = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_WAREHOUSE) {
            continue;
        }
        if (!b->has_road_access || b->distance_from_entry <= 0) {
            continue;
        }
        if (!building_storage_get_permission(BUILDING_STORAGE_PERMISSION_TRADERS, b)) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        int num_imports_for_warehouse = 0;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (!building_warehouse_is_not_accepting(r,b) && empire_can_import_resource_from_city(city_id, r)) {
                num_imports_for_warehouse++;
            }
        }
        int distance_penalty = 32;
        building *space = b;
        for (int space_cnt = 0; space_cnt < 8; space_cnt++) {
            space = building_next(space);
            if (space->id && exportable[space->subtype.warehouse_resource_id]) {
                distance_penalty -= 4;
            }
            if (num_importable && num_imports_for_warehouse && !s->empty_all) {
                for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                    if (!building_warehouse_is_not_accepting(city_trade_next_caravan_import_resource(),b)) {
                        break;
                    }
                }
                int resource = city_trade_current_caravan_import_resource();
                if (!building_warehouse_is_not_accepting(resource,b)) {
                    if (space->subtype.warehouse_resource_id == RESOURCE_NONE) {
                        distance_penalty -= 16;
                    }
                    if (space->id && importable[space->subtype.warehouse_resource_id] && space->loads_stored < 4 &&
                        space->subtype.warehouse_resource_id == resource) {
                        distance_penalty -= 8;
                    }
                }
            }
        }
        if (distance_penalty < 32) {
            int distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distance_from_entry);
            distance += distance_penalty;
            if (distance < min_distance) {
                min_distance = distance;
                min_building = b;
            }
        }
    }
    if (!min_building) {
        return 0;
    }
    if (min_building->has_road_access == 1) {
        map_point_store_result(min_building->x, min_building->y, warehouse);
    } else if (!map_has_road_access(min_building->x, min_building->y, 3, warehouse)) {
        return 0;
    }
    return min_building->id;
}

static void go_to_next_warehouse(figure *f, int x_src, int y_src, int distance_to_entry)
{
    map_point dst;
    int warehouse_id = get_closest_warehouse(f, x_src, y_src, f->empire_city_id, distance_to_entry, &dst);
    if (warehouse_id) {
        f->destination_building_id = warehouse_id;
        f->action_state = FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING;
        f->destination_x = dst.x;
        f->destination_y = dst.y;
    } else {
        const map_tile *exit = city_map_exit_point();
        f->action_state = FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING;
        f->destination_x = exit->x;
        f->destination_y = exit->y;
    }
}

void figure_trade_caravan_action(figure *f)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_100_TRADE_CARAVAN_CREATED:
            f->is_ghost = 1;
            f->wait_ticks++;
            if (f->wait_ticks > 20) {
                f->wait_ticks = 0;
                int x_base, y_base;
                int trade_center_id = city_buildings_get_trade_center();
                if (trade_center_id) {
                    building *trade_center = building_get(trade_center_id);
                    x_base = trade_center->x;
                    y_base = trade_center->y;
                } else {
                    x_base = f->x;
                    y_base = f->y;
                }
                go_to_next_warehouse(f, x_base, y_base, 0);
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING:
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->action_state = FIGURE_ACTION_102_TRADE_CARAVAN_TRADING;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    f->state = FIGURE_STATE_DEAD;
                    f->is_ghost = 1;
                    break;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_102_TRADE_CARAVAN_TRADING:
            f->wait_ticks++;
            if (f->wait_ticks > 10) {
                f->wait_ticks = 0;
                int move_on = 0;
                if (figure_trade_caravan_can_buy(f, f->destination_building_id, f->empire_city_id)) {
                    int resource = trader_get_buy_resource(f->destination_building_id, f->empire_city_id);
                    if (resource) {
                        trade_route_increase_traded(empire_city_get_route_id(f->empire_city_id), resource);
                        trader_record_bought_resource(f->trader_id, resource);
                        f->trader_amount_bought++;
                    } else {
                        move_on++;
                    }
                } else {
                    move_on++;
                }
                if (figure_trade_caravan_can_sell(f, f->destination_building_id, f->empire_city_id)) {
                    int resource = trader_get_sell_resource(f->destination_building_id, f->empire_city_id);
                    if (resource) {
                        trade_route_increase_traded(empire_city_get_route_id(f->empire_city_id), resource);
                        trader_record_sold_resource(f->trader_id, resource);
                        f->loads_sold_or_carrying++;
                    } else {
                        move_on++;
                    }
                } else {
                    move_on++;
                }
                if (move_on == 2) {
                    go_to_next_warehouse(f, f->x, f->y, -1);
                }
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING:
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->action_state = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
                    f->state = FIGURE_STATE_DEAD;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    f->state = FIGURE_STATE_DEAD;
                    break;
            }
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
    f->image_id = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->image_offset;
}

void figure_trade_caravan_donkey_action(figure *f)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_PREFER_ROADS;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;

    figure *leader = figure_get(f->leading_figure_id);
    if (f->leading_figure_id <= 0) {
        f->state = FIGURE_STATE_DEAD;
    } else {
        if (leader->action_state == FIGURE_ACTION_149_CORPSE) {
            f->state = FIGURE_STATE_DEAD;
        } else if (leader->state != FIGURE_STATE_ALIVE) {
            f->state = FIGURE_STATE_DEAD;
        } else if (leader->type != FIGURE_TRADE_CARAVAN && leader->type != FIGURE_TRADE_CARAVAN_DONKEY) {
            f->state = FIGURE_STATE_DEAD;
        } else {
            figure_movement_follow_ticks(f, 1);
        }
    }

    if (leader->is_ghost) {
        f->is_ghost = 1;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
    f->image_id = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->image_offset;
}

void figure_native_trader_action(figure *f)
{
    f->is_ghost = 0;
    f->terrain_usage = TERRAIN_USAGE_ANY;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_163_NATIVE_TRADER_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
                f->is_ghost = 1;
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_161_NATIVE_TRADER_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_162_NATIVE_TRADER_CREATED:
            f->is_ghost = 1;
            f->wait_ticks++;
            if (f->wait_ticks > 10) {
                f->wait_ticks = 0;
                map_point tile;
                int building_id = get_closest_warehouse(f, f->x, f->y, 0, -1, &tile);
                if (building_id) {
                    f->action_state = FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE;
                    f->destination_building_id = building_id;
                    f->destination_x = tile.x;
                    f->destination_y = tile.y;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_163_NATIVE_TRADER_AT_WAREHOUSE:
            f->wait_ticks++;
            if (f->wait_ticks > 10) {
                f->wait_ticks = 0;
                if (figure_trade_caravan_can_buy(f, f->destination_building_id, 0)) {
                    int resource = trader_get_buy_resource(f->destination_building_id, 0);
                    trader_record_bought_resource(f->trader_id, resource);
                    f->trader_amount_bought += 3;
                } else {
                    map_point tile;
                    int building_id = get_closest_warehouse(f, f->x, f->y, 0, -1, &tile);
                    if (building_id) {
                        f->action_state = FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE;
                        f->destination_building_id = building_id;
                        f->destination_x = tile.x;
                        f->destination_y = tile.y;
                    } else {
                        f->action_state = FIGURE_ACTION_161_NATIVE_TRADER_RETURNING;
                        f->destination_x = f->source_x;
                        f->destination_y = f->source_y;
                    }
                }
            }
            f->image_offset = 0;
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);

    if (f->action_state == FIGURE_ACTION_149_CORPSE) {
        f->image_id = image_group(GROUP_FIGURE_CARTPUSHER) + 96 + figure_image_corpse_offset(f);
        f->cart_image_id = 0;
    } else {
        f->image_id = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->image_offset;
    }
    f->cart_image_id = image_group(GROUP_FIGURE_MIGRANT_CART) +
        8 + 8 * f->resource_id; // BUGFIX should be within else statement?
    if (f->cart_image_id) {
        f->cart_image_id += dir;
        figure_image_set_cart_offset(f, dir);
    }
}

int figure_trade_ship_is_trading(figure *ship)
{
    building *b = building_get(ship->destination_building_id);
    if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_DOCK) {
        return TRADE_SHIP_BUYING;
    }
    for (int i = 0; i < 3; i++) {
        figure *f = figure_get(b->data.dock.docker_ids[i]);
        if (!b->data.dock.docker_ids[i] || f->state != FIGURE_STATE_ALIVE) {
            continue;
        }
        switch (f->action_state) {
            case FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE:
            case FIGURE_ACTION_135_DOCKER_IMPORT_GOING_TO_WAREHOUSE:
            case FIGURE_ACTION_138_DOCKER_IMPORT_RETURNING:
            case FIGURE_ACTION_139_DOCKER_IMPORT_AT_WAREHOUSE:
                return TRADE_SHIP_BUYING;
            case FIGURE_ACTION_134_DOCKER_EXPORT_QUEUE:
            case FIGURE_ACTION_136_DOCKER_EXPORT_GOING_TO_WAREHOUSE:
            case FIGURE_ACTION_137_DOCKER_EXPORT_RETURNING:
            case FIGURE_ACTION_140_DOCKER_EXPORT_AT_WAREHOUSE:
                return TRADE_SHIP_SELLING;
        }
    }
    return TRADE_SHIP_NONE;
}

static int trade_ship_lost_queue(const figure *f)
{
    building *b = building_get(f->destination_building_id);
    if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_DOCK &&
        b->num_workers > 0 && b->data.dock.trade_ship_id == f->id) {
        return 0;
    }
    return 1;
}

static int trade_ship_done_trading(figure *f)
{
    building *b = building_get(f->destination_building_id);
    if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_DOCK && b->num_workers > 0) {
        for (int i = 0; i < 3; i++) {
            if (b->data.dock.docker_ids[i]) {
                figure *docker = figure_get(b->data.dock.docker_ids[i]);
                if (docker->state == FIGURE_STATE_ALIVE && docker->action_state != FIGURE_ACTION_132_DOCKER_IDLING) {
                    return 0;
                }
            }
        }
        f->trade_ship_failed_dock_attempts++;
        if (f->trade_ship_failed_dock_attempts >= 10) {
            f->trade_ship_failed_dock_attempts = 11;
            return 1;
        }
        return 0;
    }
    return 1;
}

void figure_trade_ship_action(figure *f)
{
    f->is_ghost = 0;
    f->is_boat = 1;
    figure_image_increase_offset(f, 12);
    f->cart_image_id = 0;
    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_110_TRADE_SHIP_CREATED:
            f->loads_sold_or_carrying = 12;
            f->trader_amount_bought = 0;
            f->is_ghost = 1;
            f->wait_ticks++;
            if (f->wait_ticks > 20) {
                f->wait_ticks = 0;
                map_point tile;
                int dock_id = building_dock_get_free_destination(f->id, &tile);
                if (dock_id) {
                    f->destination_building_id = dock_id;
                    f->action_state = FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK;
                    f->destination_x = tile.x;
                    f->destination_y = tile.y;
                } else if (building_dock_get_queue_destination(&tile)) {
                    f->action_state = FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                    f->destination_x = tile.x;
                    f->destination_y = tile.y;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK:
            figure_movement_move_ticks(f, 1);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_112_TRADE_SHIP_MOORED;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
                if (!city_message_get_category_count(MESSAGE_CAT_BLOCKED_DOCK)) {
                    city_message_post(1, MESSAGE_NAVIGATION_IMPOSSIBLE, 0, 0);
                    city_message_increase_category_count(MESSAGE_CAT_BLOCKED_DOCK);
                }
            }
            if (building_get(f->destination_building_id)->state != BUILDING_STATE_IN_USE) {
                f->action_state = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->wait_ticks = 0;
                map_point river_exit = scenario_map_river_exit();
                f->destination_x = river_exit.x;
                f->destination_y = river_exit.y;
            }
            break;
        case FIGURE_ACTION_112_TRADE_SHIP_MOORED:
            if (trade_ship_lost_queue(f)) {
                f->trade_ship_failed_dock_attempts = 0;
                f->action_state = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->wait_ticks = 0;
                map_point river_entry = scenario_map_river_entry();
                f->destination_x = river_entry.x;
                f->destination_y = river_entry.y;
            } else if (trade_ship_done_trading(f)) {
                f->trade_ship_failed_dock_attempts = 0;
                f->action_state = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->wait_ticks = 0;
                map_point river_entry = scenario_map_river_entry();
                f->destination_x = river_entry.x;
                f->destination_y = river_entry.y;
                building *dst = building_get(f->destination_building_id);
                dst->data.dock.queued_docker_id = 0;
                dst->data.dock.num_ships = 0;
            }
            switch (building_get(f->destination_building_id)->data.dock.orientation) {
                case 0: f->direction = DIR_2_RIGHT; break;
                case 1: f->direction = DIR_4_BOTTOM; break;
                case 2: f->direction = DIR_6_LEFT; break;
                default:f->direction = DIR_0_TOP; break;
            }
            f->image_offset = 0;
            city_message_reset_category_count(MESSAGE_CAT_BLOCKED_DOCK);
            break;
        case FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE:
            figure_movement_move_ticks(f, 1);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_114_TRADE_SHIP_ANCHORED;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
        case FIGURE_ACTION_114_TRADE_SHIP_ANCHORED:
            f->wait_ticks++;
            if (f->wait_ticks > 40) {
                map_point tile;
                int dock_id = building_dock_get_free_destination(f->id, &tile);
                if (dock_id) {
                    f->destination_building_id = dock_id;
                    f->action_state = FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK;
                    f->destination_x = tile.x;
                    f->destination_y = tile.y;
                } else if (map_figure_at(f->grid_offset) != f->id &&
                    building_dock_get_queue_destination(&tile)) {
                    f->action_state = FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                    f->destination_x = tile.x;
                    f->destination_y = tile.y;
                }
                f->wait_ticks = 0;
            }
            f->image_offset = 0;
            break;
        case FIGURE_ACTION_115_TRADE_SHIP_LEAVING:
            figure_movement_move_ticks(f, 1);
            f->height_adjusted_ticks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_110_TRADE_SHIP_CREATED;
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previous_tile_direction);
    f->image_id = image_group(GROUP_FIGURE_SHIP) + dir;
}
