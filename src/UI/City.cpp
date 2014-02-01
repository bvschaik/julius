#include "AllWindows.h"
#include "Window.h"
#include "TopMenu.h"
#include "../Graphics.h"

void UI_City_drawBackground()
{
	Graphics_clearScreen();
	UI_TopMenu_draw();

}

/*
void __cdecl fun_drawCityScreenForeground()
{
  if ( screen_width == 1024 )
  {
    j_fun_drawCitySidepanelBorder_1024();
  }
  else
  {
    if ( screen_width == 800 )
      j_fun_drawCitySidepanelBorder_800();
  }
  if ( screen_width == 1024 )                   // black border at bottom
    j_fun_fillRect(0, screen_height - 9, screen_width, 9, 0);
  else
    j_fun_fillRect(0, screen_height - 6, screen_width, 6, 0);
  imagebuttons_redraw = 1;
  if ( mode_editor )
  {
    sub_402AE0();
    j_fun_drawGraphic(word_6E6C50, cityscreen_width_withControlpanel, 24);
    if ( screen_width == 1024 )
    {
      j_fun_drawGraphic(word_6E6C50 + 2, cityscreen_width_withControlpanel, 474);
    }
    else
    {
      if ( screen_width == 800 )
        j_fun_drawGraphic(word_6E6C50 + 1, cityscreen_width_withControlpanel, 474);
    }
    j_fun_drawCitySidepanelEditorConsole();
    j_fun_drawImageButtonCollection(cityscreen_width_withControlpanel, 24, &imagebuttons_unknown_5EEF00, 17);
    j_fun_drawCitySidepanelMinimap(1);
  }
  else
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
