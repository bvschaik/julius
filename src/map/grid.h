#ifndef MAP_GRID_H
#define MAP_GRID_H

#include "core/buffer.h"

#include <stdint.h>

int map_grid_offset(int x, int y);

int map_grid_delta(int x, int y);

int map_grid_direction_delta(int direction);

void map_grid_clear_u8(uint8_t *grid);

void map_grid_clear_i8(int8_t *grid);

void map_grid_clear_u16(uint16_t *grid);

void map_grid_init_i8(int8_t *grid, int8_t value);

void map_grid_and_u8(uint8_t *grid, uint8_t mask);

void map_grid_and_u16(uint16_t *grid, uint16_t mask);

void map_grid_copy_u8(const uint8_t *src, uint8_t *dst);

void map_grid_copy_u16(const uint16_t *src, uint16_t *dst);


void map_grid_save_state_u8(const uint8_t *grid, buffer *buf);

void map_grid_save_state_i8(const int8_t *grid, buffer *buf);

void map_grid_save_state_u16(const uint16_t *grid, buffer *buf);

void map_grid_load_state_u8(uint8_t *grid, buffer *buf);

void map_grid_load_state_i8(int8_t *grid, buffer *buf);

void map_grid_load_state_u16(uint16_t *grid, buffer *buf);

#endif // MAP_GRID_H
