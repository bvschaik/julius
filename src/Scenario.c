#include "Scenario.h"

#include "Animation.h"
#include "Building.h"
#include "core/calc.h"
#include "CityInfo.h"
#include "CityView.h"
#include "Event.h"
#include "Figure.h"
#include "Formation.h"
#include "GameFile.h"
#include "Grid.h"
#include "Loader.h"
#include "Natives.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Terrain.h"
#include "TerrainGraphics.h"

#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Event.h"
#include "Data/FileList.h"
#include "Data/Grid.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/State.h"

#include "building/storage.h"
#include "city/message.h"
#include "core/file.h"
#include "core/io.h"
#include "core/random.h"
#include "empire/empire.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/trader.h"
#include "game/difficulty.h"
#include "game/settings.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "graphics/image.h"
#include "scenario/demand_change.h"
#include "scenario/price_change.h"
#include "scenario/request.h"
#include "sound/city.h"
#include "sound/music.h"

#include <string.h>

static int mapFileExists(const char *scenarioName);
static void clearBookmarks();
static void initCustomScenario(const char *scenarioName);
static void loadScenario(const char *scenarioName);
static void readScenarioAndInitGraphics();

static void initGrids();
static void initGridTerrain();
static void initGridRandom();
static void initGridGraphicIds();

void Scenario_initialize(const char *scenarioName)
{
	int saveMissionId = Data_Settings.saveGameMissionId;
	int curMissionId = Data_Settings.currentMissionId;
	clearBookmarks();
	if (Data_Settings.isCustomScenario) {
		if (!mapFileExists(scenarioName)) {
			UI_Window_goTo(Window_City);
			return;
		}
		initCustomScenario(scenarioName);
	} else {
		if (!GameFile_loadSavedGameFromMissionPack(saveMissionId)) {
			UI_Window_goTo(Window_City);
			return;
		}
		Data_CityInfo.treasury = difficulty_adjust_money(Data_CityInfo.treasury);
	}
	Data_Settings.saveGameMissionId = saveMissionId;
	Data_Settings.currentMissionId = curMissionId;

	Data_CityInfo_Extra.startingFavor = difficulty_starting_favor();
	Data_Settings.personalSavingsLastMission = setting_personal_savings_for_mission(curMissionId);
	
	Data_CityInfo.ratingFavor = Data_CityInfo_Extra.startingFavor;
	Data_CityInfo.personalSavings = Data_Settings.personalSavingsLastMission;
	Data_CityInfo.playerRank = curMissionId;
	Data_CityInfo.salaryRank = curMissionId;
	if (Data_Settings.isCustomScenario) {
		Data_CityInfo.personalSavings = 0;
		Data_CityInfo.playerRank = Data_Scenario.playerRank;
		Data_CityInfo.salaryRank = Data_Scenario.playerRank;
	}
	if (Data_CityInfo.salaryRank > 10) {
		Data_CityInfo.salaryRank = 10;
	}
	Data_CityInfo.salaryAmount = Constant_SalaryForRank[Data_CityInfo.salaryRank];

	tutorial_init();

	if (IsTutorial1()) {
		setting_set_personal_savings_for_mission(0, 0);
		Data_Settings.personalSavingsLastMission = 0;
		Data_CityInfo.personalSavings = 0;
	}

	SidebarMenu_enableBuildingMenuItemsAndButtons();
	city_message_init_scenario();
}

static void clearBookmarks()
{
	for (int i = 0; i < 4; i++) {
		Data_CityInfo_Extra.bookmarks[i].x = -1;
		Data_CityInfo_Extra.bookmarks[i].y = -1;
	}
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
	Data_State.selectedBuilding.type = 0;
	CityInfo_init();
	Data_CityInfo_Extra.ciid = 1;
	city_message_init_scenario();
	Loader_GameState_init();
	Animation_resetTimers();
	sound_city_init();
	sound_music_reset();
	SidebarMenu_enableAllBuildingMenuItems();
	Building_clearList();
	building_storage_clear_all();
	Figure_clearList();
	enemy_armies_clear();
	figure_name_init();
    formations_clear();
	FigureRoute_clearList();
	CityInfo_initGameTime();

	loadScenario(scenarioName);

	Data_CityInfo_Extra.ciid = 1;
	Data_CityInfo.__unknown_00a2 = 1;
	Data_CityInfo.__unknown_00a3 = 1;
}

static void loadScenario(const char *scenarioName)
{
	Data_CityInfo_Extra.ciid = 1;
	strcpy(Data_FileList.selectedScenario, scenarioName);
	readScenarioAndInitGraphics();
	int hasWaterEntry = 0;
	if (Data_Scenario.riverEntryPoint.x != -1 &&
		Data_Scenario.riverEntryPoint.y != -1 &&
		Data_Scenario.riverExitPoint.x != -1 &&
		Data_Scenario.riverExitPoint.x != -1) {
		hasWaterEntry = 1;
	}
	Figure_createFishingPoints();
	Figure_createHerds();
	Figure_createFlotsam(Data_Scenario.riverEntryPoint.x, Data_Scenario.riverEntryPoint.y, hasWaterEntry);

	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();
	Routing_determineWalls();

	if (Data_Scenario.entryPoint.x == -1 || Data_Scenario.entryPoint.y == -1) {
		Data_Scenario.entryPoint.x = Data_Settings_Map.width - 1;
		Data_Scenario.entryPoint.y = Data_Settings_Map.height / 2;
	}
	Data_CityInfo.entryPointX = Data_Scenario.entryPoint.x;
	Data_CityInfo.entryPointY = Data_Scenario.entryPoint.y;
	Data_CityInfo.entryPointGridOffset = GridOffset(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);

	if (Data_Scenario.exitPoint.x == -1 || Data_Scenario.exitPoint.y == -1) {
		Data_Scenario.exitPoint.x = Data_Scenario.entryPoint.x;
		Data_Scenario.exitPoint.y = Data_Scenario.entryPoint.y;
	}
	Data_CityInfo.exitPointX = Data_Scenario.exitPoint.x;
	Data_CityInfo.exitPointY = Data_Scenario.exitPoint.y;
	Data_CityInfo.exitPointGridOffset = GridOffset(Data_CityInfo.exitPointX, Data_CityInfo.exitPointY);
	Data_CityInfo.treasury = difficulty_adjust_money(Data_Scenario.startFunds);
	Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
	game_time_init(Data_Scenario.startYear);

	// set up events
	// earthquake
	Data_Event.earthquake.gameYear = Data_Scenario.startYear + Data_Scenario.earthquakeYear;
	Data_Event.earthquake.month = 2 + (random_byte() & 7);
	switch (Data_Scenario.earthquakeSeverity) {
		default:
			Data_Event.earthquake.maxDuration = 0;
			Data_Event.earthquake.maxDelay = 0;
			break;
		case Earthquake_Small:
			Data_Event.earthquake.maxDuration = 25 + (random_byte() & 0x1f);
			Data_Event.earthquake.maxDelay = 10;
			break;
		case Earthquake_Medium:
			Data_Event.earthquake.maxDuration = 100 + (random_byte() & 0x3f);
			Data_Event.earthquake.maxDelay = 8;
			break;
		case Earthquake_Large:
			Data_Event.earthquake.maxDuration = 250 + random_byte();
			Data_Event.earthquake.maxDelay = 6;
			break;
	}
	Data_Event.earthquake.state = SpecialEvent_NotStarted;
	for (int i = 0; i < 4; i++) {
		Data_Event.earthquake.expand[i].x = Data_Scenario.earthquakePoint.x;
		Data_Event.earthquake.expand[i].y = Data_Scenario.earthquakePoint.y;
	}
	// gladiator revolt
	Data_Event.gladiatorRevolt.gameYear = Data_Scenario.startYear + Data_Scenario.gladiatorRevolt.year;
	Data_Event.gladiatorRevolt.month = 3 + (random_byte() & 3);
	Data_Event.gladiatorRevolt.endMonth = 3 + Data_Event.gladiatorRevolt.month;
	Data_Event.gladiatorRevolt.state = SpecialEvent_NotStarted;
	// emperor change
	Data_Event.emperorChange.gameYear = Data_Scenario.startYear + Data_Scenario.emperorChange.year;
	Data_Event.emperorChange.month = 1 + (random_byte() & 7);
	Data_Event.emperorChange.state = 0;
	// time limit
	if (Data_Scenario.winCriteria.timeLimitYearsEnabled) {
		Data_Event.timeLimitMaxGameYear = Data_Scenario.startYear + Data_Scenario.winCriteria.timeLimitYears;
	} else if (Data_Scenario.winCriteria.survivalYearsEnabled) {
		Data_Event.timeLimitMaxGameYear = Data_Scenario.startYear + Data_Scenario.winCriteria.survivalYears;
	} else {
		Data_Event.timeLimitMaxGameYear = 1000000 + Data_Scenario.startYear;
	}

	empire_init_scenario();
	traders_clear();
	Event_initInvasions();
	empire_determine_distant_battle_city();
	scenario_request_init();
	scenario_demand_change_init();
	scenario_price_change_init();
	SidebarMenu_enableBuildingMenuItemsAndButtons();
	image_load_climate(Data_Scenario.climate);
	image_load_enemy(Data_Scenario.enemyId);
}

static void readScenarioAndInitGraphics()
{
	initGrids();
	file_remove_extension(Data_FileList.selectedScenario);
	file_append_extension(Data_FileList.selectedScenario, "map");
	GameFile_loadScenario(Data_FileList.selectedScenario);
	file_remove_extension(Data_FileList.selectedScenario);

	Data_Settings_Map.width = Data_Scenario.mapSizeX;
	Data_Settings_Map.height = Data_Scenario.mapSizeY;
	Data_Settings_Map.gridStartOffset = Data_Scenario.gridFirstElement;
	Data_Settings_Map.gridBorderSize = Data_Scenario.gridBorderSize;

	CityView_calculateLookup();
	TerrainGraphics_updateRegionElevation(0, 0, Data_Settings_Map.width - 2, Data_Settings_Map.height - 2);
	TerrainGraphics_updateRegionWater(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateRegionEarthquake(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateAllRocks();
	Terrain_updateEntryExitFlags(0);
	TerrainGraphics_updateRegionEmptyLand(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateRegionMeadow(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateAllWalls();
	TerrainGraphics_updateRegionAqueduct(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1, 0);

	Natives_init();

	CityView_checkCameraBoundaries();

	Routing_clearLandTypeCitizen();
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();
	Routing_determineWalls();
}

static void initGrids()
{
	Grid_clearShortGrid(Data_Grid_graphicIds);
	Grid_clearUByteGrid(Data_Grid_edge);
	Grid_clearShortGrid(Data_Grid_buildingIds);
	Grid_clearShortGrid(Data_Grid_terrain);
	Grid_clearUByteGrid(Data_Grid_aqueducts);
	Grid_clearShortGrid(Data_Grid_figureIds);
	Grid_clearUByteGrid(Data_Grid_bitfields);
	Grid_clearUByteGrid(Data_Grid_spriteOffsets);
	Grid_clearUByteGrid(Data_Grid_random);
	Grid_clearByteGrid(Data_Grid_desirability);
	Grid_clearUByteGrid(Data_Grid_elevation);
	Grid_clearUByteGrid(Data_Grid_buildingDamage);
	Grid_clearUByteGrid(Data_Grid_rubbleBuildingType);
	Grid_clearUByteGrid(Data_Grid_romanSoldierConcentration);
	Grid_clearUByteGrid(Data_Grid_roadNetworks);

	TerrainGraphicsContext_init();
	initGridTerrain();
	initGridRandom();
	initGridGraphicIds();
}

static void initGridTerrain()
{
	int gridOffset = 0;
	for (int y = 0; y < Data_Settings_Map.height; y++) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			if (x < (GRID_SIZE - Data_Settings_Map.width) / 2 ||
				x >= (GRID_SIZE - Data_Settings_Map.width) / 2 + Data_Settings_Map.width) {
				Data_Grid_terrain[gridOffset] = Terrain_OutsideMap;
			}
			if (y < (GRID_SIZE - Data_Settings_Map.height) / 2 ||
				y >= (GRID_SIZE - Data_Settings_Map.height) / 2 + Data_Settings_Map.height) {
				Data_Grid_terrain[gridOffset] = Terrain_OutsideMap;
			}
		}
	}
}

static void initGridRandom()
{
	int gridOffset = 0;
	for (int y = 0; y < Data_Settings_Map.height; y++) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			random_generate_next();
			Data_Grid_random[gridOffset] = random_short();
		}
	}
}

static void initGridGraphicIds()
{
	int gridOffset = Data_Settings_Map.gridStartOffset;
	int graphicId = image_group(ID_Graphic_TerrainUglyGrass);
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			Data_Grid_graphicIds[gridOffset] = graphicId + (Data_Grid_random[gridOffset] & 7);
			if (Data_Grid_random[gridOffset] & 1) {
				Data_Grid_bitfields[gridOffset] |= Bitfield_AlternateTerrain;
			}
		}
	}
}
