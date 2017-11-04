#include "FileDialog.hpp"
#include "Window.hpp"

#include "core/calc.hpp"
#include "core/dir.hpp"
#include "core/file.hpp"

#include "../GameFile.hpp"
#include "../Graphics.hpp"
#include "../KeyboardInput.hpp"
#include "../Widget.hpp"

#include "../Data/Constants.hpp"
#include "../Data/FileList.hpp"
#include "../Data/KeyboardInput.hpp"
#include "../Data/Screen.hpp"

#include "core/time.hpp"

#include <string.h>

static void drawScrollbarDot();
static int handleScrollbarClick(const mouse *m);
static void buttonOkCancel(int isOk, int param2);
static void buttonScroll(int isDown, int numLines);
static void buttonSelectItem(int index, int numLines);

static ImageButton imageButtons[] =
{
    {344, 335, 34, 34, ImageButton_Normal, 96, 0, buttonOkCancel, Widget_Button_doNothing, 1, 0, 1},
    {392, 335, 34, 34, ImageButton_Normal, 96, 4, buttonOkCancel, Widget_Button_doNothing, 0, 0, 1},
    {464, 120, 34, 34, ImageButton_Scroll, 96, 8, buttonScroll, Widget_Button_doNothing, 0, 1, 1},
    {464, 300, 34, 34, ImageButton_Scroll, 96, 12, buttonScroll, Widget_Button_doNothing, 1, 1, 1},
};
static CustomButton customButtons[] =
{
    {160, 128, 448, 144, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 0, 0},
    {160, 144, 448, 160, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 1, 0},
    {160, 160, 448, 176, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 2, 0},
    {160, 176, 448, 192, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 3, 0},
    {160, 192, 448, 208, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 4, 0},
    {160, 208, 448, 224, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 5, 0},
    {160, 224, 448, 240, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 6, 0},
    {160, 240, 448, 256, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 7, 0},
    {160, 256, 448, 272, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 8, 0},
    {160, 272, 448, 288, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 9, 0},
    {160, 288, 448, 304, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 10, 0},
    {160, 304, 448, 320, CustomButton_Immediate, buttonSelectItem, Widget_Button_doNothing, 11, 0},
};

#define NOT_EXIST_MESSAGE_TIMEOUT 500

static time_millis messageNotExistTimeUntil;
static int dialogType;
static int focusButtonId;
static int scrollPosition;
static const dir_listing *savedGames;

void UI_FileDialog_show(int type)
{
    dialogType = type;
    messageNotExistTimeUntil = 0;
    scrollPosition = 0;

    savedGames = dir_find_files_with_extension("sav");

    strcpy(Data_FileList.selectedCity, Data_FileList.lastLoadedCity);
    Data_KeyboardInput.accepted = 0;
    KeyboardInput_initInput(2);
    KeyboardInput_home();
    KeyboardInput_end();

    UI_Window_goTo(Window_FileDialog);
}

void UI_FileDialog_drawBackground()
{
    // do nothing
}

void UI_FileDialog_drawForeground()
{
    char file[100];

    int baseOffsetX = Data_Screen.offset640x480.x;
    int baseOffsetY = Data_Screen.offset640x480.y;

    Widget_Panel_drawOuterPanel(baseOffsetX + 128, baseOffsetY + 40, 24, 21);
    Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 80, 20, 2);
    Widget_Panel_drawInnerPanel(baseOffsetX + 144, baseOffsetY + 120, 20, 13);

    // title
    if (messageNotExistTimeUntil && time_get_millis() < messageNotExistTimeUntil)
    {
        Widget_GameText_drawCentered(43, 2,
                                     baseOffsetX + 160, baseOffsetY + 50, 304, FONT_LARGE_BLACK);
    }
    else if (dialogType == FileDialogType_Delete)
    {
        Widget_GameText_drawCentered(43, 6,
                                     baseOffsetX + 160, baseOffsetY + 50, 304, FONT_LARGE_BLACK);
    }
    else
    {
        Widget_GameText_drawCentered(43, dialogType,
                                     baseOffsetX + 160, baseOffsetY + 50, 304, FONT_LARGE_BLACK);
    }
    Widget_GameText_draw(43, 5, baseOffsetX + 224, baseOffsetY + 342, FONT_NORMAL_BLACK);

    for (int i = 0; i < 12; i++)
    {
        font_t font = FONT_NORMAL_GREEN;
        if (focusButtonId == i + 1)
        {
            font = FONT_NORMAL_WHITE;
        }
        strcpy(file, savedGames->files[scrollPosition + i]);
        file_remove_extension(file);
        Widget_Text_draw(file, baseOffsetX + 160, baseOffsetY + 130 + 16 * i, font, 0);
    }

    Widget_Button_drawImageButtons(baseOffsetX, baseOffsetY, imageButtons, 4);
    Widget_Text_captureCursor();
    Widget_Text_draw(Data_FileList.selectedCity, baseOffsetX + 160, baseOffsetY + 90, FONT_NORMAL_WHITE, 0);
    Widget_Text_drawCursor(baseOffsetX + 160, baseOffsetY + 91);
    drawScrollbarDot();
}

static void drawScrollbarDot()
{
    if (savedGames->num_files > 12)
    {
        int pct;
        if (scrollPosition <= 0)
        {
            pct = 0;
        }
        else if (scrollPosition + 12 >= savedGames->num_files)
        {
            pct = 100;
        }
        else
        {
            pct = calc_percentage(scrollPosition, savedGames->num_files - 12);
        }
        int yOffset = calc_adjust_with_percentage(130, pct);
        Graphics_drawImage(image_group(ID_Graphic_PanelButton) + 39,
                           Data_Screen.offset640x480.x + 472,
                           Data_Screen.offset640x480.y + 145 + yOffset);
    }
}

void UI_FileDialog_handleMouse(const mouse *m)
{
    if (m->scrolled == SCROLL_DOWN)
    {
        buttonScroll(1, 3);
    }
    else if (m->scrolled == SCROLL_UP)
    {
        buttonScroll(0, 3);
    }

    KeyboardInput_initInput(2);
    if (Data_KeyboardInput.accepted)
    {
        Data_KeyboardInput.accepted = 0;
        buttonOkCancel(1, 0);
        return;
    }

    if (m->right.went_down)
    {
        UI_Window_goBack();
        return;
    }
    int xOffset = Data_Screen.offset640x480.x;
    int yOffset = Data_Screen.offset640x480.y;
    if (!Widget_Button_handleCustomButtons(xOffset, yOffset, customButtons, 12, &focusButtonId))
    {
        if (!Widget_Button_handleImageButtons(xOffset, yOffset, imageButtons, 4, 0))
        {
            handleScrollbarClick(m);
        }
    }
}

static int handleScrollbarClick(const mouse *m)
{
    if (savedGames->num_files <= 12)
    {
        return 0;
    }
    if (!m->left.is_down)
    {
        return 0;
    }
    int x = Data_Screen.offset640x480.x;
    int y = Data_Screen.offset640x480.y;
    if (m->x >= x + 464 && m->x <= x + 496 &&
            m->y >= y + 145 && m->y <= y + 300)
    {
        int yOffset = m->y - (y + 145);
        if (yOffset > 130)
        {
            yOffset = 130;
        }
        int pct = calc_percentage(yOffset, 130);
        scrollPosition = calc_adjust_with_percentage(savedGames->num_files - 12, pct);
        UI_Window_requestRefresh();
        return 1;
    }
    return 0;
}

static void buttonOkCancel(int isOk, int param2)
{
    if (!isOk)
    {
        UI_Window_goBack();
        return;
    }

    file_remove_extension(Data_FileList.selectedCity);
    file_append_extension(Data_FileList.selectedCity, "sav");

    if (dialogType != FileDialogType_Save && !file_exists(Data_FileList.selectedCity))
    {
        file_remove_extension(Data_FileList.selectedCity);
        messageNotExistTimeUntil = time_get_millis() + NOT_EXIST_MESSAGE_TIMEOUT;
        return;
    }
    if (dialogType == FileDialogType_Load)
    {
        GameFile_loadSavedGame(Data_FileList.selectedCity);
        UI_Window_goTo(Window_City);
    }
    else if (dialogType == FileDialogType_Save)
    {
        GameFile_writeSavedGame(Data_FileList.selectedCity);
        UI_Window_goTo(Window_City);
    }
    else if (dialogType == FileDialogType_Delete)
    {
        if (GameFile_deleteSavedGame(Data_FileList.selectedCity))
        {
            dir_find_files_with_extension("sav");
            if (scrollPosition + 12 >= savedGames->num_files)
            {
                --scrollPosition;
            }
            if (scrollPosition < 0)
            {
                scrollPosition = 0;
            }
        }
    }

    file_remove_extension(Data_FileList.selectedCity);
    strcpy(Data_FileList.lastLoadedCity, Data_FileList.selectedCity);
}

static void buttonScroll(int isDown, int numLines)
{
    if (savedGames->num_files > 12)
    {
        if (isDown)
        {
            scrollPosition += numLines;
            if (scrollPosition > savedGames->num_files - 12)
            {
                scrollPosition = savedGames->num_files - 12;
            }
        }
        else
        {
            scrollPosition -= numLines;
            if (scrollPosition < 0)
            {
                scrollPosition = 0;
            }
        }
        messageNotExistTimeUntil = 0;
    }
}

static void buttonSelectItem(int index, int param2)
{
    if (index < savedGames->num_files)
    {
        memset(Data_FileList.selectedCity, 0, FILENAME_LENGTH);
        strcpy(Data_FileList.selectedCity, savedGames->files[scrollPosition + index]);
        file_remove_extension(Data_FileList.selectedCity);
        KeyboardInput_initInput(2);
        KeyboardInput_home();
        KeyboardInput_end();
        messageNotExistTimeUntil = 0;
    }
}
