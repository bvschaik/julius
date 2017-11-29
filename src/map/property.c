#include "property.h"

#include "map/grid.h"

#include "Data/Grid.h"

enum {
    Bitfield_Size1 = 0x00,
    Bitfield_Size2 = 0x01,
    Bitfield_Size3 = 0x02,
    Bitfield_Size4 = 0x04,
    Bitfield_Size5 = 0x08,
    Bitfield_Sizes = 0x0f,
    Bitfield_NoSizes = 0xf0,
    Bitfield_Construction = 0x10,
    Bitfield_NoConstruction = 0xef,
    Bitfield_AlternateTerrain = 0x20,
    Bitfield_Deleted = 0x40,
    Bitfield_NoDeleted = 0xbf,
    Bitfield_PlazaOrEarthquake = 0x80,
    Bitfield_NoPlaza = 0x7f,
    Bitfield_NoConstructionAndDeleted = 0xaf,
    Edge_MaskX = 0x7,
    Edge_MaskY = 0x38,
    Edge_MaskXY = 0x3f,
    Edge_LeftmostTile = 0x40,
    Edge_NoLeftmostTile = 0xbf,
    Edge_NativeLand = 0x80,
    Edge_NoNativeLand = 0x7f,
};

#define EdgeXY(x,y) (8 * (y) + (x))

static grid_u8 edge_grid;
static grid_u8 bitfields_grid;

static grid_u8 edge_backup;
static grid_u8 bitfields_backup;

int map_property_is_draw_tile(int grid_offset)
{
    return edge_grid.items[grid_offset] & Edge_LeftmostTile;
}

void map_property_mark_draw_tile(int grid_offset)
{
    edge_grid.items[grid_offset] |= Edge_LeftmostTile;
}

void map_property_clear_draw_tile(int grid_offset)
{
    edge_grid.items[grid_offset] &= ~Edge_LeftmostTile;
}

int map_property_is_native_land(int grid_offset)
{
    return edge_grid.items[grid_offset] & Edge_NativeLand;
}

void map_property_mark_native_land(int grid_offset)
{
    edge_grid.items[grid_offset] |= Edge_NativeLand;
}

void map_property_clear_all_native_land()
{
    map_grid_and_u8(edge_grid.items, Edge_NoNativeLand);
}

int map_property_multi_tile_xy(int grid_offset)
{
    return edge_grid.items[grid_offset] & Edge_MaskXY;
}

int map_property_multi_tile_x(int grid_offset)
{
    return edge_grid.items[grid_offset] & Edge_MaskX;
}

int map_property_multi_tile_y(int grid_offset)
{
    return edge_grid.items[grid_offset] & Edge_MaskY;
}

int map_property_is_multi_tile_xy(int grid_offset, int x, int y)
{
    return (edge_grid.items[grid_offset] & Edge_MaskXY) == EdgeXY(x, y);
}

void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile)
{
    if (is_draw_tile) {
        edge_grid.items[grid_offset] = EdgeXY(x, y) | Edge_LeftmostTile;
    } else {
        edge_grid.items[grid_offset] = EdgeXY(x, y);
    }
}

void map_property_clear_multi_tile_xy(int grid_offset)
{
    // only keep native land marker
    edge_grid.items[grid_offset] &= Edge_NativeLand;
}

int map_property_multi_tile_size(int grid_offset)
{
    switch (bitfields_grid.items[grid_offset] & Bitfield_Sizes) {
        case Bitfield_Size2: return 2;
        case Bitfield_Size3: return 3;
        case Bitfield_Size4: return 4;
        case Bitfield_Size5: return 5;
        default: return 1;
    }
}

void map_property_set_multi_tile_size(int grid_offset, int size)
{
    bitfields_grid.items[grid_offset] &= Bitfield_NoSizes;
    switch (size) {
        case 2: bitfields_grid.items[grid_offset] |= Bitfield_Size2; break;
        case 3: bitfields_grid.items[grid_offset] |= Bitfield_Size3; break;
        case 4: bitfields_grid.items[grid_offset] |= Bitfield_Size4; break;
        case 5: bitfields_grid.items[grid_offset] |= Bitfield_Size5; break;
    }
}

int map_property_is_alternate_terrain(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & Bitfield_AlternateTerrain;
}

void map_property_set_alternate_terrain(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= Bitfield_AlternateTerrain;
}

int map_property_is_plaza_or_earthquake(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & Bitfield_PlazaOrEarthquake;
}

void map_property_mark_plaza_or_earthquake(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= Bitfield_PlazaOrEarthquake;
}

void map_property_clear_plaza_or_earthquake(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= Bitfield_NoPlaza;
}

int map_property_is_constructing(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & Bitfield_Construction;
}

void map_property_mark_constructing(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= Bitfield_Construction;
}

void map_property_clear_constructing(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= Bitfield_NoConstruction;
}

int map_property_is_deleted(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & Bitfield_Deleted;
}

void map_property_mark_deleted(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= Bitfield_Deleted;
}

void map_property_clear_deleted(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= Bitfield_NoDeleted;
}

void map_property_clear_constructing_and_deleted()
{
    map_grid_and_u8(bitfields_grid.items, Bitfield_NoConstructionAndDeleted);
}

void map_property_clear()
{
    map_grid_clear_u8(bitfields_grid.items);
    map_grid_clear_u8(edge_grid.items);
}

void map_property_backup()
{
    map_grid_copy_u8(bitfields_grid.items, bitfields_backup.items);
    map_grid_copy_u8(edge_grid.items, edge_backup.items);
}

void map_property_restore()
{
    map_grid_copy_u8(bitfields_backup.items, bitfields_grid.items);
    map_grid_copy_u8(edge_backup.items, edge_grid.items);
}

void map_property_save_state(buffer *bitfields, buffer *edge)
{
    map_grid_save_state_u8(bitfields_grid.items, bitfields);
    map_grid_save_state_u8(edge_grid.items, edge);
}

void map_property_load_state(buffer *bitfields, buffer *edge)
{
    map_grid_load_state_u8(bitfields_grid.items, bitfields);
    map_grid_load_state_u8(edge_grid.items, edge);
}
