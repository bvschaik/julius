#include "elevation.h"

#include "map/grid.h"

static grid_u8 elevation;

int map_elevation_at(int grid_offset)
{
    return elevation.items[grid_offset];
}

void map_elevation_clear()
{
    map_grid_clear_u8(elevation.items);
}

void map_elevation_save_state(buffer *buf)
{
    map_grid_save_state_u8(elevation.items, buf);
}

void map_elevation_load_state(buffer *buf)
{
    map_grid_load_state_u8(elevation.items, buf);
}
