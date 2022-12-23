#ifndef MAP_WATER_H
#define MAP_WATER_H

#include "figure/figure.h"
#include "map/point.h"

#define MAP_WATER_WATERSIDE_ROWS_NEEDED 3

typedef struct {
    struct {
        int x;
        int y;
    } start;
    struct {
        int x;
        int y;
    } outer_step;
    struct {
        int x;
        int y;
    } inner_step;
    int inner_length;
} waterside_tile_loop;

void map_water_add_building(int building_id, int x, int y, int size);

int map_water_determine_orientation(int x, int y, int size, int adjust_xy,
    int *orientation_absolute, int *orientation_relative, int check_water_in_front, int *blocked);

const waterside_tile_loop *map_water_get_waterside_tile_loop(int direction, int size);

int map_water_has_water_in_front(int x, int y, int adjust_xy, const waterside_tile_loop *loop, int *land_tiles);

int map_water_get_wharf_for_new_fishing_boat(figure *boat, map_point *tile);

int map_water_find_alternative_fishing_boat_tile(figure *boat, map_point *tile);

int map_water_find_shipwreck_tile(figure *wreck, map_point *tile);

int map_water_can_spawn_fishing_boat(int x, int y, int size, map_point *tile);

#endif // MAP_WATER_H
