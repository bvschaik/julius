#include "AllWindows.h"
#include "FileDialog.h"
#include "PopupDialog.h"

#include "game/system.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"

static void buttonClick(int param1, int param2);

static int focusButtonId;

static generic_button buttons[] = {
	{192, 100, 448, 125, GB_IMMEDIATE, buttonClick, button_none, 1, 0},
	{192, 140, 448, 165, GB_IMMEDIATE, buttonClick, button_none, 2, 0},
	{192, 180, 448, 205, GB_IMMEDIATE, buttonClick, button_none, 3, 0},
	{192, 220, 448, 245, GB_IMMEDIATE, buttonClick, button_none, 4, 0},
};

void UI_MainMenu_drawBackground()
{
    Graphics_clearScreen();
    graphics_in_dialog();
    image_draw(image_group(GROUP_MAIN_MENU_BACKGROUND), 0, 0);
    graphics_reset_dialog();
}

void UI_MainMenu_drawForeground()
{
    graphics_in_dialog();

	large_label_draw(192, 100, 16, focusButtonId == 1 ? 1 : 0);
	large_label_draw(192, 140, 16, focusButtonId == 2 ? 1 : 0);
	large_label_draw(192, 180, 16, focusButtonId == 3 ? 1 : 0);
	large_label_draw(192, 220, 16, focusButtonId == 4 ? 1 : 0);

	lang_text_draw_centered(30, 1, 192, 106, 256, FONT_NORMAL_GREEN);
	lang_text_draw_centered(30, 2, 192, 146, 256, FONT_NORMAL_GREEN);
	lang_text_draw_centered(30, 3, 192, 186, 256, FONT_NORMAL_GREEN);
	lang_text_draw_centered(30, 5, 192, 226, 256, FONT_NORMAL_GREEN);

    graphics_reset_dialog();
}

void UI_MainMenu_handleMouse(const mouse *m)
{
	generic_buttons_handle_mouse(mouse_in_dialog(m), 0, 0, buttons, 4, &focusButtonId);
}

static void confirmExit(int accepted)
{
	if (accepted) {
		system_exit();
	}
}

static void buttonClick(int param1, int param2)
{
	if (param1 == 1) {
		UI_Window_goTo(Window_NewCareerDialog);
	} else if (param1 == 2) {
		UI_FileDialog_show(FileDialogType_Load);
	} else if (param1 == 3) {
		UI_Window_goTo(Window_CCKSelection);
	} else if (param1 == 4) {
		UI_PopupDialog_show(PopupDialog_Quit, confirmExit, 1);
	}
}
