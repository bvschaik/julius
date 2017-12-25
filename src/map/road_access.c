#include "road_access.h"

#include "building/building.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/road_network.h"
#include "map/terrain.h"

#include "Data/CityInfo.h"

static void find_minimum_road_tile(int x, int y, int size, int *min_value, int *min_grid_offset)
{
    int base_offset = map_grid_offset(x, y);
    for (const int *tile_delta = map_grid_adjacent_offsets(size); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING) ||
            building_get(map_building_at(grid_offset))->type != BUILDING_GATEHOUSE) {
            if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
                int road_index = 11;
                for (int n = 0; n < 10; n++) {
                    if (Data_CityInfo.largestRoadNetworks[n].id == map_road_network_get(grid_offset)) {
                        road_index = n;
                        break;
                    }
                }
                if (road_index < *min_value) {
                    *min_value = road_index;
                    *min_grid_offset = grid_offset;
                }
            }
        }
    }
}

int map_has_road_access(int x, int y, int size, int *x_road, int *y_road)
{
    int min_value = 12;
    int min_grid_offset = map_grid_offset(x, y);
    find_minimum_road_tile(x, y, size, &min_value, &min_grid_offset);
    if (min_value < 12) {
        if (x_road && y_road) {
            *x_road = map_grid_offset_to_x(min_grid_offset);
            *y_road = map_grid_offset_to_y(min_grid_offset);
        }
        return 1;
    }
    return 0;
}

int map_has_road_access_hippodrome(int x, int y, int *x_road, int *y_road)
{
    int min_value = 12;
    int min_grid_offset = map_grid_offset(x, y);
    find_minimum_road_tile(x, y, 5, &min_value, &min_grid_offset);
    find_minimum_road_tile(x + 5, y, 5, &min_value, &min_grid_offset);
    find_minimum_road_tile(x + 10, y, 5, &min_value, &min_grid_offset);
    if (min_value < 12) {
        if (x_road && y_road) {
            *x_road = map_grid_offset_to_x(min_grid_offset);
            *y_road = map_grid_offset_to_y(min_grid_offset);
        }
        return 1;
    }
    return 0;
}

int map_has_road_access_granary(int x, int y, int *x_road, int *y_road)
{
    int rx = -1, ry = -1;
    if (map_terrain_is(map_grid_offset(x + 1, y - 1), TERRAIN_ROAD)) {
        rx = x + 1;
        ry = y - 1;
    } else if (map_terrain_is(map_grid_offset(x + 3, y + 1), TERRAIN_ROAD)) {
        rx = x + 3;
        ry = y + 1;
    } else if (map_terrain_is(map_grid_offset(x + 1, y + 3), TERRAIN_ROAD)) {
        rx = x + 1;
        ry = y + 3;
    } else if (map_terrain_is(map_grid_offset(x - 1, y + 1), TERRAIN_ROAD)) {
        rx = x - 1;
        ry = y + 1;
    }
    if (rx >= 0 && ry >= 0) {
        if (x_road && y_road) {
            *x_road = rx;
            *y_road = ry;
        }
        return 1;
    }
    return 0;
}


static int road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {\
        for (int xx = x_min; xx <= x_max; xx++) {
            if (map_terrain_is(map_grid_offset(xx, yy), TERRAIN_ROAD)) {
                if (x_road && y_road) {
                    *x_road = xx;
                    *y_road = yy;
                }
                return 1;
            }
        }
    }
    return 0;
}

int map_closest_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road)
{
    for (int r = 1; r <= radius; r++) {
        if (road_within_radius(x, y, size, r, x_road, y_road)) {
            return 1;
        }
    }
    return 0;
}
