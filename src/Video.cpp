#include "Video.h"

static struct {
	WindowId nextWindowId;
} data;

static int playVideo(const char *filename, int xOffset, int yOffset, int fullscreen)
{
	// TODO
	return 0;
}

int Video_start(const char *filename, int xOffset, int yOffset, int fullscreen, WindowId nextWindowId)
{
	data.nextWindowId = nextWindowId;
	if (playVideo(filename, xOffset, yOffset, fullscreen)) {
		UI_Window_goTo(Window_Video);
		return 1;
	}
	if (fullscreen) {
		UI_Window_goTo(nextWindowId);
	}
	return 0;
}

void Video_cancel()
{
	// TODO
}
