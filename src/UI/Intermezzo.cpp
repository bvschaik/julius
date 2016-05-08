#include "AllWindows.h"
#include "Window.h"

#include "../Time.h"
#include "../Graphics.h"
#include "../Sound.h"

#include "../Data/Constants.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

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
	TimeMillis startTime;
	TimeMillis endTime;
} data;

void UI_Intermezzo_show(int type, WindowId nextWindowId, int timeMillis)
{
	data.type = type;
	data.nextWindowId = nextWindowId;
	data.startTime = Time_getMillis();
	data.endTime = data.startTime + timeMillis;
	UI_Window_goTo(Window_Intermezzo);
}

void UI_Intermezzo_drawBackground()
{
	Graphics_clearScreen();
	int xOffset = (Data_Screen.width - 1024) / 2;
	int yOffset = (Data_Screen.height - 768) / 2;

	int graphicBase = GraphicId(ID_Graphic_IntermezzoBackground);
	if (data.type == Intermezzo_MissionBriefing) {
		Sound_stopMusic();
		Sound_stopSpeech();
		if (Data_Settings.isCustomScenario) {
			Graphics_drawImage(graphicBase + 1, xOffset, yOffset);
		} else {
			Graphics_drawImage(graphicBase + 1 + 2 * Data_Settings.saveGameMissionId, xOffset, yOffset);
			Sound_Speech_playFile(soundFilesBriefing[Data_Settings.saveGameMissionId]);
		}
		UI_Window_requestRefresh();
	} else if (data.type == Intermezzo_Fired) {
		Graphics_drawImage(graphicBase, xOffset, yOffset);
	} else if (data.type == Intermezzo_Won) {
		Sound_stopMusic();
		Sound_stopSpeech();
		if (Data_Settings.isCustomScenario) {
			Graphics_drawImage(graphicBase + 2, xOffset, yOffset);
		} else {
			Graphics_drawImage(graphicBase + 2 + 2 * Data_Settings.saveGameMissionId, xOffset, yOffset);
			Sound_Speech_playFile(soundFilesWon[Data_Settings.saveGameMissionId]);
		}
		UI_Window_requestRefresh();
	}
}

void UI_Intermezzo_handleMouse()
{
	TimeMillis currentTime = Time_getMillis();
	if (Data_Mouse.right.wentUp || currentTime > data.endTime) {
		UI_Window_goTo(data.nextWindowId);
	}
}
