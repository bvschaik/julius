#include "AllWindows.h"
#include "CityBuildings.h"
#include "Sidebar.h"
#include "TopMenu.h"
#include "Window.h"

#include "../Graphics.h"
#include "../PlayerMessage.h"
#include "../Widget.h"

#include "../Data/CityInfo.h"
#include "../Data/CityView.h"
#include "../Data/Constants.h"
#include "../Data/Event.h"
#include "../Data/Graphics.h"
#include "../Data/Scenario.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

void UI_City_drawBackground()
{
	//Graphics_clearScreen();
	UI_Sidebar_drawBackground();
	UI_TopMenu_drawBackground();
}

void UI_City_drawForeground()
{
	UI_City_drawCity();
	UI_Sidebar_drawForeground();
	UI_City_drawPausedAndTimeLeft();
	UI_CityBuildings_drawBuildingCost();
	PlayerMessage_processQueue();
}

void UI_City_drawForegroundMilitary()
{
	UI_City_drawCity();
	UI_Sidebar_drawMinimap(0);
	UI_City_drawPausedAndTimeLeft();
}

void UI_City_drawCity()
{
	UI_CityBuildings_drawForeground(
		Data_Settings_Map.camera.x, Data_Settings_Map.camera.y);
}

void UI_City_drawPausedAndTimeLeft()
{
	if (Data_Scenario.winCriteria.timeLimitYearsEnabled) {
		int years;
		if (Data_Event.timeLimitMaxGameYear <= Data_CityInfo_Extra.gameTimeYear + 1) {
			years = 0;
		} else {
			years = Data_Event.timeLimitMaxGameYear - Data_CityInfo_Extra.gameTimeYear - 1;
		}
		int totalMonths = 12 - Data_CityInfo_Extra.gameTimeMonth + 12 * years;
		Widget_Panel_drawSmallLabelButton(6, 1, 25, 15, 1, 1);
		int width = Widget_GameText_draw(6, 2, 6, 29, Font_NormalBlack);
		Widget_Text_drawNumber(totalMonths, '@', " ", 6 + width, 29, Font_NormalBlack);
	} else if (Data_Scenario.winCriteria.survivalYearsEnabled) {
		int years;
		if (Data_Event.timeLimitMaxGameYear <= Data_CityInfo_Extra.gameTimeYear + 1) {
			years = 0;
		} else {
			years = Data_Event.timeLimitMaxGameYear - Data_CityInfo_Extra.gameTimeYear - 1;
		}
		int totalMonths = 12 - Data_CityInfo_Extra.gameTimeMonth + 12 * years;
		Widget_Panel_drawSmallLabelButton(6, 1, 25, 15, 1, 1);
		int width = Widget_GameText_draw(6, 3, 6, 29, Font_NormalBlack);
		Widget_Text_drawNumber(totalMonths, '@', " ", 6 + width, 29, Font_NormalBlack);
	}
	if (Data_Settings.gamePaused) {
		int width = Data_CityView.widthInPixels;
		Widget_Panel_drawOuterPanel((width - 448) / 2, 40, 28, 3);
		Widget_GameText_drawCentered(13, 2,
			(width - 448) / 2, 58, 448, Font_NormalBlack);
	}
}

void UI_City_handleMouse()
{
	if (UI_TopMenu_handleMouseWidget()) {
		return;
	}
	if (UI_Sidebar_handleMouse()) {
		return;
	}
	UI_CityBuildings_handleMouse();
}
/*
void __cdecl fun_drawCityScreenForeground()
{
  imagebuttons_redraw = 1;
  {
    if ( sidepanel_collapsed )                  // sidepanel background image
      j_fun_drawGraphic(graphic_sidepanel, cityscreen_width_withoutControlpanel, 24);
    else
      j_fun_drawGraphic(graphic_sidepanel + 1, cityscreen_width_withControlpanel, 24);
    j_fun_drawCitySidepanelButtons();
    j_fun_drawCitySidepanelOverlayButtonText(1, cityscreen_width_withControlpanel + 4);
    j_fun_drawCitySidepanelBuildingGraphic(0, cityscreen_width_withControlpanel + 6);
    j_fun_drawCitySidepanelMinimap(1);
    if ( screen_width == 1024 )                 // relief image below panel buttons
    {
      if ( sidepanel_collapsed )
      {
        if ( sidepanel_collapsed == 1 )
          j_fun_drawGraphic(graphic_sidepanel + 5, cityscreen_width_withoutControlpanel, 474);
      }
      else
      {
        j_fun_drawGraphic(graphic_sidepanel + 4, cityscreen_width_withControlpanel, 474);
      }
    }
    else
    {
      if ( screen_width == 800 )                // relief image below panel buttons
      {
        if ( sidepanel_collapsed )
        {
          if ( sidepanel_collapsed == 1 )
            j_fun_drawGraphic(graphic_sidepanel + 3, cityscreen_width_withoutControlpanel, 474);
        }
        else
        {
          j_fun_drawGraphic(graphic_sidepanel + 2, cityscreen_width_withControlpanel, 474);
        }
      }
    }
  }
  j_fun_drawTopMenu(1);
  j_fun_drawCitySidepanelNumMessages(1);
}
*/

void UI_City_handleMouseMilitary()
{
	UI_CityBuildings_handleMouseMilitary();
}
