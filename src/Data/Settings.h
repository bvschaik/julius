#ifndef DATA_SETTINGS_H
#define DATA_SETTINGS_H

#include <stdint.h>

#define GridOffset(x,y) (Data_Settings_Map.gridStartOffset + (x) + (y) * 162)
#define GridOffsetToX(g) (((g) - Data_Settings_Map.gridStartOffset) % 162)
#define GridOffsetToY(g) (((g) - Data_Settings_Map.gridStartOffset) / 162)
#define IsInsideMap(x,y) ((x) >= 0 && (x) < Data_Settings_Map.width && (y) >= 0 && (y) < Data_Settings_Map.height)
#define IsOutsideMap(x,y,s) (x) < 0 || (x) + (s) > Data_Settings_Map.width || (y) < 0 || (y) + (s) > Data_Settings_Map.height
#define BoundToMap(x,y) \
	if ((x) < 0) (x) = 0;\
	if ((y) < 0) (y) = 0;\
	if ((x) >= Data_Settings_Map.width) (x) = Data_Settings_Map.width - 1;\
	if ((y) >= Data_Settings_Map.height) (y) = Data_Settings_Map.height - 1;
#define Bound2ToMap(xMin, yMin, xMax, yMax) \
	if ((xMin) < 0) (xMin) = 0;\
	if ((yMin) < 0) (yMin) = 0;\
	if ((xMax) >= Data_Settings_Map.width) (xMax) = Data_Settings_Map.width - 1;\
	if ((yMax) >= Data_Settings_Map.height) (yMax) = Data_Settings_Map.height - 1;

extern struct _Data_Settings_Map {
	int mapsId;
	struct {
		int x;
		int y;
	} camera;
	struct {
		int gridOffset;
		int x;
		int y;
	} current;
	int width;
	int height;
	int gridStartOffset;
	int gridBorderSize;
	int orientation;
	int __unknown2;
} Data_Settings_Map;

#endif
