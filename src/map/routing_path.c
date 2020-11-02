#include "routing_path.h"

#include "core/calc.h"
#include "core/random.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing.h"

#define MAX_PATH 500

static int direction_path[MAX_PATH];

static void adjust_tile_in_direction(int direction, int *x, int *y, int *grid_offset)
{
    switch (direction) {
        case DIR_0_TOP:
            --*y;
            break;
        case DIR_1_TOP_RIGHT:
            ++*x;
            --*y;
            break;
        case DIR_2_RIGHT:
            ++*x;
            break;
        case DIR_3_BOTTOM_RIGHT:
            ++*x;
            ++*y;
            break;
        case DIR_4_BOTTOM:
            ++*y;
            break;
        case DIR_5_BOTTOM_LEFT:
            --*x;
            ++*y;
            break;
        case DIR_6_LEFT:
            --*x;
            break;
        case DIR_7_TOP_LEFT:
            --*x;
            --*y;
            break;
    }
    *grid_offset += map_grid_direction_delta(direction);
}

int map_routing_get_path(uint8_t *path, int src_x, int src_y, int dst_x, int dst_y, int num_directions)
{
    int dst_grid_offset = map_grid_offset(dst_x, dst_y);
    int distance = map_routing_distance(dst_grid_offset);
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int num_tiles = 0;
    int last_direction = -1;
    int x = dst_x;
    int y = dst_y;
    int grid_offset = dst_grid_offset;
    int step = num_directions == 8 ? 1 : 2;

    while (distance > 1) {
        distance = map_routing_distance(grid_offset);
        int direction = -1;
        int general_direction = calc_general_direction(x, y, src_x, src_y);
        for (int d = 0; d < 8; d += step) {
            if (d != last_direction) {
                int next_offset = grid_offset + map_grid_direction_delta(d);
                int next_distance = map_routing_distance(next_offset);
                if (next_distance) {
                    if (next_distance < distance) {
                        distance = next_distance;
                        direction = d;
                    } else if (next_distance == distance && (d == general_direction || direction == -1)) {
                        distance = next_distance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &grid_offset);
        int forward_direction = (direction + 4) % 8;
        direction_path[num_tiles++] = forward_direction;
        last_direction = forward_direction;
        if (num_tiles >= MAX_PATH) {
            return 0;
        }
    }
    for (int i = 0; i < num_tiles; i++) {
        path[i] = direction_path[num_tiles - i - 1];
    }
    return num_tiles;
}

int map_routing_get_closest_tile_within_range(
    int src_x, int src_y, int dst_x, int dst_y, int num_directions, int range, int *out_x, int *out_y)
{
    int dst_grid_offset = map_grid_offset(dst_x, dst_y);
    int distance = map_routing_distance(dst_grid_offset);
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int num_tiles = 0;
    int last_direction = -1;
    int x = dst_x;
    int y = dst_y;
    int grid_offset = dst_grid_offset;
    int step = num_directions == 8 ? 1 : 2;

    while (distance > 1) {
        distance = map_routing_distance(grid_offset);
        *out_x = x;
        *out_y = y;
        if (distance <= range) {
            return 1;
        }
        int direction = -1;
        int general_direction = calc_general_direction(x, y, src_x, src_y);
        for (int d = 0; d < 8; d += step) {
            if (d != last_direction) {
                int next_offset = grid_offset + map_grid_direction_delta(d);
                int next_distance = map_routing_distance(next_offset);
                if (next_distance) {
                    if (next_distance < distance) {
                        distance = next_distance;
                        direction = d;
                    } else if (next_distance == distance && (d == general_direction || direction == -1)) {
                        distance = next_distance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &grid_offset);
        int forward_direction = (direction + 4) % 8;
        direction_path[num_tiles++] = forward_direction;
        last_direction = forward_direction;
        if (num_tiles >= MAX_PATH) {
            return 0;
        }
    }
    return 0;
}

int map_routing_get_path_on_water(uint8_t *path, int dst_x, int dst_y, int is_flotsam)
{
    int rand = random_byte() & 3;
    int dst_grid_offset = map_grid_offset(dst_x, dst_y);
    int distance = map_routing_distance(dst_grid_offset);
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int num_tiles = 0;
    int last_direction = -1;
    int x = dst_x;
    int y = dst_y;
    int grid_offset = dst_grid_offset;
    while (distance > 1) {
        int current_rand = rand;
        distance = map_routing_distance(grid_offset);
        if (is_flotsam) {
            current_rand = map_random_get(grid_offset) & 3;
        }
        int direction = -1;
        for (int d = 0; d < 8; d++) {
            if (d != last_direction) {
                int next_offset = grid_offset + map_grid_direction_delta(d);
                int next_distance = map_routing_distance(next_offset);
                if (next_distance) {
                    if (next_distance < distance) {
                        distance = next_distance;
                        direction = d;
                    } else if (next_distance == distance && rand == current_rand) {
                        // allow flotsam to wander
                        distance = next_distance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &grid_offset);
        int forward_direction = (direction + 4) % 8;
        direction_path[num_tiles++] = forward_direction;
        last_direction = forward_direction;
        if (num_tiles >= MAX_PATH) {
            return 0;
        }
    }
    for (int i = 0; i < num_tiles; i++) {
        path[i] = direction_path[num_tiles - i - 1];
    }
    return num_tiles;
}
