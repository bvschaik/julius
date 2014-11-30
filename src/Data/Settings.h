#ifndef DATA_SETTINGS_H
#define DATA_SETTINGS_H

#define IsTutorial1() (Data_Settings.currentMissionId == 0 && !Data_Settings.isCustomScenario)
#define IsTutorial2() (Data_Settings.currentMissionId == 1 && !Data_Settings.isCustomScenario)
#define IsTutorial3() (Data_Settings.currentMissionId == 2 && !Data_Settings.isCustomScenario)
#define GridOffset(x,y) (Data_Settings_Map.gridStartOffset + (x) + (y) * 162)
#define GridOffsetToX(g) (((g) - Data_Settings_Map.gridStartOffset) % 162)
#define GridOffsetToY(g) (((g) - Data_Settings_Map.gridStartOffset) / 162)
#define IsOutsideMap(x,y,s) (x) < 0 || (x) + (s) > Data_Settings_Map.width || (y) < 0 || (y) + (s) > Data_Settings_Map.height
#define BoundToMap(x,y) \
	if ((x) < 0) (x) = 0;\
	if ((y) < 0) (y) = 0;\
	if ((x) >= Data_Settings_Map.width) (x) = Data_Settings_Map.width - 1;\
	if ((y) >= Data_Settings_Map.height) (y) = Data_Settings_Map.height - 1;
#define Bound2ToMap(xMin, yMin, xMax, yMax) \
	if ((xMin) < 0) (xMin) = 0;\
	if ((yMin) < 0) (yMin) = 0;\
	if ((xMax) >= Data_Settings_Map.width) (xMax) = Data_Settings_Map.width - 1;\
	if ((yMax) >= Data_Settings_Map.height) (yMax) = Data_Settings_Map.height - 1;

extern struct _Data_Settings {
	int bpp;
	int fullscreen;
	unsigned char gamePaused;
	unsigned char __unused1;
	char cddrive;
	unsigned char soundEffectsEnabled;
	unsigned char soundMusicEnabled;
	unsigned char soundSpeechEnabled;
	short __unused2;
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
	unsigned char soundCityEnabled;
	unsigned char warningsEnabled;
	unsigned char monthlyAutosaveEnabled;
	unsigned char autoclearEnabled;
	int soundEffectsPercentage;
	int soundMusicPercentage;
	int soundSpeechPercentage;
	int soundCityPercentage;
	int __unused4;
	int ram;
	int windowedWidth; // addition
	int windowedHeight; // addition
	int maxConfirmedResolution;
	int __unused6;
	int personalSavingsPerMission[100];
	int lastVictoryVideoPlayed;
	int difficulty;
	int godsEnabled;
} Data_Settings;

extern struct _Data_Settings_Map {
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
