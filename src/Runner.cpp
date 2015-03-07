
#include "Animation.h"
#include "GameFile.h"
#include "GameTick.h"
#include "Sound.h"
#include "Time.h"
#include "UI/Window.h"

#include "Data/Settings.h"
#include "Data/State.h"

#include <cstdio>

static const TimeMillis millisPerTickPerSpeed[] = {
	0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700
};

static TimeMillis lastUpdate;

static int getElapsedTicks()
{
	TimeMillis now = Time_getMillis();
	TimeMillis diff = now - lastUpdate;
	if (now < lastUpdate) {
		diff = 10000;
	}
	int gameSpeedIndex = (100 - Data_Settings.gameSpeed) / 10;
	if (gameSpeedIndex >= 10) {
		return 0;
	} else if (gameSpeedIndex < 0) {
		gameSpeedIndex = 0;
	}

	if (Data_Settings.gamePaused) {
		return 0;
	}
	switch (UI_Window_getId()) {
		default:
			return 0;
		case Window_City:
		case Window_CityMilitary:
		case Window_SlidingSidebar:
		case Window_OverlayMenu:
			break;
	}
	if (Data_State.selectedBuilding.placementInProgress) {
		return 0;
	}
	if (Data_State.isScrollingMap) {
		return 0;
	}
	if (diff < millisPerTickPerSpeed[gameSpeedIndex] + 2) {
		return 0;
	}
	lastUpdate = now;
	return 1;//diff / (millisPerTickPerSpeed[gameSpeedIndex] + 2);
}

void Runner_run()
{
	Animation_updateTimers();
	int numTicks = getElapsedTicks();
	//if (numTicks) printf("Running for %d ticks\n", numTicks);
	for (int i = 0; i < numTicks; i++) {
		GameTick_doTick();
		GameFile_writeMissionSavedGameIfNeeded();
	}
}

void Runner_draw()
{
	UI_Window_refresh(0);
	Sound_City_play();
}
