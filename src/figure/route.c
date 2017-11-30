#include "route.h"

#include "figure/figure.h"
#include "map/routing.h"
#include "map/routing_path.h"

#include <stdint.h>

#define MAX_PATH_LENGTH 500
#define MAX_ROUTES 600

static struct {
    int figure_ids[MAX_ROUTES];
    uint8_t direction_paths[MAX_ROUTES][MAX_PATH_LENGTH];
} data;

void figure_route_clear_all()
{
    for (int i = 0; i < MAX_ROUTES; i++) {
        data.figure_ids[i] = 0;
        for (int j = 0; j < MAX_PATH_LENGTH; j++) {
            data.direction_paths[i][j] = 0;
        }
    }
}

void figure_route_clean()
{
    for (int i = 0; i < MAX_ROUTES; i++) {
        int figure_id = data.figure_ids[i];
        if (figure_id > 0 && figure_id < MAX_FIGURES) {
            struct Data_Figure *f = figure_get(figure_id);
            if (f->state != FigureState_Alive || f->routingPathId != i) {
                data.figure_ids[i] = 0;
            }
        }
    }
}

static int get_first_available()
{
    for (int i = 1; i < MAX_ROUTES; i++) {
        if (data.figure_ids[i] == 0) {
            return i;
        }
    }
    return 0;
}

void figure_route_add(int figure_id)
{
    struct Data_Figure *f = figure_get(figure_id);
    f->routingPathId = 0;
    f->routingPathCurrentTile = 0;
    f->routingPathLength = 0;
    int path_id = get_first_available();
    if (!path_id) {
        return;
    }
    int path_length;
    if (f->isBoat) {
        if (f->isBoat == 2) { // flotsam
            map_routing_calculate_distances_water_flotsam(f->x, f->y);
            path_length = map_routing_get_path_on_water(data.direction_paths[path_id], f->x, f->y,
                f->destinationX, f->destinationY, 1);
        } else {
            map_routing_calculate_distances_water_boat(f->x, f->y);
            path_length = map_routing_get_path_on_water(data.direction_paths[path_id], f->x, f->y,
                f->destinationX, f->destinationY, 0);
        }
    } else {
        // land figure
        int can_travel;
        switch (f->terrainUsage) {
            case FigureTerrainUsage_Enemy:
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destinationX, f->destinationY, f->destinationBuildingId, 5000);
                if (!can_travel) {
                    can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                        f->destinationX, f->destinationY, 0, 25000);
                    if (!can_travel) {
                        can_travel = map_routing_noncitizen_can_travel_through_everything(
                            f->x, f->y, f->destinationX, f->destinationY);
                    }
                }
                break;
            case FigureTerrainUsage_Walls:
                can_travel = map_routing_can_travel_over_walls(f->x, f->y,
                    f->destinationX, f->destinationY);
                break;
            case FigureTerrainUsage_Animal:
                can_travel = map_routing_noncitizen_can_travel_over_land(f->x, f->y,
                    f->destinationX, f->destinationY, -1, 5000);
                break;
            case FigureTerrainUsage_PreferRoads:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destinationX, f->destinationY);
                if (!can_travel) {
                    can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                        f->destinationX, f->destinationY);
                }
                break;
            case FigureTerrainUsage_Roads:
                can_travel = map_routing_citizen_can_travel_over_road_garden(f->x, f->y,
                    f->destinationX, f->destinationY);
                break;
            default:
                can_travel = map_routing_citizen_can_travel_over_land(f->x, f->y,
                    f->destinationX, f->destinationY);
                break;
        }
        if (can_travel) {
            if (f->terrainUsage == FigureTerrainUsage_Walls) {
                path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                    f->destinationX, f->destinationY, 4);
                if (path_length <= 0) {
                    path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                        f->destinationX, f->destinationY, 8);
                }
            } else {
                path_length = map_routing_get_path(data.direction_paths[path_id], f->x, f->y,
                    f->destinationX, f->destinationY, 8);
            }
        } else { // cannot travel
            path_length = 0;
        }
    }
    if (path_length) {
        data.figure_ids[path_id] = figure_id;
        f->routingPathId = path_id;
        f->routingPathLength = path_length;
    }
}

void figure_route_remove(int figure_id)
{
    struct Data_Figure *f = figure_get(figure_id);
    if (f->routingPathId > 0) {
        if (data.figure_ids[f->routingPathId] == figure_id) {
            data.figure_ids[f->routingPathId] = 0;
        }
        f->routingPathId = 0;
    }
}

int figure_route_get_direction(int path_id, int index)
{
    return data.direction_paths[path_id][index];
}

void figure_route_save_state(buffer *figures, buffer *paths)
{
    for (int i = 0; i < MAX_ROUTES; i++) {
        buffer_write_i16(figures, data.figure_ids[i]);
        buffer_write_raw(paths, data.direction_paths[i], MAX_PATH_LENGTH);
    }
}

void figure_route_load_state(buffer *figures, buffer *paths)
{
    for (int i = 0; i < MAX_ROUTES; i++) {
        data.figure_ids[i] = buffer_read_i16(figures);
        buffer_read_raw(paths, data.direction_paths[i], MAX_PATH_LENGTH);
    }
}
