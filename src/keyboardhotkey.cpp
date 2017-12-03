#include "keyboardhotkey.h"

#include "cityview.h"
#include "graphics.h"
#include "system.h"
#include "video.h"

#include <data>
#include <ui>
#include <scenario>
#include <game>

#include "figure/formation.h"

#define ExitMilitaryCommand() \
	if (UI_Window_getId() == Window_CityMilitary) {\
		UI_Window_goTo(Window_City);\
	}

static struct
{
    int ctrlDown;
    int altDown;
    int shiftDown;
    int isCheating;
} data = {0, 0, 0, 0};

static void changeGameSpeed(int isDown)
{
    if (UI_Window_getId() == Window_City)
    {
        UI_SpeedOptions_changeGameSpeed(isDown);
    }
}

static void toggleOverlay()
{
    ExitMilitaryCommand();
    if (UI_Window_getId() == Window_City)
    {
        int tmp = Data_State.previousOverlay;
        Data_State.previousOverlay = Data_State.currentOverlay;
        Data_State.currentOverlay = tmp;
        UI_Window_requestRefresh();
    }
}

static void showOverlay(int overlay)
{
    ExitMilitaryCommand();
    WindowId window = UI_Window_getId();
    if (window == Window_City)
    {
        if (Data_State.currentOverlay == overlay)
        {
            Data_State.previousOverlay = overlay;
            Data_State.currentOverlay = 0;
        }
        else
        {
            Data_State.previousOverlay = 0;
            Data_State.currentOverlay = overlay;
        }
        UI_Window_requestRefresh();
    }
}

static void togglePause()
{
    ExitMilitaryCommand();
    if (UI_Window_getId() == Window_City)
    {
        if (Data_State.gamePaused)
        {
            Data_State.gamePaused = 0;
        }
        else
        {
            Data_State.gamePaused = 1;
        }
        UI_Warning_clearAll();
    }
}

static void showAdvisor(int advisor)
{
    ExitMilitaryCommand();
    if (UI_Window_getId() == Window_Advisors)
    {
        if (UI_Advisors_getId() == advisor)
        {
            UI_Window_goTo(Window_City);
        }
        else
        {
            UI_Advisors_goToFromMessage(advisor);
        }
    }
    else if (UI_Window_getId() == Window_City)
    {
        UI_Advisors_goToFromMessage(advisor);
    }
}

static void cycleLegion()
{
    static int currentLegionId = 1;
    if (UI_Window_getId() == Window_City)
    {
        int legionId = currentLegionId;
        currentLegionId = 0;
        for (int i = 1; i <= MAX_LEGIONS; i++)
        {
            legionId++;
            if (legionId > MAX_LEGIONS)
            {
                legionId = 1;
            }
            const formation *m = formation_get(legionId);
            if (m->in_use == 1 && !m->is_herd && m->is_legion)
            {
                if (currentLegionId == 0)
                {
                    currentLegionId = legionId;
                    break;
                }
            }
        }
        if (currentLegionId > 0)
        {
            const formation *m = formation_get(currentLegionId);
            CityView_goToGridOffset(GridOffset(m->x_home, m->y_home));
            UI_Window_requestRefresh();
        }
    }
}

static void cheatInitOrInvasion()
{
    if (UI_Window_getId() == Window_BuildingInfo)
    {
        data.isCheating = UI_BuildingInfo_getBuildingType() == BUILDING_WELL;
    }
    else if (data.isCheating && UI_Window_getId() == Window_MessageDialog)
    {
        data.isCheating = 2;
        scenario_invasion_start_from_cheat();
    }
    else
    {
        data.isCheating = 0;
    }
}

static void cheatVictory()
{
    if (data.isCheating)
    {
        Data_State.forceWinCheat = 1;
    }
}

static void cheatMoney()
{
    if (data.isCheating && Data_CityInfo.treasury < 5000)
    {
        Data_CityInfo.treasury += 1000;
        Data_CityInfo.cheatedMoney += 1000;
        UI_Window_requestRefresh();
    }
}

void KeyboardHotkey_character(int c)
{
    if (data.altDown)
    {
        switch (c)
        {
        case 'X':
        case 'x':
            KeyboardHotkey_esc();
            break;
        case 'K':
        case 'k':
            cheatInitOrInvasion();
        case 'C':
        case 'c':
            cheatMoney();
            break;
        case 'V':
        case 'v':
            cheatVictory();
            break;
        }
        return;
    }
    switch (c)
    {
    case '[':
        changeGameSpeed(1);
        break;
    case ']':
        changeGameSpeed(0);
        break;
    case ' ':
        toggleOverlay();
        break;
    case 'P':
    case 'p':
        togglePause();
        break;
    case 'F':
    case 'f':
        showOverlay(Overlay_Fire);
        break;
    case 'D':
    case 'd':
        showOverlay(Overlay_Damage);
        break;
    case 'C':
    case 'c':
        showOverlay(Overlay_Crime);
        break;
    case 'T':
    case 't':
        showOverlay(Overlay_Problems);
        break;
    case 'W':
    case 'w':
        showOverlay(Overlay_Water);
        break;
    case 'L':
    case 'l':
        cycleLegion();
        break;
    case '1':
        showAdvisor(Advisor_Labor);
        break;
    case '2':
        showAdvisor(Advisor_Military);
        break;
    case '3':
        showAdvisor(Advisor_Imperial);
        break;
    case '4':
        showAdvisor(Advisor_Ratings);
        break;
    case '5':
        showAdvisor(Advisor_Trade);
        break;
    case '6':
        showAdvisor(Advisor_Population);
        break;
    case '7':
        showAdvisor(Advisor_Health);
        break;
    case '8':
        showAdvisor(Advisor_Education);
        break;
    case '9':
        showAdvisor(Advisor_Entertainment);
        break;
    case '0':
        showAdvisor(Advisor_Religion);
        break;
    case '-':
        showAdvisor(Advisor_Financial);
        break;
    case '=':
        showAdvisor(Advisor_Chief);
        break;
    }
}

void KeyboardHotkey_left()
{
    scroll_arrow_left();
}

void KeyboardHotkey_right()
{
    scroll_arrow_right();
}

void KeyboardHotkey_up()
{
    scroll_arrow_up();
}

void KeyboardHotkey_down()
{
    scroll_arrow_down();
}

void KeyboardHotkey_home()
{
    if (UI_Window_getId() == Window_City)
    {
        UI_Sidebar_rotateMap(0);
    }
}

void KeyboardHotkey_end()
{
    if (UI_Window_getId() == Window_City)
    {
        UI_Sidebar_rotateMap(1);
    }
}

static void confirmExit(int accepted)
{
    if (accepted)
    {
        System_exit();
    }
}

void KeyboardHotkey_esc()
{
    Video_stop();
    UI_PopupDialog_show(PopupDialog_Quit, confirmExit, 1);
}

static void goToBookmark(int number)
{
    if (map_bookmark_go_to(number))
    {
        CityView_checkCameraBoundaries();
        UI_Window_requestRefresh();
    }
}

static void handleBookmark(int number)
{
    ExitMilitaryCommand();
    if (UI_Window_getId() == Window_City)
    {
        if (data.ctrlDown || data.shiftDown)
        {
            map_bookmark_save(number);
        }
        else
        {
            goToBookmark(number);
        }
    }
}

static void takeScreenshot()
{
    Graphics_saveScreenshot("city.bmp");
}

void KeyboardHotkey_func(int fNumber)
{
    switch (fNumber)
    {
    case 1:
    case 2:
    case 3:
    case 4:
        handleBookmark(fNumber - 1);
        break;
    case 5: /* center window not implemented */
        ;
        break;
    case 6:
        System_toggleFullscreen();
        break;
    case 7:
        System_resize(640, 480);
        break;
    case 8:
        System_resize(800, 600);
        break;
    case 9:
        System_resize(1024, 768);
        break;
    case 12:
        takeScreenshot();
        break;
    }
}

void KeyboardHotkey_ctrl(int isDown)
{
    data.ctrlDown = isDown;
}

void KeyboardHotkey_alt(int isDown)
{
    data.altDown = isDown;
}

void KeyboardHotkey_shift(int isDown)
{
    data.shiftDown = isDown;
}

void KeyboardHotkey_resetState()
{
    data.ctrlDown = 0;
    data.altDown = 0;
    data.shiftDown = 0;
}
