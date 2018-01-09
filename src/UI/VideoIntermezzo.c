#include "VideoIntermezzo.h"

#include "graphics/graphics.h"
#include "graphics/screen.h"
#include "graphics/video.h"

static struct {
	WindowId nextWindowId;
	int width;
	int height;
} data;

void UI_VideoIntermezzo_show(const char* filename, int width, int height, WindowId nextWindowId)
{
	if (video_start(filename)) {
		data.width = width;
		data.height = height;
		data.nextWindowId = nextWindowId;
		UI_Window_goTo(Window_VideoIntermezzo);
	} else {
		UI_Window_goTo(nextWindowId);
	}
}

void UI_VideoIntermezzo_init()
{
	video_init();
}

void UI_VideoIntermezzo_drawBackground()
{
	graphics_clear_screen();
}

void UI_VideoIntermezzo_drawForeground()
{
	video_draw((screen_width() - data.width) / 2, (screen_height() - data.height) / 2);
}

void UI_VideoIntermezzo_handleMouse(const mouse *m)
{
	if (m->left.went_up || m->right.went_up || video_is_finished()) {
		video_stop();
		UI_Window_goTo(data.nextWindowId);
	}
}

