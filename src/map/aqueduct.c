#include "aqueduct.h"

#include "map/grid.h"

#define WATER_ACCESS_OFFSET 7
#define IMAGE_MASK 0x7f

static grid_u8 aqueduct;
static grid_u8 aqueduct_backup;

int map_aqueduct_has_water_access_at(int grid_offset)
{
    return aqueduct.items[grid_offset] >> WATER_ACCESS_OFFSET;
}

int map_aqueduct_image_at(int grid_offset)
{
    return aqueduct.items[grid_offset] & IMAGE_MASK;
}

void map_aqueduct_set_water_access(int grid_offset, int value)
{
    aqueduct.items[grid_offset] = (value << WATER_ACCESS_OFFSET) | (aqueduct.items[grid_offset] & IMAGE_MASK);
}

void map_aqueduct_set_image(int grid_offset, int value)
{
    aqueduct.items[grid_offset] = (aqueduct.items[grid_offset] & ~IMAGE_MASK) | value;
}

void map_aqueduct_remove(int grid_offset)
{
    aqueduct.items[grid_offset] = 0;
    if (map_aqueduct_image_at(grid_offset + map_grid_delta(0, -1)) == 5) {
        map_aqueduct_set_image(grid_offset + map_grid_delta(0, -1), 1);
    }
    if (map_aqueduct_image_at(grid_offset + map_grid_delta(1, 0)) == 6) {
        map_aqueduct_set_image(grid_offset + map_grid_delta(1, 0), 2);
    }
    if (map_aqueduct_image_at(grid_offset + map_grid_delta(0, 1)) == 5) {
        map_aqueduct_set_image(grid_offset + map_grid_delta(0, 1), 3);
    }
    if (map_aqueduct_image_at(grid_offset + map_grid_delta(-1, 0)) == 6) {
        map_aqueduct_set_image(grid_offset + map_grid_delta(-1, 0), 4);
    }
}

void map_aqueduct_clear(void)
{
    map_grid_clear_u8(aqueduct.items);
}

void map_aqueduct_backup(void)
{
    map_grid_copy_u8(aqueduct.items, aqueduct_backup.items);
}

void map_aqueduct_restore(void)
{
    map_grid_copy_u8(aqueduct_backup.items, aqueduct.items);
}

void map_aqueduct_save_state(buffer *buf, buffer *backup)
{
    map_grid_save_state_u8(aqueduct.items, buf);
    map_grid_save_state_u8(aqueduct_backup.items, backup);
}

void map_aqueduct_load_state(buffer *buf, buffer *backup)
{
    map_grid_load_state_u8(aqueduct.items, buf);
    map_grid_load_state_u8(aqueduct_backup.items, backup);
}
