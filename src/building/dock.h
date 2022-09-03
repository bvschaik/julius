#ifndef BUILDING_DOCK_H
#define BUILDING_DOCK_H

#include "building/building.h"
#include "map/point.h"

#define MAX_DOCKS 10

typedef enum {
    SHIP_DOCK_REQUEST_1_DOCKING = 1,
    SHIP_DOCK_REQUEST_2_FIRST_QUEUE = 2,
    SHIP_DOCK_REQUEST_4_SECOND_QUEUE = 4,
    SHIP_DOCK_REQUEST_6_ANY_QUEUE = 6,
    SHIP_DOCK_REQUEST_7_ANY = 7,
} ship_dock_request_type;

int building_dock_count_idle_dockers(const building *dock);

void building_dock_update_open_water_access(void);

int building_dock_is_connected_to_open_water(int x, int y);

int building_dock_get_destination(int ship_id, int exclude_dock_id, map_point *tile);

int building_dock_get_closer_free_destination(int ship_id, ship_dock_request_type request_type, map_point *tile);

int building_dock_request_docking(int ship_id, int dock_id, map_point *tile);

void building_dock_get_ship_request_tile(const building *dock, ship_dock_request_type request_type, map_point *tile);

int building_dock_is_working(int dock_id);

int building_dock_accepts_ship(int ship_id, int dock_id);

int building_dock_reposition_anchored_ship(int ship_id, map_point *tile);

int building_dock_can_import_from_ship(building *dock, int ship_id);

int building_dock_can_export_to_ship(building *dock, int ship_id);

int building_dock_can_trade_with_route(int route_id, int dock_id);

void building_dock_set_can_trade_with_route(int route_id, int dock_id, int can_trade);
#endif // BUILDING_DOCK_H
