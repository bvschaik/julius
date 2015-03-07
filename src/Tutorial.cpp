#include "Tutorial.h"

#include "PlayerMessage.h"
#include "SidebarMenu.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Scenario.h"
#include "Data/Tutorial.h"

static void refreshSidebarButtons()
{
	SidebarMenu_enableBuildingMenuItemsAndButtons();
}

void Tutorial_onFire()
{
	if (!Data_Tutorial.tutorial1.fire) {
		Data_Tutorial.tutorial1.fire = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, 53, 0, 0);
	}
}

void Tutorial_onCollapse()
{
	if (!Data_Tutorial.tutorial1.collapse) {
		Data_Tutorial.tutorial1.collapse = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, 54, 0, 0);
	}
}

void Tutorial_onFilledGranary()
{
	if (!Data_Tutorial.tutorial2.granaryBuilt) {
		Data_Tutorial.tutorial2.granaryBuilt = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, 56, 0, 0);
	}
}

void Tutorial_onAddToWarehouse()
{
	if (Data_CityInfo.resourceStored[Resource_Pottery] >= 1 &&
			!Data_Tutorial.tutorial2.potteryMade) {
		Data_Tutorial.tutorial2.potteryMade = 1;
		Data_Tutorial.tutorial2.potteryMadeYear = Data_CityInfo_Extra.gameTimeYear;
		refreshSidebarButtons();
		PlayerMessage_post(1, 61, 0, 0); // trade
	}
}

void Tutorial_onDayTick()
{
	if (Data_Tutorial.tutorial1.fire && !Data_CityInfo.tutorial1FireMessageShown) {
		Data_CityInfo.tutorial1FireMessageShown = 1;
	}
	if (Data_Tutorial.tutorial3.disease && !Data_CityInfo.tutorial3DiseaseMessageShown) {
		Data_CityInfo.tutorial3DiseaseMessageShown = 1;
		PlayerMessage_post(1, 119, 0, 0);
	}
	if (Data_Tutorial.tutorial2.granaryBuilt) {
		if (!Data_Tutorial.tutorial2.population250Reached && Data_CityInfo.population >= 250) {
			Data_Tutorial.tutorial2.population250Reached = 1;
			refreshSidebarButtons();
			PlayerMessage_post(1, 57, 0, 0);
		}
	}
	if (Data_Tutorial.tutorial2.population250Reached) {
		if (!Data_Tutorial.tutorial2.population450Reached && Data_CityInfo.population >= 450) {
			Data_Tutorial.tutorial2.population450Reached = 1;
			refreshSidebarButtons();
			PlayerMessage_post(1, 60, 0, 0);
		}
	}
	if (Data_Tutorial.tutorial1.fire && !Data_Tutorial.tutorial1.senateBuilt) {
		int populationAlmost = Data_CityInfo.population >= Data_Scenario.winCriteria_population - 20;
		if (!Data_CityInfo_Extra.gameTimeDay || populationAlmost) {
			if (Data_CityInfo.buildingSenateGridOffset) {
				Data_CityInfo.tutorial1SenateBuilt++;
			}
			if (Data_CityInfo.tutorial1SenateBuilt > 0 || populationAlmost) {
				Data_Tutorial.tutorial1.senateBuilt = 1;
				refreshSidebarButtons();
				PlayerMessage_post(1, 59, 0, 0);
			}
		}
	}
}

