#ifndef MAP_ROAD_ACCESS_H
#define MAP_ROAD_ACCESS_H

int map_has_road_access(int x, int y, int size, int *x_road, int *y_road);

int map_has_road_access_hippodrome(int x, int y, int *x_road, int *y_road);

int map_has_road_access_granary(int x, int y, int *x_road, int *y_road);

int map_closest_road_within_radius(int x, int y, int size, int radius, int *x_road, int *y_road);

#endif // MAP_ROAD_ACCESS_H
