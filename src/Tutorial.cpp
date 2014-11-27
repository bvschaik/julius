#include "Tutorial.h"

#include "PlayerMessage.h"
#include "SidebarMenu.h"
#include "UI/AllWindows.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Tutorial.h"

static void drawBackground()
{
	// TODO: why???
	if (UI_Window_getId() == Window_City) {
		UI_City_drawBackground();
	}
}

void Tutorial_onFire()
{
	if (!Data_Tutorial.tutorial1.fire) {
		Data_Tutorial.tutorial1.fire = 1;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
		drawBackground();
		PlayerMessage_post(1, 53, 0, 0);
	}
}

void Tutorial_onCollapse()
{
	if (!Data_Tutorial.tutorial1.collapse) {
		Data_Tutorial.tutorial1.collapse = 1;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
		drawBackground();
		PlayerMessage_post(1, 54, 0, 0);
	}
}

void Tutorial_onFilledGranary()
{
	if (!Data_Tutorial.tutorial2.granaryBuilt) {
		Data_Tutorial.tutorial2.granaryBuilt = 1;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
		drawBackground();
		PlayerMessage_post(1, 56, 0, 0);
	}
}

void Tutorial_onAddToWarehouse()
{
	if (Data_CityInfo.resourceStored[Resource_Pottery] >= 1 &&
			!Data_Tutorial.tutorial2.potteryMade) {
		Data_Tutorial.tutorial2.potteryMade = 1;
		Data_Tutorial.tutorial2.potteryMadeYear = Data_CityInfo_Extra.gameTimeYear;
		SidebarMenu_enableBuildingMenuItemsAndButtons();
		drawBackground();
		PlayerMessage_post(1, 61, 0, 0); // trade
	}
}
