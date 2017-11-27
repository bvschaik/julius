#ifndef MAP_SOLDIER_STRENGTH_H
#define MAP_SOLDIER_STRENGTH_H

void map_soldier_strength_clear();

void map_soldier_strength_add(int x, int y, int radius, int amount);

int map_soldier_strength_get(int grid_offset);

int map_soldier_strength_get_max(int x, int y, int radius, int *out_x, int *out_y);

#endif // MAP_SOLDIER_STRENGTH_H
