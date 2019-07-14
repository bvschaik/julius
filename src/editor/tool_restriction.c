#include "tool_restriction.h"

#include "map/elevation.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/terrain.h"

static const int TILE_GRID_OFFSETS[] = { 0, 162, 1, 163 };

static const int ACCESS_RAMP_TILE_OFFSETS_BY_ORIENTATION[4][6] = {
    {162, 163, 324, 325, 0, 1},
    {0, 162, -1, 161, 1, 163},
    {0, 1, -162, -161, 162, 163},
    {1, 163, 2, 164, 0, 162},
};

static int is_clear_terrain(const map_tile *tile)
{
    return !map_terrain_is(tile->grid_offset, TERRAIN_NOT_CLEAR ^ TERRAIN_ROAD);
}

static int is_edge(const map_tile *tile)
{
    return tile->x == 0 || tile->y == 0 || tile->x == map_grid_width() - 1 || tile->y == map_grid_height() - 1;
}

static int is_deep_water(const map_tile *tile)
{
    return map_terrain_is(tile->grid_offset, TERRAIN_WATER) &&
        map_terrain_count_directly_adjacent_with_type(tile->grid_offset, TERRAIN_WATER) == 4;
}

int editor_tool_can_place_flag(tool_type type, const map_tile *tile)
{
    switch (type) {
        case TOOL_ENTRY_POINT:
        case TOOL_EXIT_POINT:
        case TOOL_INVASION_POINT:
            return is_edge(tile) && is_clear_terrain(tile);

        case TOOL_EARTHQUAKE_POINT:
        case TOOL_HERD_POINT:
            return is_clear_terrain(tile);

        case TOOL_FISHING_POINT:
            return map_terrain_is(tile->grid_offset, TERRAIN_WATER);

        case TOOL_RIVER_ENTRY_POINT:
        case TOOL_RIVER_EXIT_POINT:
            return is_edge(tile) && is_deep_water(tile);

        default:
            return 0;
    }
}

int editor_tool_can_place_access_ramp(const map_tile *tile, int *orientation_index)
{
    if (!map_grid_is_inside(tile->x, tile->y, 2)) {
        return 0;
    }
    for (int orientation = 0; orientation < 4; orientation++) {
        int right_tiles = 0;
        int wrong_tiles = 0;
        int top_elevation = 0;
        for (int index = 0; index < 6; index++) {
            int tile_offset = tile->grid_offset + ACCESS_RAMP_TILE_OFFSETS_BY_ORIENTATION[orientation][index];
            int elevation = map_elevation_at(tile_offset);
            if (index < 2) {
                if (map_terrain_is(tile_offset, TERRAIN_ELEVATION)) {
                    right_tiles++;
                } else {
                    wrong_tiles++;
                }
                top_elevation = elevation;
            } else if (index < 4) {
                if (map_terrain_is(tile_offset, TERRAIN_ELEVATION)) {
                    if (elevation == top_elevation) {
                        wrong_tiles++;
                    } else {
                        right_tiles++;
                    }
                } else if (elevation >= top_elevation) {
                    right_tiles++;
                } else {
                    wrong_tiles++;
                }
            } else {
                if (map_terrain_is(tile_offset, TERRAIN_ELEVATION | TERRAIN_ACCESS_RAMP)) {
                    wrong_tiles++;
                } else if (elevation >= top_elevation) {
                    wrong_tiles++;
                } else {
                    right_tiles++;
                }
            }
        }
        if (right_tiles == 6) {
            *orientation_index = orientation;
            return 1;
        }
    }
    return 0;
}

int editor_tool_can_place_building(const map_tile *tile, int num_tiles, int *blocked_tiles)
{
    int blocked = 0;
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = tile->grid_offset + TILE_GRID_OFFSETS[i];
        int forbidden_terrain = map_terrain_get(tile_offset) & TERRAIN_NOT_CLEAR;
        if (forbidden_terrain || map_has_figure_at(tile_offset)) {
            blocked_tiles[i] = blocked = 1;
        } else {
            blocked_tiles[i] = 0;
        }
    }
    return blocked;
}
