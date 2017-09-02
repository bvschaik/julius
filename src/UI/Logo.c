#include "AllWindows.h"

#include "../Graphics.h"
#include "../Sound.h"
#include "../Widget.h"

#include "../Data/Constants.h"
#include "../Data/Screen.h"
#include "../Data/Mouse.h"
#include "../Data/Types.h"

#include "graphics/image.h"

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
		320, Font_NormalPlain, COLOR_WHITE);
}

void UI_Logo_handleMouse()
{
	if (Data_Mouse.left.wentUp || Data_Mouse.right.wentUp) {
		UI_Window_goTo(Window_MainMenu);
	}
}
