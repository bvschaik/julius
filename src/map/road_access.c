#include "road_access.h"

#include "building/building.h"
#include "building/roadblock.h"
#include "core/config.h"
#include "city/map.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/road_network.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

static void find_minimum_road_tile(int x, int y, int size, int *min_value, int *min_grid_offset)
{
    int base_offset = map_grid_offset(x, y);
    for (const int *tile_delta = map_grid_adjacent_offsets(size); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        if (!map_terrain_is(grid_offset, TERRAIN_BUILDING) ||
            building_get(map_building_at(grid_offset))->type != BUILDING_GATEHOUSE) {
            if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
		if (building_get(map_building_at(grid_offset))->type == BUILDING_ROADBLOCK) continue;
                int road_index = city_map_road_network_index(map_road_network_get(grid_offset));
                if (road_index < *min_value) {
                    *min_value = road_index;
                    *min_grid_offset = grid_offset;
                }
            }
        }
    }
}

int map_has_road_access(int x, int y, int size, map_point *road)
{
    int min_value = 12;
    int min_grid_offset = map_grid_offset(x, y);
    find_minimum_road_tile(x, y, size, &min_value, &min_grid_offset);
    if (min_value < 12) {
        if (road) {
            map_point_store_result(map_grid_offset_to_x(min_grid_offset), map_grid_offset_to_y(min_grid_offset), road);
        }
        return 1;
    }
    return 0;
}

int map_has_road_access_hippodrome(int x, int y, map_point *road)
{
    int min_value = 12;
    int min_grid_offset = map_grid_offset(x, y);
    find_minimum_road_tile(x, y, 5, &min_value, &min_grid_offset);
    find_minimum_road_tile(x + 5, y, 5, &min_value, &min_grid_offset);
    find_minimum_road_tile(x + 10, y, 5, &min_value, &min_grid_offset);
    if (min_value < 12) {
        if (road) {
            map_point_store_result(map_grid_offset_to_x(min_grid_offset), map_grid_offset_to_y(min_grid_offset), road);
        }
        return 1;
    }
    return 0;
}

int map_has_road_access_granary(int x, int y, map_point *road)
{
    int rx = -1, ry = -1;
    if (map_terrain_is(map_grid_offset(x + 1, y - 1), TERRAIN_ROAD) &&
    (building_get(map_building_at(map_grid_offset(x + 1, y - 1)))->type != BUILDING_ROADBLOCK)) {
        rx = x + 1;
        ry = y - 1;
    } else if (map_terrain_is(map_grid_offset(x + 3, y + 1), TERRAIN_ROAD) &&
    (building_get(map_building_at(map_grid_offset(x + 3, y + 1)))->type != BUILDING_ROADBLOCK)) {
        rx = x + 3;
        ry = y + 1;
    } else if (map_terrain_is(map_grid_offset(x + 1, y + 3), TERRAIN_ROAD) &&
    (building_get(map_building_at(map_grid_offset(x + 1, y + 3)))->type != BUILDING_ROADBLOCK)) {
        rx = x + 1;
        ry = y + 3;
    } else if (map_terrain_is(map_grid_offset(x - 1, y + 1), TERRAIN_ROAD) &&
    (building_get(map_building_at(map_grid_offset(x - 1, y + 1)))->type != BUILDING_ROADBLOCK)) {
        rx = x - 1;
        ry = y + 1;
    }
    if (rx >= 0 && ry >= 0) {
        if (road) {
            map_point_store_result(rx, ry, road);
        }
        return 1;
    }
    return 0;
}


static int road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            if (map_terrain_is(map_grid_offset(xx, yy), TERRAIN_ROAD)) {
                // Don't spawn walkers on roadblocks
		if (building_get(map_building_at(map_grid_offset(xx, yy)))->type == BUILDING_ROADBLOCK) continue;
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

static int reachable_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            if (map_terrain_is(grid_offset, TERRAIN_ROAD)) {
                if (map_routing_distance(grid_offset) > 0) {
                    if (x_road && y_road) {
                        *x_road = xx;
                        *y_road = yy;
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}

int map_closest_reachable_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road)
{
    for (int r = 1; r <= radius; r++) {
        if (reachable_road_within_radius(x, y, size, r, x_road, y_road)) {
            return 1;
        }
    }
    return 0;
}

int map_road_to_largest_network(int x, int y, int size, int *x_road, int *y_road)
{
    int min_index = 12;
    int min_grid_offset = -1;
    int base_offset = map_grid_offset(x, y);
    for (const int *tile_delta = map_grid_adjacent_offsets(size); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        if (map_terrain_is(grid_offset, TERRAIN_ROAD) && map_routing_distance(grid_offset) > 0) {
            int index = city_map_road_network_index(map_road_network_get(grid_offset));
            if (index < min_index) {
                min_index = index;
                min_grid_offset = grid_offset;
            }
        }
    }
    if (min_index < 12) {
        *x_road = map_grid_offset_to_x(min_grid_offset);
        *y_road = map_grid_offset_to_y(min_grid_offset);
        return min_grid_offset;
    }
    int min_dist = 100000;
    min_grid_offset = -1;
    for (const int *tile_delta = map_grid_adjacent_offsets(size); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        int dist = map_routing_distance(grid_offset);
        if (dist > 0 && dist < min_dist) {
            min_dist = dist;
            min_grid_offset = grid_offset;
        }
    }
    if (min_grid_offset >= 0) {
        *x_road = map_grid_offset_to_x(min_grid_offset);
        *y_road = map_grid_offset_to_y(min_grid_offset);
        return min_grid_offset;
    }
    return -1;
}

static void check_road_to_largest_network_hippodrome(int x, int y, int *min_index, int *min_grid_offset)
{
    int base_offset = map_grid_offset(x, y);
    for (const int *tile_delta = map_grid_adjacent_offsets(5); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        if (map_terrain_is(grid_offset, TERRAIN_ROAD) && map_routing_distance(grid_offset) > 0) {
            int index = city_map_road_network_index(map_road_network_get(grid_offset));
            if (index < *min_index) {
                *min_index = index;
                *min_grid_offset = grid_offset;
            }
        }
    }
}

static void check_min_dist_hippodrome(int base_offset, int x_offset, int *min_dist, int *min_grid_offset, int *min_x_offset)
{
    for (const int *tile_delta = map_grid_adjacent_offsets(5); *tile_delta; tile_delta++) {
        int grid_offset = base_offset + *tile_delta;
        int dist = map_routing_distance(grid_offset);
        if (dist > 0 && dist < *min_dist) {
            *min_dist = dist;
            *min_grid_offset = grid_offset;
            *min_x_offset = x_offset;
        }
    }
}

int map_road_to_largest_network_hippodrome(int x, int y, int *x_road, int *y_road)
{
    int min_index = 12;
    int min_grid_offset = -1;
    check_road_to_largest_network_hippodrome(x, y, &min_index, &min_grid_offset);
    check_road_to_largest_network_hippodrome(x + 5, y, &min_index, &min_grid_offset);
    check_road_to_largest_network_hippodrome(x + 10, y, &min_index, &min_grid_offset);

    if (min_index < 12) {
        *x_road = map_grid_offset_to_x(min_grid_offset);
        *y_road = map_grid_offset_to_y(min_grid_offset);
        return min_grid_offset;
    }

    int min_dist = 100000;
    min_grid_offset = -1;
    int min_x_offset = -1;
    check_min_dist_hippodrome(map_grid_offset(x, y), 0, &min_dist, &min_grid_offset, &min_x_offset);
    check_min_dist_hippodrome(map_grid_offset(x + 5, y), 5, &min_dist, &min_grid_offset, &min_x_offset);
    check_min_dist_hippodrome(map_grid_offset(x + 10, y), 10, &min_dist, &min_grid_offset, &min_x_offset);

    if (min_grid_offset >= 0) {
        *x_road = map_grid_offset_to_x(min_grid_offset) + min_x_offset;
        *y_road = map_grid_offset_to_y(min_grid_offset);
        return min_grid_offset + min_x_offset;
    }
    return -1;
}

static int terrain_is_road_like(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_ROAD | TERRAIN_ACCESS_RAMP) ? 1 : 0;
}

static int get_adjacent_road_tile_for_roaming(int grid_offset, roadblock_permission perm)
{
	int is_road = terrain_is_road_like(grid_offset);
	if (map_terrain_is(grid_offset, TERRAIN_BUILDING)) {
		building* b = building_get(map_building_at(grid_offset));
		if (b->type == BUILDING_GATEHOUSE) {
			is_road = 0;
		}
		else if (b->type == BUILDING_ROADBLOCK) {
			if (!building_roadblock_get_permission(perm,b)) {
			    is_road = 0;
			}
		}
		else if (b->type == BUILDING_GRANARY) {
			if (map_routing_citizen_is_road(grid_offset)) {
				if (config_get(CONFIG_GP_CH_DYNAMIC_GRANARIES)) {
					if (map_property_multi_tile_xy(grid_offset) == EDGE_X1Y1 || map_has_adjacent_road_tiles(grid_offset) || map_has_adjacent_granary_road(grid_offset)) {
						is_road = 1;
					}
				}
				else {
					is_road = 1;
				}

			}
		}
		else if (b->type == BUILDING_TRIUMPHAL_ARCH) {
			if (map_routing_citizen_is_road(grid_offset)) {
				is_road = 1;
			}
		}
	}
	return is_road;
}

int map_get_adjacent_road_tiles_for_roaming(int grid_offset, int *road_tiles, int perm)
{
    road_tiles[1] = road_tiles[3] = road_tiles[5] = road_tiles[7] = 0;

    road_tiles[0] = get_adjacent_road_tile_for_roaming(grid_offset + map_grid_delta(0, -1), perm);
    road_tiles[2] = get_adjacent_road_tile_for_roaming(grid_offset + map_grid_delta(1, 0), perm);
    road_tiles[4] = get_adjacent_road_tile_for_roaming(grid_offset + map_grid_delta(0, 1), perm);
    road_tiles[6] = get_adjacent_road_tile_for_roaming(grid_offset + map_grid_delta(-1, 0), perm);

    return road_tiles[0] + road_tiles[2] + road_tiles[4] + road_tiles[6];
}


int map_get_diagonal_road_tiles_for_roaming(int grid_offset, int *road_tiles)
{
    road_tiles[1] = terrain_is_road_like(grid_offset + map_grid_delta(1, -1));
    road_tiles[3] = terrain_is_road_like(grid_offset + map_grid_delta(1, 1));
    road_tiles[5] = terrain_is_road_like(grid_offset + map_grid_delta(-1, 1));
    road_tiles[7] = terrain_is_road_like(grid_offset + map_grid_delta(-1, -1));

    int max_stretch = 0;
    int stretch = 0;
    for (int i = 0; i < 16; i++) {
        if (road_tiles[i % 8]) {
            stretch++;
            if (stretch > max_stretch) {
                max_stretch = stretch;
            }
        } else {
            stretch = 0;
        }
    }
    return max_stretch;
}

int map_has_adjacent_road_tiles(int grid_offset)
{
    int adjacent_roads = terrain_is_road_like(grid_offset + map_grid_delta(0, -1))
    + terrain_is_road_like(grid_offset + map_grid_delta(1, 0))
    + terrain_is_road_like(grid_offset + map_grid_delta(0, 1))
    + terrain_is_road_like(grid_offset + map_grid_delta(-1, 0));
    return adjacent_roads;



}

int map_has_adjacent_granary_road(int grid_offset)	
{
	int tiles[4];
	tiles[0] = grid_offset + map_grid_delta(0, -1);
	tiles[1] = grid_offset + map_grid_delta(1, 0);
	tiles[2] = grid_offset + map_grid_delta(0, 1);
	tiles[3] = grid_offset + map_grid_delta(-1, 0);
	for (int i = 0; i < 4; i++) {
		if (building_get(map_building_at(tiles[i]))->type != BUILDING_GRANARY) continue;
		switch (map_property_multi_tile_xy(tiles[i])) {
		case EDGE_X1Y0:
		case EDGE_X0Y1:
		case EDGE_X2Y1:
		case EDGE_X1Y2:
            return 1;
		}
	}
	return 0;
}
