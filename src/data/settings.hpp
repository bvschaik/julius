#ifndef DATA_SETTINGS_H
#define DATA_SETTINGS_H

#include <stdint.h>

#define IsTutorial1() (Data_Settings.currentMissionId == 0 && !Data_Settings.isCustomScenario)
#define IsTutorial2() (Data_Settings.currentMissionId == 1 && !Data_Settings.isCustomScenario)
#define IsTutorial3() (Data_Settings.currentMissionId == 2 && !Data_Settings.isCustomScenario)
#define GridOffset(x,y) (Data_Settings_Map.gridStartOffset + (x) + (y) * 162)
#define GridOffsetToX(g) (((g) - Data_Settings_Map.gridStartOffset) % 162)
#define GridOffsetToY(g) (((g) - Data_Settings_Map.gridStartOffset) / 162)
#define IsInsideMap(x,y) ((x) >= 0 && (x) < Data_Settings_Map.width && (y) >= 0 && (y) < Data_Settings_Map.height)
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

extern struct _Data_Settings
{
    int _bpp;
    int _fullscreen;
    unsigned char _gamePaused;
    unsigned char __unused1;
    char _cddrive;
    unsigned char _soundEffectsEnabled;
    unsigned char _soundMusicEnabled;
    unsigned char _soundSpeechEnabled;
    short __unused2;
    int _resolutionId;
    int _gameSpeed;
    int _scrollSpeed;
    char playerName[32];
    int __unused3[4];
    int _lastAdvisor;
    int saveGameMissionId;
    int _mouseTooltips;
    int startingFavor;
    int personalSavingsLastMission;
    int currentMissionId;
    int isCustomScenario;
    unsigned char _soundCityEnabled;
    unsigned char _warningsEnabled;
    unsigned char _monthlyAutosaveEnabled;
    unsigned char _autoclearEnabled;
    int _soundEffectsPercentage;
    int _soundMusicPercentage;
    int _soundSpeechPercentage;
    int _soundCityPercentage;
    int __unused4;
    int _ram;
    int _windowedWidth; // addition
    int _windowedHeight; // addition
    int _maxConfirmedResolution;
    int __unused6;
    int _personalSavingsPerMission[100];
    int _lastVictoryVideoPlayed;
    int _difficulty;
    int _godsEnabled;
} Data_Settings;

extern struct _Data_Settings_Map
{
    int mapsId;
    struct
    {
        int x;
        int y;
    } camera;
    struct
    {
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
