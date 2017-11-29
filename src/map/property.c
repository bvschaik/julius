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

int map_property_multi_tile_size(int grid_offset)
{
    switch (Data_Grid_bitfields[grid_offset] & Bitfield_Sizes) {
        case Bitfield_Size2: return 2;
        case Bitfield_Size3: return 3;
        case Bitfield_Size4: return 4;
        case Bitfield_Size5: return 5;
        default: return 1;
    }
}

void map_property_set_multi_tile_size(int grid_offset, int size)
{
    Data_Grid_bitfields[grid_offset] &= Bitfield_NoSizes;
    switch (size) {
        case 2: Data_Grid_bitfields[grid_offset] |= Bitfield_Size2; break;
        case 3: Data_Grid_bitfields[grid_offset] |= Bitfield_Size3; break;
        case 4: Data_Grid_bitfields[grid_offset] |= Bitfield_Size4; break;
        case 5: Data_Grid_bitfields[grid_offset] |= Bitfield_Size5; break;
    }
}

int map_property_is_alternate_terrain(int grid_offset)
{
    return Data_Grid_bitfields[grid_offset] & Bitfield_AlternateTerrain;
}

void map_property_set_alternate_terrain(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] |= Bitfield_AlternateTerrain;
}

int map_property_is_plaza_or_earthquake(int grid_offset)
{
    return Data_Grid_bitfields[grid_offset] & Bitfield_PlazaOrEarthquake;
}

void map_property_mark_plaza_or_earthquake(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] |= Bitfield_PlazaOrEarthquake;
}

void map_property_clear_plaza_or_earthquake(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] &= Bitfield_NoPlaza;
}

int map_property_is_constructing(int grid_offset)
{
    return Data_Grid_bitfields[grid_offset] & Bitfield_Overlay;
}

void map_property_mark_constructing(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] |= Bitfield_Overlay;
}

void map_property_clear_constructing(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] &= Bitfield_NoOverlay;
}

int map_property_is_deleted(int grid_offset)
{
    return Data_Grid_bitfields[grid_offset] & Bitfield_Deleted;
}

void map_property_mark_deleted(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] |= Bitfield_Deleted;
}

void map_property_clear_deleted(int grid_offset)
{
    Data_Grid_bitfields[grid_offset] &= Bitfield_NoDeleted;
}
