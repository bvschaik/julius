
#include "AllWindows.h"

#include "../Data/CityInfo.h"
#include "../Data/State.h"

#include "core/lang.h"
#include "core/string.h"
#include "game/file.h"
#include "game/mission.h"
#include "game/tutorial.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/rich_text.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/criteria.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/city.h"
#include "window/mission_selection.h"

static void startMission(int param1, int param2);
static void briefingBack(int param1, int param2);

static const int goalOffsetsX[] = {32, 288, 32, 288, 288, 288};
static const int goalOffsetsY[] = {95, 95, 117, 117, 73, 135};

static image_button imageButtonBackToSelection = {
	0, 0, 31, 20, IB_NORMAL, 90, 8, briefingBack, button_none, 0, 0, 1
};
static image_button imageButtonStartMission = {
	0, 0, 27, 27, IB_NORMAL, 92, 56, startMission, button_none, 1, 0, 1
};

static int focusButton = 0;

void UI_MissionStart_show()
{
	int select = 1;
	if (window_is(Window_MissionSelection)) {
		select = 0;
	}
	if (!game_mission_has_choice()) {
		select = 0;
	}
	if (select) {
		UI_Window_goTo(Window_MissionSelection);
	} else {
		UI_Intermezzo_show(Intermezzo_MissionBriefing, Window_MissionBriefingInitial, 1000);
		Data_State.missionBriefingShown = 0;
	}
}

void UI_MissionStart_Briefing_init()
{
	rich_text_reset(0);
}

void UI_MissionStart_Briefing_drawBackground()
{
	if (!Data_State.missionBriefingShown) {
		Data_State.missionBriefingShown = 1;
		if (!game_file_start_scenario(scenario_name())) {
            window_city_show();
            return;
		}
	}
	
	graphics_in_dialog();
	int textId = 200 + scenario_campaign_mission();
	
	outer_panel_draw(16, 32, 38, 27);
	text_draw(lang_get_message(textId)->title.text, 32, 48, FONT_LARGE_BLACK, 0);
	text_draw(lang_get_message(textId)->subtitle.text, 32, 78, FONT_NORMAL_BLACK, 0);

	lang_text_draw(62, 7, 376, 433, FONT_NORMAL_BLACK);
	if (window_is(Window_MissionBriefingInitial) && game_mission_has_choice()) {
		lang_text_draw(13, 4, 66, 435, FONT_NORMAL_BLACK);
	}
	
	inner_panel_draw(32, 96, 33, 5);
	lang_text_draw(62, 10, 48, 104, FONT_NORMAL_WHITE);
	int goalIndex = 0;
	if (scenario_criteria_population_enabled()) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		label_draw(16 + x, 32 + y, 15, 1);
		int width = lang_text_draw(62, 11, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
		text_draw_number(scenario_criteria_population(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
	}
	if (scenario_criteria_culture_enabled()) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		label_draw(16 + x, 32 + y, 15, 1);
		int width = lang_text_draw(62, 12, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
		text_draw_number(scenario_criteria_culture(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
	}
	if (scenario_criteria_prosperity_enabled()) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		label_draw(16 + x, 32 + y, 15, 1);
		int width = lang_text_draw(62, 13, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
		text_draw_number(scenario_criteria_prosperity(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
	}
	if (scenario_criteria_peace_enabled()) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		label_draw(16 + x, 32 + y, 15, 1);
		int width = lang_text_draw(62, 14, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
		text_draw_number(scenario_criteria_peace(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
	}
	if (scenario_criteria_favor_enabled()) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		label_draw(16 + x, 32 + y, 15, 1);
		int width = lang_text_draw(62, 15, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
		text_draw_number(scenario_criteria_favor(), '@', " ", 16 + x + 8 + width, 32 + y + 3, FONT_NORMAL_RED);
	}
	int immediateGoalText = tutorial_get_immediate_goal_text();
	if (immediateGoalText) {
		int x = goalOffsetsX[2];
		int y = goalOffsetsY[2];
		goalIndex++;
		label_draw(16 + x, 32 + y, 31, 1);
		lang_text_draw(62, immediateGoalText, 16 + x + 8, 32 + y + 3, FONT_NORMAL_RED);
	}
	
	inner_panel_draw(32, 184, 33, 15);
	
	rich_text_set_fonts(FONT_NORMAL_WHITE, FONT_NORMAL_RED);
	rich_text_init(lang_get_message(textId)->content.text, 64, 184, 31, 15, 0);

	graphics_set_clip_rectangle(35, 187, 522, 234);
	rich_text_draw(lang_get_message(textId)->content.text, 48, 196, 496, 14, 0);
	graphics_reset_clip_rectangle();

    graphics_reset_dialog();
}

void UI_MissionStart_BriefingInitial_drawForeground()
{
    graphics_in_dialog();

	rich_text_draw_scrollbar();
	image_buttons_draw(516, 426, &imageButtonStartMission, 1);
	if (game_mission_has_choice()) {
		image_buttons_draw(26, 428, &imageButtonBackToSelection, 1);
	}

	graphics_reset_dialog();
}

void UI_MissionStart_BriefingReview_drawForeground()
{
    graphics_in_dialog();

	rich_text_draw_scrollbar();
	image_buttons_draw(516, 426, &imageButtonStartMission, 1);

    graphics_reset_dialog();
}

void UI_MissionStart_BriefingInitial_handleMouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);

	if (image_buttons_handle_mouse(m_dialog, 516, 426, &imageButtonStartMission, 1, 0)) {
		return;
	}
	if (game_mission_has_choice()) {
		if (image_buttons_handle_mouse(m_dialog, 26, 428, &imageButtonBackToSelection, 1, 0)) {
			return;
		}
	}
	rich_text_handle_mouse(m_dialog);
}

void UI_MissionStart_BriefingReview_handleMouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);

	if (image_buttons_handle_mouse(m_dialog, 516, 426, &imageButtonStartMission, 1, 0)) {
		return;
	}
	rich_text_handle_mouse(m);
}

static void briefingBack(int param1, int param2)
{
	if (window_is(Window_MissionBriefingInitial)) {
		sound_speech_stop();
		window_mission_selection_show();
	}
}

static void startMission(int param1, int param2)
{
    if (window_is(Window_NewCareerDialog) || window_is(Window_MissionSelection)) {
        UI_MissionStart_show();
    } else {
        sound_speech_stop();
        sound_music_reset();
        window_city_show();
        Data_CityInfo.missionSavedGameWritten = 0;
    }
    window_invalidate();
}
