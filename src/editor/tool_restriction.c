#include "tool_restriction.h"

#include "city/warning.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/terrain.h"

#define OFFSET(x,y) (x + GRID_SIZE * y)

static const int TILE_GRID_OFFSETS[] = { 0, GRID_SIZE, 1, GRID_SIZE + 1 };

static const int ACCESS_RAMP_TILE_OFFSETS_BY_ORIENTATION[4][6] = {
    {OFFSET(0,1), OFFSET(1,1), OFFSET(0,2), OFFSET(1,2), OFFSET(0,0), OFFSET(1,0)},
    {OFFSET(0,0), OFFSET(0,1), OFFSET(-1,0), OFFSET(-1,1), OFFSET(1,0), OFFSET(1,1)},
    {OFFSET(0,0), OFFSET(1,0), OFFSET(0,-1), OFFSET(1,-1), OFFSET(0,1), OFFSET(1,1)},
    {OFFSET(1,0), OFFSET(1,1), OFFSET(2,0), OFFSET(2,1), OFFSET(0,0), OFFSET(0,1)},
};

static int is_clear_terrain(const map_tile *tile, int *warning)
{
    int result = !map_terrain_is(tile->grid_offset, TERRAIN_NOT_CLEAR ^ TERRAIN_ROAD);
    if (!result && warning) {
        *warning = WARNING_EDITOR_CANNOT_PLACE;
    }
    return result;
}

static int is_edge(const map_tile *tile, int *warning)
{
    int result = tile->x == 0 || tile->y == 0 || tile->x == map_grid_width() - 1 || tile->y == map_grid_height() - 1;
    if (!result && warning) {
        *warning = WARNING_EDITOR_NEED_MAP_EDGE;
    }
    return result;
}

static int is_water(const map_tile *tile, int *warning)
{
    int result = map_terrain_is(tile->grid_offset, TERRAIN_WATER);
    if (!result && warning) {
        *warning = WARNING_EDITOR_NEED_OPEN_WATER;
    }
    return result;
}

static int is_deep_water(const map_tile *tile, int *warning)
{
    int result = map_terrain_is(tile->grid_offset, TERRAIN_WATER) &&
        map_terrain_count_directly_adjacent_with_type(tile->grid_offset, TERRAIN_WATER) == 4;
    if (!result && warning) {
        *warning = WARNING_EDITOR_NEED_OPEN_WATER;
    }
    return result;
}

int editor_tool_can_place_flag(tool_type type, const map_tile *tile, int *warning)
{
    switch (type) {
        case TOOL_ENTRY_POINT:
        case TOOL_EXIT_POINT:
        case TOOL_INVASION_POINT:
            return is_clear_terrain(tile, warning) && is_edge(tile, warning);

        case TOOL_EARTHQUAKE_POINT:
        case TOOL_HERD_POINT:
            return is_clear_terrain(tile, warning);

        case TOOL_FISHING_POINT:
            return is_water(tile, warning);

        case TOOL_RIVER_ENTRY_POINT:
        case TOOL_RIVER_EXIT_POINT:
            return is_edge(tile, warning) && is_deep_water(tile, warning);

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
            if (orientation_index) {
                *orientation_index = orientation;
            }
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
            blocked = 1;
            if (blocked_tiles) blocked_tiles[i] = 1;
        } else {
            if (blocked_tiles) blocked_tiles[i] = 0;
        }
    }
    return !blocked;
}
