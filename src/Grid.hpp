#ifndef GRID_H
#define GRID_H

void Grid_clearByteGrid(char *grid);
void Grid_clearUByteGrid(unsigned char *grid);
void Grid_clearShortGrid(unsigned short *grid);

void Grid_andByteGrid(unsigned char *grid, unsigned char mask);
void Grid_andShortGrid(unsigned short *grid, unsigned short mask);

void Grid_copyByteGrid(const unsigned char *src, unsigned char *dst);
void Grid_copyShortGrid(const unsigned short *src, unsigned short *dst);

#endif
