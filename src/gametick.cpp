#include "gametick.h"

#include "building.h"
#include "cityinfo.h"
#include "desirability.h"
#include "event.h"
#include "figure.h"
#include "figureaction.h"
#include "formation.h"
#include "gamefile.h"
#include "houseevolution.h"
#include "housepopulation.h"
#include "natives.h"
#include "playermessage.h"
#include "resource.h"
#include "routing.h"
#include "security.h"
#include "sidebarmenu.h"
#include "sound.h"
#include "terraingraphics.h"
#include "trader.h"
#include "game/tutorial.h"
#include "undo.h"
#include "utilitymanagement.h"
#include "ui/allwindows.h"
#include "ui/sidebar.h"
#include "ui/window.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "city/message.h"

#include "data/cityinfo.hpp"
#include "data/settings.hpp"
#include "data/state.hpp"
#include "city/culture.h"

#include "core/random.h"
#include "game/settings.h"
#include "game/time.h"

#include <stdio.h>

static void advanceDay();
static void advanceMonth();
static void advanceYear();

void GameTick_doTick()
{
    printf("TICK %d.%d.%d\n", game_time_month(), game_time_day(), game_time_tick());
    random_generate_next();
    Undo::updateAvailable();
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
    switch (game_time_tick())
    {
    case 1:
        CityInfo_Gods_calculateMoods(1);
        break;
    case 2:
        Sound_Music_update();
        break;
    case 3:
        UI_Sidebar_requestMinimapRefresh();
        break;
    case 4:
        Event_Caesar_update();
        break;
    case 5:
        Formation_Tick_updateAll(0);
        break;
    case 6:
        Natives_checkLand();
        break;
    case 7:
        UtilityManagement::determineRoadNetworks();
        break;
    case 8:
        Resource_gatherGranaryGettingInfo();
        break;
    case 10:
        Building_updateHighestIds();
        break;
    case 12:
        Building_decayHousesCovered();
        break;
    case 16:
        Resource_calculateWarehouseStocks();
        break;
    case 17:
        CityInfo_Resource_calculateFoodAndSupplyRomeWheat();
        break;
    case 18:
        Resource_calculateWorkshopStocks();
        break;
    case 19:
        BUILDING_DOCK_updateOpenWaterAccess();
        break;
    case 20:
        Building_Industry_updateProduction();
        break;
    case 21:
        Building_GameTick_checkAccessToRome();
        break;
    case 22:
        HousePopulation_updateRoom();
        break;
    case 23:
        HousePopulation_updateMigration();
        break;
    case 24:
        HousePopulation_evictOvercrowded();
        break;
    case 25:
        CityInfo_Labor_update();
        break;
    case 27:
        UtilityManagement::updateReservoirFountain();
        break;
    case 28:
        UtilityManagement::updateHouseWaterAccess();
        break;
    case 29:
        Formation_Tick_updateAll(1);
        break;
    case 30:
        UI_Sidebar_requestMinimapRefresh();
        break;
    case 31:
        FigureGeneration_generateFiguresForBuildings();
        break;
    case 32:
        Trader_tick();
        break;
    case 33:
        CityInfo_Tick_countBuildingTypes();
        city_culture_update_coverage();
        break;
    case 34:
        CityInfo_Tick_distributeTreasuryOverForumsAndSenates();
        break;
    case 35:
        HouseEvolution_Tick_decayCultureService();
        break;
    case 36:
        HouseEvolution_Tick_calculateCultureServiceAggregates();
        break;
    case 37:
        Desirability_update();
        break;
    case 38:
        Building_setDesirability();
        break;
    case 39:
        HouseEvolution_Tick_evolveAndConsumeResources();
        break;
    case 40:
        Building_GameTick_updateState();
        break;
    case 43:
        Security_Tick_updateBurningRuins();
        break;
    case 44:
        Security_Tick_checkFireCollapse();
        break;
    case 45:
        Security_Tick_generateCriminal();
        break;
    case 46:
        Building_Industry_updateDoubleWheatProduction();
        break;
    case 48:
        CityInfo_Finance_decayTaxCollectorAccess();
        break;
    case 49:
        CityInfo_Culture_calculateEntertainment();
        break;
    }
    if (game_time_advance_tick())
    {
        advanceDay();
    }
}

static void advanceDay()
{
    if (game_time_advance_day())
    {
        advanceMonth();
    }
    if (game_time_day() == 0 || game_time_day() == 8)
    {
        CityInfo_Population_calculateSentiment();
    }
    Tutorial::on_day_tick();
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
    city_message_decrease_delays();

    TerrainGraphics_updateAllRoads();
    TerrainGraphics_updateRegionWater(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
    Routing_determineLandCitizen();
    PlayerMessage_sortMessages();

    if (game_time_advance_month())
    {
        advanceYear();
    }
    else
    {
        CityInfo_Ratings_calculate(0);
    }

    CityInfo_Population_recordMonthlyPopulation();
    CityInfo_Gods_checkFestival();
    Tutorial::on_month_tick();
    if (setting_monthly_autosave())
    {
        GameFile_writeSavedGame("last.sav");
    }
}

static void advanceYear()
{
    empire_handle_expand_event();
    Data_State.undoAvailable = 0;
    game_time_advance_year();
    CityInfo_Population_requestYearlyUpdate();
    CityInfo_Finance_handleYearChange();
    empire_city_reset_yearly_trade_amounts();
    Security_Tick_updateFireSpreadDirection();
    CityInfo_Ratings_calculate(1);
    Data_CityInfo.godBlessingNeptuneDoubleTrade = 0;
}
