#include "GameTick.h"

#include "Building.h"
#include "CityInfo.h"
#include "Desirability.h"
#include "Empire.h"
#include "Event.h"
#include "Figure.h"
#include "FigureAction.h"
#include "Formation.h"
#include "GameFile.h"
#include "HouseEvolution.h"
#include "HousePopulation.h"
#include "Natives.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Routing.h"
#include "Security.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "Trader.h"
#include "Tutorial.h"
#include "Undo.h"
#include "UtilityManagement.h"
#include "UI/AllWindows.h"
#include "UI/Sidebar.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Settings.h"
#include "Data/State.h"

#include "core/random.h"

#include <stdio.h>

static void advanceDay();
static void advanceMonth();
static void advanceYear();

void GameTick_doTick()
{
	printf("TICK %d.%d.%d\n", Data_CityInfo_Extra.gameTimeMonth, Data_CityInfo_Extra.gameTimeDay, Data_CityInfo_Extra.gameTimeTick);
	random_generate_next();
	Undo_updateAvailable();
	GameTick_advance();
	FigureAction_handle();
	Event_handleEarthquake();
	Event_handleGladiatorRevolt();
	Event_handleEmperorChange();
	CityInfo_Victory_check();
}

void GameTick_advance()
{
	// NB: these ticks are noop:
	// 0, 9, 11, 13, 14, 15, 26, 41, 42, 47
	switch (Data_CityInfo_Extra.gameTimeTick) {
		case 1: CityInfo_Gods_calculateMoods(1); break;
		case 2: Sound_Music_update(); break;
		case 3: UI_Sidebar_requestMinimapRefresh(); break;
		case 4: Event_Caesar_update(); break;
		case 5: Formation_Tick_updateAll(0); break;
		case 6: Natives_checkLand(); break;
		case 7: UtilityManagement_determineRoadNetworks(); break;
		case 8: Resource_gatherGranaryGettingInfo(); break;
		case 10: Building_updateHighestIds(); break;
		case 12: Building_decayHousesCovered(); break;
		case 16: Resource_calculateWarehouseStocks(); break;
		case 17: CityInfo_Resource_calculateFoodAndSupplyRomeWheat(); break;
		case 18: Resource_calculateWorkshopStocks(); break;
		case 19: BUILDING_DOCK_updateOpenWaterAccess(); break;
		case 20: Building_Industry_updateProduction(); break;
		case 21: Building_GameTick_checkAccessToRome(); break;
		case 22: HousePopulation_updateRoom(); break;
		case 23: HousePopulation_updateMigration(); break;
		case 24: HousePopulation_evictOvercrowded(); break;
		case 25: CityInfo_Labor_update(); break;
		case 27: UtilityManagement_updateReservoirFountain(); break;
		case 28: UtilityManagement_updateHouseWaterAccess(); break;
		case 29: Formation_Tick_updateAll(1); break;
		case 30: UI_Sidebar_requestMinimapRefresh(); break;
		case 31: FigureGeneration_generateFiguresForBuildings(); break;
		case 32: Trader_tick(); break;
		case 33: CityInfo_Tick_countBuildingTypes(); CityInfo_Culture_updateCoveragePercentages(); break;
		case 34: CityInfo_Tick_distributeTreasuryOverForumsAndSenates(); break;
		case 35: HouseEvolution_Tick_decayCultureService(); break;
		case 36: HouseEvolution_Tick_calculateCultureServiceAggregates(); break;
		case 37: Desirability_update(); break;
		case 38: Building_setDesirability(); break;
		case 39: HouseEvolution_Tick_evolveAndConsumeResources(); break;
		case 40: Building_GameTick_updateState(); break;
		case 43: Security_Tick_updateBurningRuins(); break;
		case 44: Security_Tick_checkFireCollapse(); break;
		case 45: Security_Tick_generateCriminal(); break;
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
		Data_CityInfo_Extra.gameTimeDay = 0;
		advanceMonth();
	}
	if (Data_CityInfo_Extra.gameTimeDay == 0 || Data_CityInfo_Extra.gameTimeDay == 8) {
		CityInfo_Population_calculateSentiment();
	}
	Tutorial_onDayTick();
}

static void advanceMonth()
{
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
	Formation_Tick_updateRestMorale();
	PlayerMessage_updateMessageDelays();

	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionWater(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	Routing_determineLandCitizen();
	PlayerMessage_sortMessages();

	Data_CityInfo_Extra.gameTimeMonth++;
	if (Data_CityInfo_Extra.gameTimeMonth > 11) {
		Data_CityInfo_Extra.gameTimeMonth = 0;
		advanceYear();
	} else {
		CityInfo_Ratings_calculate(0);
	}

	CityInfo_Population_recordMonthlyPopulation();
	CityInfo_Gods_checkFestival();
	Tutorial_onMonthTick();
	if (Data_Settings.monthlyAutosaveEnabled) {
		GameFile_writeSavedGame("last.sav");
	}
}

static void advanceYear()
{
	Empire_handleExpandEvent();
	Data_State.undoAvailable = 0;
	Data_CityInfo_Extra.gameTimeYear++;
	CityInfo_Population_requestYearlyUpdate();
	CityInfo_Finance_handleYearChange();
	Empire_resetYearlyTradeAmounts();
	Security_Tick_updateFireSpreadDirection();
	CityInfo_Ratings_calculate(1);
	Data_CityInfo.godBlessingNeptuneDoubleTrade = 0;
}
