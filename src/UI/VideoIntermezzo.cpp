#include "VideoIntermezzo.h"

#include "../Video.h"

#include "../Graphics.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"

static struct {
	WindowId nextWindowId;
	int width;
	int height;
} data;

void VideoIntermezzo_show(const char* filename, int width, int height, WindowId nextWindowId)
{
	if (!Video_start(filename)) {
		data.width = width;
		data.height = height;
		UI_Window_goTo(nextWindowId);
	} else {
		data.nextWindowId = nextWindowId;
		UI_Window_goTo(Window_VideoIntermezzo);
	}
}

void VideoIntermezzo_init()
{
	Video_init();
}

void VideoIntermezzo_drawBackground()
{
	Graphics_clearScreen();
}

void VideoIntermezzo_drawForeground()
{
	Video_draw((Data_Screen.width - data.width) / 2, (Data_Screen.height - data.height) / 2);
}

void VideoIntermezzo_handleMouse()
{
	if (Data_Mouse.left.wentUp || Data_Mouse.right.wentUp || Video_isFinished()) {
		Video_stop();
		UI_Window_goTo(data.nextWindowId);
	}
}

