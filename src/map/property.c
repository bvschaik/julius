#include "property.h"

#include "map/grid.h"
#include "map/random.h"

enum {
    BIT_SIZE1 = 0x00,
    BIT_SIZE2 = 0x01,
    BIT_SIZE3 = 0x02,
    BIT_SIZE4 = 0x04,
    BIT_SIZE5 = 0x08,
    BIT_SIZE7 = 0x0f,
    BIT_SIZES = 0x0f,
    BIT_NO_SIZES = 0xf0,
    BIT_CONSTRUCTION = 0x10,
    BIT_NO_CONSTRUCTION = 0xef,
    BIT_ALTERNATE_TERRAIN = 0x20,
    BIT_DELETED = 0x40,
    BIT_NO_DELETED = 0xbf,
    BIT_PLAZA_OR_EARTHQUAKE = 0x80,
    BIT_NO_PLAZA = 0x7f,
    BIT_NO_CONSTRUCTION_AND_DELETED = 0xaf,
    EDGE_MASK_X = 0x7,
    EDGE_MASK_Y = 0x38,
    EDGE_MASK_XY = 0x3f,
    EDGE_LEFTMOST_TILE = 0x40,
    EDGE_NO_LEFTMOST_TILE = 0xbf,
    EDGE_NATIVE_LAND = 0x80,
    EDGE_NO_NATIVE_LAND = 0x7f,
};

static grid_u8 edge_grid;
static grid_u8 bitfields_grid;

static grid_u8 edge_backup;
static grid_u8 bitfields_backup;

static int edge_for(int x, int y)
{
    return 8 * y + x;
}

int map_property_is_draw_tile(int grid_offset)
{
    return edge_grid.items[grid_offset] & EDGE_LEFTMOST_TILE;
}

int map_property_is_draw_tile_from_buffer(buffer *edge, int grid_offset)
{
    buffer_set(edge, grid_offset);
    return buffer_read_u8(edge) & EDGE_LEFTMOST_TILE;
}

void map_property_mark_draw_tile(int grid_offset)
{
    edge_grid.items[grid_offset] |= EDGE_LEFTMOST_TILE;
}

void map_property_clear_draw_tile(int grid_offset)
{
    edge_grid.items[grid_offset] &= ~EDGE_LEFTMOST_TILE;
}

int map_property_is_native_land(int grid_offset)
{
    return edge_grid.items[grid_offset] & EDGE_NATIVE_LAND;
}

void map_property_mark_native_land(int grid_offset)
{
    edge_grid.items[grid_offset] |= EDGE_NATIVE_LAND;
}

void map_property_clear_all_native_land(void)
{
    map_grid_and_u8(edge_grid.items, EDGE_NO_NATIVE_LAND);
}

int map_property_multi_tile_xy(int grid_offset)
{
    return edge_grid.items[grid_offset] & EDGE_MASK_XY;
}

int map_property_multi_tile_x(int grid_offset)
{
    return edge_grid.items[grid_offset] & EDGE_MASK_X;
}

int map_property_multi_tile_y(int grid_offset)
{
    return edge_grid.items[grid_offset] & EDGE_MASK_Y;
}

int map_property_is_multi_tile_xy(int grid_offset, int x, int y)
{
    return (edge_grid.items[grid_offset] & EDGE_MASK_XY) == edge_for(x, y);
}

void map_property_set_multi_tile_xy(int grid_offset, int x, int y, int is_draw_tile)
{
    if (is_draw_tile) {
        edge_grid.items[grid_offset] = edge_for(x, y) | EDGE_LEFTMOST_TILE;
    } else {
        edge_grid.items[grid_offset] = edge_for(x, y);
    }
}

void map_property_clear_multi_tile_xy(int grid_offset)
{
    // only keep native land marker
    edge_grid.items[grid_offset] &= EDGE_NATIVE_LAND;
}

int map_property_multi_tile_size(int grid_offset)
{
    switch (bitfields_grid.items[grid_offset] & BIT_SIZES) {
        case BIT_SIZE2: return 2;
        case BIT_SIZE3: return 3;
        case BIT_SIZE4: return 4;
        case BIT_SIZE5: return 5;
        case BIT_SIZE7: return 7;
        default: return 1;
    }
}

int map_property_multi_tile_size_from_buffer(buffer *bitfields, int grid_offset)
{
    buffer_set(bitfields, grid_offset);
    switch (buffer_read_u8(bitfields) & BIT_SIZES) {
        case BIT_SIZE2: return 2;
        case BIT_SIZE3: return 3;
        case BIT_SIZE4: return 4;
        case BIT_SIZE5: return 5;
        case BIT_SIZE7: return 7;
        default: return 1;
    }
}

void map_property_set_multi_tile_size(int grid_offset, int size)
{
    bitfields_grid.items[grid_offset] &= BIT_NO_SIZES;
    switch (size) {
        case 2: bitfields_grid.items[grid_offset] |= BIT_SIZE2; break;
        case 3: bitfields_grid.items[grid_offset] |= BIT_SIZE3; break;
        case 4: bitfields_grid.items[grid_offset] |= BIT_SIZE4; break;
        case 5: bitfields_grid.items[grid_offset] |= BIT_SIZE5; break;
        case 7: bitfields_grid.items[grid_offset] |= BIT_SIZE7; break;

    }
}

void map_property_init_alternate_terrain(void)
{
    int map_width, map_height;
    map_grid_size(&map_width, &map_height);
    for (int y = 0; y < map_height; y++) {
        for (int x = 0; x < map_width; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (map_random_get(grid_offset) & 1) {
                bitfields_grid.items[grid_offset] |= BIT_ALTERNATE_TERRAIN;
            }
        }
    }
}

int map_property_is_alternate_terrain(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & BIT_ALTERNATE_TERRAIN;
}

int map_property_is_plaza_or_earthquake(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & BIT_PLAZA_OR_EARTHQUAKE;
}

void map_property_mark_plaza_or_earthquake(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= BIT_PLAZA_OR_EARTHQUAKE;
}

void map_property_clear_plaza_or_earthquake(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= BIT_NO_PLAZA;
}

int map_property_is_constructing(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & BIT_CONSTRUCTION;
}

void map_property_mark_constructing(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= BIT_CONSTRUCTION;
}

void map_property_clear_constructing(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= BIT_NO_CONSTRUCTION;
}

int map_property_is_deleted(int grid_offset)
{
    return bitfields_grid.items[grid_offset] & BIT_DELETED;
}

void map_property_mark_deleted(int grid_offset)
{
    bitfields_grid.items[grid_offset] |= BIT_DELETED;
}

void map_property_clear_deleted(int grid_offset)
{
    bitfields_grid.items[grid_offset] &= BIT_NO_DELETED;
}

void map_property_clear_constructing_and_deleted(void)
{
    map_grid_and_u8(bitfields_grid.items, BIT_NO_CONSTRUCTION_AND_DELETED);
}

void map_property_clear(void)
{
    map_grid_clear_u8(bitfields_grid.items);
    map_grid_clear_u8(edge_grid.items);
}

void map_property_backup(void)
{
    map_grid_copy_u8(bitfields_grid.items, bitfields_backup.items);
    map_grid_copy_u8(edge_grid.items, edge_backup.items);
}

void map_property_restore(void)
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
