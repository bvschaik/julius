#include "terrain.h"

#include "map/grid.h"

static grid_u16 terrain_grid_backup;

int map_terrain_is(int grid_offset, int terrain)
{
    return Data_Grid_terrain[grid_offset] & terrain;
}

void map_terrain_add(int grid_offset, int terrain)
{
    Data_Grid_terrain[grid_offset] |= terrain;
}

void map_terrain_remove(int grid_offset, int terrain)
{
    Data_Grid_terrain[grid_offset] &= ~terrain;
}

void map_terrain_backup()
{
    map_grid_copy_u16(Data_Grid_terrain, terrain_grid_backup.items);
}

void map_terrain_restore()
{
    map_grid_copy_u16(terrain_grid_backup.items, Data_Grid_terrain);
}

void map_terrain_clear()
{
    map_grid_clear_u16(Data_Grid_terrain);
}

void map_terrain_save_state(buffer *buf)
{
    map_grid_save_state_u16(Data_Grid_terrain, buf);
}

void map_terrain_load_state(buffer *buf)
{
    map_grid_load_state_u16(Data_Grid_terrain, buf);
}
