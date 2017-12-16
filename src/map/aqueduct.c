#include "aqueduct.h"

#include "map/grid.h"

static grid_u8 aqueduct_backup;

int map_aqueduct_at(int grid_offset)
{
    return Data_Grid_aqueducts[grid_offset];
}

void map_aqueduct_remove(int grid_offset)
{
    Data_Grid_aqueducts[grid_offset] = 0;
    if (Data_Grid_aqueducts[grid_offset + map_grid_delta(0, -1)] == 5) {
        Data_Grid_aqueducts[grid_offset + map_grid_delta(0, -1)] = 1;
    }
    if (Data_Grid_aqueducts[grid_offset + map_grid_delta(1, 0)] == 6) {
        Data_Grid_aqueducts[grid_offset + map_grid_delta(1, 0)] = 2;
    }
    if (Data_Grid_aqueducts[grid_offset + map_grid_delta(0, 1)] == 5) {
        Data_Grid_aqueducts[grid_offset + map_grid_delta(0, 1)] = 3;
    }
    if (Data_Grid_aqueducts[grid_offset + map_grid_delta(-1, 0)] == 6) {
        Data_Grid_aqueducts[grid_offset + map_grid_delta(-1, 0)] = 4;
    }
}

void map_aqueduct_clear()
{
    map_grid_clear_u8(Data_Grid_aqueducts);
}

void map_aqueduct_backup()
{
    map_grid_copy_u8(Data_Grid_aqueducts, aqueduct_backup.items);
}

void map_aqueduct_restore()
{
    map_grid_copy_u8(aqueduct_backup.items, Data_Grid_aqueducts);
}

void map_aqueduct_save_state(buffer *buf, buffer *backup)
{
    map_grid_save_state_u8(Data_Grid_aqueducts, buf);
    map_grid_save_state_u8(aqueduct_backup.items, backup);
}

void map_aqueduct_load_state(buffer *buf, buffer *backup)
{
    map_grid_load_state_u8(Data_Grid_aqueducts, buf);
    map_grid_load_state_u8(aqueduct_backup.items, backup);
}
