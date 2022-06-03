#include "elevation.h"

#include "map/data.h"
#include "map/grid.h"
#include "map/terrain.h"

static grid_u8 elevation;

int map_elevation_at(int grid_offset)
{
    return elevation.items[grid_offset];
}

void map_elevation_set(int grid_offset, int value)
{
    elevation.items[grid_offset] = value;
}

void map_elevation_clear(void)
{
    map_grid_clear_u8(elevation.items);
}

static void fix_cliff_tiles(int grid_offset)
{
    // reduce elevation when the surrounding tiles are at least 2 lower
    // treat edges as being equal elevations
    int max = elevation.items[grid_offset] - 1;
    if ((elevation.items[grid_offset + map_grid_delta(-1, 0)] < max && map_terrain_get(grid_offset + map_grid_delta(-1, 0)) != TERRAIN_MAP_EDGE) ||
        (elevation.items[grid_offset + map_grid_delta(0, -1)] < max && map_terrain_get(grid_offset + map_grid_delta(0, -1)) != TERRAIN_MAP_EDGE) ||
        (elevation.items[grid_offset + map_grid_delta(1, 0)] < max && map_terrain_get(grid_offset + map_grid_delta(1, 0)) != TERRAIN_MAP_EDGE) ||
        (elevation.items[grid_offset + map_grid_delta(0, 1)] < max && map_terrain_get(grid_offset + map_grid_delta(0, 1)) != TERRAIN_MAP_EDGE)) {
        elevation.items[grid_offset]--;
    }
}

void map_elevation_remove_cliffs(void)
{
    // elevation is max 5, so we need 4 passes to fix the lot
    for (int level = 0; level < 4; level++) {
        int grid_offset = map_data.start_offset;
        for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
            for (int x = 0; x < map_data.width; x++, grid_offset++) {
                if (elevation.items[grid_offset] > 0) {
                    fix_cliff_tiles(grid_offset);
                }
            }
        }
    }
}

void map_elevation_save_state(buffer *buf)
{
    map_grid_save_state_u8(elevation.items, buf);
}

void map_elevation_load_state(buffer *buf)
{
    map_grid_load_state_u8(elevation.items, buf);
}
