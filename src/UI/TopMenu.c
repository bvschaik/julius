#include "TopMenu.h"
#include "Window.h"
#include "AllWindows.h"
#include "Advisors.h"
#include "FileDialog.h"
#include "MessageDialog.h"
#include "PopupDialog.h"
#include "../Graphics.h"
#include "../Data/CityInfo.h"
#include "../Data/Screen.h"

#include "building/construction.h"
#include "core/string.h"
#include "city/finance.h"
#include "game/file.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/system.h"
#include "game/time.h"
#include "game/undo.h"
#include "graphics/lang_text.h"
#include "graphics/menu.h"
#include "graphics/text.h"
#include "scenario/property.h"

static void refreshSidebarButtons();

static void menuFile_newGame(int param);
static void menuFile_replayMap(int param);
static void menuFile_loadGame(int param);
static void menuFile_saveGame(int param);
static void menuFile_deleteGame(int param);
static void menuFile_exitGame(int param);

static void menuOptions_display(int param);
static void menuOptions_sound(int param);
static void menuOptions_speed(int param);
static void menuOptions_difficulty(int param);

static void menuHelp_help(int param);
static void menuHelp_mouseHelp(int param);
static void menuHelp_warnings(int param);
static void menuHelp_about(int param);

static void menuAdvisors_goTo(int advisor);

static menu_item menuFile[] = {
	{0, 1, menuFile_newGame, 0},
	{20, 2, menuFile_replayMap, 0},
	{40, 3, menuFile_loadGame, 0},
	{60, 4, menuFile_saveGame, 0},
	{80, 6, menuFile_deleteGame, 0},
	{100, 5, menuFile_exitGame, 0},
};

static menu_item menuOptions[] = {
	{0, 1, menuOptions_display, 0},
	{20, 2, menuOptions_sound, 0},
	{40, 3, menuOptions_speed, 0},
	{60, 6, menuOptions_difficulty, 0},
};

static menu_item menuHelp[] = {
	{0, 1, menuHelp_help, 0},
	{20, 2, menuHelp_mouseHelp, 0},
	{40, 5, menuHelp_warnings, 0},
	{60, 7, menuHelp_about, 0},
};

static menu_item menuAdvisors[] = {
	{0, 1, menuAdvisors_goTo, 1},
	{20, 2, menuAdvisors_goTo, 2},
	{40, 3, menuAdvisors_goTo, 3},
	{60, 4, menuAdvisors_goTo, 4},
	{80, 5, menuAdvisors_goTo, 5},
	{100, 6, menuAdvisors_goTo, 6},
	{120, 7, menuAdvisors_goTo, 7},
	{140, 8, menuAdvisors_goTo, 8},
	{160, 9, menuAdvisors_goTo, 9},
	{180, 10, menuAdvisors_goTo, 10},
	{200, 11, menuAdvisors_goTo, 11},
	{220, 12, menuAdvisors_goTo, 12},
};

static menu_bar_item menu[] = {
	{10, 0, 6, 1, menuFile, 6},
	{10, 0, 6, 2, menuOptions, 4},
	{10, 0, 6, 3, menuHelp, 4},
	{10, 0, 6, 4, menuAdvisors, 12},
};

static int offsetFunds;
static int offsetPopulation;
static int offsetDate;

static int openSubMenu = 0;
static int focusMenuId;
static int focusSubMenuId;

static struct {
	int population;
	int treasury;
	int month;
} drawn;

static void set_text_for_tooltips()
{
    switch (setting_tooltips()) {
    case TOOLTIPS_NONE: menuHelp[1].text_number = 2; break;
    case TOOLTIPS_SOME: menuHelp[1].text_number = 3; break;
    case TOOLTIPS_FULL: menuHelp[1].text_number = 4; break;
    }
}

static void set_text_for_warnings()
{
    menuHelp[2].text_number = setting_warnings() ? 6 : 5;
}

void UI_TopMenu_initFromSettings()
{
    set_text_for_tooltips();
    set_text_for_warnings();
}

void UI_TopMenu_drawBackground()
{
	refreshSidebarButtons();
	menu_bar_draw(menu, 4);

	int width;
	color_t treasureColor = COLOR_WHITE;
    int treasury = city_finance_treasury();
	if (treasury < 0) {
		treasureColor = COLOR_RED;
	}
	if (Data_Screen.width < 800) {
		offsetFunds = 338;
		offsetPopulation = 453;
		offsetDate = 547;
		
		width = lang_text_draw_colored(6, 0, 350, 5, FONT_NORMAL_PLAIN, treasureColor);
		text_draw_number_colored(treasury, '@', " ", 346 + width, 5, FONT_NORMAL_PLAIN, treasureColor);

		width = lang_text_draw(6, 1, 458, 5, FONT_NORMAL_GREEN);
		text_draw_number(Data_CityInfo.population, '@', " ", 450 + width, 5, FONT_NORMAL_GREEN);

		width = lang_text_draw(25, game_time_month(), 552, 5, FONT_NORMAL_GREEN);
		lang_text_draw_year_condensed(game_time_year(), 541 + width, 5, FONT_NORMAL_GREEN);
	} else if (Data_Screen.width < 1024) {
		offsetFunds = 338;
		offsetPopulation = 458;
		offsetDate = 652;
		
		width = lang_text_draw_colored(6, 0, 350, 5, FONT_NORMAL_PLAIN, treasureColor);
		text_draw_number_colored(treasury, '@', " ", 346 + width, 5, FONT_NORMAL_PLAIN, treasureColor);

		width = lang_text_draw_colored(6, 1, 470, 5, FONT_NORMAL_PLAIN, COLOR_WHITE);
		text_draw_number_colored(Data_CityInfo.population, '@', " ", 466 + width, 5, FONT_NORMAL_PLAIN, COLOR_WHITE);

		width = lang_text_draw_colored(25, game_time_month(), 655, 5, FONT_NORMAL_PLAIN, COLOR_YELLOW);
		lang_text_draw_year_colored(game_time_year(), 655 + width, 5, FONT_NORMAL_PLAIN, COLOR_YELLOW);
	} else {
		offsetFunds = 493;
		offsetPopulation = 637;
		offsetDate = 852;
		
		width = lang_text_draw_colored(6, 0, 495, 5, FONT_NORMAL_PLAIN, treasureColor);
		text_draw_number_colored(treasury, '@', " ", 501 + width, 5, FONT_NORMAL_PLAIN, treasureColor);

		width = lang_text_draw_colored(6, 1, 645, 5, FONT_NORMAL_PLAIN, COLOR_WHITE);
		text_draw_number_colored(Data_CityInfo.population, '@', " ", 651 + width, 5, FONT_NORMAL_PLAIN, COLOR_WHITE);

		width = lang_text_draw_colored(25, game_time_month(), 850, 5, FONT_NORMAL_PLAIN, COLOR_YELLOW);
		lang_text_draw_year_colored(game_time_year(), 850 + width, 5, FONT_NORMAL_PLAIN, COLOR_YELLOW);
	}
	drawn.treasury = treasury;
	drawn.population = Data_CityInfo.population;
	drawn.month = game_time_month();
}

void UI_TopMenu_drawBackgroundIfNecessary()
{
	if (drawn.treasury != city_finance_treasury() ||
		drawn.population != Data_CityInfo.population ||
		drawn.month != game_time_month()) {
		UI_TopMenu_drawBackground();
	}
}

static void refreshSidebarButtons()
{
	int blockWidth = 24;
	int graphicBase = image_group(GROUP_TOP_MENU_SIDEBAR);
	for (int i = 0; i * blockWidth < Data_Screen.width; i++) {
		Graphics_drawImage(graphicBase + i % 8, i * blockWidth, 0);
	}
	// black panels for funds/pop/time
	if (Data_Screen.width < 800) {
		Graphics_drawImage(graphicBase + 14, 336, 0);
	} else if (Data_Screen.width < 1024) {
		Graphics_drawImage(graphicBase + 14, 336, 0);
		Graphics_drawImage(graphicBase + 14, 456, 0);
		Graphics_drawImage(graphicBase + 14, 648, 0);
	} else {
		Graphics_drawImage(graphicBase + 14, 480, 0);
		Graphics_drawImage(graphicBase + 14, 624, 0);
		Graphics_drawImage(graphicBase + 14, 840, 0);
	}
}

void UI_TopMenu_drawForeground()
{
	if (!openSubMenu) {
		return;
	}
	UI_City_drawCity();
	menu_draw(&menu[openSubMenu-1], focusSubMenuId);
}

static void clearState()
{
	openSubMenu = 0;
	focusMenuId = 0;
	focusSubMenuId = 0;
}

static int handleMouseSubmenu(const mouse *m)
{
	if (m->right.went_up) {
		clearState();
		UI_Window_goBack();
		return 1;
	}
	int menuId = menu_bar_handle_mouse(m, menu, 4, &focusMenuId);
	if (menuId && menuId != openSubMenu) {
		openSubMenu = menuId;
	}
	if (!menu_handle_mouse(m, &menu[openSubMenu-1], &focusSubMenuId)) {
		if (m->left.went_down) {
			clearState();
			UI_Window_goBack();
			return 1;
		}
	}
	return 0;
}

static int getFundsPopDate(int mouse_x, int mouse_y)
{
	if (mouse_y < 4 || mouse_y >= 18) {
		return 0;
	}
	if (mouse_x > offsetFunds && mouse_x < offsetFunds + 128) {
		return 1;
	}
	if (mouse_x > offsetPopulation && mouse_x < offsetPopulation + 128) {
		return 2;
	}
	if (mouse_x > offsetDate && mouse_x < offsetDate + 128) {
		return 3;
	}
	return 0;
}

static int handleTopMenuRightClick(int type)
{
	if (!type) {
		return 0;
	}
	if (type == 1) { // funds
		UI_MessageDialog_show(15, 0);
	} else if (type == 2) { // population
		UI_MessageDialog_show(16, 0);
	} else if (type == 3) { // date
		UI_MessageDialog_show(17, 0);
	}
	return 1;
}

static int handleMouseMenu(const mouse *m)
{
	int menuId = menu_bar_handle_mouse(m, menu, 4, &focusMenuId);
	if (menuId && m->left.went_down) {
		openSubMenu = menuId;
		UI_Window_goTo(Window_TopMenu);
		return 1;
	}
	if (m->right.went_up) {
		return handleTopMenuRightClick(getFundsPopDate(m->x, m->y));
	}
	return 0;
}

int UI_TopMenu_handleMouseWidget(const mouse *m)
{
	if (openSubMenu) {
		return handleMouseSubmenu(m);
	} else {
		return handleMouseMenu(m);
	}
}

int UI_TopMenu_getTooltipText(struct TooltipContext *c)
{
	if (focusMenuId) {
		return 49 + focusMenuId;
	}
	int buttonId = getFundsPopDate(c->mouse_x, c->mouse_y);
	if (buttonId) {
		return 59 + buttonId;
	}
	return 0;
}

void UI_TopMenu_handleMouse(const mouse *m)
{
	UI_TopMenu_handleMouseWidget(m);
}

static void menuFile_newGame(int param)
{
	clearState();
	building_construction_clear_type();
	game_undo_disable();
	game_state_reset_overlay();
	UI_Window_goTo(Window_MainMenu);
}

static void menuFile_replayMap(int param)
{
	clearState();
	building_construction_clear_type();
	if (scenario_is_custom()) {
		game_file_start_scenario(scenario_name());
		UI_Window_goTo(Window_City);
	} else {
		UI_Window_goTo(Window_MissionSelection);
		UI_MissionStart_show();
	}
}

static void menuFile_loadGame(int param)
{
	clearState();
	building_construction_clear_type();
	UI_Window_goTo(Window_City);
	UI_FileDialog_show(FileDialogType_Load);
}

static void menuFile_saveGame(int param)
{
	clearState();
	UI_Window_goTo(Window_City);
	UI_FileDialog_show(FileDialogType_Save);
}

static void menuFile_deleteGame(int param)
{
	clearState();
	UI_Window_goTo(Window_City);
	UI_FileDialog_show(FileDialogType_Delete);
}

static void menuFile_confirmExit(int accepted)
{
	if (accepted) {
		system_exit();
	} else {
		UI_Window_goTo(Window_City);
	}
}

static void menuFile_exitGame(int param)
{
	clearState();
	UI_PopupDialog_show(PopupDialog_Quit, menuFile_confirmExit, 1);
}

static void menuOptions_display(int param)
{
	clearState();
	UI_Window_goTo(Window_DisplayOptions);
}

static void menuOptions_sound(int param)
{
	clearState();
	UI_Window_goTo(Window_SoundOptions);
}

static void menuOptions_speed(int param)
{
	clearState();
	UI_Window_goTo(Window_SpeedOptions);
}

static void menuOptions_difficulty(int param)
{
	clearState();
	UI_Window_goTo(Window_DifficultyOptions);
}

static void menuHelp_help(int param)
{
	clearState();
	UI_Window_goBack();
	UI_MessageDialog_show(MessageDialog_Help, 0);
}

static void menuHelp_mouseHelp(int param)
{
    setting_cycle_tooltips();
    set_text_for_tooltips();
}

static void menuHelp_warnings(int param)
{
    setting_toggle_warnings();
    set_text_for_warnings();
}

static void menuHelp_about(int param)
{
	clearState();
	UI_Window_goBack();
	UI_MessageDialog_show(MessageDialog_About, 0);
}


static void menuAdvisors_goTo(int advisor)
{
	clearState();
	UI_Window_goBack();
	UI_Advisors_goToFromMessage(advisor);
}
