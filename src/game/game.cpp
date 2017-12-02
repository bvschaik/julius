#include "game.h"

#include "loader.h"
#include "game/game_settings.h"
#include "system.h"
#include "video.h"

#include <sound>
#include <data>
#include <scenario>
#include <ui>

#include "building/model.h"
#include "core/debug.h"
#include "core/lang.h"
#include "core/random.h"
#include "game/settings.h"
#include "graphics/image.h"

#include <string.h>
#include "game/app.hpp"
#include "debug/logger.hpp"

static void errlog(const char *msg)
{
    debug_log(msg, 0, 0);
}

static void loadDefaultNames()
{
    // TODO move out
    scenario_set_player_name(lang_get_string(9, 5));
    strcpy(Data_FileList.selectedCity, (char*)lang_get_string(9, 6));
    strcpy(Data_FileList.lastLoadedCity, (char*)lang_get_string(9, 6));
    strcpy(Data_FileList.selectedScenario, (char*)lang_get_string(9, 7));
}

int Game::preInit()
{
    Settings_load();
    settings_load();

    scenario_settings_init();
    Data_State.gamePaused = 0;
    UI_TopMenu_initFromSettings(); // TODO eliminate need for this

    if (!lang_load("c3.eng", "c3_mm.eng"))
    {
        errlog("ERR: 'c3.eng' or 'c3_mm.eng' files not found or too large.");
        return 0;
    }
    loadDefaultNames();
    random_init();
    return 1;
}

int Game::init()
{
    System_initCursors();
    Logger::registerWriter(Logger::consolelog, "");
    Logger::registerWriter(Logger::filelog, app.workdir());

    if (!image_init())
    {
        errlog("ERR: unable to init graphics");
        return 0;
    }

    if (!image_load_climate(CLIMATE_CENTRAL))
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

    sound_system_init();
    Loader_GameState_init();
    UI_Window_goTo(Window_Logo);
    return 1;
}

void Game::exit()
{
    Video_shutdown();
    Settings_save();
    settings_save();
    sound_system_shutdown();
}
