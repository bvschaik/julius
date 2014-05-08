#include "Grid.h"

#include "Data/Grid.h"

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
