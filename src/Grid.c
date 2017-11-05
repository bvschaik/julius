#include "Grid.h"

#include "data/Grid.hpp"

#include <string.h>

void Grid_clearByteGrid(char *grid)
{
	memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(signed char));
}

void Grid_clearUByteGrid(unsigned char *grid)
{
	memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(unsigned char));
}

void Grid_clearShortGrid(unsigned short *grid)
{
	memset(grid, 0, GRID_SIZE * GRID_SIZE * sizeof(unsigned short));
}

void Grid_andByteGrid(unsigned char *grid, unsigned char mask)
{
	for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
		grid[i] &= mask;
	}
}

void Grid_andShortGrid(unsigned short *grid, unsigned short mask)
{
	for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
		grid[i] &= mask;
	}
}

void Grid_copyByteGrid(const unsigned char *src, unsigned char *dst)
{
	memcpy(dst, src, GRID_SIZE * GRID_SIZE * sizeof(unsigned char));
}

void Grid_copyShortGrid(const unsigned short *src, unsigned short *dst)
{
	memcpy(dst, src, GRID_SIZE * GRID_SIZE * sizeof(unsigned short));
}
