
#include "animation.h"
#include "gamefile.h"
#include "gametick.h"
#include "sound.h"
#include "ui/window.h"

#include "data/settings.hpp"
#include "data/state.hpp"

#include "core/time.h"
#include "game/settings.h"

static const time_millis millisPerTickPerSpeed[] = {
	0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700
};

static time_millis lastUpdate;

static int getElapsedTicks()
{
	time_millis now = time_get_millis();
	time_millis diff = now - lastUpdate;
	if (now < lastUpdate) {
		diff = 10000;
	}
	int gameSpeedIndex = (100 - setting_game_speed()) / 10;
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
    // run twice as fast on 100% speed
	return gameSpeedIndex == 0 ? 2 : 1;
}

void Runner_run()
{
	Animation_updateTimers();
	int numTicks = getElapsedTicks();
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
