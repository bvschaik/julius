#include "routing_path.h"

#include "core/calc.h"
#include "core/random.h"
#include "map/grid.h"

#include "Data/Grid.h"

#define MAX_PATH 500

static int direction_path[MAX_PATH];

static void adjust_tile_in_direction(int direction, int *x, int *y, int *grid_offset)
{
    switch (direction) {
        case DIR_0_TOP:
            --(*y);
            break;
        case DIR_1_TOP_RIGHT:
            ++(*x);
            --(*y);
            break;
        case DIR_2_RIGHT:
            ++(*x);
            break;
        case DIR_3_BOTTOM_RIGHT:
            ++(*x);
            ++(*y);
            break;
        case DIR_4_BOTTOM:
            ++(*y);
            break;
        case DIR_5_BOTTOM_LEFT:
            --(*x);
            ++(*y);
            break;
        case DIR_6_LEFT:
            --(*x);
            break;
        case DIR_7_TOP_LEFT:
            --(*x);
            --(*y);
            break;
    }
    (*grid_offset) += map_grid_direction_delta(direction);
}

int map_routing_get_path(uint8_t *path, int src_x, int src_y, int dst_x, int dst_y, int num_directions)
{
    int dstGridOffset = map_grid_offset(dst_x, dst_y);
    int distance = Data_Grid_routingDistance[dstGridOffset];
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int numTiles = 0;
    int lastDirection = -1;
    int x = dst_x;
    int y = dst_y;
    int gridOffset = dstGridOffset;
    int step = num_directions == 8 ? 1 : 2;

    while (distance > 1) {
        distance = Data_Grid_routingDistance[gridOffset];
        int direction = -1;
        int generalDirection = calc_general_direction(x, y, src_x, src_y);
        for (int d = 0; d < 8; d += step) {
            if (d != lastDirection) {
                int nextOffset = gridOffset + map_grid_direction_delta(d);
                int nextDistance = Data_Grid_routingDistance[nextOffset];
                if (nextDistance) {
                    if (nextDistance < distance) {
                        distance = nextDistance;
                        direction = d;
                    } else if (nextDistance == distance && (d == generalDirection || direction == -1)) {
                        distance = nextDistance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &gridOffset);
        int forwardDirection = (direction + 4) % 8;
        direction_path[numTiles++] = forwardDirection;
        lastDirection = forwardDirection;
        if (numTiles >= MAX_PATH) {
            return 0;
        }
    }
    for (int i = 0; i < numTiles; i++) {
        path[i] = direction_path[numTiles - i - 1];
    }
    return numTiles;
}

int map_routing_get_closest_tile_within_range(int src_x, int src_y, int dst_x, int dst_y, int num_directions, int range, int *out_x, int *out_y)
{
    int dstGridOffset = map_grid_offset(dst_x, dst_y);
    int distance = Data_Grid_routingDistance[dstGridOffset];
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int numTiles = 0;
    int lastDirection = -1;
    int x = dst_x;
    int y = dst_y;
    int gridOffset = dstGridOffset;
    int step = num_directions == 8 ? 1 : 2;

    while (distance > 1) {
        distance = Data_Grid_routingDistance[gridOffset];
        *out_x = x;
        *out_y = y;
        if (distance <= range) {
            return 1;
        }
        int direction = -1;
        int generalDirection = calc_general_direction(x, y, src_x, src_y);
        for (int d = 0; d < 8; d += step) {
            if (d != lastDirection) {
                int nextOffset = gridOffset + map_grid_direction_delta(d);
                int nextDistance = Data_Grid_routingDistance[nextOffset];
                if (nextDistance) {
                    if (nextDistance < distance) {
                        distance = nextDistance;
                        direction = d;
                    } else if (nextDistance == distance && (d == generalDirection || direction == -1)) {
                        distance = nextDistance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &gridOffset);
        int forwardDirection = (direction + 4) % 8;
        direction_path[numTiles++] = forwardDirection;
        lastDirection = forwardDirection;
        if (numTiles >= MAX_PATH) {
            return 0;
        }
    }
    return 0;
}

int map_routing_get_path_on_water(uint8_t *path, int src_x, int src_y, int dst_x, int dst_y, int is_flotsam)
{
    int rand = random_byte() & 3;
    int dstGridOffset = map_grid_offset(dst_x, dst_y);
    int distance = Data_Grid_routingDistance[dstGridOffset];
    if (distance <= 0 || distance >= 998) {
        return 0;
    }

    int numTiles = 0;
    int lastDirection = -1;
    int x = dst_x;
    int y = dst_y;
    int gridOffset = dstGridOffset;
    while (distance > 1) {
        int currentRand = rand;
        distance = Data_Grid_routingDistance[gridOffset];
        if (is_flotsam) {
            currentRand = Data_Grid_random[gridOffset] & 3;
        }
        int direction = -1;
        for (int d = 0; d < 8; d++) {
            if (d != lastDirection) {
                int nextOffset = gridOffset + map_grid_direction_delta(d);
                int nextDistance = Data_Grid_routingDistance[nextOffset];
                if (nextDistance) {
                    if (nextDistance < distance) {
                        distance = nextDistance;
                        direction = d;
                    } else if (nextDistance == distance && rand == currentRand) {
                        // allow flotsam to wander
                        distance = nextDistance;
                        direction = d;
                    }
                }
            }
        }
        if (direction == -1) {
            return 0;
        }
        adjust_tile_in_direction(direction, &x, &y, &gridOffset);
        int forwardDirection = (direction + 4) % 8;
        direction_path[numTiles++] = forwardDirection;
        lastDirection = forwardDirection;
        if (numTiles >= MAX_PATH) {
            return 0;
        }
    }
    for (int i = 0; i < numTiles; i++) {
        path[i] = direction_path[numTiles - i - 1];
    }
    return numTiles;
}
