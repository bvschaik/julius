#include "AllWindows.h"
#include "CityBuildings.h"
#include "Window.h"
#include "TopMenu.h"
#include "Sidebar.h"
#include "../Graphics.h"
#include "../Data/CityInfo.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Screen.h"
#include "../Data/State.h"
#include "../Data/Settings.h"

void UI_City_drawBackground()
{
	Graphics_clearScreen();
	UI_Sidebar_drawBackground();
	UI_TopMenu_drawBackground();
}

void UI_City_drawForeground()
{
	UI_City_drawCity();
}

void UI_City_drawCity()
{
	UI_CityBuildings_drawForeground(
		Data_Settings_Map.camera.x, Data_Settings_Map.camera.y);
}

void UI_City_handleMouse()
{
	UI_TopMenu_handleMouse();
	UI_Sidebar_handleMouse();
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
