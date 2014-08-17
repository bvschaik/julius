#ifndef TERRAIN_PRIVATE_H
#define TERRAIN_PRIVATE_H

#include "Data/Grid.h"
#include "Data/Settings.h"

#define FOREACH_ALL(block) \
	{int gridOffset = Data_Settings_Map.gridStartOffset;\
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {\
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {\
			block;\
		}\
	}}

#define BOUND_REGION() \
	if (xMin < 0) xMin = 0;\
	if (yMin < 0) yMin = 0;\
	if (xMax >= Data_Settings_Map.width) xMax = Data_Settings_Map.width - 1;\
	if (yMax >= Data_Settings_Map.height) yMax = Data_Settings_Map.height - 1;

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
