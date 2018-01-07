#include "AllWindows.h"

#include "../Widget.h"

#include "../Data/Screen.h"

#include "game/settings.h"
#include "graphics/arrow_button.h"

static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static void arrowButtonGame(int param1, int param2);
static void arrowButtonScroll(int param1, int param2);

static CustomButton buttons[] = {
	{144, 232, 336, 252, CustomButton_Immediate, buttonOk, Widget_Button_doNothing, 1, 0},
	{144, 262, 336, 282, CustomButton_Immediate, buttonCancel, Widget_Button_doNothing, 1, 0},
};

static arrow_button arrowButtons[] = {
	{112, 100, 17, 24, arrowButtonGame, 1, 0},
	{136, 100, 15, 24, arrowButtonGame, 0, 0},
	{112, 136, 17, 24, arrowButtonScroll, 1, 0},
	{136, 136, 15, 24, arrowButtonScroll, 0, 0},
};

static int original_gameSpeed;
static int original_scrollSpeed;

static int focusButtonId;

void UI_SpeedOptions_init()
{
	original_gameSpeed = setting_game_speed();
	original_scrollSpeed = setting_scroll_speed();
}

void UI_SpeedOptions_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 96, baseOffsetY + 80,
		18, 14
	);
	
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 144, baseOffsetY + 232,
		12, focusButtonId == 1 ? 1 : 2
	);
	Widget_Panel_drawSmallLabelButton(
		baseOffsetX + 144, baseOffsetY + 262,
		12, focusButtonId == 2 ? 1 : 2
	);
	
	Widget_GameText_drawCentered(45, 0,
		baseOffsetX + 128, baseOffsetY + 92,
		224, FONT_LARGE_BLACK
	);
	Widget_GameText_drawCentered(45, 4,
		baseOffsetX + 128, baseOffsetY + 236,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_drawCentered(45, 1,
		baseOffsetX + 128, baseOffsetY + 266,
		224, FONT_NORMAL_GREEN
	);
	Widget_GameText_draw(45, 2,
		baseOffsetX + 144, baseOffsetY + 146,
		FONT_NORMAL_PLAIN
	);
	Widget_Text_drawPercentage(
		setting_game_speed(),
		baseOffsetX + 296, baseOffsetY + 146,
		FONT_NORMAL_PLAIN
	);
	Widget_GameText_draw(45, 3,
		baseOffsetX + 144, baseOffsetY + 182,
		FONT_NORMAL_PLAIN
	);
	Widget_Text_drawPercentage(
		setting_scroll_speed(),
		baseOffsetX + 296, baseOffsetY + 182,
		FONT_NORMAL_PLAIN
	);

	arrow_buttons_draw(
		baseOffsetX + 128, baseOffsetY + 40,
		arrowButtons, 4
	);
}

void UI_SpeedOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		const mouse *m_dialog = mouse_in_dialog(m);
		if (!Widget_Button_handleCustomButtons(m_dialog, buttons, 2, &focusButtonId)) {
			arrow_buttons_handle_mouse(m_dialog, 128, 40, arrowButtons, 4);
		}
	}
}

static void buttonOk(int param1, int param2)
{
	UI_Window_goTo(Window_City);
}

static void buttonCancel(int param1, int param2)
{
    setting_reset_speeds(original_gameSpeed, original_scrollSpeed);
	UI_Window_goTo(Window_City);
}

static void arrowButtonGame(int param1, int param2)
{
    if (param1) {
        setting_decrease_game_speed();
    } else {
        setting_increase_game_speed();
    }
	UI_Window_requestRefresh();
}

static void arrowButtonScroll(int param1, int param2)
{
	if (param1) {
	    setting_decrease_scroll_speed();
	} else {
	    setting_increase_scroll_speed();
	}
	UI_Window_requestRefresh();
}
