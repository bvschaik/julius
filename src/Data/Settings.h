#ifndef DATA_SETTINGS_H
#define DATA_SETTINGS_H

#define IsTutorial1() (Data_Settings.currentMissionId == 0 && !Data_Settings.isCustomScenario)
#define IsTutorial2() (Data_Settings.currentMissionId == 1 && !Data_Settings.isCustomScenario)
#define IsTutorial3() (Data_Settings.currentMissionId == 2 && !Data_Settings.isCustomScenario)
#define GridOffset(x,y) (Data_Settings_Map.gridStartOffset + (x) + (y) * 162)
#define GridOffsetToX(g) (((g) - Data_Settings_Map.gridStartOffset) % 162)
#define GridOffsetToY(g) (((g) - Data_Settings_Map.gridStartOffset) / 162)
#define IsOutsideMap(x,y,s) (x) < 0 || (x) + (s) > Data_Settings_Map.width || (y) < 0 || (y) + (s) > Data_Settings_Map.height

extern struct Data_Settings {
	int bpp;
	int fullscreen;
	char gamePaused;
	char __unused1;
	char cddrive;
	char soundEffectsEnabled;
	char soundMusicEnabled;
	char soundSpeechEnabled;
	char __unused2[2];
	int resolutionId;
	int gameSpeed;
	int scrollSpeed;
	char playerName[32];
	int __unused3[4];
	int lastAdvisor;
	int saveGameMissionId;
	int mouseTooltips;
	int startingFavor;
	int personalSavingsLastMission;
	int currentMissionId;
	int isCustomScenario;
	char soundCityEnabled;
	char warningsEnabled;
	char monthlyAutosaveEnabled;
	char autoclearEnabled;
	int soundEffectsPercentage;
	int soundMusicPercentage;
	int soundSpeechPercentage;
	int soundCityPercentage;
	int __unused4;
	int ram;
	int __unused5[2];
	int maxConfirmedResolution;
	int __unused6;
	int personalSavingsPerMission[100];
	int lastVictoryVideoPlayed;
	int difficulty;
	int godsEnabled;
} Data_Settings;

extern struct Data_Settings_Map {
	int mapsId;
	struct {
		int x;
		int y;
	} camera;
	struct {
		int gridOffset;
		int x;
		int y;
	} current;
	int width;
	int height;
	int gridStartOffset;
	int gridBorderSize;
	int orientation;
	int __unknown2;
} Data_Settings_Map;

#endif
