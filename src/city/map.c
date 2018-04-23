#include "map.h"

#include "city/data_private.h"
#include "map/grid.h"

const map_tile *city_map_entry_flag()
{
    return &city_data.map.entry_flag;
}

const map_tile *city_map_exit_flag()
{
    return &city_data.map.exit_flag;
}

static int set_tile(map_tile *tile, int x, int y)
{
    int grid_offset = map_grid_offset(x, y);
    tile->x = x;
    tile->y = y;
    tile->grid_offset = grid_offset;
    return grid_offset;
}

int city_map_set_entry_flag(int x, int y)
{
    return set_tile(&city_data.map.entry_flag, x, y);
}

int city_map_set_exit_flag(int x, int y)
{
    return set_tile(&city_data.map.exit_flag, x, y);
}
