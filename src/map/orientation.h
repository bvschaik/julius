#ifndef MAP_ORIENTATION_H
#define MAP_ORIENTATION_H

void map_orientation_change(int counter_clockwise);

int map_orientation_for_gatehouse(int x, int y);

int map_orientation_for_triumphal_arch(int x, int y);

void map_orientation_update_buildings();

#endif // MAP_ORIENTATION_H
