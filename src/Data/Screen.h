#ifndef DATA_SCREEN_H
#define DATA_SCREEN_H

#include "Types.h"

#define SCREEN_REF(x, y) (&Data_Screen.drawBuffer[(x) + Data_Screen.width * (y)])

extern struct Data_Screen {
	int width;
	int height;
	struct {
		int x, y;
	} offset640x480;
	int format; // 555, 565, ...
	Color *drawBuffer; //[800*600];
} Data_Screen;

#endif
