#include "FileDialog.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/lang.h"
#include "core/string.h"
#include "core/time.h"
#include "game/file.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/keyboard.h"

#include <string.h>

static void drawScrollbarDot();
static int handleScrollbarClick(const mouse *m);
static void buttonOkCancel(int isOk, int param2);
static void buttonScroll(int isDown, int numLines);
static void buttonSelectItem(int index, int numLines);

static image_button imageButtons[] = {
	{344, 335, 34, 34, IB_NORMAL, 96, 0, buttonOkCancel, button_none, 1, 0, 1},
	{392, 335, 34, 34, IB_NORMAL, 96, 4, buttonOkCancel, button_none, 0, 0, 1},
	{464, 120, 34, 34, IB_SCROLL, 96, 8, buttonScroll, button_none, 0, 1, 1},
	{464, 300, 34, 34, IB_SCROLL, 96, 12, buttonScroll, button_none, 1, 1, 1},
};
static generic_button customButtons[] = {
	{160, 128, 448, 144, GB_IMMEDIATE, buttonSelectItem, button_none, 0, 0},
	{160, 144, 448, 160, GB_IMMEDIATE, buttonSelectItem, button_none, 1, 0},
	{160, 160, 448, 176, GB_IMMEDIATE, buttonSelectItem, button_none, 2, 0},
	{160, 176, 448, 192, GB_IMMEDIATE, buttonSelectItem, button_none, 3, 0},
	{160, 192, 448, 208, GB_IMMEDIATE, buttonSelectItem, button_none, 4, 0},
	{160, 208, 448, 224, GB_IMMEDIATE, buttonSelectItem, button_none, 5, 0},
	{160, 224, 448, 240, GB_IMMEDIATE, buttonSelectItem, button_none, 6, 0},
	{160, 240, 448, 256, GB_IMMEDIATE, buttonSelectItem, button_none, 7, 0},
	{160, 256, 448, 272, GB_IMMEDIATE, buttonSelectItem, button_none, 8, 0},
	{160, 272, 448, 288, GB_IMMEDIATE, buttonSelectItem, button_none, 9, 0},
	{160, 288, 448, 304, GB_IMMEDIATE, buttonSelectItem, button_none, 10, 0},
	{160, 304, 448, 320, GB_IMMEDIATE, buttonSelectItem, button_none, 11, 0},
};

#define NOT_EXIST_MESSAGE_TIMEOUT 500

static time_millis messageNotExistTimeUntil;
static int dialogType;
static int focusButtonId;
static int scrollPosition;
static const dir_listing *savedGames;

static char saved_game[FILE_NAME_MAX];
static char last_loaded_file[FILE_NAME_MAX] = "";

void UI_FileDialog_show(int type)
{
    if (strlen(last_loaded_file) == 0) {
        strncpy(last_loaded_file, string_to_ascii(lang_get_string(9, 6)), FILE_NAME_MAX);
    }
	dialogType = type;
	messageNotExistTimeUntil = 0;
	scrollPosition = 0;

	savedGames = dir_find_files_with_extension("sav");

	UI_Window_goTo(Window_FileDialog);

    strncpy(saved_game, last_loaded_file, FILE_NAME_MAX);
    keyboard_start_capture((uint8_t*) saved_game, 64, 0, 280, FONT_NORMAL_WHITE);
}

void UI_FileDialog_drawBackground()
{
	// do nothing
}

void UI_FileDialog_drawForeground()
{
    graphics_in_dialog();
	char file[100];

	outer_panel_draw(128, 40, 24, 21);
	inner_panel_draw(144, 80, 20, 2);
	inner_panel_draw(144, 120, 20, 13);

    // title
	if (messageNotExistTimeUntil && time_get_millis() < messageNotExistTimeUntil) {
		lang_text_draw_centered(43, 2, 160, 50, 304, FONT_LARGE_BLACK);
	} else if (dialogType == FileDialogType_Delete) {
		lang_text_draw_centered(43, 6, 160, 50, 304, FONT_LARGE_BLACK);
	} else {
		lang_text_draw_centered(43, dialogType, 160, 50, 304, FONT_LARGE_BLACK);
	}
	lang_text_draw(43, 5, 224, 342, FONT_NORMAL_BLACK);

	for (int i = 0; i < 12; i++) {
		font_t font = FONT_NORMAL_GREEN;
		if (focusButtonId == i + 1) {
			font = FONT_NORMAL_WHITE;
		}
		strcpy(file, savedGames->files[scrollPosition + i]);
		file_remove_extension(file);
		text_draw(string_from_ascii(file), 160, 130 + 16 * i, font, 0);
	}

	image_buttons_draw(0, 0, imageButtons, 4);
	text_capture_cursor(keyboard_cursor_position());
	text_draw(string_from_ascii(saved_game), 160, 90, FONT_NORMAL_WHITE, 0);
	text_draw_cursor(160, 91, keyboard_is_insert());
	drawScrollbarDot();

    graphics_reset_dialog();
}

static void drawScrollbarDot()
{
	if (savedGames->num_files > 12) {
		int pct;
		if (scrollPosition <= 0) {
			pct = 0;
		} else if (scrollPosition + 12 >= savedGames->num_files) {
			pct = 100;
		} else {
			pct = calc_percentage(scrollPosition, savedGames->num_files - 12);
		}
		int yOffset = calc_adjust_with_percentage(130, pct);
		image_draw(image_group(GROUP_PANEL_BUTTON) + 39, 472, 145 + yOffset);
	}
}

void UI_FileDialog_handleMouse(const mouse *m)
{
	if (m->scrolled == SCROLL_DOWN) {
		buttonScroll(1, 3);
	} else if (m->scrolled == SCROLL_UP) {
		buttonScroll(0, 3);
	}

    if (keyboard_input_is_accepted()) {
        buttonOkCancel(1, 0);
        return;
    }

	if (m->right.went_down) {
		UI_Window_goBack();
		return;
	}
	const mouse *m_dialog = mouse_in_dialog(m);
	if (!generic_buttons_handle_mouse(m_dialog, 0, 0, customButtons, 12, &focusButtonId)) {
		if (!image_buttons_handle_mouse(m_dialog, 0, 0, imageButtons, 4, 0)) {
			handleScrollbarClick(m);
		}
	}
}

static int handleScrollbarClick(const mouse *m)
{
    const mouse *m_dialog = mouse_in_dialog(m);
	if (savedGames->num_files <= 12) {
		return 0;
	}
	if (!m_dialog->left.is_down) {
		return 0;
	}
	if (m_dialog->x >= 464 && m_dialog->x <= 496 && m_dialog->y >= 145 && m_dialog->y <= 300) {
		int yOffset = m_dialog->y - 145;
		if (yOffset > 130) {
			yOffset = 130;
		}
		int pct = calc_percentage(yOffset, 130);
		scrollPosition = calc_adjust_with_percentage(savedGames->num_files - 12, pct);
		window_invalidate();
		return 1;
	}
	return 0;
}

static void buttonOkCancel(int isOk, int param2)
{
	if (!isOk) {
		UI_Window_goBack();
		return;
	}

	file_remove_extension(saved_game);
	file_append_extension(saved_game, "sav");

	if (dialogType != FileDialogType_Save && !file_exists(saved_game)) {
		file_remove_extension(saved_game);
		messageNotExistTimeUntil = time_get_millis() + NOT_EXIST_MESSAGE_TIMEOUT;
		return;
	}
	if (dialogType == FileDialogType_Load) {
		game_file_load_saved_game(saved_game);
        keyboard_stop_capture();
		UI_Window_goTo(Window_City);
	} else if (dialogType == FileDialogType_Save) {
		game_file_write_saved_game(saved_game);
        keyboard_stop_capture();
		UI_Window_goTo(Window_City);
	} else if (dialogType == FileDialogType_Delete) {
		if (game_file_delete_saved_game(saved_game)) {
			dir_find_files_with_extension("sav");
			if (scrollPosition + 12 >= savedGames->num_files) {
				--scrollPosition;
			}
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
	}
	
	file_remove_extension(saved_game);
	strncpy(last_loaded_file, saved_game, FILE_NAME_MAX);
}

static void buttonScroll(int isDown, int numLines)
{
	if (savedGames->num_files > 12) {
		if (isDown) {
			scrollPosition += numLines;
			if (scrollPosition > savedGames->num_files - 12) {
				scrollPosition = savedGames->num_files - 12;
			}
		} else {
			scrollPosition -= numLines;
			if (scrollPosition < 0) {
				scrollPosition = 0;
			}
		}
		messageNotExistTimeUntil = 0;
	}
}

static void buttonSelectItem(int index, int numLines)
{
	if (index < savedGames->num_files) {
		strncpy(saved_game, savedGames->files[scrollPosition + index], FILE_NAME_MAX);
		file_remove_extension(saved_game);
		keyboard_refresh();
		messageNotExistTimeUntil = 0;
	}
}
