#include "Window.h"
#include "AllWindows.h"
#include "../Widget.h"
#include "../Graphics.h"

#include "../Data/Screen.h"

#include "core/string.h"
#include "game/settings.h"
#include "input/keyboard.h"
#include "scenario/property.h"
#include "scenario/scenario.h"

static void startMission(int param1, int param2);

static ImageButton imageButtonStartMission = {
	0, 0, 27, 27, ImageButton_Normal, 92, 56, startMission, Widget_Button_doNothing, 1, 0, 1
};

static uint8_t player_name[32];

void UI_NewCareerDialog_init()
{
    setting_clear_personal_savings();
    scenario_settings_init();
    string_copy(scenario_player_name(), player_name, 32);
    keyboard_start_capture(player_name, 32, 1, 280, FONT_NORMAL_WHITE);
}

void UI_NewCareerDialog_drawBackground()
{
	Graphics_clearScreen();
	Graphics_drawImage(image_group(GROUP_MAIN_MENU_BACKGROUND),
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
}

void UI_NewCareerDialog_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(xOffset + 128, yOffset + 160, 24, 8);
	Widget_GameText_drawCentered(31, 0, xOffset + 128, yOffset + 172, 384, FONT_LARGE_BLACK);
	Widget_GameText_draw(13, 5, xOffset + 352, yOffset + 256, FONT_NORMAL_BLACK);
	Widget_Panel_drawInnerPanel(xOffset + 160, yOffset + 208, 20, 2);
	Widget_Text_captureCursor(keyboard_cursor_position());
	Widget_Text_draw(player_name, xOffset + 176, yOffset + 216, FONT_NORMAL_WHITE, 0);
	Widget_Text_drawCursor(xOffset + 176, yOffset + 217, keyboard_is_insert());
	
	Widget_Button_drawImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1);
}

void UI_NewCareerDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		UI_Window_goTo(Window_MainMenu);
	}

	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	if (Widget_Button_handleImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1, 0)) {
		return;
	}
	if (keyboard_input_is_accepted()) {
		startMission(0, 0);
	}
}

static void startMission(int param1, int param2)
{
    scenario_set_player_name(player_name);
	UI_MissionStart_show();
}
