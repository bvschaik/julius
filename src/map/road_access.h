#ifndef MAP_ROAD_ACCESS_H
#define MAP_ROAD_ACCESS_H

#include "building/roadblock.h"
#include "map/point.h"

int map_has_road_access(int x, int y, int size, map_point *road);

int map_has_road_access_rotation(int rotation, int x, int y, int size, map_point *road);

int map_has_road_access_hippodrome(int x, int y, map_point *road);

int map_has_road_access_hippodrome_rotation(int x, int y, map_point *road, int rotation);

int map_has_road_access_granary(int x, int y, map_point *road);

int map_has_road_access_monument_construction(int x, int y, int size);

int map_closest_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road);

int map_closest_reachable_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road);

int map_closest_reachable_spot_within_radius(int x, int y, int size, int radius, int *x_point, int *y_point);

int map_road_to_largest_network_rotation(int rotation, int x, int y, int size, int *x_road, int *y_road);

int map_road_to_largest_network(int x, int y, int size, int *x_road, int *y_road);

int map_road_to_largest_network_hippodrome(int x, int y, int *x_road, int *y_road, int rotated);

int map_road_to_largest_network_monument_construction(int x, int y, int size, int *x_road, int *y_road);

int map_get_adjacent_road_tiles_for_roaming(int grid_offset, int *road_tiles, int p);

int map_get_diagonal_road_tiles_for_roaming(int grid_offset, int *road_tiles);

#endif // MAP_ROAD_ACCESS_H
