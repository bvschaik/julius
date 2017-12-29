#include "terrain.h"

#include "map/grid.h"
#include "map/ring.h"

static grid_u16 terrain_grid;
static grid_u16 terrain_grid_backup;

int map_terrain_is(int grid_offset, int terrain)
{
    return terrain_grid.items[grid_offset] & terrain;
}

int map_terrain_get(int grid_offset)
{
    return terrain_grid.items[grid_offset];
}

void map_terrain_set(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] = terrain;
}

void map_terrain_add(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] |= terrain;
}

void map_terrain_remove(int grid_offset, int terrain)
{
    terrain_grid.items[grid_offset] &= ~terrain;
}

void map_terrain_add_with_radius(int x, int y, int size, int radius, int terrain)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            map_terrain_add(map_grid_offset(xx, yy), terrain);
        }
    }
}

void map_terrain_remove_with_radius(int x, int y, int size, int radius, int terrain)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            map_terrain_remove(map_grid_offset(xx, yy), terrain);
        }
    }
}

void map_terrain_remove_all(int terrain)
{
    map_grid_and_u16(terrain_grid.items, ~terrain);
}

int map_terrain_count_directly_adjacent_with_type(int grid_offset, int terrain)
{
    int count = 0;
    if (map_terrain_is(grid_offset + map_grid_delta(0, -1), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, 0), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(0, 1), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), terrain)) {
        count++;
    }
    return count;
}

int map_terrain_count_diagonally_adjacent_with_type(int grid_offset, int terrain)
{
    int count = 0;
    if (map_terrain_is(grid_offset + map_grid_delta(1, -1), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, 1), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), terrain)) {
        count++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(-1, -1), terrain)) {
        count++;
    }
    return count;
}

int map_terrain_has_adjacent_x_with_type(int grid_offset, int terrain)
{
    if (map_terrain_is(grid_offset + map_grid_delta(0, -1), terrain) ||
        map_terrain_is(grid_offset + map_grid_delta(0, 1), terrain)) {
        return 1;
    }
    return 0;
}

int map_terrain_has_adjacent_y_with_type(int grid_offset, int terrain)
{
    if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), terrain) ||
        map_terrain_is(grid_offset + map_grid_delta(1, 0), terrain)) {
        return 1;
    }
    return 0;
}

int map_terrain_exists_tile_in_area_with_type(int x, int y, int size, int terrain)
{
    for (int yy = y; yy < y + size; yy++) {
        for (int xx = x; xx < x + size; xx++) {
            if (map_grid_is_inside(xx, yy, 1) && terrain_grid.items[map_grid_offset(xx, yy)] & terrain) {
                return 1;
            }
        }
    }
    return 0;
}

int map_terrain_exists_tile_in_radius_with_type(int x, int y, int size, int radius, int terrain)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            if (map_terrain_is(map_grid_offset(xx, yy), terrain)) {
                return 1;
            }
        }
    }
    return 0;
}

int map_terrain_exists_clear_tile_in_radius(int x, int y, int size, int radius, int except_grid_offset,
                                            int *x_tile, int *y_tile)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            int grid_offset = map_grid_offset(xx, yy);
            if (grid_offset != except_grid_offset && !terrain_grid.items[grid_offset]) {
                *x_tile = xx;
                *y_tile = yy;
                return 1;
            }
        }
    }
    *x_tile = x_max;
    *y_tile = y_max;
    return 0;
}

int map_terrain_all_tiles_in_radius_are(int x, int y, int size, int radius, int terrain)
{
    int x_min, y_min, x_max, y_max;
    map_grid_get_area(x, y, size, radius, &x_min, &y_min, &x_max, &y_max);

    for (int yy = y_min; yy <= y_max; yy++) {
        for (int xx = x_min; xx <= x_max; xx++) {
            if (!map_terrain_is(map_grid_offset(xx, yy), terrain)) {
                return 0;
            }
        }
    }
    return 1;
}

int map_terrain_has_only_meadow_in_ring(int x, int y, int distance)
{
    int start = map_ring_start(1, distance);
    int end = map_ring_end(1, distance);
    int baseOffset = map_grid_offset(x, y);
    for (int i = start; i < end; i++) {
        const ring_tile *tile = map_ring_tile(i);
        if (map_ring_is_inside_map(x + tile->x, y + tile->y)) {
            if (!map_terrain_is(baseOffset + tile->grid_offset, TERRAIN_MEADOW)) {
                return 0;
            }
        }
    }
    return 1;
}


void map_terrain_backup()
{
    map_grid_copy_u16(terrain_grid.items, terrain_grid_backup.items);
}

void map_terrain_restore()
{
    map_grid_copy_u16(terrain_grid_backup.items, terrain_grid.items);
}

void map_terrain_clear()
{
    map_grid_clear_u16(terrain_grid.items);
}

void map_terrain_save_state(buffer *buf)
{
    map_grid_save_state_u16(terrain_grid.items, buf);
}

void map_terrain_load_state(buffer *buf)
{
    map_grid_load_state_u16(terrain_grid.items, buf);
}
