#include "Game.h"

#include "Language.h"
#include "Loader.h"
#include "Settings.h"
#include "Sound.h"
#include "System.h"
#include "Video.h"

#include "UI/Window.h"

#include "Data/Constants.h"

#include "core/debug.h"
#include "core/random.h"

#include <stdio.h>

static void errlog(const char *msg)
{
    debug_log(msg, 0, 0);
}

int Game_preInit()
{
	Settings_load();
	if (!Language_load("c3.eng", "c3_mm.eng")) {
		errlog("ERR: 'c3.eng' or 'c3_mm.eng' files not found or too large.");
		return 0;
	}
	Language_loadDefaultNames();
	random_init();
	return 1;
}

int Game_init()
{
	System_initCursors();
	if (!Loader_Graphics_initGraphics()) {
		errlog("ERR: unable to init graphics");
		return 0;
	}
	
	if (!Loader_Graphics_loadMainGraphics(Climate_Central)) {
		errlog("ERR: unable to load main graphics");
		return 0;
	}
	if (!Loader_Graphics_loadEnemyGraphics(EnemyType_0_Barbarian)) {
		errlog("ERR: unable to load enemy graphics");
		return 0;
	}

	if (!Loader_Model_loadC3ModelTxt()) {
		errlog("ERR: unable to load c3_model.txt");
		return 0;
	}

	Sound_init();
	Loader_GameState_init();
	UI_Window_goTo(Window_Logo);
	return 1;
}

void Game_exit()
{
	Video_shutdown();
	Settings_save();
	Sound_shutdown();
}
