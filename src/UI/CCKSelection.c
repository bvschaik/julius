#include "Window.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/string.h"
#include "game/file.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "scenario/criteria.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/speech.h"

#include <string.h>

static void drawScenarioList();
static void drawScrollbarDot();
static void drawScenarioInfo();
static int handleScrollbarClick(const mouse *m);
static void buttonSelectItem(int index, int param2);
static void buttonScroll(int isDown, int numLines);
static void buttonStartScenario(int param1, int param2);

static image_button imageButtons[] = {
	{276, 210, 34, 34, IB_NORMAL, 96, 8, buttonScroll, button_none, 0, 1, 1},
	{276, 440, 34, 34, IB_NORMAL, 96, 12, buttonScroll, button_none, 1, 1, 1},
	{600, 440, 27, 27, IB_NORMAL, 92, 56, buttonStartScenario, button_none, 1, 0, 1},
};

static generic_button customButtons[] = {
	{28, 220, 280, 236, GB_IMMEDIATE, buttonSelectItem, button_none, 0, 0},
	{28, 236, 280, 252, GB_IMMEDIATE, buttonSelectItem, button_none, 1, 0},
	{28, 252, 280, 268, GB_IMMEDIATE, buttonSelectItem, button_none, 2, 0},
	{28, 268, 280, 284, GB_IMMEDIATE, buttonSelectItem, button_none, 3, 0},
	{28, 284, 280, 300, GB_IMMEDIATE, buttonSelectItem, button_none, 4, 0},
	{28, 300, 280, 316, GB_IMMEDIATE, buttonSelectItem, button_none, 5, 0},
	{28, 316, 280, 332, GB_IMMEDIATE, buttonSelectItem, button_none, 6, 0},
	{28, 332, 280, 348, GB_IMMEDIATE, buttonSelectItem, button_none, 7, 0},
	{28, 348, 280, 364, GB_IMMEDIATE, buttonSelectItem, button_none, 8, 0},
	{28, 364, 280, 380, GB_IMMEDIATE, buttonSelectItem, button_none, 9, 0},
	{28, 380, 280, 396, GB_IMMEDIATE, buttonSelectItem, button_none, 10, 0},
	{28, 396, 280, 412, GB_IMMEDIATE, buttonSelectItem, button_none, 11, 0},
	{28, 412, 280, 428, GB_IMMEDIATE, buttonSelectItem, button_none, 12, 0},
	{28, 428, 280, 444, GB_IMMEDIATE, buttonSelectItem, button_none, 13, 0},
	{28, 444, 280, 460, GB_IMMEDIATE, buttonSelectItem, button_none, 14, 0},
};

static int scrollPosition;
static int focusButtonId;
static int selectedItem;
static char selectedScenario[FILE_NAME_MAX];

static const dir_listing *scenarios;

void UI_CCKSelection_init()
{
	scenario_set_custom(2);
	scenarios = dir_find_files_with_extension("map");
	scrollPosition = 0;
	focusButtonId = 0;
	buttonSelectItem(0, 0);
}

void UI_CCKSelection_drawBackground()
{
	image_draw_fullscreen_background(image_group(GROUP_CCK_BACKGROUND));
    graphics_in_dialog();
	inner_panel_draw(280, 242, 2, 12);
	drawScenarioList();
	drawScrollbarDot();
	drawScenarioInfo();
    graphics_reset_dialog();
}

static void drawScenarioList()
{
	inner_panel_draw(16, 210, 16, 16);
	for (int i = 0; i < 15; i++) {
		font_t font = FONT_NORMAL_GREEN;
		if (focusButtonId == i + 1) {
			font = FONT_NORMAL_WHITE;
		} else if (!focusButtonId && selectedItem == i + scrollPosition) {
			font = FONT_NORMAL_WHITE;
		}
		char file[FILE_NAME_MAX];
		strcpy(file, scenarios->files[i + scrollPosition]);
		file_remove_extension(file);
		text_draw(string_from_ascii(file), 24, 220 + 16 * i, font, 0);
	}
}

static void drawScrollbarDot()
{
	if (scenarios->num_files > 15) {
		int pct;
		if (scrollPosition <= 0) {
			pct = 0;
		} else if (scrollPosition + 15 >= scenarios->num_files) {
			pct = 100;
		} else {
			pct = calc_percentage(scrollPosition, scenarios->num_files - 15);
		}
		int yOffset = calc_adjust_with_percentage(164, pct);
		image_draw(image_group(GROUP_PANEL_BUTTON) + 39, 284, 245 + yOffset);
	}
}

static void drawScenarioInfo()
{
	image_draw(image_group(GROUP_SCENARIO_IMAGE) + scenario_image_id(), 78, 36);

	text_draw_centered(string_from_ascii(selectedScenario), 335, 25, 260, FONT_LARGE_BLACK, 0);
	text_draw_centered(scenario_brief_description(), 335, 60, 260, FONT_NORMAL_WHITE, 0);
	lang_text_draw_year(scenario_property_start_year(), 410, 90, FONT_LARGE_BLACK);
	lang_text_draw_centered(44, 77 + scenario_property_climate(), 335, 150, 260, FONT_NORMAL_BLACK);

	// map size
	int textId;
	switch (scenario_map_size()) {
		case 40: textId = 121; break;
		case 60: textId = 122; break;
		case 80: textId = 123; break;
		case 100: textId = 124; break;
		case 120: textId = 125; break;
		default: textId = 126; break;
	}
	lang_text_draw_centered(44, textId, 335, 170, 260, FONT_NORMAL_BLACK);

	// military
	int numInvasions = scenario_invasion_count();
	if (numInvasions <= 0) {
		textId = 112;
	} else if (numInvasions <= 2) {
		textId = 113;
	} else if (numInvasions <= 4) {
		textId = 114;
	} else if (numInvasions <= 10) {
		textId = 115;
	} else {
		textId = 116;
	}
	lang_text_draw_centered(44, textId, 335, 190, 260, FONT_NORMAL_BLACK);

	lang_text_draw_centered(32, 11 + scenario_property_player_rank(), 335, 210, 260, FONT_NORMAL_BLACK);
	if (scenario_is_open_play()) {
		lang_text_draw_multiline(145, scenario_open_play_id(), 345, 270, 260, FONT_NORMAL_BLACK);
	} else {
		lang_text_draw_centered(44, 127, 335, 262, 260, FONT_NORMAL_BLACK);
		int width;
		if (scenario_criteria_culture_enabled()) {
			width = text_draw_number(scenario_criteria_culture(), '@', " ", 410, 290, FONT_NORMAL_BLACK);
			lang_text_draw(44, 129, 410 + width, 290, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_prosperity_enabled()) {
			width = text_draw_number(scenario_criteria_prosperity(), '@', " ", 410, 306, FONT_NORMAL_BLACK);
			lang_text_draw(44, 130, 410 + width, 306, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_peace_enabled()) {
			width = text_draw_number(scenario_criteria_peace(), '@', " ", 410, 322, FONT_NORMAL_BLACK);
			lang_text_draw(44, 131, 410 + width, 322, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_favor_enabled()) {
			width = text_draw_number(scenario_criteria_favor(), '@', " ", 410, 338, FONT_NORMAL_BLACK);
			lang_text_draw(44, 132, 410 + width, 338, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_population_enabled()) {
			width = text_draw_number(scenario_criteria_population(), '@', " ", 410, 354, FONT_NORMAL_BLACK);
			lang_text_draw(44, 133, 410 + width, 354, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_time_limit_enabled()) {
			width = text_draw_number(scenario_criteria_time_limit_years(), '@', " ", 410, 370, FONT_NORMAL_BLACK);
			lang_text_draw(44, 134, 410 + width, 370, FONT_NORMAL_BLACK);
		}
		if (scenario_criteria_survival_enabled()) {
			width = text_draw_number(scenario_criteria_survival_years(), '@', " ", 410, 386, FONT_NORMAL_BLACK);
			lang_text_draw(44, 135, 410 + width, 386, FONT_NORMAL_BLACK);
		}
	}
	lang_text_draw(44, 136, 420, 446, FONT_NORMAL_BLACK);
}

void UI_CCKSelection_drawForeground()
{
    graphics_in_dialog();
	image_buttons_draw(0, 0, imageButtons, 3);
	drawScenarioList();
    graphics_reset_dialog();
}

void UI_CCKSelection_handleMouse(const mouse *m)
{
	if (m->scrolled == SCROLL_DOWN) {
		buttonScroll(1, 3);
	} else if (m->scrolled == SCROLL_UP) {
		buttonScroll(0, 3);
	}
	if (handleScrollbarClick(m)) {
		return;
	}
	const mouse *m_dialog = mouse_in_dialog(m);
	if (image_buttons_handle_mouse(m_dialog, 0, 0, imageButtons, 3, 0)) {
		return;
	}
	generic_buttons_handle_mouse(m_dialog, 0, 0, customButtons, 15, &focusButtonId);
}

static int handleScrollbarClick(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
	if (scenarios->num_files <= 15) {
		return 0;
	}
	if (!m_dialog->left.is_down) {
		return 0;
	}
	if (m_dialog->x >= 280 && m_dialog->x <= 312 &&
		    m_dialog->y >= 245 && m_dialog->y <= 434) {
		int yOffset = m_dialog->y - 245;
		if (yOffset > 164) {
			yOffset = 164;
		}
		int pct = calc_percentage(yOffset, 164);
		scrollPosition = calc_adjust_with_percentage(scenarios->num_files - 15, pct);
		UI_Window_requestRefresh();
		return 1;
	}
	return 0;
}

static void buttonSelectItem(int index, int param2)
{
	if (index >= scenarios->num_files) {
		return;
	}
	selectedItem = scrollPosition + index;
	strcpy(selectedScenario, scenarios->files[selectedItem]);
	game_file_load_scenario_data(selectedScenario);
	file_remove_extension(selectedScenario);
	UI_Window_requestRefresh();
}

static void buttonScroll(int isDown, int numLines)
{
	if (scenarios->num_files > 15) {
		if (isDown) {
			scrollPosition += numLines;
			if (scrollPosition > scenarios->num_files - 15) {
				scrollPosition = scenarios->num_files - 15;
			}
		} else {
			scrollPosition -= numLines;
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
	}
}

static void buttonStartScenario(int param1, int param2)
{
	sound_speech_stop();
	game_file_start_scenario(string_from_ascii(selectedScenario));
	UI_Window_goTo(Window_City);
}
