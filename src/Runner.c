#include "Runner.h"

#include "GameFile.h"
#include "GameTick.h"
#include "UI/Window.h"

#include "Data/State.h"

#include "building/construction.h"
#include "game/animation.h"
#include "game/settings.h"
#include "input/scroll.h"
#include "sound/city.h"

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
	int ticks_per_frame = 1;
	if (gameSpeedIndex >= 10) {
		return 0;
	} else if (gameSpeedIndex < 0) {
        ticks_per_frame = setting_game_speed() / 100;
		gameSpeedIndex = 0;
	}

	if (Data_State.gamePaused) {
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
	if (building_construction_in_progress()) {
		return 0;
	}
	if (scroll_in_progress()) {
		return 0;
	}
	if (diff < millisPerTickPerSpeed[gameSpeedIndex] + 2) {
		return 0;
	}
	lastUpdate = now;
	return ticks_per_frame;
}

void Runner_run()
{
	game_animation_update();
	int numTicks = getElapsedTicks();
	for (int i = 0; i < numTicks; i++) {
		GameTick_doTick();
		GameFile_writeMissionSavedGameIfNeeded();
	}
}

void Runner_draw()
{
	UI_Window_refresh(0);
	sound_city_play();
}
