#include "figure.h"

#include "map/grid.h"

static grid_u16 figures;

int map_has_figure_at(int grid_offset)
{
    return figures.items[grid_offset] > 0;
}

int map_figure_at(int grid_offset)
{
    return figures.items[grid_offset];
}

void map_figure_add(figure *f)
{
    if (f->gridOffset < 0) {
        return;
    }
    f->numPreviousFiguresOnSameTile = 0;

    if (figures.items[f->gridOffset]) {
        figure *next = figure_get(figures.items[f->gridOffset]);
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
        figures.items[f->gridOffset] = f->id;
    }
}

void map_figure_update(figure *f)
{
    f->numPreviousFiguresOnSameTile = 0;

    figure *next = figure_get(figures.items[f->gridOffset]);
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
    if (f->gridOffset < 0 || !figures.items[f->gridOffset]) {
        return;
    }

    if (figures.items[f->gridOffset] == f->id) {
        figures.items[f->gridOffset] = f->nextFigureIdOnSameTile;
    } else {
        figure *prev = figure_get(figures.items[f->gridOffset]);
        while (prev->id && prev->nextFigureIdOnSameTile != f->id) {
            prev = figure_get(prev->nextFigureIdOnSameTile);
        }
        prev->nextFigureIdOnSameTile = f->nextFigureIdOnSameTile;
    }
    f->nextFigureIdOnSameTile = 0;
}

int map_figure_foreach_until(int grid_offset, int (*callback)(figure *f))
{
    if (figures.items[grid_offset] > 0) {
        int figure_id = figures.items[grid_offset];
        while (figure_id) {
            figure *f = figure_get(figure_id);
            int result = callback(f);
            if (result) {
                return result;
            }
            figure_id = f->nextFigureIdOnSameTile;
        }
    }
    return 0;
}

void map_figure_clear()
{
    map_grid_clear_u16(figures.items);
}

void map_figure_save_state(buffer *buf)
{
    map_grid_save_state_u16(figures.items, buf);
}

void map_figure_load_state(buffer *buf)
{
    map_grid_load_state_u16(figures.items, buf);
}
