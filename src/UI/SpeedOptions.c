#include "AllWindows.h"

#include "game/settings.h"
#include "graphics/arrow_button.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"

static void buttonOk(int param1, int param2);
static void buttonCancel(int param1, int param2);

static void arrowButtonGame(int param1, int param2);
static void arrowButtonScroll(int param1, int param2);

static generic_button buttons[] = {
	{144, 232, 336, 252, GB_IMMEDIATE, buttonOk, button_none, 1, 0},
	{144, 262, 336, 282, GB_IMMEDIATE, buttonCancel, button_none, 1, 0},
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
    graphics_in_dialog();

	outer_panel_draw(96, 80, 18, 14);
	// ok/cancel labels
	label_draw(144, 232, 12, focusButtonId == 1 ? 1 : 2);
	label_draw(144, 262, 12, focusButtonId == 2 ? 1 : 2);
	
    // title
	lang_text_draw_centered(45, 0, 128, 92, 224, FONT_LARGE_BLACK);
    // ok/cancel label texts
	lang_text_draw_centered(45, 4, 128, 236, 224, FONT_NORMAL_GREEN);
	lang_text_draw_centered(45, 1, 128, 266, 224, FONT_NORMAL_GREEN);
    // game speed
	lang_text_draw(45, 2, 144, 146, FONT_NORMAL_PLAIN);
	Widget_Text_drawPercentage(setting_game_speed(), 296, 146, FONT_NORMAL_PLAIN);
    // scroll speed
	lang_text_draw(45, 3, 144, 182, FONT_NORMAL_PLAIN);
	Widget_Text_drawPercentage(setting_scroll_speed(), 296, 182,FONT_NORMAL_PLAIN);

	arrow_buttons_draw(128, 40, arrowButtons, 4);
    graphics_reset_dialog();
}

void UI_SpeedOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		const mouse *m_dialog = mouse_in_dialog(m);
		if (!generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 2, &focusButtonId)) {
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
