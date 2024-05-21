#ifndef MAP_TERRAIN_H
#define MAP_TERRAIN_H

#include "core/buffer.h"

enum {
    TERRAIN_TREE                 = 1 << 0,
    TERRAIN_ROCK                 = 1 << 1,
    TERRAIN_WATER                = 1 << 2,
    TERRAIN_BUILDING             = 1 << 3,
    TERRAIN_SHRUB                = 1 << 4,
    TERRAIN_GARDEN               = 1 << 5,
    TERRAIN_ROAD                 = 1 << 6,
    TERRAIN_RESERVOIR_RANGE      = 1 << 7,
    TERRAIN_AQUEDUCT             = 1 << 8,
    TERRAIN_ELEVATION            = 1 << 9,
    TERRAIN_ACCESS_RAMP          = 1 << 10,
    TERRAIN_MEADOW               = 1 << 11,
    TERRAIN_RUBBLE               = 1 << 12,
    TERRAIN_FOUNTAIN_RANGE       = 1 << 13,
    TERRAIN_WALL                 = 1 << 14,
    TERRAIN_GATEHOUSE            = 1 << 15,
    TERRAIN_ORIGINALLY_TREE      = 1 << 16,
    TERRAIN_HIGHWAY_TOP_LEFT     = 1 << 17,
    TERRAIN_HIGHWAY_BOTTOM_LEFT  = 1 << 18,
    TERRAIN_HIGHWAY_TOP_RIGHT    = 1 << 19,
    TERRAIN_HIGHWAY_BOTTOM_RIGHT = 1 << 20,

    // Combined
    TERRAIN_HIGHWAY               = TERRAIN_HIGHWAY_TOP_LEFT | TERRAIN_HIGHWAY_BOTTOM_LEFT |
                                    TERRAIN_HIGHWAY_TOP_RIGHT | TERRAIN_HIGHWAY_BOTTOM_RIGHT,
                                    
    TERRAIN_WALL_OR_GATEHOUSE     = TERRAIN_WALL | TERRAIN_GATEHOUSE,
    
    TERRAIN_NOT_CLEAR_EXCEPT_ROAD = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB |
                                    TERRAIN_GARDEN | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |
                                    TERRAIN_RUBBLE | TERRAIN_WALL_OR_GATEHOUSE | TERRAIN_HIGHWAY,
                                    
    TERRAIN_NOT_CLEAR             = TERRAIN_NOT_CLEAR_EXCEPT_ROAD | TERRAIN_ROAD,
    
    TERRAIN_CLEARABLE             = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB |
                                    TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_AQUEDUCT | TERRAIN_RUBBLE |
                                    TERRAIN_WALL_OR_GATEHOUSE | TERRAIN_HIGHWAY,
                                    
    TERRAIN_IMPASSABLE            = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB |
                                    TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_WALL_OR_GATEHOUSE,
                                    
    TERRAIN_IMPASSABLE_ENEMY      = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_SHRUB | TERRAIN_GARDEN |
                                    TERRAIN_ELEVATION | TERRAIN_RUBBLE,
                                    
    TERRAIN_IMPASSABLE_WOLF       = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB |
                                    TERRAIN_GARDEN | TERRAIN_AQUEDUCT | TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP |
                                    TERRAIN_RUBBLE | TERRAIN_WALL_OR_GATEHOUSE,
                                    
    TERRAIN_ELEVATION_ROCK        = TERRAIN_ELEVATION | TERRAIN_ROCK,

    TERRAIN_ALL                   = TERRAIN_TREE | TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SHRUB |
                                    TERRAIN_GARDEN | TERRAIN_ROAD| TERRAIN_AQUEDUCT | TERRAIN_ELEVATION |
                                    TERRAIN_ACCESS_RAMP | TERRAIN_MEADOW | TERRAIN_RUBBLE | TERRAIN_FOUNTAIN_RANGE |
                                    TERRAIN_WALL_OR_GATEHOUSE | TERRAIN_HIGHWAY,
                                    
    TERRAIN_MAP_EDGE              = TERRAIN_TREE | TERRAIN_WATER,
};

int map_terrain_is(int grid_offset, int terrain);

int map_terrain_is_superset(int grid_offset, unsigned int terrain_sum);

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
 * @param terrain_sum Terrain bitmask to be checked for.
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
