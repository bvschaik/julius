#include "AllWindows.h"
#include "Window.h"
#include "TopMenu.h"
#include "../Graphics.h"
#include "../Data/Constants.h"
#include "../Data/Graphics.h"
#include "../Data/Screen.h"

static void drawFillerBorders();
static void drawSidebar();

// TODO move to sidebar
static int sidebar_collapsed = 0;

void UI_City_drawBackground()
{
	Graphics_clearScreen();
	drawSidebar();
	drawFillerBorders();
	UI_TopMenu_drawBackground();
}

static void drawSidebar()
{
	int graphicBase = GraphicId(ID_Graphic_SidePanel);
	// TODO move offset calculation to Data_Screen var
	int xOffsetPanel = Data_Screen.width - (Data_Screen.width - 20) % 60;
	if (sidebar_collapsed) {
		xOffsetPanel -= 42;
		Graphics_drawImage(graphicBase, xOffsetPanel, 24);
	} else {
		xOffsetPanel -= 162;
		Graphics_drawImage(graphicBase + 1, xOffsetPanel, 24);
	}
    //j_fun_drawCitySidepanelButtons();
    //j_fun_drawCitySidepanelOverlayButtonText(1, cityscreen_width_withControlpanel + 4);
    //j_fun_drawCitySidepanelBuildingGraphic(0, cityscreen_width_withControlpanel + 6);
    //j_fun_drawCitySidepanelMinimap(1);
	//j_fun_drawCitySidepanelNumMessages(1);

	// relief images below panel
	int yOffset = 474;
	while (Data_Screen.width - yOffset > 0) {
		if (Data_Screen.width - yOffset <= 120) {
			Graphics_drawImage(graphicBase + 2 + sidebar_collapsed, xOffsetPanel, yOffset);
			yOffset += 120;
		} else {
			Graphics_drawImage(graphicBase + 4 + sidebar_collapsed, xOffsetPanel, yOffset);
			yOffset += 285;
		}
	}
}

static void drawFillerBorders()
{
	int borderRightWidth = (Data_Screen.width - 20) % 60;
	if (borderRightWidth) {
		int graphicId = GraphicId(ID_Graphic_TopMenuSidebar) + 13;
		if (borderRightWidth > 24) {
			// larger border
			graphicId -= 1;
		}
		int xOffset = Data_Screen.width - borderRightWidth;
		for (int yOffset = 24; yOffset < Data_Screen.height; yOffset += 24) {
			Graphics_drawImage(graphicId, xOffset, yOffset);
		}
	}

	int borderBottomHeight = (Data_Screen.height - 24) % 15;
	Graphics_fillRect(0, Data_Screen.height - borderBottomHeight, Data_Screen.width, borderBottomHeight, Color_Orange);
}


void UI_City_handleMouse()
{
	UI_TopMenu_handleMouse();
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
