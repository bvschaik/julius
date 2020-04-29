#include "figure.h"

#include "map/grid.h"

static grid_u16 figures;

int map_has_figure_at(int grid_offset)
{
    return map_grid_is_valid_offset(grid_offset) && figures.items[grid_offset] > 0;
}

int map_figure_at(int grid_offset)
{
    return map_grid_is_valid_offset(grid_offset) ? figures.items[grid_offset] : 0;
}

void map_figure_add(figure *f)
{
    if (!map_grid_is_valid_offset(f->grid_offset)) {
        return;
    }
    f->figures_on_same_tile_index = 0;
    f->next_figure_id_on_same_tile = 0;

    if (figures.items[f->grid_offset]) {
        figure *next = figure_get(figures.items[f->grid_offset]);
        f->figures_on_same_tile_index++;
        while (next->next_figure_id_on_same_tile) {
            next = figure_get(next->next_figure_id_on_same_tile);
            f->figures_on_same_tile_index++;
        }
        if (f->figures_on_same_tile_index > 20) {
            f->figures_on_same_tile_index = 20;
        }
        next->next_figure_id_on_same_tile = f->id;
    } else {
        figures.items[f->grid_offset] = f->id;
    }
}

void map_figure_update(figure *f)
{
    if (!map_grid_is_valid_offset(f->grid_offset)) {
        return;
    }
    f->figures_on_same_tile_index = 0;

    figure *next = figure_get(figures.items[f->grid_offset]);
    while (next->id) {
        if (next->id == f->id) {
            return;
        }
        f->figures_on_same_tile_index++;
        next = figure_get(next->next_figure_id_on_same_tile);
    }
    if (f->figures_on_same_tile_index > 20) {
        f->figures_on_same_tile_index = 20;
    }
}

void map_figure_delete(figure *f)
{
    if (!map_grid_is_valid_offset(f->grid_offset) || !figures.items[f->grid_offset]) {
        f->next_figure_id_on_same_tile = 0;
        return;
    }

    if (figures.items[f->grid_offset] == f->id) {
        figures.items[f->grid_offset] = f->next_figure_id_on_same_tile;
    } else {
        figure *prev = figure_get(figures.items[f->grid_offset]);
        while (prev->id && prev->next_figure_id_on_same_tile != f->id) {
            prev = figure_get(prev->next_figure_id_on_same_tile);
        }
        prev->next_figure_id_on_same_tile = f->next_figure_id_on_same_tile;
    }
    f->next_figure_id_on_same_tile = 0;
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
            figure_id = f->next_figure_id_on_same_tile;
        }
    }
    return 0;
}

void map_figure_clear(void)
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
