#ifndef CITYVIEW_H
#define CITYVIEW_H

void CityView_setViewport(
	int xOffset, int yOffset, int widthInTiles, int heightInTiles);

void CityView_calculateLookup();

void CityView_gridOffsetToXYCoords(int gridOffset, int *xInTiles, int *yInTiles);

void CityView_rotateLeft();

void CityView_rotateRight();

#endif
