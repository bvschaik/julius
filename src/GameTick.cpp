
#include "Building.h"
#include "CityInfo.h"
#include "Desirability.h"
#include "Empire.h"
#include "Event.h"
#include "GameFile.h"
#include "HouseEvolution.h"
#include "HousePopulation.h"
#include "Natives.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "Trader.h"
#include "WaterManagement.h"
#include "UI/AllWindows.h"
#include "UI/Sidebar.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/State.h"
#include "Data/Tutorial.h"

static void advanceDay();
static void advanceMonth();
static void advanceYear();

void GameTick_advance()
{
	// NB: these ticks are noop:
	// 0, 9, 11, 13, 14, 15, 26, 41, 42, 47
	switch (Data_CityInfo_Extra.gameTimeTick) {
		case 1: CityInfo_Gods_calculateMoods(1); break;
		case 2: Sound_Music_update(); break;
		case 3: UI_Sidebar_requestMinimapRefresh(); break;
		case 4: break; // TODO caesar invasion
		case 5: break; // TODO formations 0
		case 6: Natives_checkLand(); break;
		case 7: break; // TODO unknown
		case 8: break; // TODO unknown
		case 10: Building_updateHighestIds(); break;
		case 12: Building_decayHousesCovered(); break;
		case 16: Resource_calculateWarehouseStocks(); break;
		case 17: CityInfo_Resource_calculateFoodAndSupplyRomeWheat(); break;
		case 18: Resource_calculateWorkshopStocks(); break;
		case 19: Building_Dock_updateOpenWaterAccess(); break;
		case 20: Building_Industry_updateProduction(); break;
		case 21: break; // TODO rome pathing access
		case 22: HousePopulation_updateRoom(); break;
		case 23: HousePopulation_updateMigration(); break;
		case 24: HousePopulation_evictOvercrowded(); break;
		case 25: CityInfo_Labor_update(); break;
		case 27: WaterManagement_updateReservoirFountain(); break;
		case 28: WaterManagement_updateHouseWaterAccess(); break;
		case 29: break; // TODO formations 1
		case 30: UI_Sidebar_requestMinimapRefresh(); break;
		case 31: break; // TODO walker generation
		case 32: Trader_tick(); break;
		case 33: CityInfo_Tick_countBuildingTypes(); CityInfo_Culture_updateCoveragePercentages(); break;
		case 34: CityInfo_Tick_distributeTreasuryOverForumsAndSenates(); break;
		case 35: HouseEvolution_Tick_decayCultureService(); break;
		case 36: HouseEvolution_Tick_calculateCultureServiceAggregates(); break;
		case 37: Desirability_update(); break;
		case 38: Building_setDesirability(); break;
		case 39: HouseEvolution_Tick_evolveAndConsumeResources(); break;
		case 40: break; // TODO clear deleted buildings
		case 43: break; // TODO burning ruin
		case 44: break; // TODO crime fire damage
		case 45: break; // TODO criminal
		case 46: Building_Industry_updateDoubleWheatProduction(); break;
		case 48: CityInfo_Finance_decayTaxCollectorAccess(); break;
		case 49: CityInfo_Culture_calculateEntertainment(); break;
	}
	Data_CityInfo_Extra.gameTimeTick++;
	if (Data_CityInfo_Extra.gameTimeTick >= 50) {
		Data_CityInfo_Extra.gameTimeTick = 0;
		advanceDay();
	}
}

static void advanceDay()
{
	Data_CityInfo_Extra.gameTimeDay++;
	Data_CityInfo_Extra.gameTimeTotalDays++;
	if (Data_CityInfo_Extra.gameTimeDay > 15) {
		advanceMonth();
	}
	if (Data_CityInfo_Extra.gameTimeDay == 0 || Data_CityInfo_Extra.gameTimeDay == 8) {
		CityInfo_Population_calculateSentiment();
	}
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
			SidebarMenu_enableBuildingMenuItems();
			SidebarMenu_enableBuildingButtons();
			if (UI_Window_getId() == Window_City) {
				UI_City_drawBackground();
			}
			PlayerMessage_post(1, 57, 0, 0);
		}
	}
	if (Data_Tutorial.tutorial2.population250Reached) {
		if (!Data_Tutorial.tutorial2.population450Reached && Data_CityInfo.population >= 450) {
			Data_Tutorial.tutorial2.population450Reached = 1;
			SidebarMenu_enableBuildingMenuItems();
			SidebarMenu_enableBuildingButtons();
			if (UI_Window_getId() == Window_City) {
				UI_City_drawBackground();
			}
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
				SidebarMenu_enableBuildingMenuItems();
				SidebarMenu_enableBuildingButtons();
				if (UI_Window_getId() == Window_City) {
					UI_City_drawBackground();
				}
				PlayerMessage_post(1, 59, 0, 0);
			}
		}
	}
}

static void advanceMonth()
{
	Data_CityInfo_Extra.gameTimeDay = 0;
	Data_CityInfo.populationNewcomersThisMonth = 0;
	Data_CityInfo.monthsSinceFestival++;

	CityInfo_Population_updateHealthRate();
	Event_handleRandomEvents();
	CityInfo_Finance_handleMonthChange();
	CityInfo_Resource_housesConsumeFood();
	Event_handleDistantBattle();
	Event_handleInvasions();
	Event_handleRequests();
	Event_handleDemandChanges();
	Event_handlePricesChanges();
	CityInfo_Victory_updateMonthsToGovern();
	//TODO j_fun_tickMonth_updateLegionMorale();
	PlayerMessage_updateMessageDelays();

	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionWater(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	Routing_determineLandCitizen();
	PlayerMessage_sortMessages();

	Data_CityInfo_Extra.gameTimeMonth++;
	if (Data_CityInfo_Extra.gameTimeMonth > 11) {
		advanceYear();
	} else {
		CityInfo_Ratings_calculate(0);
	}

	CityInfo_Population_recordMonthlyPopulation();
	CityInfo_Gods_checkFestival();
	/*
    if ( setting_currentMissionId == 2 )
    {
      if ( gametime_month == 5 )
      {
        if ( !setting_isCustomScenario )
        {
          if ( messageDelay_9[0] <= 0 )
          {
            message_usePopup = 1;
            messageDelay_9[0] = 1200;
            j_fun_postMessageToPlayer(58, 0, 0);
          }
        }
      }
    }
	*/
	if (Data_Settings.monthlyAutosaveEnabled) {
		GameFile_writeSavedGame("last.sav");
	}
}

static void advanceYear()
{
	Data_CityInfo_Extra.gameTimeMonth = 0;
	Empire_handleExpandEvent();
	Data_State.undoAvailable = 0;
	Data_CityInfo_Extra.gameTimeYear++;
	CityInfo_Population_requestYearlyUpdate();
	CityInfo_Finance_handleYearChange();
	Empire_resetYearlyTradeAmounts();
	WaterManagement_updateFireSpreadDirection();
	CityInfo_Ratings_calculate(1);
	Data_CityInfo.godBlessingNeptuneDoubleTrade = 0;
}
