#include "window.h"
#include "allwindows.h"
#include "widget.h"
#include "graphics.h"
#include "keyboardinput.h"
#include "game/game_settings.h"

#include "data/constants.hpp"
#include "data/keyboardinput.hpp"
#include "data/screen.hpp"
#include "data/settings.hpp"

static void startMission(int param1, int param2);

static ImageButton imageButtonStartMission =
{
    0, 0, 27, 27, ImageButton_Normal, 92, 56, startMission, Widget_Button_doNothing, 1, 0, 1
};

void UI_NewCareerDialog_init()
{
    Settings_clearMissionSettings();
}

void UI_NewCareerDialog_drawBackground()
{
    Graphics_clearScreen();
    Graphics_drawImage(image_group(ID_Graphic_MainMenuBackground),
                       Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
}

void UI_NewCareerDialog_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    Widget_Panel_drawOuterPanel(xOffset + 128, yOffset + 160, 24, 8);
    Widget_GameText_drawCentered(31, 0, xOffset + 128, yOffset + 172, 384, FONT_LARGE_BLACK);
    Widget_GameText_draw(13, 5, xOffset + 352, yOffset + 256, FONT_NORMAL_BLACK);
    Widget_Panel_drawInnerPanel(xOffset + 160, yOffset + 208, 20, 2);
    Widget_Text_captureCursor();
    Widget_Text_draw(Data_Settings.playerName, xOffset + 176, yOffset + 216, FONT_NORMAL_WHITE, 0);
    Widget_Text_drawCursor(xOffset + 176, yOffset + 217);

    Widget_Button_drawImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1);
}

void UI_NewCareerDialog_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        UI_Window_goTo(Window_MainMenu);
    }

    KeyboardInput_initInput(1);

    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    if (Widget_Button_handleImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1, 0))
    {
        return;
    }
    if (Data_KeyboardInput.accepted)
    {
        startMission(0, 0);
    }
}

static void startMission(int param1, int param2)
{
    UI_MissionStart_show();
}
