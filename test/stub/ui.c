#include "UI/Advisors.h"
#include "UI/MessageDialog.h"
#include "UI/Window.h"

void UI_Tooltip_resetTimer()
{}

void UI_Window_goTo(WindowId windowId)
{}

WindowId UI_Window_getId()
{
    return Window_City;
}

void UI_Window_requestRefresh()
{}

void UI_Window_refresh(int force)
{}

void UI_MessageDialog_show(int textId, int backgroundIsProvided)
{}

void UI_MessageDialog_setPlayerMessage(int year, int month,
                                       int param1, int param2,
                                       int messageAdvisor, int usePopup)
{}

void UI_PopupDialog_show(int msgId, void (*okFunc)(int), int hasOkCancelButtons)
{}

void UI_Intermezzo_show(int type, WindowId nextWindowId, int timeMillis)
{}

void UI_VideoIntermezzo_show(const char* filename, int width, int height, WindowId nextWindowId)
{}

void UI_Sidebar_enableBuildingButtons()
{}

void UI_Sidebar_requestMinimapRefresh()
{}

void UI_Sidebar_rotateMap(int clockWise)
{}

void UI_PlayerMessageList_resetScroll()
{}

void UI_TopMenu_initFromSettings()
{}

int UI_Advisors_getId()
{
    return 0;
}

void UI_Advisors_goToFromMessage(int advisor)
{}

int UI_BuildingInfo_getBuildingType()
{
    return 0;
}