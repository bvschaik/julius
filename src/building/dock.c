#include "dock.h"

#include "map/figure.h"
#include "map/grid.h"
#include "map/routing.h"
#include "map/terrain.h"
#include "scenario/map.h"

#include "Data/CityInfo.h"

int building_dock_count_idle_dockers(const building *dock)
{
    int numIdle = 0;
    for (int i = 0; i < 3; i++) {
        if (dock->data.other.dockFigureIds[i]) {
            figure *f = figure_get(dock->data.other.dockFigureIds[i]);
            if (f->actionState == FIGURE_ACTION_132_DOCKER_IDLING ||
                f->actionState == FIGURE_ACTION_133_DOCKER_IMPORT_QUEUE) {
                numIdle++;
            }
        }
    }
    return numIdle;
}

void building_dock_update_open_water_access()
{
    map_point river_entry = scenario_map_river_entry();
    map_routing_calculate_distances_water_boat(river_entry.x, river_entry.y);
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_IN_USE && !b->houseSize && b->type == BUILDING_DOCK) {
            if (map_terrain_is_adjacent_to_open_water(b->x, b->y, 3)) {
                b->hasWaterAccess = 1;
            } else {
                b->hasWaterAccess = 0;
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

int building_dock_get_free_destination(int ship_id, int *x_tile, int *y_tile)
{
    if (Data_CityInfo.numWorkingDocks <= 0) {
        return 0;
    }
    int dockId = 0;
    for (int i = 0; i < 10; i++) {
        dockId = Data_CityInfo.workingDockBuildingIds[i];
        if (!dockId) continue;
        building *dock = building_get(dockId);
        if (!dock->data.other.boatFigureId || dock->data.other.boatFigureId == ship_id) {
            break;
        }
    }
    // BUG: when 10 docks in city, always takes last one... regardless of whether it is free
    if (dockId <= 0) {
        return 0;
    }
    building *dock = building_get(dockId);
    *x_tile = dock->x;
    *y_tile = dock->y;
    switch (dock->data.other.dockOrientation) {
        case 0: *x_tile += 1; *y_tile -= 1; break;
        case 1: *x_tile += 3; *y_tile += 1; break;
        case 2: *x_tile += 1; *y_tile += 3; break;
        default: *x_tile -= 1; *y_tile += 1; break;
    }
    dock->data.other.boatFigureId = ship_id;
    return dockId;
}

int building_dock_get_queue_destination(int* x_tile, int* y_tile)
{
    if (Data_CityInfo.numWorkingDocks <= 0) {
        return 0;
    }
    // first queue position
    for (int i = 0; i < 10; i++) {
        int dockId = Data_CityInfo.workingDockBuildingIds[i];
        if (!dockId) continue;
        building *dock = building_get(dockId);
        *x_tile = dock->x;
        *y_tile = dock->y;
        switch (dock->data.other.dockOrientation) {
            case 0: *x_tile += 2; *y_tile -= 2; break;
            case 1: *x_tile += 4; *y_tile += 2; break;
            case 2: *x_tile += 2; *y_tile += 4; break;
            default: *x_tile -= 2; *y_tile += 2; break;
        }
        if (!map_has_figure_at(map_grid_offset(*x_tile, *y_tile))) {
            return dockId;
        }
    }
    // second queue position
    for (int i = 0; i < 10; i++) {
        int dockId = Data_CityInfo.workingDockBuildingIds[i];
        if (!dockId) continue;
        building *dock = building_get(dockId);
        *x_tile = dock->x;
        *y_tile = dock->y;
        switch (dock->data.other.dockOrientation) {
            case 0: *x_tile += 2; *y_tile -= 3; break;
            case 1: *x_tile += 5; *y_tile += 2; break;
            case 2: *x_tile += 2; *y_tile += 5; break;
            default: *x_tile -= 3; *y_tile += 2; break;
        }
        if (!map_has_figure_at(map_grid_offset(*x_tile, *y_tile))) {
            return dockId;
        }
    }
    return 0;
}
