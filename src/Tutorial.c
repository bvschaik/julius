#include "Tutorial.h"

#include "PlayerMessage.h"
#include "SidebarMenu.h"

#include "data/cityinfo.hpp"
#include "data/constants.hpp"
#include "data/message.hpp"
#include "data/scenario.hpp"
#include "data/settings.hpp"
#include "data/tutorial.hpp"

#include "game/time.h"

static void refreshSidebarButtons()
{
	SidebarMenu_enableBuildingMenuItemsAndButtons();
}

void Tutorial_onFire()
{
	if (!Data_Tutorial.tutorial1.fire) {
		Data_Tutorial.tutorial1.fire = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, Message_53_TutorialFirstFire, 0, 0);
	}
}

void Tutorial_onCollapse()
{
	if (!Data_Tutorial.tutorial1.collapse) {
		Data_Tutorial.tutorial1.collapse = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, Message_54_TutorialFirstCollapse, 0, 0);
	}
}

void Tutorial_onFilledGranary()
{
	if (!Data_Tutorial.tutorial2.granaryBuilt) {
		Data_Tutorial.tutorial2.granaryBuilt = 1;
		refreshSidebarButtons();
		PlayerMessage_post(1, Message_56_TutorialProvidingWater, 0, 0);
	}
}

void Tutorial_onAddToWarehouse()
{
	if (Data_CityInfo.resourceStored[Resource_Pottery] >= 1 &&
			!Data_Tutorial.tutorial2.potteryMade) {
		Data_Tutorial.tutorial2.potteryMade = 1;
		Data_Tutorial.tutorial2.potteryMadeYear = game_time_year();
		refreshSidebarButtons();
		PlayerMessage_post(1, Message_61_TutorialTrade, 0, 0);
	}
}

void Tutorial_onDayTick()
{
	if (Data_Tutorial.tutorial1.fire && !Data_CityInfo.tutorial1FireMessageShown) {
		Data_CityInfo.tutorial1FireMessageShown = 1;
	}
	if (Data_Tutorial.tutorial3.disease && !Data_CityInfo.tutorial3DiseaseMessageShown) {
		Data_CityInfo.tutorial3DiseaseMessageShown = 1;
		PlayerMessage_post(1, Message_119_TutorialHealth, 0, 0);
	}
	if (Data_Tutorial.tutorial2.granaryBuilt) {
		if (!Data_Tutorial.tutorial2.population250Reached && Data_CityInfo.population >= 250) {
			Data_Tutorial.tutorial2.population250Reached = 1;
			refreshSidebarButtons();
			PlayerMessage_post(1, Message_57_TutorialGrowingYourCity, 0, 0);
		}
	}
	if (Data_Tutorial.tutorial2.population250Reached) {
		if (!Data_Tutorial.tutorial2.population450Reached && Data_CityInfo.population >= 450) {
			Data_Tutorial.tutorial2.population450Reached = 1;
			refreshSidebarButtons();
			PlayerMessage_post(1, Message_60_TutorialTaxesIndustry, 0, 0);
		}
	}
	if (Data_Tutorial.tutorial1.fire && !Data_Tutorial.tutorial1.senateBuilt) {
		int populationAlmost = Data_CityInfo.population >= Data_Scenario.winCriteria_population - 20;
		if (!game_time_day() || populationAlmost) {
			if (Data_CityInfo.buildingSenateGridOffset) {
				Data_CityInfo.tutorial1SenateBuilt++;
			}
			if (Data_CityInfo.tutorial1SenateBuilt > 0 || populationAlmost) {
				Data_Tutorial.tutorial1.senateBuilt = 1;
				refreshSidebarButtons();
				PlayerMessage_post(1, Message_59_TutorialReligion, 0, 0);
			}
		}
	}
}

void Tutorial_onMonthTick()
{
	if (IsTutorial3()) {
		if (game_time_month() == 5) {
			if (Data_Message.messageDelay[MessageDelay_Tutorial3] <= 0) {
				Data_Message.messageDelay[MessageDelay_Tutorial3] = 1200;
				PlayerMessage_post(1, Message_58_TutorialHungerHaltsImmigrants, 0, 0);
			}
		}
	}
}

