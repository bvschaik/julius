#include "AllWindows.h"

#include "core/time.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/screen.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "sound/speech.h"

static const char soundFilesBriefing[][32] = {
	"wavs/01b.wav",
	"wavs/02b.wav",
	"wavs/03b.wav",
	"wavs/04b.wav",
	"wavs/05b.wav",
	"wavs/06b.wav",
	"wavs/07b.wav",
	"wavs/08b.wav",
	"wavs/09b.wav",
	"wavs/10b.wav",
	"wavs/11b.wav",
	"wavs/12b.wav",
	"wavs/13b.wav",
	"wavs/14b.wav",
	"wavs/15b.wav",
	"wavs/16b.wav",
	"wavs/17b.wav",
	"wavs/18b.wav",
	"wavs/19b.wav",
	"wavs/20b.wav",
	"wavs/21b.wav",
	"wavs/22b.wav",
};

static const char soundFilesWon[][32] = {
	"wavs/01w.wav",
	"wavs/02w.wav",
	"wavs/03w.wav",
	"wavs/04w.wav",
	"wavs/05w.wav",
	"wavs/06w.wav",
	"wavs/07w.wav",
	"wavs/08w.wav",
	"wavs/09w.wav",
	"wavs/10w.wav",
	"wavs/11w.wav",
	"wavs/12w.wav",
	"wavs/13w.wav",
	"wavs/14w.wav",
	"wavs/15w.wav",
	"wavs/16w.wav",
	"wavs/17w.wav",
	"wavs/18w.wav",
	"wavs/19w.wav",
	"wavs/20w.wav",
	"wavs/21w.wav",
	"wavs/22w.wav",
};

static struct {
	int type;
	WindowId nextWindowId;
	time_millis startTime;
	time_millis endTime;
} data;

void UI_Intermezzo_show(int type, WindowId nextWindowId, int timeMillis)
{
	data.type = type;
	data.nextWindowId = nextWindowId;
	data.startTime = time_get_millis();
	data.endTime = data.startTime + timeMillis;
	UI_Window_goTo(Window_Intermezzo);
}

void UI_Intermezzo_drawBackground()
{
	graphics_clear_screen();
	int xOffset = (screen_width() - 1024) / 2;
	int yOffset = (screen_height() - 768) / 2;

    int mission = scenario_campaign_mission();
	int graphicBase = image_group(GROUP_INTERMEZZO_BACKGROUND);
	if (data.type == Intermezzo_MissionBriefing) {
		sound_music_stop();
		sound_speech_stop();
		if (scenario_is_custom()) {
			image_draw(graphicBase + 1, xOffset, yOffset);
		} else {
			image_draw(graphicBase + 1 + 2 * mission, xOffset, yOffset);
			sound_speech_play_file(soundFilesBriefing[mission]);
		}
		window_invalidate();
	} else if (data.type == Intermezzo_Fired) {
		image_draw(graphicBase, xOffset, yOffset);
	} else if (data.type == Intermezzo_Won) {
		sound_music_stop();
		sound_speech_stop();
		if (scenario_is_custom()) {
			image_draw(graphicBase + 2, xOffset, yOffset);
		} else {
			image_draw(graphicBase + 2 + 2 * mission, xOffset, yOffset);
			sound_speech_play_file(soundFilesWon[mission]);
		}
		window_invalidate();
	}
}

void UI_Intermezzo_handleMouse(const mouse *m)
{
	time_millis currentTime = time_get_millis();
	if (m->right.went_up || currentTime > data.endTime) {
		UI_Window_goTo(data.nextWindowId);
	}
}
