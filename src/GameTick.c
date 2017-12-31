#include "GameTick.h"

#include "Building.h"
#include "CityInfo.h"
#include "Formation.h"
#include "GameFile.h"
#include "HouseEvolution.h"
#include "TerrainGraphics.h"
#include "UI/Sidebar.h"

#include "Data/CityInfo.h"
#include "Data/State.h"

#include "building/count.h"
#include "building/dock.h"
#include "building/figure.h"
#include "building/granary.h"
#include "building/house_population.h"
#include "building/house_service.h"
#include "building/industry.h"
#include "building/maintenance.h"
#include "building/warehouse.h"
#include "city/culture.h"
#include "city/emperor.h"
#include "city/gods.h"
#include "city/health.h"
#include "city/labor.h"
#include "city/message.h"
#include "city/population.h"
#include "city/resource.h"
#include "city/sentiment.h"
#include "city/trade.h"
#include "core/random.h"
#include "empire/city.h"
#include "figure/formation.h"
#include "figuretype/crime.h"
#include "game/settings.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "map/desirability.h"
#include "map/natives.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/water_supply.h"
#include "scenario/demand_change.h"
#include "scenario/distant_battle.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/empire.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/price_change.h"
#include "scenario/random_event.h"
#include "scenario/request.h"
#include "sound/music.h"

#include <stdio.h>

static void advanceDay();
static void advanceMonth();
static void advanceYear();

void GameTick_doTick()
{
    if (game_time_tick() == 0) {
        printf("TICK %d.%d.%d.%d\n", game_time_year(), game_time_month(), game_time_day(), game_time_tick());
    }
	random_generate_next();
	game_undo_reduce_time_available();
	GameTick_advance();
	figure_action_handle();
	scenario_earthquake_process();
	scenario_gladiator_revolt_process();
	scenario_emperor_change_process();
	CityInfo_Victory_check();
}

void GameTick_advance()
{
	// NB: these ticks are noop:
	// 0, 9, 11, 13, 14, 15, 26, 41, 42, 47
	switch (game_time_tick()) {
		case 1: city_gods_calculate_moods(1); break;
		case 2: sound_music_update(); break;
		case 3: UI_Sidebar_requestMinimapRefresh(); break;
		case 4: city_emperor_update(); break;
		case 5: Formation_Tick_updateAll(0); break;
		case 6: map_natives_check_land(); break;
		case 7: map_road_network_update(); break;
		case 8: building_granaries_calculate_stocks(); break;
		case 10: Building_updateHighestIds(); break;
		case 12: Building_decayHousesCovered(); break;
		case 16: building_warehouses_calculate_stocks(); break;
		case 17: city_resource_calculate_food_stocks_and_supply_wheat(); break;
		case 18: building_calculate_workshop_stocks(); break;
		case 19: building_dock_update_open_water_access(); break;
		case 20: building_industry_update_production(); break;
		case 21: Building_GameTick_checkAccessToRome(); break;
		case 22: house_population_update_room(); break;
		case 23: house_population_update_migration(); break;
		case 24: house_population_evict_overcrowded(); break;
		case 25: city_labor_update(); break;
		case 27: map_water_supply_update_reservoir_fountain(); break;
		case 28: map_water_supply_update_houses(); break;
		case 29: Formation_Tick_updateAll(1); break;
		case 30: UI_Sidebar_requestMinimapRefresh(); break;
		case 31: building_figure_generate(); break;
		case 32: city_trade_update(); break;
		case 33: building_count_update(); city_culture_update_coverage(); break;
		case 34: CityInfo_Tick_distributeTreasuryOverForumsAndSenates(); break;
		case 35: house_service_decay_culture(); break;
		case 36: house_service_calculate_culture_aggregates(); break;
		case 37: map_desirability_update(); break;
		case 38: Building_setDesirability(); break;
		case 39: HouseEvolution_Tick_evolveAndConsumeResources(); break;
		case 40: Building_GameTick_updateState(); break;
		case 43: building_maintenance_update_burning_ruins(); break;
		case 44: building_maintenance_check_fire_collapse(); break;
		case 45: figure_generate_criminals(); break;
		case 46: building_industry_update_wheat_production(); break;
		case 48: house_service_decay_tax_collector(); break;
		case 49: city_culture_calculate_entertainment(); break;
	}
	if (game_time_advance_tick()) {
		advanceDay();
	}
}

static void advanceDay()
{
	if (game_time_advance_day()) {
		advanceMonth();
	}
	if (game_time_day() == 0 || game_time_day() == 8) {
		city_sentiment_update();
	}
	tutorial_on_day_tick();
}

static void advanceMonth()
{
	Data_CityInfo.populationNewcomersThisMonth = 0;
	Data_CityInfo.monthsSinceFestival++;

	city_health_update();
	scenario_random_event_process();
	CityInfo_Finance_handleMonthChange();
	city_resource_consume_food();
	scenario_distant_battle_process();
	scenario_invasion_process();
	scenario_request_process();
	scenario_demand_change_process();
	scenario_price_change_process();
	CityInfo_Victory_updateMonthsToGovern();
	formation_update_monthly_morale_at_rest();
	city_message_decrease_delays();

	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionWater(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	map_routing_update_land_citizen();
	city_message_sort_and_compact();

	if (game_time_advance_month()) {
		advanceYear();
	} else {
		CityInfo_Ratings_calculate(0);
	}

	city_population_record_monthly();
	city_gods_check_festival();
	tutorial_on_month_tick();
	if (setting_monthly_autosave()) {
		GameFile_writeSavedGame("last.sav");
	}
}

static void advanceYear()
{
	scenario_empire_process_expansion();
	game_undo_disable();
	game_time_advance_year();
	city_population_request_yearly_update();
	CityInfo_Finance_handleYearChange();
	empire_city_reset_yearly_trade_amounts();
	building_maintenance_update_fire_direction();
	CityInfo_Ratings_calculate(1);
	Data_CityInfo.godBlessingNeptuneDoubleTrade = 0;
}
