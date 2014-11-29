#include "Settings.h"
#include "FileSystem.h"

#include "UI/TopMenu.h"
#include "Data/Model.h"
#include "Data/Settings.h"

static void loadDefaults()
{
	Data_Settings.bpp = 16;
	Data_Settings.fullscreen = 1;
	Data_Settings.gamePaused = 0;
	Data_Settings.ram = 15;
	Data_Settings.cddrive = 'D';
	Data_Settings.soundEffectsEnabled = 1;
	Data_Settings.soundMusicEnabled = 1;
	Data_Settings.soundSpeechEnabled = 1;
	Data_Settings.soundCityEnabled = 1;
	Data_Settings.warningsEnabled = 1;
	Data_Settings.mouseTooltips = 2;
	Data_Settings.gameSpeed = 90;
	Data_Settings.scrollSpeed = 70;
	Data_Settings.difficulty = 3; // hard
	Data_Settings.godsEnabled = 1;
	Data_Settings.soundEffectsPercentage = 100;
	Data_Settings.soundMusicPercentage = 80;
	Data_Settings.soundSpeechPercentage = 100;
	Data_Settings.soundCityPercentage = 100;
	Data_Settings.resolutionId = 2;
}

void Settings_load()
{
	loadDefaults();
	FileSystem_readFileIntoBuffer("c3map.inf", &Data_Settings_Map);
	FileSystem_readFileIntoBuffer("c3.inf", &Data_Settings);
	Data_Settings.gamePaused = 0;
	UI_TopMenu_initFromSettings();
}

void Settings_save()
{
	FileSystem_writeBufferToFile("c3.inf", &Data_Settings, 560);
	FileSystem_writeBufferToFile("c3map.inf", &Data_Settings_Map, 48);
}

void Settings_clearMissionSettings()
{
	for (int i = 0; i < 100; i++) {
		Data_Settings.personalSavingsPerMission[i] = 0;
	}
	Data_Settings.startingFavor = Data_Model_Difficulty.startingFavor[Data_Settings.difficulty];
	Data_Settings.personalSavingsLastMission = 0;
	Data_Settings.currentMissionId = 0;
	Data_Settings.saveGameMissionId = 0;
}
