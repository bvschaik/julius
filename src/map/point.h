#ifndef MAP_POINT_H
#define MAP_POINT_H

typedef struct {
    int x;
    int y;
} map_point;

typedef struct {
    int x;
    int y;
    int grid_offset;
} map_tile;

#endif // MAP_POINT_H
