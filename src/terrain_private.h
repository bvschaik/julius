#ifndef TERRAIN_PRIVATE_H
#define TERRAIN_PRIVATE_H

#include <data>

#define DELTA(x, y) ((y) * GRID_SIZE + (x))

#define FOREACH_ALL(block) \
	{int gridOffset = Data_State.map.gridStartOffset;\
	for (int y = 0; y < Data_State.map.height; y++, gridOffset += Data_State.map.gridBorderSize) {\
		for (int x = 0; x < Data_State.map.width; x++, gridOffset++) {\
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
