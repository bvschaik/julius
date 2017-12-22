#include "ring.h"

#include "map/grid.h"

#include "Data/State.h"

static const int ADJACENT_OFFSETS[][21] = {
    {0},
    {-162, 1, 162, -1, 0},
    {-162, -161, 2, 164, 325, 324, 161, -1, 0},
    {-162, -161, -160, 3, 165, 327, 488, 487, 486, 323, 161, -1, 0},
    {-162, -161, -160, -159, 4, 166, 328, 490, 651, 650, 649, 648, 485, 323, 161, -1, 0},
    {-162, -161, -160, -159, -158, 5, 167, 329, 491, 653, 814, 813, 812, 811, 810, 647, 485, 323, 161, -1, 0},
};

static struct {
    ring_tile tiles[1080];
    int index[6][7];
} data;

void map_ring_init()
{
    int index = 0;
    int x, y;
    for (int size = 1; size <= 5; size++) {
        for (int dist = 1; dist <= 6; dist++) {
            data.index[size][dist] = index;
            // top row, from x=0
            for (y = -dist, x = 0; x < size + dist; x++, index++) {
                data.tiles[index].x = x;
                data.tiles[index].y = y;
            }
            // right row down
            for (x = size + dist - 1, y = -dist + 1; y < size + dist; y++, index++) {
                data.tiles[index].x = x;
                data.tiles[index].y = y;
            }
            // bottom row to the left
            for (y = size + dist - 1, x = size + dist - 2; x >= -dist; x--, index++) {
                data.tiles[index].x = x;
                data.tiles[index].y = y;
            }
            // exception (bug in game): size 4 distance 2, left corner is off by x+1, y-1
            if (size == 4 && dist == 2) {
                data.tiles[index-1].x += 1;
                data.tiles[index-1].y -= 1;
            }
            // left row up
            for (x = -dist, y = size + dist - 2; y >= -dist; y--, index++) {
                data.tiles[index].x = x;
                data.tiles[index].y = y;
            }
            // top row up to x=0
            for (y = -dist, x = -dist + 1; x < 0; x++, index++) {
                data.tiles[index].x = x;
                data.tiles[index].y = y;
            }
        }
    }
    for (int i = 0; i < index; i++) {
        data.tiles[i].grid_offset = map_grid_delta(data.tiles[i].x, data.tiles[i].y);
    }
}

int map_ring_start(int size, int distance)
{
    return data.index[size][distance];
}

int map_ring_end(int size, int distance)
{
    return map_ring_start(size, distance) + 4 * (size - 1) + 8 * distance;
}

int map_ring_is_inside_map(int x, int y)
{
    return x >= -1 && x <= Data_State.map.width &&
        y >= -1 && y <= Data_State.map.height;
}

const ring_tile *map_ring_tile(int index)
{
    return &data.tiles[index];
}

const int *map_adjacent_offsets(int size)
{
    return ADJACENT_OFFSETS[size];
}
