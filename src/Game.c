#include "Game.h"

#include "System.h"

#include "UI/Window.h"
#include "UI/TopMenu.h"

#include "building/construction.h"
#include "building/model.h"
#include "core/debug.h"
#include "core/lang.h"
#include "core/random.h"
#include "core/time.h"
#include "figure/type.h"
#include "game/animation.h"
#include "game/file.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/tick.h"
#include "graphics/image.h"
#include "graphics/video.h"
#include "input/scroll.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/system.h"

static const time_millis MILLIS_PER_TICK_PER_SPEED[] = {
    0, 20, 35, 55, 80, 110, 160, 240, 350, 500, 700
};

static time_millis lastUpdate;

static void errlog(const char *msg)
{
    debug_log(msg, 0, 0);
}

int Game_preInit()
{
    settings_load();
    scenario_settings_init();
    game_state_unpause();
    UI_TopMenu_initFromSettings(); // TODO eliminate need for this

	if (!lang_load("c3.eng", "c3_mm.eng")) {
		errlog("ERR: 'c3.eng' or 'c3_mm.eng' files not found or too large.");
		return 0;
	}
	scenario_set_player_name(lang_get_string(9, 5));
	random_init();
	return 1;
}

int Game_init()
{
	System_initCursors();
	if (!image_init()) {
		errlog("ERR: unable to init graphics");
		return 0;
	}
	
	if (!image_load_climate(CLIMATE_CENTRAL)) {
		errlog("ERR: unable to load main graphics");
		return 0;
	}
	if (!image_load_enemy(ENEMY_0_BARBARIAN)) {
		errlog("ERR: unable to load enemy graphics");
		return 0;
	}

	if (!model_load()) {
		errlog("ERR: unable to load c3_model.txt");
		return 0;
	}

	sound_system_init();
	game_state_init();
	UI_Window_goTo(Window_Logo);
	return 1;
}

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

    if (game_state_is_paused()) {
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
    if (diff < MILLIS_PER_TICK_PER_SPEED[gameSpeedIndex] + 2) {
        return 0;
    }
    lastUpdate = now;
    return ticks_per_frame;
}

void Game_run()
{
    game_animation_update();
    int numTicks = getElapsedTicks();
    for (int i = 0; i < numTicks; i++) {
        game_tick_run();
        game_file_write_mission_saved_game();
    }
}

void Game_draw()
{
    UI_Window_refresh(0);
    sound_city_play();
}

void Game_exit()
{
    video_shutdown();
    settings_save();
    sound_system_shutdown();
}
