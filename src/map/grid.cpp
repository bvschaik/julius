#include "grid.h"

#include <string.h>

#include <data>

static const int DIRECTION_DELTA[] = {-162, -161, 1, 163, 162, 161, -1, -163};

int map_grid_offset(int x, int y)
{
    return GridOffset(x, y);
}

int map_grid_delta(int x, int y)
{
    return y * GRID_SIZE + x;
}

int map_grid_direction_delta(int direction)
{
    if (direction >= 0 && direction < 8)
    {
        return DIRECTION_DELTA[direction];
    }
    else
    {
        return 0;
    }
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
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        grid[i] &= mask;
    }
}

void map_grid_and_u16(uint16_t *grid, uint16_t mask)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
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

void map_grid_save_state_u8(const uint8_t *grid, buffer *buf)
{
    buffer_write_raw(buf, grid, GRID_SIZE * GRID_SIZE);
}

void map_grid_save_state_i8(const int8_t *grid, buffer *buf)
{
    buffer_write_raw(buf, grid, GRID_SIZE * GRID_SIZE);
}

void map_grid_save_state_u16(const uint16_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        buffer_write_u16(buf, grid[i]);
    }
}

void map_grid_load_state_u8(uint8_t *grid, buffer *buf)
{
    buffer_read_raw(buf, grid, GRID_SIZE * GRID_SIZE);
}

void map_grid_load_state_i8(int8_t *grid, buffer *buf)
{
    buffer_read_raw(buf, grid, GRID_SIZE * GRID_SIZE);
}

void map_grid_load_state_u16(uint16_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        grid[i] = buffer_read_u16(buf);
    }
}
