#ifndef MAP_GRID_H
#define MAP_GRID_H

#include <stdint.h>

int map_grid_offset(int x, int y);

void map_grid_clear_u8(uint8_t *grid);

void map_grid_clear_i8(int8_t *grid);

void map_grid_clear_u16(uint16_t *grid);

void map_grid_init_i8(int8_t *grid, int8_t value);

void map_grid_and_u8(uint8_t *grid, uint8_t mask);

void map_grid_and_u16(uint16_t *grid, uint16_t mask);

void map_grid_copy_u8(const uint8_t *src, uint8_t *dst);

int map_grid_delta(int x, int y);

void map_grid_copy_u16(const uint16_t *src, uint16_t *dst);

int map_grid_direction_delta(int direction);

#endif // MAP_GRID_H
