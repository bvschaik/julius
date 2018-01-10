
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

static void startMission(int param1, int param2);
static void briefingBack(int param1, int param2);

static const int backgroundGraphicOffset[] = {
	0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0
};
static const int campaignHasChoice[] = {
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
struct CampaignSelection {
	int xPeaceful;
	int yPeaceful;
	int xMilitary;
	int yMilitary;
};
static const struct CampaignSelection campaignSelection[12] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{292, 182, 353, 232},
	{118, 202, 324, 286},
	{549, 285, 224, 121},
	{173, 109, 240, 292},
	{576, 283, 19, 316},
	{97, 240, 156, 59},
	{127, 300, 579, 327},
	{103, 35, 410, 109},
	{191, 153, 86, 8},
	{200, 300, 400, 300},
};

static const int goalOffsetsX[] = {32, 288, 32, 288, 288, 288};
static const int goalOffsetsY[] = {95, 95, 117, 117, 73, 135};

static image_button imageButtonStartMission = {
	0, 0, 27, 27, IB_NORMAL, 92, 56, startMission, button_none, 1, 0, 1
};
static image_button imageButtonBackToSelection = {
	0, 0, 31, 20, IB_NORMAL, 90, 8, briefingBack, button_none, 0, 0, 1
};

static int focusButton = 0;

static struct {
	int choice;
} data;

void UI_MissionStart_show()
{
	int select = 1;
	if (window_is(Window_MissionSelection)) {
		select = 0;
	}
	if (!campaignHasChoice[scenario_campaign_rank()]) {
		select = 0;
	}
	if (select) {
		data.choice = 0;
		UI_Window_goTo(Window_MissionSelection);
	} else {
		UI_Intermezzo_show(Intermezzo_MissionBriefing, Window_MissionBriefingInitial, 1000);
		Data_State.missionBriefingShown = 0;
	}
}

void UI_MissionStart_Selection_drawBackground()
{
	int rank = scenario_campaign_rank();
	
	image_draw_fullscreen_background(image_group(GROUP_SELECT_MISSION_BACKGROUND));
    graphics_in_dialog();
	image_draw(image_group(GROUP_SELECT_MISSION) + backgroundGraphicOffset[rank], 0, 0);
	lang_text_draw(144, 1 + 3 * rank, 20, 410, FONT_LARGE_BLACK);
	if (data.choice) {
		lang_text_draw_multiline(144, 1 + 3 * rank + data.choice, 20, 440, 560, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_multiline(144, 0, 20, 440, 560, FONT_NORMAL_BLACK);
	}
	graphics_reset_dialog();
}

static int isMouseHit(const mouse *m, int x, int y, int size)
{
	return
		x <= m->x && m->x < x + size &&
		y <= m->y && m->y < y + size;
}

void UI_MissionStart_Selection_drawForeground()
{
    graphics_in_dialog();

	if (data.choice > 0) {
		image_buttons_draw(580, 410, &imageButtonStartMission, 1);
	}

	int rank = scenario_campaign_rank();
	int xPeaceful = campaignSelection[rank].xPeaceful - 4;
	int yPeaceful = campaignSelection[rank].yPeaceful - 4;
	int xMilitary = campaignSelection[rank].xMilitary - 4;
	int yMilitary = campaignSelection[rank].yMilitary - 4;
	int graphicId = image_group(GROUP_SELECT_MISSION_BUTTON);
	if (data.choice == 0) {
		image_draw(focusButton == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
		image_draw(focusButton == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
	} else if (data.choice == 1) {
		image_draw(focusButton == 1 ? graphicId + 1 : graphicId + 2, xPeaceful, yPeaceful);
		image_draw(focusButton == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
	} else {
		image_draw(focusButton == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
		image_draw(focusButton == 2 ? graphicId + 1 : graphicId + 2, xMilitary, yMilitary);
	}
	graphics_reset_dialog();
}

void UI_MissionStart_Selection_handleMouse(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);

    int rank = scenario_campaign_rank();
    int xPeaceful = campaignSelection[rank].xPeaceful - 4;
    int yPeaceful = campaignSelection[rank].yPeaceful - 4;
    int xMilitary = campaignSelection[rank].xMilitary - 4;
    int yMilitary = campaignSelection[rank].yMilitary - 4;
    focusButton = 0;
    if (isMouseHit(m_dialog, xPeaceful, yPeaceful, 44)) {
        focusButton = 1;
    }
    if (isMouseHit(m_dialog, xMilitary, yMilitary, 44)) {
        focusButton = 2;
    }

	if (m_dialog->right.went_up) {
		UI_MissionStart_show();
	}
	if (data.choice > 0) {
		if (image_buttons_handle_mouse(m_dialog, 580, 410, &imageButtonStartMission, 1, 0)) {
			return;
		}
	}
	if (m_dialog->left.went_up) {
		if (isMouseHit(m_dialog, xPeaceful, yPeaceful, 44)) {
			scenario_set_campaign_mission(game_mission_peaceful());
			data.choice = 1;
			window_invalidate();
			sound_speech_play_file("wavs/fanfare_nu1.wav");
		}
		if (isMouseHit(m_dialog, xMilitary, yMilitary, 44)) {
			scenario_set_campaign_mission(game_mission_military());
			data.choice = 2;
			window_invalidate();
			sound_speech_play_file("wavs/fanfare_nu5.wav");
		}
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
            UI_Window_goTo(Window_City);
            return;
		}
	}
	
	graphics_in_dialog();
	int textId = 200 + scenario_campaign_mission();
	
	outer_panel_draw(16, 32, 38, 27);
	text_draw(lang_get_message(textId)->title.text, 32, 48, FONT_LARGE_BLACK, 0);
	text_draw(lang_get_message(textId)->subtitle.text, 32, 78, FONT_NORMAL_BLACK, 0);

	lang_text_draw(62, 7, 376, 433, FONT_NORMAL_BLACK);
	if (window_is(Window_MissionBriefingInitial) && campaignHasChoice[scenario_campaign_rank()]) {
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
	if (campaignHasChoice[scenario_campaign_rank()]) {
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
	if (campaignHasChoice[scenario_campaign_rank()]) {
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
		UI_Window_goTo(Window_MissionSelection);
	}
}

static void startMission(int param1, int param2)
{
    if (window_is(Window_NewCareerDialog) || window_is(Window_MissionSelection)) {
        UI_MissionStart_show();
    } else {
        sound_speech_stop();
        sound_music_reset();
        UI_Window_goTo(Window_City);
        Data_CityInfo.missionSavedGameWritten = 0;
    }
    window_invalidate();
}
