#ifndef MAP_WATER_H
#define MAP_WATER_H

#include "figure/figure.h"

void map_water_add_building(int building_id, int x, int y, int size, int image_id);

int map_water_determine_orientation_size2(int x, int y, int adjust_xy,
                                          int *orientation_absolute, int *orientation_relative);

int map_water_determine_orientation_size3(int x, int y, int adjust_xy,
                                          int *orientation_absolute, int *orientation_relative);

int map_water_get_wharf_for_new_fishing_boat(figure *boat, int *x_tile, int *y_tile);

int map_water_find_alternative_fishing_boat_tile(figure *boat, int *x_tile, int *y_tile);

int map_water_find_shipwreck_tile(figure *wreck, int *x_tile, int *y_tile);

int map_water_can_spawn_fishing_boat(int x, int y, int size, int *x_tile, int *y_tile);

#endif // MAP_WATER_H
