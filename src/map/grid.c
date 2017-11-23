#include "grid.h"

#include <string.h>

#include "Data/Grid.h"
#include "Data/State.h"

int map_grid_offset(int x, int y)
{
    return GridOffset(x, y);
}

int map_grid_delta(int x, int y)
{
    return y * GRID_SIZE + x;
}

void map_grid_clear_i8(int8_t *grid)
{
    memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(int8_t));
}

void map_grid_clear_u8(uint8_t *grid)
{
    memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(uint8_t));
}

void map_grid_clear_u16(uint16_t *grid)
{
    memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(uint16_t));
}

void map_grid_init_i8(int8_t *grid, int8_t value)
{
    memset(grid, value, GRID_SIZE * GRID_SIZE * sizeof(int8_t));
}

void map_grid_and_u8(uint8_t *grid, uint8_t mask)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] &= mask;
    }
}

void map_grid_and_u16(uint16_t *grid, uint16_t mask)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] &= mask;
    }
}

void map_grid_copy_u8(const uint8_t *src, uint8_t *dst)
{
    memcpy(dst, src, GRID_SIZE * GRID_SIZE * sizeof(uint8_t));
}

void map_grid_copy_u16(const uint16_t *src, uint16_t *dst)
{
    memcpy(dst, src, GRID_SIZE * GRID_SIZE * sizeof(uint16_t));
}
