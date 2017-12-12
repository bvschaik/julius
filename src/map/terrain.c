#include "terrain.h"

#include "map/grid.h"

static grid_u16 terrain_grid;
static grid_u16 terrain_grid_backup;

int map_terrain_is(int grid_offset, int terrain)
{
    return terrain_grid.items[grid_offset] & terrain;
}

int map_terrain_get(int grid_offset)
{
    return terrain_grid.items[grid_offset];
}

void map_terrain_set(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] = terrain;
}

void map_terrain_add(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] |= terrain;
}

void map_terrain_remove(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] &= ~terrain;
}

void map_terrain_remove_all(int terrain)
{
    map_grid_and_u16(terrain_grid.items, ~terrain);
}

void map_terrain_backup()
{
    map_grid_copy_u16(terrain_grid.items, terrain_grid_backup.items);
}

void map_terrain_restore()
{
    map_grid_copy_u16(terrain_grid_backup.items, terrain_grid.items);
}

void map_terrain_clear()
{
    map_grid_clear_u16(terrain_grid.items);
}

void map_terrain_save_state(buffer *buf)
{
    map_grid_save_state_u16(terrain_grid.items, buf);
}

void map_terrain_load_state(buffer *buf)
{
    map_grid_load_state_u16(terrain_grid.items, buf);
}
