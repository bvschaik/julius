#include "game_settings.h"

#include <ui>
#include <data>
#include <game>

#include "core/io.h"

void Settings_load()
{
    io_read_file_into_buffer("c3map.inf", &Data_Settings_Map, 48);
    Settings_clearMissionSettings();
    io_read_file_into_buffer("c3.inf", &Data_Settings, 560);
    Data_State.gamePaused = 0;
    UI_TopMenu_initFromSettings();
}

void Settings_save()
{
    io_write_buffer_to_file("c3.inf", &Data_Settings, 560);
    io_write_buffer_to_file("c3map.inf", &Data_Settings_Map, 48);
}

void Settings_clearMissionSettings()
{
    setting_clear_personal_savings();
    Data_Settings.startingFavor = difficulty_starting_favor();
    Data_Settings.personalSavingsLastMission = 0;
    Data_Settings.currentMissionId = 0;
    Data_Settings.isCustomScenario = 0;
    Data_Settings.saveGameMissionId = 0;
}
