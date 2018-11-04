#include "aqueduct.h"

#include "map/grid.h"

/**
 * The aqueduct grid is used in two ways:
 * 1) to mark water/no water (0/1, see map/water_supply.c)
 * 2) to store image IDs for the aqueduct (0-15)
 * This leads to some strange results
 */
static grid_u8 aqueduct;
static grid_u8 aqueduct_backup;

int map_aqueduct_at(int grid_offset)
{
    return aqueduct.items[grid_offset];
}

void map_aqueduct_set(int grid_offset, int value)
{
    aqueduct.items[grid_offset] = value;
}

void map_aqueduct_remove(int grid_offset)
{
    aqueduct.items[grid_offset] = 0;
    if (aqueduct.items[grid_offset + map_grid_delta(0, -1)] == 5) {
        aqueduct.items[grid_offset + map_grid_delta(0, -1)] = 1;
    }
    if (aqueduct.items[grid_offset + map_grid_delta(1, 0)] == 6) {
        aqueduct.items[grid_offset + map_grid_delta(1, 0)] = 2;
    }
    if (aqueduct.items[grid_offset + map_grid_delta(0, 1)] == 5) {
        aqueduct.items[grid_offset + map_grid_delta(0, 1)] = 3;
    }
    if (aqueduct.items[grid_offset + map_grid_delta(-1, 0)] == 6) {
        aqueduct.items[grid_offset + map_grid_delta(-1, 0)] = 4;
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
