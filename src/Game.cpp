#include "Game.hpp"

#include "Loader.hpp"
#include "Settings.hpp"
#include "Sound.hpp"
#include "System.hpp"
#include "Video.hpp"

#include "UI/Window.hpp"

#include "Data/Constants.hpp"
#include "Data/FileList.hpp"
#include "Data/Settings.hpp"

#include "building/model.hpp"
#include "core/debug.hpp"
#include "core/lang.hpp"
#include "core/random.hpp"
#include "game/settings.hpp"
#include "graphics/image.hpp"

#include <string.h>

static void errlog(const char *msg)
{
    debug_log(msg, 0, 0);
}

static void loadDefaultNames()
{
    // TODO move out
    strcpy((char*)Data_Settings.playerName, (char*)lang_get_string(9, 5));
    strcpy(Data_FileList.selectedCity, (char*)lang_get_string(9, 6));
    strcpy(Data_FileList.lastLoadedCity, (char*)lang_get_string(9, 6));
    strcpy(Data_FileList.selectedScenario, (char*)lang_get_string(9, 7));
}

int Game_preInit()
{
    Settings_load();
    settings_load();
    if (!lang_load("c3.eng", "c3_mm.eng"))
    {
        errlog("ERR: 'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return 0;
    }
    loadDefaultNames();
    random_init();
    return 1;
}

int Game_init()
{
    System_initCursors();
    if (!image_init())
    {
        errlog("ERR: unable to init graphics");
        return 0;
    }

    if (!image_load_climate(Climate_Central))
    {
        errlog("ERR: unable to load main graphics");
        return 0;
    }
    if (!image_load_enemy(EnemyType_0_Barbarian))
    {
        errlog("ERR: unable to load enemy graphics");
        return 0;
    }

    if (!model_load())
    {
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
    settings_save();
    Settings_save();
    Sound_shutdown();
}
