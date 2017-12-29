#include "Scenario.h"

#include "Building.h"
#include "CityInfo.h"
#include "CityView.h"
#include "GameFile.h"
#include "Loader.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/State.h"

#include "building/construction.h"
#include "building/storage.h"
#include "city/message.h"
#include "core/calc.h"
#include "core/file.h"
#include "core/string.h"
#include "empire/empire.h"
#include "figure/enemy_army.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figuretype/animal.h"
#include "figuretype/water.h"
#include "game/animation.h"
#include "game/difficulty.h"
#include "game/settings.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/bookmark.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/image_context.h"
#include "map/natives.h"
#include "map/property.h"
#include "map/random.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/soldier_strength.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "scenario/criteria.h"
#include "scenario/demand_change.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/price_change.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/music.h"

#include <string.h>

static int mapFileExists(const char *scenarioName);
static void initCustomScenario(const char *scenarioName);
static void loadScenario(const char *scenarioName);
static void readScenarioAndInitGraphics(const char *scenarioName);

static void initGrids();

int Scenario_initialize(const char *scenarioName)
{
	int mission = scenario_campaign_mission();
	int rank = scenario_campaign_rank();
	map_bookmarks_clear();
	if (scenario_is_custom()) {
		if (!mapFileExists(scenarioName)) {
			return 0;
		}
		initCustomScenario(scenarioName);
	} else {
		if (!GameFile_loadSavedGameFromMissionPack(mission)) {
			return 0;
		}
		Data_CityInfo.treasury = difficulty_adjust_money(Data_CityInfo.treasury);
	}
	scenario_set_campaign_mission(mission);
	scenario_set_campaign_rank(rank);

    if (scenario_is_tutorial_1()) {
        setting_set_personal_savings_for_mission(0, 0);
    }

    scenario_settings_init_mission();

	Data_CityInfo.ratingFavor = scenario_starting_favor();
	Data_CityInfo.personalSavings = scenario_starting_personal_savings();
	Data_CityInfo.playerRank = rank;
	Data_CityInfo.salaryRank = rank;
	if (scenario_is_custom()) {
		Data_CityInfo.personalSavings = 0;
		Data_CityInfo.playerRank = scenario_property_player_rank();
		Data_CityInfo.salaryRank = scenario_property_player_rank();
	}
	if (Data_CityInfo.salaryRank > 10) {
		Data_CityInfo.salaryRank = 10;
	}
	Data_CityInfo.salaryAmount = Constant_SalaryForRank[Data_CityInfo.salaryRank];

	tutorial_init();

	SidebarMenu_enableBuildingMenuItemsAndButtons();
	city_message_init_scenario();
	return 1;
}

static int mapFileExists(const char *scenarioName)
{
	char filename[FILE_NAME_MAX];
	strcpy(filename, scenarioName);
	file_remove_extension(filename);
	file_append_extension(filename, "map");
	return file_exists(filename);
}

static void initCustomScenario(const char *scenarioName)
{
	Data_State.winState = WinState_None;
	Data_State.forceWinCheat = 0;
	building_construction_clear_type();
	CityInfo_init();
	Data_CityInfo_Extra.ciid = 1;
	city_message_init_scenario();
	Loader_GameState_init();
	game_animation_init();
	sound_city_init();
	sound_music_reset();
	SidebarMenu_enableAllBuildingMenuItems();
	Building_clearList();
	building_storage_clear_all();
	figure_init_scenario();
	enemy_armies_clear();
	figure_name_init();
    formations_clear();
	figure_route_clear_all();
	CityInfo_initGameTime();

	loadScenario(scenarioName);

	Data_CityInfo_Extra.ciid = 1;
	Data_CityInfo.__unknown_00a2 = 1;
	Data_CityInfo.__unknown_00a3 = 1;
}

static void loadScenario(const char *scenarioName)
{
	Data_CityInfo_Extra.ciid = 1;
	readScenarioAndInitGraphics(scenarioName);

	figure_create_fishing_points();
	figure_create_herds();
	figure_create_flotsam();

	map_routing_update_land();
	map_routing_update_water();
	map_routing_update_walls();

	scenario_map_init_entry_exit();

    map_point entry = scenario_map_entry();
    map_point exit = scenario_map_exit();

	Data_CityInfo.entryPointX = entry.x;
	Data_CityInfo.entryPointY = entry.y;
	Data_CityInfo.entryPointGridOffset = map_grid_offset(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);

	Data_CityInfo.exitPointX = exit.x;
	Data_CityInfo.exitPointY = exit.y;
	Data_CityInfo.exitPointGridOffset = map_grid_offset(Data_CityInfo.exitPointX, Data_CityInfo.exitPointY);

	Data_CityInfo.treasury = difficulty_adjust_money(scenario_initial_funds());
	Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
	game_time_init(scenario_property_start_year());

	// set up events
	scenario_earthquake_init();
	scenario_gladiator_revolt_init();
	scenario_emperor_change_init();
	scenario_criteria_init_max_year();

	empire_init_scenario();
	traders_clear();
	scenario_invasion_init();
	empire_determine_distant_battle_city();
	scenario_request_init();
	scenario_demand_change_init();
	scenario_price_change_init();
	SidebarMenu_enableBuildingMenuItemsAndButtons();
	image_load_climate(scenario_property_climate());
	image_load_enemy(scenario_property_enemy());
}

static void readScenarioAndInitGraphics(const char *scenarioName)
{
    char filename[FILE_NAME_MAX];
    strncpy(filename, scenarioName, FILE_NAME_MAX);
	initGrids();
	file_remove_extension(filename);
	file_append_extension(filename, "map");
	GameFile_loadScenario(filename);
	file_remove_extension(filename);

	scenario_set_name(string_from_ascii(filename));
	scenario_map_init();

	CityView_calculateLookup();
	TerrainGraphics_updateRegionElevation(0, 0, Data_State.map.width - 2, Data_State.map.height - 2);
	TerrainGraphics_updateRegionWater(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionEarthquake(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllRocks();
	Terrain_updateEntryExitFlags(0);
	TerrainGraphics_updateRegionEmptyLand(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateRegionMeadow(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);
	TerrainGraphics_updateAllWalls();
	TerrainGraphics_updateRegionAqueduct(0, 0, Data_State.map.width - 1, Data_State.map.height - 1, 0);

	map_natives_init();

	CityView_checkCameraBoundaries();

	map_routing_update_all();
}

static void initGrids()
{
	map_image_clear();
	map_building_clear();
	map_terrain_clear();
	map_aqueduct_clear();
	map_figure_clear();
	map_property_clear();
	map_sprite_clear();
	map_random_clear();
	map_desirability_clear();
	map_elevation_clear();
	map_soldier_strength_clear();
	map_road_network_clear();

	map_image_context_init();
	map_random_init();
}
