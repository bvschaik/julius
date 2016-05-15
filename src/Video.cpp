#include "Video.h"

extern "C" {
  #include "Video/smacker.h"
}

#include "Sound.h"
#include "UI/AllWindows.h"

static struct {
	int isPlaying;
	int isFullscreen;
	int xOffset;
	int yOffset;
	WindowId parentWindowId;
	WindowId nextWindowId;
	smk video;
} data;

static void doFrame()
{
	// TODO
}

static int playVideo(const char *filename, int xOffset, int yOffset, int fullscreen)
{
	data.isPlaying = 0;
	data.video = 0;
	data.xOffset = xOffset;
	data.yOffset = yOffset;
	data.isFullscreen = fullscreen;
	
	// color flags?
	data.video = smk_open_file(filename, SMK_MODE_DISK);
	if (data.video) {
		Sound_stopMusic();
		Sound_stopSpeech();
		// refresh video buffer?
		data.isPlaying = 1;
		doFrame();
		return 1;
	} else {
		return 0;
	}
}

static void endVideo()
{
	data.isPlaying = 0;
	UI_Window_goTo(data.nextWindowId);
	Sound_Music_reset();
	Sound_Music_update();
	if (data.isFullscreen) {
		UI_Window_requestRefresh();
	}
}


int Video_start(const char *filename, int xOffset, int yOffset, int fullscreen, WindowId nextWindowId)
{
	data.nextWindowId = nextWindowId;
	if (playVideo(filename, xOffset, yOffset, fullscreen)) {
		if (!fullscreen) {
			data.parentWindowId = UI_Window_getId();
		}
		UI_Window_goTo(Window_Video);
		return 1;
	}
	if (fullscreen) {
		UI_Window_goTo(nextWindowId);
	}
	return 0;
}

void Video_stop()
{
	if (data.isPlaying) {
		if (data.video) {
			smk_close(data.video);
			data.video = 0;
		}
		endVideo();
	}
}

void Video_shutdown()
{
	if (data.isPlaying) {
		if (data.video) {
			smk_close(data.video);
			data.video = 0;
		}
	}
}
