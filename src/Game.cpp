#include "Game.h"

#include "Language.h"
#include "Loader.h"
#include "Random.h"
#include "Settings.h"
#include "Sound.h"
#include "System.h"

#include "UI/Window.h"

#include "Data/Constants.h"

#include <stdio.h>

static void log(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

int Game_preInit()
{
	Settings_load();
	if (!Language_load("c3.eng", "c3_mm.eng")) {
		log("ERR: 'c3.eng' or 'c3_mm.eng' files not found or too large.");
		return 0;
	}
	Language_loadDefaultNames();
	Random_init();
	return 1;
}

int Game_init()
{
	System_initCursors();
	if (!Loader_Graphics_initGraphics()) {
		log("ERR: unable to load graphics");
		return 0;
	}
	
	printf("Load images: %d\n", Loader_Graphics_loadMainGraphics(Climate_Central));
	printf("Load enemies: %d\n", Loader_Graphics_loadEnemyGraphics(EnemyType_0_Barbarian));

	if (!Loader_Model_loadC3ModelTxt()) {
		log("ERR: unable to load c3_model.txt");
		return 0;
	}

	Sound_init();
	Loader_GameState_init();
	//TODO? fun_setCityScreenWidths();
	UI_Window_goTo(Window_MainMenu);
	return 1;
}

void Game_exit()
{
	Settings_save();
	Sound_shutdown();
}

