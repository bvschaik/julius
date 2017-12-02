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

void map_figure_add(figure *f)
{
    if (f->gridOffset < 0) {
        return;
    }
    f->numPreviousFiguresOnSameTile = 0;

    if (Data_Grid_figureIds[f->gridOffset]) {
        figure *next = figure_get(Data_Grid_figureIds[f->gridOffset]);
        f->numPreviousFiguresOnSameTile++;
        while (next->nextFigureIdOnSameTile) {
            next = figure_get(next->nextFigureIdOnSameTile);
            f->numPreviousFiguresOnSameTile++;
        }
        if (f->numPreviousFiguresOnSameTile > 20) {
            f->numPreviousFiguresOnSameTile = 20;
        }
        next->nextFigureIdOnSameTile = f->id;
    } else {
        Data_Grid_figureIds[f->gridOffset] = f->id;
    }
}

void map_figure_update(figure *f)
{
    f->numPreviousFiguresOnSameTile = 0;

    figure *next = figure_get(Data_Grid_figureIds[f->gridOffset]);
    while (next->id) {
        if (next->id == f->id) {
            return;
        }
        f->numPreviousFiguresOnSameTile++;
        next = figure_get(next->nextFigureIdOnSameTile);
    }
    if (f->numPreviousFiguresOnSameTile > 20) {
        f->numPreviousFiguresOnSameTile = 20;
    }
}

void map_figure_delete(figure *f)
{
    if (f->gridOffset < 0 || !Data_Grid_figureIds[f->gridOffset]) {
        return;
    }

    if (Data_Grid_figureIds[f->gridOffset] == f->id) {
        Data_Grid_figureIds[f->gridOffset] = f->nextFigureIdOnSameTile;
    } else {
        figure *prev = figure_get(Data_Grid_figureIds[f->gridOffset]);
        while (prev->id && prev->nextFigureIdOnSameTile != f->id) {
            prev = figure_get(prev->nextFigureIdOnSameTile);
        }
        prev->nextFigureIdOnSameTile = f->nextFigureIdOnSameTile;
    }
    f->nextFigureIdOnSameTile = 0;
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
