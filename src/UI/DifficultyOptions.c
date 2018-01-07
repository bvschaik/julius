
#include "Window.h"
#include "../Widget.h"
#include "../Data/Screen.h"

#include "game/settings.h"
#include "graphics/arrow_button.h"

static void arrowButtonDifficulty(int param1, int param2);
static void arrowButtonGods(int param1, int param2);

static arrow_button arrowButtons[] = {
	{0, 54, 15, 24, arrowButtonDifficulty, 0, 0},
	{24, 54, 17, 24, arrowButtonDifficulty, 1, 0},
	{0, 102, 21, 24, arrowButtonGods, 2, 0}
};

void UI_DifficultyOptions_drawForeground()
{
	int baseOffsetX = Data_Screen.offset640x480.x;
	int baseOffsetY = Data_Screen.offset640x480.y;
	
	Widget_Panel_drawOuterPanel(
		baseOffsetX + 48, baseOffsetY + 80,
		24, 12
	);
	
	Widget_GameText_drawCentered(153, 0,
		baseOffsetX + 48, baseOffsetY + 94,
		384, FONT_LARGE_BLACK
	);
	
	Widget_GameText_drawCentered(153, setting_difficulty() + 1,
		baseOffsetX + 80, baseOffsetY + 142,
		224, FONT_NORMAL_BLACK
	);
	Widget_GameText_drawCentered(
		153, setting_gods_enabled() ? 7 : 6,
		baseOffsetX + 80, baseOffsetY + 190,
		224, FONT_NORMAL_BLACK
	);
	arrow_buttons_draw(
		baseOffsetX + 288, baseOffsetY + 80,
		arrowButtons, 3
	);
	Widget_GameText_drawCentered(153, 8,
		baseOffsetX + 48, baseOffsetY + 246,
		384, FONT_NORMAL_BLACK
	);
}

void UI_DifficultyOptions_handleMouse(const mouse *m)
{
	if (m->right.went_up) {
		// cancel dialog
		UI_Window_goTo(Window_City);
	} else {
		arrow_buttons_handle_mouse(mouse_in_dialog(m), 288, 80, arrowButtons, 4);
	}
}

static void arrowButtonDifficulty(int param1, int param2)
{
    if (param1) {
        setting_decrease_difficulty();
    } else {
        setting_increase_difficulty();
    }
	UI_Window_requestRefresh();
}

static void arrowButtonGods(int param1, int param2)
{
    setting_toggle_gods_enabled();
	UI_Window_requestRefresh();
}

