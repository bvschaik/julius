#include "trader.h"

#include "building/building.h"
#include "building/dock.h"
#include "building/warehouse.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/finance.h"
#include "city/message.h"
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

#include "Data/CityInfo.h"

int figure_create_trade_caravan(int x, int y, int city_id)
{
    figure *caravan = figure_create(FIGURE_TRADE_CARAVAN, x, y, DIR_0_TOP);
    caravan->empireCityId = city_id;
    caravan->actionState = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    caravan->waitTicks = 10;
    // donkey 1
    figure *donkey1 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY, x, y, DIR_0_TOP);
    donkey1->actionState = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    donkey1->inFrontFigureId = caravan->id;
    // donkey 2
    figure *donkey2 = figure_create(FIGURE_TRADE_CARAVAN_DONKEY, x, y, DIR_0_TOP);
    donkey2->actionState = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
    donkey2->inFrontFigureId = donkey1->id;
    return caravan->id;
}

int figure_create_trade_ship(int x, int y, int city_id)
{
    figure *ship = figure_create(FIGURE_TRADE_SHIP, x, y, DIR_0_TOP);
    ship->empireCityId = city_id;
    ship->actionState = FIGURE_ACTION_110_TRADE_SHIP_CREATED;
    ship->waitTicks = 10;
    return ship->id;
}

static void advance_next_import_resource_caravan()
{
    Data_CityInfo.tradeNextImportResourceCaravan++;
    if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
        Data_CityInfo.tradeNextImportResourceCaravan = 1;
    }
}

int figure_trade_caravan_can_buy(figure *trader, int warehouse_id, int city_id)
{
    building *warehouse = building_get(warehouse_id);
    if (warehouse->type != BUILDING_WAREHOUSE) {
        return 0;
    }
    if (trader->traderAmountBought >= 8) {
        return 0;
    }
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && space->loadsStored > 0 &&
            empire_can_export_resource_to_city(city_id, space->subtype.warehouseResourceId)) {
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
    if (trader->loadsSoldOrCarrying >= 8) {
        return 0;
    }
    const building_storage *storage = building_storage_get(warehouse->storage_id);
    if (storage->empty_all) {
        return 0;
    }
    int num_importable = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        if (storage->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
            if (empire_can_import_resource_from_city(city_id, r)) {
                num_importable++;
            }
        }
    }
    if (num_importable <= 0) {
        return 0;
    }
    int can_import = 0;
    if (storage->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING &&
        empire_can_import_resource_from_city(city_id, Data_CityInfo.tradeNextImportResourceCaravan)) {
        can_import = 1;
    } else {
        for (int i = RESOURCE_MIN; i < RESOURCE_MAX; i++) {
            advance_next_import_resource_caravan();
            if (storage->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING &&
                    empire_can_import_resource_from_city(city_id, Data_CityInfo.tradeNextImportResourceCaravan)) {
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
            if (space->id > 0 && space->loadsStored < 4) {
                if (!space->loadsStored) {
                    // empty space
                    return 1;
                }
                if (empire_can_import_resource_from_city(city_id, space->subtype.warehouseResourceId)) {
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
        int resource = space->subtype.warehouseResourceId;
        if (space->loadsStored > 0 && empire_can_export_resource_to_city(city_id, resource)) {
            // update stocks
            Data_CityInfo.resourceSpaceInWarehouses[resource]++;
            Data_CityInfo.resourceStored[resource]--;
            space->loadsStored--;
            if (space->loadsStored <= 0) {
                space->subtype.warehouseResourceId = RESOURCE_NONE;
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
    int imp = 1;
    while (imp < 16 && !empire_can_import_resource_from_city(city_id, Data_CityInfo.tradeNextImportResourceCaravan)) {
        imp++;
        advance_next_import_resource_caravan();
    }
    if (imp >= 16) {
        return 0;
    }
    int resource_to_import = Data_CityInfo.tradeNextImportResourceCaravan;
    // add to existing bay with room
    building *space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && space->loadsStored > 0 && space->loadsStored < 4 &&
            space->subtype.warehouseResourceId == resource_to_import) {
            building_warehouse_space_add_import(space, resource_to_import);
            advance_next_import_resource_caravan();
            return resource_to_import;
        }
    }
    // add to empty bay
    space = warehouse;
    for (int i = 0; i < 8; i++) {
        space = building_next(space);
        if (space->id > 0 && !space->loadsStored) {
            building_warehouse_space_add_import(space, resource_to_import);
            advance_next_import_resource_caravan();
            return resource_to_import;
        }
    }
    // find another importable resource that can be added to this warehouse
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        Data_CityInfo.tradeNextImportResourceCaravanBackup++;
        if (Data_CityInfo.tradeNextImportResourceCaravanBackup > 15) {
            Data_CityInfo.tradeNextImportResourceCaravanBackup = 1;
        }
        resource_to_import = Data_CityInfo.tradeNextImportResourceCaravanBackup;
        if (empire_can_import_resource_from_city(city_id, resource_to_import)) {
            space = warehouse;
            for (int i = 0; i < 8; i++) {
                space = building_next(space);
                if (space->id > 0 && space->loadsStored < 4 && space->subtype.warehouseResourceId == resource_to_import) {
                    building_warehouse_space_add_import(space, resource_to_import);
                    return resource_to_import;
                }
            }
        }
    }
    return 0;
}

static int get_closest_warehouse(const figure *f, int x, int y, int city_id, int distance_from_entry,
                                 int *x_warehouse, int *y_warehouse)
{
    int exportable[RESOURCE_MAX];
    int importable[RESOURCE_MAX];
    exportable[RESOURCE_NONE] = 0;
    importable[RESOURCE_NONE] = 0;
    for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
        exportable[r] = empire_can_export_resource_to_city(city_id, r);
        if (f->traderAmountBought >= 8) {
            exportable[r] = 0;
        }
        if (city_id) {
            importable[r] = empire_can_import_resource_from_city(city_id, r);
        } else { // exclude own city (id=0), shouldn't happen, but still..
            importable[r] = 0;
        }
        if (f->loadsSoldOrCarrying >= 8) {
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
        if (!b->hasRoadAccess || b->distanceFromEntry <= 0) {
            continue;
        }
        const building_storage *s = building_storage_get(b->storage_id);
        int num_imports_for_warehouse = 0;
        for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
            if (s->resource_state[r] != BUILDING_STORAGE_STATE_NOT_ACCEPTING && empire_can_import_resource_from_city(city_id, r)) {
                num_imports_for_warehouse++;
            }
        }
        int distance_penalty = 32;
        building *space = b;
        for (int space_cnt = 0; space_cnt < 8; space_cnt++) {
            space = building_next(space);
            if (space->id && exportable[space->subtype.warehouseResourceId]) {
                distance_penalty -= 4;
            }
            if (num_importable && num_imports_for_warehouse && !s->empty_all) {
                for (int r = RESOURCE_MIN; r < RESOURCE_MAX; r++) {
                    Data_CityInfo.tradeNextImportResourceCaravan++;
                    if (Data_CityInfo.tradeNextImportResourceCaravan > 15) {
                        Data_CityInfo.tradeNextImportResourceCaravan = 1;
                    }
                    if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
                        break;
                    }
                }
                if (s->resource_state[Data_CityInfo.tradeNextImportResourceCaravan] != BUILDING_STORAGE_STATE_NOT_ACCEPTING) {
                    if (space->subtype.warehouseResourceId == RESOURCE_NONE) {
                        distance_penalty -= 16;
                    }
                    if (space->id && importable[space->subtype.warehouseResourceId] && space->loadsStored < 4 &&
                        space->subtype.warehouseResourceId == Data_CityInfo.tradeNextImportResourceCaravan) {
                        distance_penalty -= 8;
                    }
                }
            }
        }
        if (distance_penalty < 32) {
            int distance = calc_distance_with_penalty(b->x, b->y, x, y, distance_from_entry, b->distanceFromEntry);
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
    if (min_building->hasRoadAccess == 1) {
        *x_warehouse = min_building->x;
        *y_warehouse = min_building->y;
    } else if (!map_has_road_access(min_building->x, min_building->y, 3, x_warehouse, y_warehouse)) {
        return 0;
    }
    return min_building->id;
}

static void go_to_next_warehouse(figure *f, int x_src, int y_src, int distance_to_entry)
{
    int x_dst, y_dst;
    int warehouse_id = get_closest_warehouse(f, x_src, y_src, f->empireCityId, distance_to_entry, &x_dst, &y_dst);
    if (warehouse_id) {
        f->destinationBuildingId = warehouse_id;
        f->actionState = FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING;
        f->destinationX = x_dst;
        f->destinationY = y_dst;
    } else {
        f->actionState = FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING;
        f->destinationX = Data_CityInfo.exitPointX;
        f->destinationY = Data_CityInfo.exitPointY;
    }
}

void figure_trade_caravan_action(figure *f)
{
    f->isGhost = 0;
    f->terrainUsage = FigureTerrainUsage_PreferRoads;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_100_TRADE_CARAVAN_CREATED:
            f->isGhost = 1;
            f->waitTicks++;
            if (f->waitTicks > 20) {
                f->waitTicks = 0;
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
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_101_TRADE_CARAVAN_ARRIVING:
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->actionState = FIGURE_ACTION_102_TRADE_CARAVAN_TRADING;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    f->state = FigureState_Dead;
                    f->isGhost = 1;
                    break;
            }
            if (building_get(f->destinationBuildingId)->state != BUILDING_STATE_IN_USE) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_102_TRADE_CARAVAN_TRADING:
            f->waitTicks++;
            if (f->waitTicks > 10) {
                f->waitTicks = 0;
                int move_on = 0;
                if (figure_trade_caravan_can_buy(f, f->destinationBuildingId, f->empireCityId)) {
                    int resource = trader_get_buy_resource(f->destinationBuildingId, f->empireCityId);
                    if (resource) {
                        trade_route_increase_traded(empire_city_get_route_id(f->empireCityId), resource);
                        trader_record_bought_resource(f->traderId, resource);
                        f->traderAmountBought++;
                    } else {
                        move_on++;
                    }
                } else {
                    move_on++;
                }
                if (figure_trade_caravan_can_sell(f, f->destinationBuildingId, f->empireCityId)) {
                    int resource = trader_get_sell_resource(f->destinationBuildingId, f->empireCityId);
                    if (resource) {
                        trade_route_increase_traded(empire_city_get_route_id(f->empireCityId), resource);
                        trader_record_sold_resource(f->traderId, resource);
                        f->loadsSoldOrCarrying++;
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
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_103_TRADE_CARAVAN_LEAVING:
            figure_movement_move_ticks(f, 1);
            switch (f->direction) {
                case DIR_FIGURE_AT_DESTINATION:
                    f->actionState = FIGURE_ACTION_100_TRADE_CARAVAN_CREATED;
                    f->state = FigureState_Dead;
                    break;
                case DIR_FIGURE_REROUTE:
                    figure_route_remove(f);
                    break;
                case DIR_FIGURE_LOST:
                    f->state = FigureState_Dead;
                    break;
            }
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    f->graphicId = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->graphicOffset;
}

void figure_trade_caravan_donkey_action(figure *f)
{
    f->isGhost = 0;
    f->terrainUsage = FigureTerrainUsage_PreferRoads;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;

    figure *leader = figure_get(f->inFrontFigureId);
    if (f->inFrontFigureId <= 0) {
        f->state = FigureState_Dead;
    } else {
        if (leader->actionState == FIGURE_ACTION_149_CORPSE) {
            f->state = FigureState_Dead;
        } else if (leader->state != FigureState_Alive) {
            f->state = FigureState_Dead;
        } else if (leader->type != FIGURE_TRADE_CARAVAN && leader->type != FIGURE_TRADE_CARAVAN_DONKEY) {
            f->state = FigureState_Dead;
        } else {
            figure_movement_follow_ticks(f, 1);
        }
    }

    if (leader->isGhost) {
        f->isGhost = 1;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    f->graphicId = image_group(GROUP_FIGURE_TRADE_CARAVAN) + dir + 8 * f->graphicOffset;
}

void figure_native_trader_action(figure *f)
{
    f->isGhost = 0;
    f->terrainUsage = FigureTerrainUsage_Any;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_163_NATIVE_TRADER_AT_WAREHOUSE;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
                f->isGhost = 1;
            }
            if (building_get(f->destinationBuildingId)->state != BUILDING_STATE_IN_USE) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_161_NATIVE_TRADER_RETURNING:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
        case FIGURE_ACTION_162_NATIVE_TRADER_CREATED:
            f->isGhost = 1;
            f->waitTicks++;
            if (f->waitTicks > 10) {
                f->waitTicks = 0;
                int x_tile, y_tile;
                int building_id = get_closest_warehouse(f, f->x, f->y, 0, -1, &x_tile, &y_tile);
                if (building_id) {
                    f->actionState = FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE;
                    f->destinationBuildingId = building_id;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_163_NATIVE_TRADER_AT_WAREHOUSE:
            f->waitTicks++;
            if (f->waitTicks > 10) {
                f->waitTicks = 0;
                if (figure_trade_caravan_can_buy(f, f->destinationBuildingId, 0)) {
                    int resource = trader_get_buy_resource(f->destinationBuildingId, 0);
                    trader_record_bought_resource(f->traderId, resource);
                    f->traderAmountBought += 3;
                } else {
                    int x_tile, y_tile;
                    int building_id = get_closest_warehouse(f, f->x, f->y, 0, -1, &x_tile, &y_tile);
                    if (building_id) {
                        f->actionState = FIGURE_ACTION_160_NATIVE_TRADER_GOING_TO_WAREHOUSE;
                        f->destinationBuildingId = building_id;
                        f->destinationX = x_tile;
                        f->destinationY = y_tile;
                    } else {
                        f->actionState = FIGURE_ACTION_161_NATIVE_TRADER_RETURNING;
                        f->destinationX = f->sourceX;
                        f->destinationY = f->sourceY;
                    }
                }
            }
            f->graphicOffset = 0;
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    
    if (f->actionState == FIGURE_ACTION_149_CORPSE) {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + 96 + figure_image_corpse_offset(f);
        f->cartGraphicId = 0;
    } else {
        f->graphicId = image_group(GROUP_FIGURE_CARTPUSHER) + dir + 8 * f->graphicOffset;
    }
    f->cartGraphicId = image_group(GROUP_FIGURE_MIGRANT_CART) +
        8 + 8 * f->resourceId; // BUGFIX should be within else statement?
    if (f->cartGraphicId) {
        f->cartGraphicId += dir;
        figure_image_set_cart_offset(f, dir);
    }
}

int figure_trade_ship_is_trading(figure *ship)
{
    building *b = building_get(ship->destinationBuildingId);
    if (b->state != BUILDING_STATE_IN_USE || b->type != BUILDING_DOCK) {
        return TRADE_SHIP_BUYING;
    }
    for (int i = 0; i < 3; i++) {
        figure *f = figure_get(b->data.dock.docker_ids[i]);
        if (!b->data.dock.docker_ids[i] || f->state != FigureState_Alive) {
            continue;
        }
        switch (f->actionState) {
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
    building *b = building_get(f->destinationBuildingId);
    if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_DOCK &&
        b->numWorkers > 0 && b->data.dock.trade_ship_id == f->id) {
        return 0;
    }
    return 1;
}

static int trade_ship_done_trading(figure *f)
{
    building *b = building_get(f->destinationBuildingId);
    if (b->state == BUILDING_STATE_IN_USE && b->type == BUILDING_DOCK && b->numWorkers > 0) {
        for (int i = 0; i < 3; i++) {
            if (b->data.dock.docker_ids[i]) {
                figure *docker = figure_get(b->data.dock.docker_ids[i]);
                if (docker->state == FigureState_Alive && docker->actionState != FIGURE_ACTION_132_DOCKER_IDLING) {
                    return 0;
                }
            }
        }
        f->tradeShipFailedDockAttempts++;
        if (f->tradeShipFailedDockAttempts >= 10) {
            f->tradeShipFailedDockAttempts = 11;
            return 1;
        }
        return 0;
    }
    return 1;
}

void figure_trade_ship_action(figure *f)
{
    f->isGhost = 0;
    f->isBoat = 1;
    figure_image_increase_offset(f, 12);
    f->cartGraphicId = 0;
    switch (f->actionState) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_110_TRADE_SHIP_CREATED:
            f->loadsSoldOrCarrying = 12;
            f->traderAmountBought = 0;
            f->isGhost = 1;
            f->waitTicks++;
            if (f->waitTicks > 20) {
                f->waitTicks = 0;
                int x_tile, y_tile;
                int dock_id = building_dock_get_free_destination(f->id, &x_tile, &y_tile);
                if (dock_id) {
                    f->destinationBuildingId = dock_id;
                    f->actionState = FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                } else if (building_dock_get_queue_destination(&x_tile, &y_tile)) {
                    f->actionState = FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                } else {
                    f->state = FigureState_Dead;
                }
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_112_TRADE_SHIP_MOORED;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
                if (!city_message_get_category_count(MESSAGE_CAT_BLOCKED_DOCK)) {
                    city_message_post(1, MESSAGE_NAVIGATION_IMPOSSIBLE, 0, 0);
                    city_message_increase_category_count(MESSAGE_CAT_BLOCKED_DOCK);
                }
            }
            if (building_get(f->destinationBuildingId)->state != BUILDING_STATE_IN_USE) {
                f->actionState = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->waitTicks = 0;
                map_point river_exit = scenario_map_river_exit();
                f->destinationX = river_exit.x;
                f->destinationY = river_exit.y;
            }
            break;
        case FIGURE_ACTION_112_TRADE_SHIP_MOORED:
            if (trade_ship_lost_queue(f)) {
                f->tradeShipFailedDockAttempts = 0;
                f->actionState = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->waitTicks = 0;
                map_point river_entry = scenario_map_river_entry();
                f->destinationX = river_entry.x;
                f->destinationY = river_entry.y;
            } else if (trade_ship_done_trading(f)) {
                f->tradeShipFailedDockAttempts = 0;
                f->actionState = FIGURE_ACTION_115_TRADE_SHIP_LEAVING;
                f->waitTicks = 0;
                map_point river_entry = scenario_map_river_entry();
                f->destinationX = river_entry.x;
                f->destinationY = river_entry.y;
                building *dst = building_get(f->destinationBuildingId);
                dst->data.dock.queued_docker_id = 0;
                dst->data.dock.num_ships = 0;
            }
            switch (building_get(f->destinationBuildingId)->data.dock.orientation) {
                case 0: f->direction = DIR_2_RIGHT; break;
                case 1: f->direction = DIR_4_BOTTOM; break;
                case 2: f->direction = DIR_6_LEFT; break;
                default:f->direction = DIR_0_TOP; break;
            }
            f->graphicOffset = 0;
            city_message_reset_category_count(MESSAGE_CAT_BLOCKED_DOCK);
            break;
        case FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_114_TRADE_SHIP_ANCHORED;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
        case FIGURE_ACTION_114_TRADE_SHIP_ANCHORED:
            f->waitTicks++;
            if (f->waitTicks > 40) {
                int x_tile, y_tile;
                int dockId = building_dock_get_free_destination(f->id, &x_tile, &y_tile);
                if (dockId) {
                    f->destinationBuildingId = dockId;
                    f->actionState = FIGURE_ACTION_111_TRADE_SHIP_GOING_TO_DOCK;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                } else if (map_figure_at(f->gridOffset) != f->id &&
                    building_dock_get_queue_destination(&x_tile, &y_tile)) {
                    f->actionState = FIGURE_ACTION_113_TRADE_SHIP_GOING_TO_DOCK_QUEUE;
                    f->destinationX = x_tile;
                    f->destinationY = y_tile;
                }
                f->waitTicks = 0;
            }
            f->graphicOffset = 0;
            break;
        case FIGURE_ACTION_115_TRADE_SHIP_LEAVING:
            figure_movement_move_ticks(f, 1);
            f->heightAdjustedTicks = 0;
            if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->actionState = FIGURE_ACTION_110_TRADE_SHIP_CREATED;
                f->state = FigureState_Dead;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FigureState_Dead;
            }
            break;
    }
    int dir = figure_image_normalize_direction(f->direction < 8 ? f->direction : f->previousTileDirection);
    f->graphicId = image_group(GROUP_FIGURE_SHIP) + dir;
}
