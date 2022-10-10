#ifndef MAP_TERRAIN_H
#define MAP_TERRAIN_H

#include "core/buffer.h"

enum {
    TERRAIN_TREE = 1,
    TERRAIN_ROCK = 2,
    TERRAIN_WATER = 4,
    TERRAIN_BUILDING = 8,
    TERRAIN_SHRUB = 0x10,
    TERRAIN_GARDEN = 0x20,
    TERRAIN_ROAD = 0x40,
    TERRAIN_RESERVOIR_RANGE = 0x80,
    TERRAIN_AQUEDUCT = 0x100,
    TERRAIN_ELEVATION = 0x200,
    TERRAIN_ACCESS_RAMP = 0x400,
    TERRAIN_MEADOW = 0x800,
    TERRAIN_RUBBLE = 0x1000,
    TERRAIN_FOUNTAIN_RANGE = 0x2000,
    TERRAIN_WALL = 0x4000,
    TERRAIN_GATEHOUSE = 0x8000,
    TERRAIN_ORIGINALLY_TREE = 0x10000,
    TERRAIN_HIGHWAY_TOP_LEFT = 0x20000,
    TERRAIN_HIGHWAY_BOTTOM_LEFT = 0x40000,
    TERRAIN_HIGHWAY_TOP_RIGHT = 0x80000,
    TERRAIN_HIGHWAY_BOTTOM_RIGHT = 0x100000,
    TERRAIN_HIGHWAY = TERRAIN_HIGHWAY_TOP_LEFT | TERRAIN_HIGHWAY_BOTTOM_LEFT | TERRAIN_HIGHWAY_TOP_RIGHT | TERRAIN_HIGHWAY_BOTTOM_RIGHT,
    // combined
    TERRAIN_WALL_OR_GATEHOUSE = TERRAIN_WALL | TERRAIN_GATEHOUSE,
    TERRAIN_NOT_CLEAR = 0xd77f | TERRAIN_HIGHWAY,
    TERRAIN_NOT_CLEAR_EXCEPT_ROAD = 0xd73f | TERRAIN_HIGHWAY,
    TERRAIN_CLEARABLE = 0xd17f | TERRAIN_HIGHWAY,
    TERRAIN_IMPASSABLE = 0xc75f,
    TERRAIN_IMPASSABLE_ENEMY = 0x1237,
    TERRAIN_IMPASSABLE_WOLF = 0xd73f,
    TERRAIN_ELEVATION_ROCK = 0x202,
    TERRAIN_ALL = 0xffff | TERRAIN_HIGHWAY,
    TERRAIN_MAP_EDGE = TERRAIN_TREE | TERRAIN_WATER,
};

int map_terrain_is(int grid_offset, int terrain);

int map_terrain_is_superset(int grid_offset, int terrain_sum);

int map_terrain_get(int grid_offset);

int map_terrain_get_from_buffer_16(buffer *buf, int grid_offset);

int map_terrain_get_from_buffer_32(buffer *buf, int grid_offset);

void map_terrain_set(int grid_offset, int terrain);

void map_terrain_add(int grid_offset, int terrain);

void map_terrain_remove(int grid_offset, int terrain);

void map_terrain_add_with_radius(int x, int y, int size, int radius, int terrain);

void map_terrain_remove_with_radius(int x, int y, int size, int radius, int terrain);

void map_terrain_remove_all(int terrain);

/**
 * Check orthogonal neighbours of a tile if they contain a terrain.
 * @param grid_offset Tile which neighbours will be checked.
 * @param terrain Terrain bitmask to be checked for.
 * @return 1 if any orthogonal tiles matches at least one terrain from the bitmask, 0 otherwise.
 */
int map_terrain_count_directly_adjacent_with_type(int grid_offset, int terrain);

/**
 * Check orthogonal neighbours of a tile if they contain a terrain.
 * @param grid_offset Tile which neighbours will be checked.
 * @param terrain Terrain bitmask to be checked for.
 * @return 1 if any orthogonal tiles matches all terrains from the bitmask, 0 otherwise.
 */
int map_terrain_count_directly_adjacent_with_types(int grid_offset, int terrain_sum);

int map_terrain_count_diagonally_adjacent_with_type(int grid_offset, int terrain);

int map_terrain_has_adjacent_x_with_type(int grid_offset, int terrain);

int map_terrain_has_adjacent_y_with_type(int grid_offset, int terrain);

int map_terrain_exists_tile_in_area_with_type(int x, int y, int size, int terrain);

int map_terrain_exists_tile_in_radius_with_type(int x, int y, int size, int radius, int terrain);

int map_terrain_exists_rock_in_radius(int x, int y, int size, int radius);

int map_terrain_exists_clear_tile_in_radius(int x, int y, int size, int radius, int except_grid_offset,
    int *x_tile, int *y_tile);

int map_terrain_all_tiles_in_radius_are(int x, int y, int size, int radius, int terrain);

int map_terrain_has_only_rocks_trees_in_ring(int x, int y, int distance);

int map_terrain_has_only_meadow_in_ring(int x, int y, int distance);

int map_terrain_is_adjacent_to_wall(int x, int y, int size);

int map_terrain_is_adjacent_to_water(int x, int y, int size);

int map_terrain_is_adjacent_to_open_water(int x, int y, int size);

int map_terrain_get_adjacent_road_or_clear_land(int x, int y, int size, int *x_tile, int *y_tile);

void map_terrain_add_roadblock_road(int x, int y);
void map_terrain_add_gatehouse_roads(int x, int y, int orientation);
void map_terrain_add_triumphal_arch_roads(int x, int y, int orientation);

void map_terrain_backup(void);

void map_terrain_restore(void);

void map_terrain_clear(void);

void map_terrain_init_outside_map(void);

void map_terrain_save_state(buffer *buf);
void map_terrain_save_state_legacy(buffer *buf);

void map_terrain_load_state(buffer *buf, int expanded_terrain_data, buffer *images, int legacy_image_buffer);

#endif // MAP_TERRAIN_H
