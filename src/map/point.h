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

/**
 * Stores the X and Y to the passed point.
 * This also stores the X and Y it for later retrieval using map_point_get_last_result().
 * We do this because the original game uses global variables for passing result X/Y coordinates,
 * which leads in certain cases to re-use of the previous values. We need to emulate that.
 *
 * @param x X value to store
 * @param y Y value to store
 * @param point Point structure to store X and Y in
 */
void map_point_store_result(int x, int y, map_point *point);

void map_point_get_last_result(map_point *point);

#endif // MAP_POINT_H
