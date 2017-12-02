#ifndef MAP_RING_H
#define MAP_RING_H

typedef struct {
    int x;
    int y;
    int grid_offset;
} ring_tile;

void map_ring_init();

int map_ring_start(int size, int distance);

int map_ring_end(int size, int distance);

const ring_tile *map_ring_tile(int index);

#endif // MAP_RING_H
