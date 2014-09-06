
#include "AllWindows.h"
#include "Window.h"

#include "../Graphics.h"
#include "../Scenario.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/FileList.h"
#include "../Data/Language.h"
#include "../Data/Mouse.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"
#include "../Data/State.h"
#include "../Data/Tutorial.h"

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

static ImageButton imageButtonStartMission = {
	0, 0, 27, 27, 4, 92, 56, startMission, Widget_Button_doNothing, 1, 0, 0, 0, 1, 0
};
static ImageButton imageButtonBackToSelection = {
	0, 0, 31, 20, 4, 90, 8, briefingBack, Widget_Button_doNothing, 1, 0, 0, 0, 0, 0
};

static struct {
	int choice;
} data;

void UI_MissionStart_show()
{
	int select = 1;
	if (UI_Window_getId() == Window_MissionSelection) {
		select = 0;
	}
	if (!campaignHasChoice[Data_Settings.currentMissionId]) {
		select = 0;
	}
	if (select) {
		data.choice = 0;
		UI_Window_goTo(Window_MissionSelection);
	} else {
		UI_Intermezzo_show(0, Window_MissionBriefingInitial, 1000);
		Data_State.missionBriefingShown = 0;
	}
}

void UI_MissionStart_Selection_drawBackground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	int missionId = Data_Settings.currentMissionId;
	
	Graphics_drawFullScreenImage(GraphicId(ID_Graphic_SelectMissionBackground));
	Graphics_drawImage(GraphicId(ID_Graphic_SelectMission) +
		backgroundGraphicOffset[missionId],
		xOffset, yOffset);
	Widget_GameText_draw(144, 1 + 3 * missionId, xOffset + 20, yOffset + 410, Font_LargeBlack);
	if (data.choice) {
		Widget_GameText_drawMultiline(144, 1 + 3 * missionId + data.choice,
			xOffset + 20, yOffset + 440, 560, Font_NormalBlack);
	} else {
		Widget_GameText_drawMultiline(144, 0,
			xOffset + 20, yOffset + 440, 560, Font_NormalBlack);
	}
}

static int isMouseHit(int x, int y, int size)
{
	return
		x <= Data_Mouse.x && Data_Mouse.x < x + size &&
		y <= Data_Mouse.y && Data_Mouse.y < y + size;
}

void UI_MissionStart_Selection_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;

	if (data.choice > 0) {
		Widget_Button_drawImageButtons(xOffset + 580, yOffset + 410, &imageButtonStartMission, 1);
	}

	int missionId = Data_Settings.currentMissionId;
	int xPeaceful = xOffset + campaignSelection[missionId].xPeaceful - 4;
	int yPeaceful = yOffset + campaignSelection[missionId].yPeaceful - 4;
	int xMilitary = xOffset + campaignSelection[missionId].xMilitary - 4;
	int yMilitary = yOffset + campaignSelection[missionId].yMilitary - 4;
	int selected = 0;
	if (isMouseHit(xPeaceful, yPeaceful, 44)) {
		selected = 1;
	}
	if (isMouseHit(xMilitary, yMilitary, 44)) {
		selected = 2;
	}
	int graphicId = GraphicId(ID_Graphic_SelectMissionButton);
	if (data.choice == 0) {
		Graphics_drawImage(selected == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
		Graphics_drawImage(selected == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
	} else if (data.choice == 1) {
		Graphics_drawImage(selected == 1 ? graphicId + 1 : graphicId + 2, xPeaceful, yPeaceful);
		Graphics_drawImage(selected == 2 ? graphicId + 1 : graphicId, xMilitary, yMilitary);
	} else {
		Graphics_drawImage(selected == 1 ? graphicId + 1 : graphicId, xPeaceful, yPeaceful);
		Graphics_drawImage(selected == 2 ? graphicId + 1 : graphicId + 2, xMilitary, yMilitary);
	}
}

void UI_MissionStart_Selection_handleMouse()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;

	if (Data_Mouse.right.wentUp) {
		UI_MissionStart_show();
	}
	if (data.choice > 0) {
		if (Widget_Button_handleImageButtons(xOffset + 580, yOffset + 410, &imageButtonStartMission, 1)) {
			return;
		}
	}
	if (Data_Mouse.left.wentUp) {
		int missionId = Data_Settings.currentMissionId;
		int xPeaceful = xOffset + campaignSelection[missionId].xPeaceful - 4;
		int yPeaceful = yOffset + campaignSelection[missionId].yPeaceful - 4;
		int xMilitary = xOffset + campaignSelection[missionId].xMilitary - 4;
		int yMilitary = yOffset + campaignSelection[missionId].yMilitary - 4;

		if (isMouseHit(xPeaceful, yPeaceful, 44)) {
			Data_Settings.saveGameMissionId = Constant_MissionIds[missionId].peaceful;
			data.choice = 1;
			UI_Window_requestRefresh();
			Sound_Speech_playFile("wavs/fanfare_nu1.wav");
		}
		if (isMouseHit(xMilitary, yMilitary, 44)) {
			Data_Settings.saveGameMissionId = Constant_MissionIds[missionId].military;
			data.choice = 2;
			UI_Window_requestRefresh();
			Sound_Speech_playFile("wavs/fanfare_nu5.wav");
		}
	}
}

void UI_MissionStart_Briefing_init()
{
	Widget_RichText_reset(0);
}

void UI_MissionStart_Briefing_drawBackground()
{
	if (!Data_State.missionBriefingShown) {
		Data_State.missionBriefingShown = 1;
		Scenario_initialize(Data_FileList.selectedScenario);
		if (UI_Window_getId() == Window_City) {
			return;
		}
	}
	
	int textId = 200 + Data_Settings.saveGameMissionId;
	int xOffset = Data_Screen.offset640x480.x + 16;
	int yOffset = Data_Screen.offset640x480.y + 32;
	
	Widget_Panel_drawOuterPanel(xOffset, yOffset, 38, 27);
	Widget_Text_draw(MessageText(textId, titleOffset), xOffset + 16, yOffset + 16, Font_LargeBlack, 0);
	Widget_GameText_draw(62, 7, xOffset + 360, yOffset + 401, Font_NormalBlack);
	if (UI_Window_getId() == Window_MissionBriefingInitial && Data_Settings.currentMissionId >= 2) {
		Widget_GameText_draw(13, 4, xOffset + 50, yOffset + 403, Font_NormalBlack);
	}
	
	Widget_Panel_drawInnerPanel(xOffset + 16, yOffset + 64, 33, 5);
	Widget_GameText_draw(62, 10, xOffset + 32, yOffset + 72, Font_NormalWhite);
	int goalIndex = 0;
	if (Data_Scenario.winCriteria_populationEnabled) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 15, 0, 1);
		int width = Widget_GameText_draw(62, 11, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
		Widget_Text_drawNumber(Data_Scenario.winCriteria_population, '@', " ",
			xOffset + x + 8 + width, yOffset + y + 3, Font_NormalRed);
	}
	if (Data_Scenario.winCriteria.cultureEnabled) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 15, 0, 1);
		int width = Widget_GameText_draw(62, 12, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
		Widget_Text_drawNumber(Data_Scenario.winCriteria.culture, '@', " ",
			xOffset + x + 8 + width, yOffset + y + 3, Font_NormalRed);
	}
	if (Data_Scenario.winCriteria.prosperityEnabled) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 15, 0, 1);
		int width = Widget_GameText_draw(62, 13, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
		Widget_Text_drawNumber(Data_Scenario.winCriteria.prosperity, '@', " ",
			xOffset + x + 8 + width, yOffset + y + 3, Font_NormalRed);
	}
	if (Data_Scenario.winCriteria.peaceEnabled) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 15, 0, 1);
		int width = Widget_GameText_draw(62, 14, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
		Widget_Text_drawNumber(Data_Scenario.winCriteria.peace, '@', " ",
			xOffset + x + 8 + width, yOffset + y + 3, Font_NormalRed);
	}
	if (Data_Scenario.winCriteria.favorEnabled) {
		int x = goalOffsetsX[goalIndex];
		int y = goalOffsetsY[goalIndex];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 15, 0, 1);
		int width = Widget_GameText_draw(62, 15, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
		Widget_Text_drawNumber(Data_Scenario.winCriteria.favor, '@', " ",
			xOffset + x + 8 + width, yOffset + y + 3, Font_NormalRed);
	}
	if (IsTutorial1()) {
		int x = goalOffsetsX[2];
		int y = goalOffsetsY[2];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 31, 0, 1);
		int text;
		if (!Data_Tutorial.tutorial1.fire && !Data_Tutorial.tutorial1.crime) {
			text = 17;
		} else if (!Data_Tutorial.tutorial1.collapse) {
			text = 18;
		} else if (!Data_Tutorial.tutorial1.senateBuilt) {
			text = 19;
		} else {
			text = 20;
		}
		Widget_GameText_draw(62, text, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
	} else if (IsTutorial2()) {
		int x = goalOffsetsX[2];
		int y = goalOffsetsY[2];
		goalIndex++;
		Widget_Panel_drawSmallLabelButton(6, xOffset + x, yOffset + y, 31, 0, 1);
		int text;
		if (!Data_Tutorial.tutorial2.granaryBuilt) {
			text = 21;
		} else if (!Data_Tutorial.tutorial2.population250Reached) {
			text = 22;
		} else if (!Data_Tutorial.tutorial2.population450Reached) {
			text = 23;
		} else if (!Data_Tutorial.tutorial2.potteryMade) {
			text = 24;
		} else {
			text = 25;
		}
		Widget_GameText_draw(62, text, xOffset + x + 8, yOffset + y + 3, Font_NormalRed);
	}
	
	Widget_Panel_drawInnerPanel(xOffset + 16, yOffset + 152, 33, 15);
	
	Widget_RichText_setFonts(Font_NormalWhite, Font_NormalRed);
	Widget_RichText_init(MessageText(textId, contentOffset),
		xOffset + 48, yOffset + 152, 31, 15, 0);

	Graphics_setClipRectangle(xOffset + 19, yOffset + 155, 522, 234);
	Widget_RichText_draw(MessageText(textId, contentOffset),
		xOffset + 32, yOffset + 164, 496, 14, 0);
	Graphics_resetClipRectangle();
}

void UI_MissionStart_BriefingInitial_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x + 16;
	int yOffset = Data_Screen.offset640x480.y + 32;

	Widget_RichText_drawScrollbar();
	Widget_Button_drawImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1);
	if (Data_Settings.currentMissionId >= 2) {
		Widget_Button_drawImageButtons(xOffset + 10, yOffset + 396, &imageButtonBackToSelection, 1);
	}
}

void UI_MissionStart_BriefingReview_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x + 16;
	int yOffset = Data_Screen.offset640x480.y + 32;

	Widget_RichText_drawScrollbar();
	Widget_Button_drawImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1);
}

static void handleBriefingScroll()
{
	Widget_RichText_handleScrollbar();

	if (Data_Mouse.scrollDown) {
		Widget_RichText_scroll(1, 3);
	} else if (Data_Mouse.scrollUp) {
		Widget_RichText_scroll(0, 3);
	}
}

void UI_MissionStart_BriefingInitial_handleMouse()
{
	int xOffset = Data_Screen.offset640x480.x + 16;
	int yOffset = Data_Screen.offset640x480.y + 32;

	if (Widget_Button_handleImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1)) {
		return;
	}
	if (Data_Settings.currentMissionId >= 2) {
		if (Widget_Button_handleImageButtons(xOffset + 10, yOffset + 396, &imageButtonBackToSelection, 1)) {
			return;
		}
	}
	handleBriefingScroll();
}

void UI_MissionStart_BriefingReview_handleMouse()
{
	int xOffset = Data_Screen.offset640x480.x + 16;
	int yOffset = Data_Screen.offset640x480.y + 32;

	if (Widget_Button_handleImageButtons(xOffset + 500, yOffset + 394, &imageButtonStartMission, 1)) {
		return;
	}
	handleBriefingScroll();
}

static void briefingBack(int param1, int param2)
{
	if (UI_Window_getId() == Window_MissionBriefingInitial) {
		Sound_stopSpeech();
		UI_Window_goTo(Window_MissionSelection);
	}
}

static void startMission(int param1, int param2)
{
	// TODO mouse over buttons??
	// TODO dword_9D7B90??
	Data_State.previousOverlay = 0;
	switch (UI_Window_getId()) {
		case Window_NewCareerDialog:
		case Window_MissionSelection:
			UI_MissionStart_show();
			break;
		default:
			Sound_stopSpeech();
			Sound_Music_reset();
			UI_Window_goTo(Window_City);
			Data_CityInfo.missionSavedGameWritten = 0;
			break;
	}
	UI_Window_requestRefresh();
}
