#include "figure.h"

#include "map/grid.h"

#include "Data/Grid.h"

int map_has_figure_at(int grid_offset)
{
    return Data_Grid_figureIds[grid_offset] > 0;
}

int map_figure_at(int grid_offset)
{
    return Data_Grid_figureIds[grid_offset];
}

void map_figure_clear()
{
    map_grid_clear_u16(Data_Grid_figureIds);
}

void map_figure_save_state(buffer *buf)
{
    map_grid_save_state_u16(Data_Grid_figureIds, buf);
}

void map_figure_load_state(buffer *buf)
{
    map_grid_load_state_u16(Data_Grid_figureIds, buf);
}
