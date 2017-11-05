#ifndef TERRAIN_PRIVATE_H
#define TERRAIN_PRIVATE_H

#include "data/grid.hpp"
#include "data/settings.hpp"

#define DELTA(x, y) ((y) * GRID_SIZE + (x))

#define FOREACH_ALL(block) \
	{int gridOffset = Data_Settings_Map.gridStartOffset;\
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {\
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {\
			block;\
		}\
	}}

#define BOUND_REGION() Bound2ToMap(xMin, yMin, xMax, yMax);

#define FOREACH_REGION(block) \
	{int gridOffset = GridOffset(xMin, yMin);\
	for (int yy = yMin; yy <= yMax; yy++) {\
		for (int xx = xMin; xx <= xMax; xx++) {\
			block;\
			++gridOffset;\
		}\
		gridOffset += 162 - (xMax - xMin + 1);\
	}}

#endif
