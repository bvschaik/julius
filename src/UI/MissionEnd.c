#include "AllWindows.h"

#include "../Graphics.h"

#include "../Data/CityInfo.h"
#include "../Data/Screen.h"
#include "../Data/State.h"

#include "city/finance.h"
#include "city/victory.h"
#include "game/mission.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/generic_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/music.h"
#include "sound/speech.h"

static void victoryAccept(int param1, int param2);
static void victoryContinueGoverning(int duration, int param2);
static void firedAccept(int param1, int param2);

static generic_button victoryButtons[] = {
	{32, 112, 416, 132, GB_ON_MOUSE_UP, victoryAccept, button_none, 0, 0},
	{32, 144, 416, 164, GB_IMMEDIATE, victoryContinueGoverning, button_none, 1, 0},
	{32, 176, 416, 196, GB_IMMEDIATE, victoryContinueGoverning, button_none, 2, 0},
};
static generic_button firedButtons[] = {
	{64, 208, 384, 228, GB_IMMEDIATE, firedAccept, button_none, 0, 0},
};

static int focusButtonId = 0;

void UI_VictoryIntermezzo_init()
{
	sound_music_reset();
	UI_Intermezzo_show(Intermezzo_Won, Window_MissionEnd, 1000);
}

void UI_MissionEnd_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;
	if (city_victory_state() != VICTORY_STATE_WON) {
		// lost mission
		outer_panel_draw(xOffset, yOffset - 112, 34, 16);
		lang_text_draw_centered(62, 1, xOffset, yOffset - 96, 544, FONT_LARGE_BLACK);
		lang_text_draw_multiline(62, 16, xOffset + 16, yOffset - 56, 496, FONT_NORMAL_BLACK);
		return;
	}
	// won mission
	outer_panel_draw(xOffset, yOffset, 34, 18);
	lang_text_draw_centered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);
	
	inner_panel_draw(xOffset + 16, yOffset + 56, 32, 7);

	if (scenario_is_custom()) {
		lang_text_draw_multiline(147, 20, xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
	} else {
		lang_text_draw_multiline(147, scenario_campaign_mission(), xOffset + 32, yOffset + 64, 496, FONT_NORMAL_WHITE);
	}
	int width = lang_text_draw(148, 0, xOffset + 40, yOffset + 180, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.ratingCulture, '@', " ",
		xOffset + 40 + width, yOffset + 180, FONT_NORMAL_BLACK);

	width = lang_text_draw(148, 1, xOffset + 300, yOffset + 180, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.ratingProsperity, '@', " ",
		xOffset + 300 + width, yOffset + 180, FONT_NORMAL_BLACK);

	width = lang_text_draw(148, 2, xOffset + 40, yOffset + 200, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.ratingPeace, '@', " ",
		xOffset + 40 + width, yOffset + 200, FONT_NORMAL_BLACK);

	width = lang_text_draw(148, 3, xOffset + 300, yOffset + 200, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.ratingFavor, '@', " ",
		xOffset + 300 + width, yOffset + 200, FONT_NORMAL_BLACK);

	width = lang_text_draw(148, 4, xOffset + 40, yOffset + 220, FONT_NORMAL_BLACK);
	text_draw_number(Data_CityInfo.population, '@', " ",
		xOffset + 40 + width, yOffset + 220, FONT_NORMAL_BLACK);

	width = lang_text_draw(148, 5, xOffset + 300, yOffset + 220, FONT_NORMAL_BLACK);
	text_draw_number(city_finance_treasury(), '@', " ",
		xOffset + 300 + width, yOffset + 220, FONT_NORMAL_BLACK);

	lang_text_draw_centered(13, 1, xOffset + 16, yOffset + 260, 512, FONT_NORMAL_BLACK);
}

void UI_MissionEnd_drawForeground()
{
	if (city_victory_state() != VICTORY_STATE_WON) {
		UI_VictoryDialog_drawForeground();
	}
}

static void advanceToNextMission()
{
    // TODO move out of UI code
	setting_set_personal_savings_for_mission(scenario_campaign_rank() + 1, Data_CityInfo.personalSavings);
	scenario_set_campaign_rank(scenario_campaign_rank() + 1);

	Data_CityInfo.victoryHasWonScenario = 0;
	Data_CityInfo.victoryContinueMonths = 0;
	Data_CityInfo.victoryContinueMonthsChosen = 0;

	game_undo_disable();
	game_state_reset_overlay();

	if (scenario_campaign_rank() >= 11 || scenario_is_custom()) {
		UI_Window_goTo(Window_MainMenu);
		if (!scenario_is_custom()) {
            setting_clear_personal_savings();
            scenario_settings_init();
            scenario_set_campaign_rank(2);
		}
	} else {
		scenario_set_campaign_mission(game_mission_peaceful());
		UI_MissionStart_show();
	}
}

void UI_MissionEnd_handleMouse(const mouse *m)
{
	if (city_victory_state() == VICTORY_STATE_WON) {
		if (m->right.went_up) {
			sound_music_stop();
			sound_speech_stop();
			advanceToNextMission();
		}
	} else {
		generic_buttons_handle_mouse(mouse_in_dialog(m), 48, 16,
			firedButtons, 1, &focusButtonId);
	}
}

void UI_VictoryDialog_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;

	outer_panel_draw(xOffset, yOffset, 34, 15);
	if (scenario_campaign_rank() < 10 || scenario_is_custom()) {
		lang_text_draw_centered(62, 0, xOffset, yOffset + 16, 544, FONT_LARGE_BLACK);
		lang_text_draw_centered(62, 2, xOffset, yOffset + 47, 544, FONT_NORMAL_BLACK);
		lang_text_draw_centered(32, Data_CityInfo.playerRank + 1, xOffset, yOffset + 66, 544, FONT_LARGE_BLACK);
	} else {
		text_draw_centered(scenario_player_name(), xOffset, yOffset + 16, 512, FONT_LARGE_BLACK, 0);
		lang_text_draw_multiline(62, 26, xOffset + 16, yOffset + 47, 480, FONT_NORMAL_BLACK);
	}
}

void UI_VictoryDialog_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x + 48;
	int yOffset = Data_Screen.offset640x480.y + 128;

	if (city_victory_state() == VICTORY_STATE_WON) {
		large_label_draw(xOffset + 32, yOffset + 112, 30, focusButtonId == 1);
		if (scenario_campaign_rank() < 10 || scenario_is_custom()) {
			lang_text_draw_centered(62, 3,
				xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
		} else {
			lang_text_draw_centered(62, 27,
				xOffset + 32, yOffset + 118, 480, FONT_NORMAL_GREEN);
		}
		if (scenario_campaign_rank() >= 2 || scenario_is_custom()) {
			// Continue for 2/5 years
			large_label_draw(xOffset + 32, yOffset + 144, 30, focusButtonId == 2);
			lang_text_draw_centered(62, 4, xOffset + 32, yOffset + 150, 480, FONT_NORMAL_GREEN);

			large_label_draw(xOffset + 32, yOffset + 176, 30, focusButtonId == 3);
			lang_text_draw_centered(62, 5, xOffset + 32, yOffset + 182, 480, FONT_NORMAL_GREEN);
		}
	} else {
		// lost
		large_label_draw(xOffset + 32, yOffset + 96, 30, focusButtonId == 1);
		lang_text_draw_centered(62, 6, xOffset + 32, yOffset + 102, 480, FONT_NORMAL_GREEN);
	}
}

void UI_VictoryDialog_handleMouse(const mouse *m)
{
	int numButtons;
	if (scenario_campaign_rank() >= 2 || scenario_is_custom()) {
		numButtons = 3;
	} else {
		numButtons = 1;
	}
	generic_buttons_handle_mouse(mouse_in_dialog(m), 48, 128, victoryButtons, numButtons, &focusButtonId);
}

static void victoryAccept(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void victoryContinueGoverning(int duration, int param2)
{
	Data_CityInfo.victoryHasWonScenario = 1;
	if (duration == 1) {
		Data_CityInfo.victoryContinueMonths += 24;
		Data_CityInfo.victoryContinueMonthsChosen = 24;
		Data_CityInfo.salaryRank = 0;
		Data_CityInfo.salaryAmount = 0;
		city_finance_update_salary();
	} else if (duration == 2) {
		Data_CityInfo.victoryContinueMonths += 60;
		Data_CityInfo.victoryContinueMonthsChosen = 60;
		Data_CityInfo.salaryRank = 0;
		Data_CityInfo.salaryAmount = 0;
		city_finance_update_salary();
	}
	UI_Window_goTo(Window_City);
	city_victory_reset();
	sound_music_reset();
	sound_music_update();
}

static void firedAccept(int param1, int param2)
{
	Data_CityInfo.victoryHasWonScenario = 0;
	Data_CityInfo.victoryContinueMonths = 0;
	Data_CityInfo.victoryContinueMonthsChosen = 0;
	game_undo_disable();
	if (scenario_is_custom()) {
		UI_Window_goTo(Window_MainMenu);
	} else {
		UI_MissionStart_show();
	}
}
