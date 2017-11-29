#include "property.h"

#include "map/grid.h"

#include "Data/Grid.h"

int map_property_is_draw_tile(int grid_offset)
{
    return Data_Grid_edge[grid_offset] & Edge_LeftmostTile;
}

void map_property_mark_draw_tile(int grid_offset)
{
    Data_Grid_edge[grid_offset] |= Edge_LeftmostTile;
}

void map_property_clear_draw_tile(int grid_offset)
{
    Data_Grid_edge[grid_offset] &= ~Edge_LeftmostTile;
}

int map_property_is_native_land(int grid_offset)
{
    return Data_Grid_edge[grid_offset] & Edge_NativeLand;
}

void map_property_mark_native_land(int grid_offset)
{
    Data_Grid_edge[grid_offset] |= Edge_NativeLand;
}

void map_property_clear_all_native_land()
{
    map_grid_and_u8(Data_Grid_edge, Edge_NoNativeLand);
}

void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile)
{
    if (is_draw_tile) {
        Data_Grid_edge[grid_offset] = EdgeXY(x, y) | Edge_LeftmostTile;
    } else {
        Data_Grid_edge[grid_offset] = EdgeXY(x, y);
    }
}

void map_property_clear_multi_tile_xy(int grid_offset)
{
    // only keep native land marker
    Data_Grid_edge[grid_offset] &= Edge_NativeLand;
}

int map_property_multi_tile_xy(int grid_offset)
{
    return Data_Grid_edge[grid_offset] & Edge_MaskXY;
}

int map_property_multi_tile_x(int grid_offset)
{
    return Data_Grid_edge[grid_offset] & Edge_MaskX;
}

int map_property_multi_tile_y(int grid_offset)
{
    return Data_Grid_edge[grid_offset] & Edge_MaskY;
}

int map_property_is_multi_tile_xy(int grid_offset, int x, int y)
{
    return (Data_Grid_edge[grid_offset] & Edge_MaskXY) == EdgeXY(x, y);
}
