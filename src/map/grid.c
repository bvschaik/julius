#include "grid.h"

#include "map/data.h"

#include <string.h>

#define OFFSET(x,y) (x + GRID_SIZE * y)

struct map_data_t map_data;

static const int DIRECTION_DELTA[] = {
    -OFFSET(0,1), OFFSET(1,-1), 1, OFFSET(1,1), OFFSET(0,1), OFFSET(-1,1), -1, -OFFSET(1,1)
};

static const int ADJACENT_OFFSETS[][29] = {
    {0},
    {OFFSET(0,-1), OFFSET(1,0), OFFSET(0,1), OFFSET(-1,0), 0},
    {OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,0), OFFSET(2,1), OFFSET(1,2), OFFSET(0,2), OFFSET(-1,1), OFFSET(-1,0), 0},
    {
        OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,-1),
        OFFSET(3,0), OFFSET(3,1), OFFSET(3,2),
        OFFSET(2,3), OFFSET(1,3), OFFSET(0,3),
        OFFSET(-1,2), OFFSET(-1,1), OFFSET(-1,0), 0
    },
    {
        OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,-1), OFFSET(3,-1),
        OFFSET(4,0), OFFSET(4,1), OFFSET(4,2), OFFSET(4,3),
        OFFSET(3,4), OFFSET(2,4), OFFSET(1,4), OFFSET(0,4),
        OFFSET(-1,3), OFFSET(-1,2), OFFSET(-1,1), OFFSET(-1,0), 0
    },
    {
        OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,-1), OFFSET(3,-1), OFFSET(4,-1),
        OFFSET(5,0), OFFSET(5,1), OFFSET(5,2), OFFSET(5,3), OFFSET(5,4),
        OFFSET(4,5), OFFSET(3,5), OFFSET(2,5), OFFSET(1,5), OFFSET(0,5),
        OFFSET(-1,4), OFFSET(-1,3), OFFSET(-1,2), OFFSET(-1,1), OFFSET(-1,0), 0
    },
    { 0
    },
    {   OFFSET(0,-1), OFFSET(1,-1), OFFSET(2,-1), OFFSET(3,-1), OFFSET(4,-1), OFFSET(5,-1), OFFSET(6,-1),
        OFFSET(7,0), OFFSET(7,1), OFFSET(7,2), OFFSET(7,3), OFFSET(7,4), OFFSET(7,5), OFFSET(7,6),
        OFFSET(6,7), OFFSET(5,7), OFFSET(4,7), OFFSET(3,7), OFFSET(2,7), OFFSET(1,7), OFFSET(0,7),
        OFFSET(-1,6), OFFSET(-1,5), OFFSET(-1,4), OFFSET(-1,3), OFFSET(-1,2), OFFSET(-1,1), OFFSET(-1,0), 0
    }
};

void map_grid_init(int width, int height, int start_offset, int border_size)
{
    map_data.width = width;
    map_data.height = height;
    map_data.start_offset = start_offset;
    map_data.border_size = border_size;
}

int map_grid_is_valid_offset(int grid_offset)
{
    return grid_offset >= 0 && grid_offset < GRID_SIZE * GRID_SIZE;
}

int map_grid_offset(int x, int y)
{
    return map_data.start_offset + x + y * GRID_SIZE;
}

int map_grid_offset_to_x(int grid_offset)
{
    return (grid_offset - map_data.start_offset) % GRID_SIZE;
}

int map_grid_offset_to_y(int grid_offset)
{
    return (grid_offset - map_data.start_offset) / GRID_SIZE;
}

int map_grid_delta(int x, int y)
{
    return y * GRID_SIZE + x;
}

int map_grid_add_delta(int grid_offset, int x, int y)
{
    int raw_x = grid_offset % GRID_SIZE;
    int raw_y = grid_offset / GRID_SIZE;
    if (raw_x + x < 0 || raw_x + x >= GRID_SIZE ||
        raw_y + y < 0 || raw_y + y >= GRID_SIZE) {
        return -1;
    }
    return grid_offset + map_grid_delta(x, y);
}

int map_grid_direction_delta(int direction)
{
    if (direction >= 0 && direction < 8) {
        return DIRECTION_DELTA[direction];
    } else {
        return 0;
    }
}

void map_grid_size(int *width, int *height)
{
    *width = map_data.width;
    *height = map_data.height;
}

int map_grid_width(void)
{
    return map_data.width;
}

int map_grid_height(void)
{
    return map_data.height;
}

void map_grid_bound(int *x, int *y)
{
    if (*x < 0) {
        *x = 0;
    }
    if (*y < 0) {
        *y = 0;
    }
    if (*x >= map_data.width) {
        *x = map_data.width - 1;
    }
    if (*y >= map_data.height) {
        *y = map_data.height - 1;
    }
}

void map_grid_bound_area(int *x_min, int *y_min, int *x_max, int *y_max)
{
    if (*x_min < 0) {
        *x_min = 0;
    }
    if (*y_min < 0) {
        *y_min = 0;
    }
    if (*x_max >= map_data.width) {
        *x_max = map_data.width - 1;
    }
    if (*y_max >= map_data.height) {
        *y_max = map_data.height - 1;
    }
}

void map_grid_get_area(int x, int y, int size, int radius,
                       int *x_min, int *y_min, int *x_max, int *y_max)
{
    *x_min = x - radius;
    *y_min = y - radius;
    *x_max = x + size + radius - 1;
    *y_max = y + size + radius - 1;
    map_grid_bound_area(x_min, y_min, x_max, y_max);
}

void map_grid_start_end_to_area(
    int x_start, int y_start, int x_end, int y_end, int *x_min, int *y_min, int *x_max, int *y_max)
{
    if (x_start < x_end) {
        *x_min = x_start;
        *x_max = x_end;
    } else {
        *x_min = x_end;
        *x_max = x_start;
    }
    if (y_start < y_end) {
        *y_min = y_start;
        *y_max = y_end;
    } else {
        *y_min = y_end;
        *y_max = y_start;
    }
    map_grid_bound_area(x_min, y_min, x_max, y_max);
}

int map_grid_is_inside(int x, int y, int size)
{
    return x >= 0 && x + size <= map_data.width && y >= 0 && y + size <= map_data.height;
}

const int *map_grid_adjacent_offsets(int size)
{
    return ADJACENT_OFFSETS[size];
}

void map_grid_get_corner_tiles(int start_x, int start_y, int x, int y, int *c1x, int *c1y, int *c2x, int *c2y)
{
    if (x - start_x != 0) {
        *c1x = x;
        *c1y = y - 1;
        *c2x = x;
        *c2y = y + 1;
        return;
    } else {
        *c1x = x - 1;
        *c1y = y;
        *c2x = x + 1;
        *c2y = y;
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

void map_grid_clear_u32(uint32_t *grid)
{
    memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(uint32_t));
}

void map_grid_clear_i16(int16_t *grid)
{
    memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(int16_t));
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

void map_grid_and_u32(uint32_t *grid, uint32_t mask)
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

void map_grid_copy_u32(const uint32_t *src, uint32_t *dst)
{
    memcpy(dst, src, GRID_SIZE * GRID_SIZE * sizeof(uint32_t));
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
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        buffer_write_u16(buf, grid[i]);
    }
}

void map_grid_save_state_u32_to_u16(const uint32_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        buffer_write_u16(buf, (uint16_t) grid[i]);
    }
}

void map_grid_save_state_u32(const uint32_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        buffer_write_u32(buf, grid[i]);
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
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] = buffer_read_u16(buf);
    }
}

void map_grid_load_state_u16_to_u32(uint32_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] = buffer_read_u16(buf);
    }
}

void map_grid_load_state_u32(uint32_t *grid, buffer *buf)
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        grid[i] = buffer_read_u32(buf);
    }
}
