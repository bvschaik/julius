#include "Settings.h"

#include "UI/TopMenu.h"
#include "Data/Settings.h"
#include "Data/State.h"

#include "core/io.h"
#include "game/difficulty.h"
#include "game/settings.h"
#include "scenario/scenario.h"

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
    scenario_settings_init();
}
