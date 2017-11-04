#include "AllWindows.hpp"

#include "../Graphics.hpp"
#include "../Sound.hpp"
#include "../Widget.hpp"

#include "../Data/Constants.hpp"
#include "../Data/Screen.hpp"

#include "graphics/image.hpp"

void UI_Logo_init()
{
    Sound_Music_playIntro();
}

void UI_Logo_drawBackground()
{
    Graphics_clearScreen();
    Graphics_drawImage(image_group(ID_Graphic_Logo), Data_Screen.offset640x480.x, Data_Screen.offset640x480.y);
    Widget_GameText_drawCenteredColored(13, 7,
                                        (Data_Screen.width - 320) / 2, Data_Screen.offset640x480.y + 462,
                                        320, FONT_NORMAL_PLAIN, COLOR_WHITE);
}

void UI_Logo_handleMouse(const mouse *m)
{
    if (m->left.went_up || m->right.went_up)
    {
        UI_Window_goTo(Window_MainMenu);
    }
}
