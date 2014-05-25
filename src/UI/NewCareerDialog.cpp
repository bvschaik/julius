#include "Window.h"
#include "../Widget.h"
#include "../Graphics.h"
#include "../KeyboardInput.h"
#include "../Data/Constants.h"
#include "../Data/Mouse.h"
#include "../Data/Screen.h"
#include "../Data/Settings.h"

void UI_NewCareerDialog_drawBackground()
{
	Graphics_clearScreen();
	Graphics_drawImage(GraphicId(ID_Graphic_MainMenuBackground),
		Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
}

void UI_NewCareerDialog_drawForeground()
{
	int xOffset = Data_Screen.offset640x480.x;
	int yOffset = Data_Screen.offset640x480.y;
	Widget_Panel_drawOuterPanel(xOffset + 128, yOffset + 160, 24, 8);
	Widget_GameText_drawCentered(31, 0, xOffset + 128, yOffset + 172, 384, Font_LargeBlack);
	Widget_GameText_draw(13, 5, xOffset + 352, yOffset + 256, Font_NormalBlack);
	Widget_Panel_drawInnerPanel(xOffset + 160, yOffset + 208, 20, 2);
	Widget_Text_captureCursor();
	Widget_Text_draw(Data_Settings.playerName, xOffset + 176, yOffset + 216, Font_NormalWhite, 0);
	Widget_Text_drawCursor(xOffset + 176, yOffset + 217);
}

void UI_NewCareerDialog_handleMouse()
{
	if (Data_Mouse.right.wentUp) {
		UI_Window_goTo(Window_MainMenu);
	}
	/*
        KeyboardInput_initInput(1);

          if ( !j_fun_handleImageButtonClick(
                  screen_640x480_x + 464,
                  screen_640x480_y + 249,
                  &imagebuttons_newCampaign,
                  1) )
          {
            if ( input_accepted )
            {
              j_fun_showMissionBriefingOnScenarioStart();
              window_redrawRequest = 1;
            }
          }
          break;
	*/
}
