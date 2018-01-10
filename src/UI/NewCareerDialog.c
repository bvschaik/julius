#include "AllWindows.h"

#include "core/string.h"
#include "game/settings.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "input/keyboard.h"
#include "scenario/property.h"
#include "scenario/scenario.h"

static void startMission(int param1, int param2);

static image_button imageButtonStartMission = {
	0, 0, 27, 27, IB_NORMAL, 92, 56, startMission, button_none, 1, 0, 1
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
	graphics_clear_screen();
    graphics_in_dialog();
	image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
}

void UI_NewCareerDialog_drawForeground()
{
    graphics_in_dialog();
	outer_panel_draw(128, 160, 24, 8);
	lang_text_draw_centered(31, 0, 128, 172, 384, FONT_LARGE_BLACK);
	lang_text_draw(13, 5, 352, 256, FONT_NORMAL_BLACK);
	inner_panel_draw(160, 208, 20, 2);
	text_capture_cursor(keyboard_cursor_position());
	text_draw(player_name, 176, 216, FONT_NORMAL_WHITE, 0);
	text_draw_cursor(176, 217, keyboard_is_insert());
	
	image_buttons_draw(464, 249, &imageButtonStartMission, 1);

    graphics_reset_dialog();
}

void UI_NewCareerDialog_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
        keyboard_stop_capture();
		UI_Window_goTo(Window_MainMenu);
	}

	if (image_buttons_handle_mouse(mouse_in_dialog(m), 464, 249, &imageButtonStartMission, 1, 0)) {
		return;
	}
	if (keyboard_input_is_accepted()) {
		startMission(0, 0);
	}
}

static void startMission(int param1, int param2)
{
    keyboard_stop_capture();
    scenario_set_player_name(player_name);
	UI_MissionStart_show();
}
