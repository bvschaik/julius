#include "AllWindows.h"
#include "CityBuildings.h"
#include "Sidebar.h"
#include "TopMenu.h"

#include "../Widget.h"

#include "../Data/CityView.h"
#include "../Data/State.h"

#include "city/message.h"
#include "city/view.h"
#include "game/state.h"
#include "game/time.h"
#include "graphics/panel.h"
#include "scenario/criteria.h"

void UI_City_drawBackground()
{
	//Graphics_clearScreen();
	UI_Sidebar_drawBackground();
	UI_TopMenu_drawBackground();
}

void UI_City_drawForeground()
{
	UI_TopMenu_drawBackgroundIfNecessary();
	UI_City_drawCity();
	UI_Sidebar_drawForeground();
	UI_City_drawPausedAndTimeLeft();
	UI_CityBuildings_drawBuildingCost();
    if (UI_Window_getId() == Window_City) {
        city_message_process_queue();
    }
}

void UI_City_drawForegroundMilitary()
{
	UI_TopMenu_drawBackgroundIfNecessary();
	UI_City_drawCity();
	UI_Sidebar_drawMinimap(0);
	UI_City_drawPausedAndTimeLeft();
}

void UI_City_drawCity()
{
    int x, y;
    city_view_get_camera(&x, &y);
	UI_CityBuildings_drawForeground(x, y);
}

void UI_City_drawPausedAndTimeLeft()
{
	if (scenario_criteria_time_limit_enabled()) {
		int years;
		if (scenario_criteria_max_year() <= game_time_year() + 1) {
			years = 0;
		} else {
			years = scenario_criteria_max_year() - game_time_year() - 1;
		}
		int totalMonths = 12 - game_time_month() + 12 * years;
		label_draw(1, 25, 15, 1);
		int width = Widget_GameText_draw(6, 2, 6, 29, FONT_NORMAL_BLACK);
		Widget_Text_drawNumber(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
	} else if (scenario_criteria_survival_enabled()) {
		int years;
		if (scenario_criteria_max_year() <= game_time_year() + 1) {
			years = 0;
		} else {
			years = scenario_criteria_max_year() - game_time_year() - 1;
		}
		int totalMonths = 12 - game_time_month() + 12 * years;
		label_draw(1, 25, 15, 1);
		int width = Widget_GameText_draw(6, 3, 6, 29, FONT_NORMAL_BLACK);
		Widget_Text_drawNumber(totalMonths, '@', " ", 6 + width, 29, FONT_NORMAL_BLACK);
	}
	if (game_state_is_paused()) {
		int width = Data_CityView.widthInPixels;
		outer_panel_draw((width - 448) / 2, 40, 28, 3);
		Widget_GameText_drawCentered(13, 2,
			(width - 448) / 2, 58, 448, FONT_NORMAL_BLACK);
	}
}

void UI_City_handleMouse(const mouse *m)
{
	if (UI_TopMenu_handleMouseWidget(m)) {
		return;
	}
	if (UI_Sidebar_handleMouse(m)) {
		return;
	}
	UI_CityBuildings_handleMouse(m);
}

void UI_City_getTooltip(struct TooltipContext *c)
{
	int textId = UI_TopMenu_getTooltipText(c);
	if (!textId) {
		textId = UI_Sidebar_getTooltipText();
	}
	if (textId) {
		c->type = TooltipType_Button;
		c->textId = textId;
		return;
	}
	UI_CityBuildings_getTooltip(c);
}

void UI_City_handleMouseMilitary(const mouse *m)
{
	UI_CityBuildings_handleMouseMilitary(m);
}
