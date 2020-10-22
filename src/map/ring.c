#include "ring.h"

#include "map/data.h"
#include "map/grid.h"

static struct {
    ring_tile tiles[1700];
    int index[8][7];
} data;

void map_ring_init(void)
{
    int index = 0;
    int x, y;
    for (int size = 1; size <= 7; size++) {
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
    return x >= -1 && x <= map_data.width &&
        y >= -1 && y <= map_data.height;
}

const ring_tile *map_ring_tile(int index)
{
    return &data.tiles[index];
}
