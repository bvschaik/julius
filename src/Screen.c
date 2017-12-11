#include "Screen.h"

#include "CityView.h"
#include "Graphics.h"
#include "Data/Screen.h"

#include "city/warning.h"
#include "graphics/color.h"

#include <stdlib.h>
#include <string.h>

static void setSize(int width, int height)
{
	Data_Screen.width = width;
	Data_Screen.height = height;
	Data_Screen.offset640x480.x = (Data_Screen.width - 640) / 2;
	Data_Screen.offset640x480.y = (Data_Screen.height - 480) / 2;
	Graphics_setClipRectangle(0, 0, width, height);
	CityView_setViewport();
	CityView_checkCameraBoundaries();
	city_warning_clear_all();
}

void Screen_setResolution(int width, int height)
{
	if (Data_Screen.drawBuffer) {
		free(Data_Screen.drawBuffer);
	}
	Data_Screen.drawBuffer = malloc(width * height * sizeof(color_t));
	memset(Data_Screen.drawBuffer, 0, width * height * sizeof(color_t));
	setSize(width, height);
}

void Screen_setResolutionWithPixels(int width, int height, void *pixels)
{
	Data_Screen.drawBuffer = (color_t*) pixels;
	setSize(width, height);
}
