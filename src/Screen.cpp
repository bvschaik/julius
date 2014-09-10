#include "Screen.h"

#include "CityView.h"
#include "Graphics.h"
#include "UI/Warning.h"
#include "Data/Screen.h"

#include <stdlib.h>

void Screen_setResolution(int width, int height)
{
	if (Data_Screen.drawBuffer) {
		free(Data_Screen.drawBuffer);
	}
	Data_Screen.drawBuffer = malloc(width * height * sizeof(ScreenColor));
	Data_Screen.format = 565;
	Data_Screen.width = width;
	Data_Screen.height = height;
	Data_Screen.offset640x480.x = (Data_Screen.width - 640) / 2;
	Data_Screen.offset640x480.y = (Data_Screen.height - 480) / 2;
	Graphics_setClipRectangle(0, 0, width, height);
	CityView_setViewport();
	CityView_checkCameraBoundaries();
	UI_Warning_clearAll();
}

void Screen_setResolution(int width, int height, void *pixels)
{
	Data_Screen.drawBuffer = (ScreenColor*) pixels;
	Data_Screen.format = 565;
	Data_Screen.width = width;
	Data_Screen.height = height;
	Data_Screen.offset640x480.x = (Data_Screen.width - 640) / 2;
	Data_Screen.offset640x480.y = (Data_Screen.height - 480) / 2;
	Graphics_setClipRectangle(0, 0, width, height);
	CityView_setViewport();
	CityView_checkCameraBoundaries();
	UI_Warning_clearAll();
}
