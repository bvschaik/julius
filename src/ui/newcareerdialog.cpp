#include "graphics.h"

#include <data>
#include <ui>
#include <game>
#include <scenario>
#include <input>

#include <cstring>

static void startMission(int param1, int param2);

static ImageButton imageButtonStartMission =
{
    0, 0, 27, 27, ImageButton_Normal, 92, 56, startMission, Widget::Button::doNothing, 1, 0, 1
};

static uint8_t player_name[32];

void UI_NewCareerDialog_init()
{
    setting_clear_personal_savings();
    scenario_settings_init();

    std::strncpy(player_name, scenario_player_name(), 32);
    keyboard_start_capture(player_name, 32, 1, 280, FONT_NORMAL_WHITE);
}

void UI_NewCareerDialog_drawBackground()
{
    Graphics_clearScreen();
    Graphics_drawImage(image_group(GROUP_MAIN_MENU_BACKGROUND),
                       Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
}

void UI_NewCareerDialog_drawForeground()
{
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    Widget::Panel::drawOuterPanel(xOffset + 128, yOffset + 160, 24, 8);
    Widget_GameText_drawCentered(31, 0, xOffset + 128, yOffset + 172, 384, FONT_LARGE_BLACK);
    Widget_GameText_draw(13, 5, xOffset + 352, yOffset + 256, FONT_NORMAL_BLACK);
    Widget::Panel::drawInnerPanel(xOffset + 160, yOffset + 208, 20, 2);
    Widget::Text::captureCursor(keyboard_cursor_position());
    Widget::Text::draw(player_name, xOffset + 176, yOffset + 216, FONT_NORMAL_WHITE, 0);
    Widget::Text::drawCursor(xOffset + 176, yOffset + 217, keyboard_is_insert());

    Widget::Button::drawImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1);
}

void UI_NewCareerDialog_handleMouse(const mouse *m)
{
    if (m->right.went_up)
    {
        keyboard_stop_capture();
        UI_Window_goTo(Window_MainMenu);
    }

    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    if (Widget::Button::handleImageButtons(xOffset + 464, yOffset + 249, &imageButtonStartMission, 1, 0))
    {
        return;
    }
    if (keyboard_input_is_accepted())
    {
        startMission(0, 0);
    }
}

static void startMission(int param1, int param2)
{
    keyboard_stop_capture();
    scenario_set_player_name(player_name);
    UI_MissionStart_show();
}
