#include "routing_path.h"

#include "core/calc.h"
#include "core/random.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/routing.h"
#include "map/terrain.h"

#define MAX_PATH 500

static int direction_path[MAX_PATH];

static void adjust_tile_in_direction(int direction, int *x, int *y, int *grid_offset)
{
    switch (direction) {
        case DIR_0_TOP:
            -- *y;
            break;
        case DIR_1_TOP_RIGHT:
            ++ *x;
            -- *y;
            break;
        case DIR_2_RIGHT:
            ++ *x;
            break;
        case DIR_3_BOTTOM_RIGHT:
            ++ *x;
            ++ *y;
            break;
        case DIR_4_BOTTOM:
            ++ *y;
            break;
        case DIR_5_BOTTOM_LEFT:
            -- *x;
            ++ *y;
            break;
        case DIR_6_LEFT:
            -- *x;
            break;
        case DIR_7_TOP_LEFT:
            -- *x;
            -- *y;
            break;
    }
    *grid_offset += map_grid_direction_delta(direction);
}

static int is_equal_distance_but_better_direction(int distance, int next_distance, int direction, int next_direction)
{
    if (next_distance != distance) {
        return 0;
    } else if (direction == -1) {
        return 1;
    // prefer going in "straight" directions as opposed to diagonals if the distances are equal.
    // this helps prevent units from zig-zagging instead of moving in a straight line and makes
    // up for the removal of the general_direction calculation, which tended to make unit movement
    // look weird as units would try to move directly towards their destination even if there was
    // an obstacle in the way.
    } else if (direction % 2 == 1 && next_direction % 2 == 0) {
        return 1;
    }
    return 0;
}

static int next_is_better(
    int base_distance,
    int distance,
    int next_distance,
    int direction,
    int next_direction,
    int is_highway,
    int next_is_highway
)
{
    // always prefer highways so walkers don't cut across empty land
    if (!is_highway && next_is_highway && next_distance < base_distance) {
        return 1;
    } else if (is_highway && !next_is_highway) {
        return 0;
    } else if (next_distance < distance) {
        return 1;
    } else if (is_equal_distance_but_better_direction(distance, next_distance, direction, next_direction)) {
        return 1;
    }
    return 0;
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
        int base_distance = map_routing_distance(grid_offset);
        distance = base_distance;
        int direction = -1;
        int is_highway = 0;
        for (int next_direction = 0; next_direction < 8; next_direction += step) {
            if (next_direction != last_direction) {
                int next_offset = grid_offset + map_grid_direction_delta(next_direction);
                int next_distance = map_routing_distance(next_offset);
                int next_is_highway = map_terrain_is(next_offset, TERRAIN_HIGHWAY);
                if (next_distance && next_is_better(base_distance, distance, next_distance, direction, next_direction, is_highway, next_is_highway)) {
                    distance = next_distance;
                    direction = next_direction;
                    is_highway = next_is_highway;
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
