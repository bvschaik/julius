#include "Scenario.h"

#include "Animation.h"
#include "Building.h"
#include "Calc.h"
#include "CityInfo.h"
#include "CityView.h"
#include "Empire.h"
#include "Event.h"
#include "FileSystem.h"
#include "Formation.h"
#include "GameFile.h"
#include "Loader.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "Trader.h"
#include "Walker.h"

#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/FileList.h"
#include "Data/Graphics.h"
#include "Data/Grid.h"
#include "Data/Model.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/State.h"
#include "Data/Tutorial.h"

#include <string.h>
#include <stdio.h> // debug

static int mapFileExists(const char *scenarioName);
static void clearBookmarks();
static void setTutorialFlags(int missionId);
static void initCustomScenario(const char *scenarioName);
static void loadScenario(const char *scenarioName);
static void readScenarioAndInitGraphics();

static void initGrids();
static void initGridTerrain();
static void initGridRandom();
static void initGridGraphicIds();

void Scenario_initialize(const char *scenarioName)
{
	int missionId = Data_Settings.saveGameMissionId;
	clearBookmarks();
	if (Data_Settings.isCustomScenario) {
		if (!mapFileExists(scenarioName)) {
			UI_Window_goTo(Window_City);
			return;
		}
		initCustomScenario(scenarioName);
	} else {
		if (!GameFile_loadSavedGameFromMissionPack(missionId)) {
			UI_Window_goTo(Window_City);
			return;
		}
		Data_CityInfo.treasury = Calc_adjustWithPercentage(Data_CityInfo.treasury,
			Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
	}
	Data_Settings.saveGameMissionId = missionId;

	Data_CityInfo_Extra.startingFavor = Data_Model_Difficulty.startingFavor[Data_Settings.difficulty];
	Data_Settings.personalSavingsLastMission = Data_Settings.personalSavingsPerMission[missionId];
	
	Data_CityInfo.ratingFavor = Data_CityInfo_Extra.startingFavor;
	Data_CityInfo.personalSavings = Data_Settings.personalSavingsLastMission;
	Data_CityInfo.playerRank = missionId;
	Data_CityInfo.salaryRank = missionId;
	if (Data_Settings.isCustomScenario) {
		Data_CityInfo.personalSavings = 0;
		Data_CityInfo.playerRank = Data_Scenario.playerRank;
		Data_CityInfo.salaryRank = Data_Scenario.playerRank;
	}
	if (Data_CityInfo.salaryRank > 10) {
		Data_CityInfo.salaryRank = 10;
	}
	Data_CityInfo.salaryAmount = Constant_SalaryForRank[Data_CityInfo.salaryRank];

	setTutorialFlags(missionId);

	if (IsTutorial1()) {
		Data_Settings.personalSavingsPerMission[0] = 0;
		Data_Settings.personalSavingsLastMission = 0;
		Data_CityInfo.personalSavings = 0;
	}

	SidebarMenu_enableBuildingButtons();
	SidebarMenu_enableBuildingMenuItems();
	PlayerMessage_initList();
}

static void clearBookmarks()
{
	for (int i = 0; i < 4; i++) {
		Data_CityInfo_Extra.bookmarks[i].x = -1;
		Data_CityInfo_Extra.bookmarks[i].y = -1;
	}
}

static void setTutorialFlags(int missionId)
{
	int tut1, tut2, tut3;
	if (Data_Settings.isCustomScenario) {
		tut1 = tut2 = tut3 = 1;
	} else if (missionId == 0) {
		tut1 = tut2 = 0;
		tut3 = 1;
	} else if (missionId == 1) {
		tut1 = 1;
		tut2 = 0;
		tut3 = 1;
	} else if (missionId == 3) {
		tut1 = 1;
		tut2 = 1;
		tut3 = 0;
	} else {
		tut1 = tut2 = tut3 = 1;
	}

	Data_Tutorial.tutorial1.fire = tut1;
	Data_Tutorial.tutorial1.crime = tut1;
	Data_Tutorial.tutorial1.collapse = tut1;
	Data_Tutorial.tutorial1.senateBuilt = tut1;
	Data_CityInfo.tutorial1FireMessageShown = tut1;

	Data_Tutorial.tutorial2.granaryBuilt = tut2;
	Data_Tutorial.tutorial2.population250Reached = tut2;
	Data_Tutorial.tutorial2.population450Reached = tut2;
	Data_Tutorial.tutorial2.potteryMade = tut2;
	Data_Tutorial.tutorial2.potteryMadeYear = tut2;

	Data_Tutorial.tutorial3.disease = tut3;
	Data_CityInfo.tutorial3DiseaseMessageShown = tut3;
}

static int mapFileExists(const char *scenarioName)
{
	char filename[FILESYSTEM_MAX_FILENAME];
	strcpy(filename, scenarioName);
	FileSystem_removeExtension(filename);
	FileSystem_appendExtension(filename, "map");
	return FileSystem_fileExists(filename);
}

static void initCustomScenario(const char *scenarioName)
{
	Data_State.winState = 0;
	Data_State.forceWinCheat = 0;
	Data_State.selectedBuilding.type = 0;
	CityInfo_init();
	PlayerMessage_initList();
	Loader_GameState_init();
	Animation_resetTimers();
	Sound_City_init();
	Sound_Music_reset();
	SidebarMenu_enableAllBuildingMenuItems();
	Building_clearList();
	BuildingStorage_clearList();
	Walker_clearList();
	Formation_clearInvasionInfo();
	WalkerName_init();
	Formation_clearList();
	WalkerName_init();
	CityInfo_initGameTime();

	loadScenario(scenarioName);

	Data_CityInfo.__unknown_00a2 = 1;
	Data_CityInfo.__unknown_00a3 = 1;
	Empire_determineDistantBattleCity();
}

static void loadScenario(const char *scenarioName)
{
	// TODO
	strcpy(Data_FileList.selectedScenario, scenarioName);
	readScenarioAndInitGraphics();
/*
  hasWaterEntry = 0;
  ciid = 1;
  j_fun_memcpy(&map_settings_startYear, &scn_settings_startYear, 1720);
  if ( !mode_editor )
  {
    if ( map_riverEntry_x != -1 )
    {
      if ( map_riverEntry_y != -1 )
      {
        if ( map_riverExit_x != -1 )
        {
          if ( map_riverExit_y != -1 )
            hasWaterEntry = 1;
        }
      }
    }
    j_fun_createFishHerdFlotsamWalkers(map_riverEntry_x, map_riverEntry_y, hasWaterEntry);
  }
  j_fun_calculateOpenGroundCitizen();
  j_fun_calculateOpenGroundNonCitizen();
  j_fun_calculateWaterDepth();
  j_fun_calculateWallPaths();
  if ( scn_entryPoint_x == -1 || scn_entryPoint_y == -1 )
  {
    scn_entryPoint_x = setting_map_width - 1;
    scn_entryPoint_y = setting_map_height / 2;
  }
  cityinfo_entry_x[18068 * ciid] = scn_entryPoint_x;
  cityinfo_entry_y[18068 * ciid] = scn_entryPoint_y;
  cityinfo_entry_gridOffset[9034 * ciid] = 162 * (unsigned __int8)cityinfo_entry_y[18068 * ciid]
                                         + (unsigned __int8)cityinfo_entry_x[18068 * ciid]
                                         + (_WORD)setting_map_startGridOffset;
  if ( scn_exitPoint_x == -1 || scn_exitPoint_y == -1 )
  {
    scn_exitPoint_x = scn_entryPoint_x;
    scn_exitPoint_y = scn_entryPoint_y;
  }
  cityinfo_exit_x[18068 * ciid] = scn_exitPoint_x;
  cityinfo_exit_y[18068 * ciid] = scn_exitPoint_y;
  cityinfo_exit_gridOffset[9034 * ciid] = 162 * (unsigned __int8)cityinfo_exit_y[18068 * ciid]
                                        + (unsigned __int8)cityinfo_exit_x[18068 * ciid]
                                        + (_WORD)setting_map_startGridOffset;
  cityinfo_treasury[4517 * ciid] = j_fun_adjustWithPercentage(scn_startFunds, difficulty_moneypct[setting_difficulty]);
  cityinfo_finance_balance_lastyear[4517 * ciid] = cityinfo_treasury[4517 * ciid];
  gametime_year = scn_settings_startYear;
  event_earthquake_gameYear = scn_event_earthquake_year + scn_settings_startYear;
  event_earthquake_month = (random_7f_1 & 7) + 2;
  if ( scn_event_earthquake_severity )
  {
    switch ( scn_event_earthquake_severity )
    {
      case 1:
        event_earthquake_maxDuration = (random_7f_1 & 0x1F) + 25;
        event_earthquake_maxDamage = 10;
        break;
      case 2:
        event_earthquake_maxDuration = (random_7f_1 & 0x3F) + 100;
        event_earthquake_maxDamage = 8;
        break;
      case 3:
        event_earthquake_maxDuration = random_7f_1 + 250;
        event_earthquake_maxDamage = 6;
        break;
    }
  }
  else
  {
    event_earthquake_maxDuration = 0;
    event_earthquake_maxDamage = 0;
  }
  event_earthquake_state = 0;
  for ( i = 0; i < 4; ++i )
  {
    dword_929660[2 * i] = scn_earthquake_x;
    dword_929664[2 * i] = scn_earthquake_y;
  }
  event_gladiatorRevolt_gameYear = scn_event_gladiatorRevolt_year + scn_settings_startYear;
  event_gladiatorRevolt_month = (random_7f_1 & 3) + 3;
  event_gladiatorRevold_endMonth = event_gladiatorRevolt_month + 3;
  event_gladiatorRevolt_state = 0;
  event_emperorChange_gameYear = scn_event_emperorChange_year + scn_settings_startYear;
  event_emperorChange_month = (random_7f_1 & 7) + 1;
  event_emperorChange_state = 0;
  if ( scn_win_timeLimit_on )
  {
    timeLimit_maxGameYear = scn_win_timeLimit + scn_settings_startYear;
  }
  else
  {
    if ( scn_win_survivalTime_on )
      timeLimit_maxGameYear = scn_win_survivalTime + scn_settings_startYear;
    else
      timeLimit_maxGameYear = scn_settings_startYear + 1000000;
  }
*/

	Empire_load(1, Data_Scenario.empireId);
	Empire_initCities();
	Trader_clearList();
	//TODO Event_initInvasions();
	Empire_determineDistantBattleCity();
	Event_initRequests();
	Event_initDemandChanges();
	Event_initPriceChanges();
	Data_Empire.scrollX = Data_Empire_Index[Data_Scenario.empireId].initialScrollX;
	Data_Empire.scrollY = Data_Empire_Index[Data_Scenario.empireId].initialScrollY;
	Empire_checkScrollBoundaries();
	SidebarMenu_enableBuildingButtons();
	SidebarMenu_enableBuildingMenuItems();
	Loader_Graphics_loadMainGraphics(Data_Scenario.climate);
	Loader_Graphics_loadEnemyGraphics(Data_Scenario.enemyId);
}

static void readScenarioAndInitGraphics()
{
	initGrids();
	FileSystem_removeExtension(Data_FileList.selectedScenario);
	FileSystem_appendExtension(Data_FileList.selectedScenario, "map");
	GameFile_loadScenario(Data_FileList.selectedScenario);
	FileSystem_removeExtension(Data_FileList.selectedScenario);
	// TODO remove
	initGridGraphicIds();

	Empire_initTradeAmountCodes();
	Data_Settings_Map.width = Data_Scenario.mapSizeX;
	Data_Settings_Map.height = Data_Scenario.mapSizeY;
	Data_Settings_Map.gridStartOffset = Data_Scenario.gridFirstElement;
	Data_Settings_Map.gridBorderSize = Data_Scenario.gridBorderSize;

	CityView_calculateLookup();
	/*
  j_fun_determineGraphicIdsForElevation(0, 0, setting_map_width - 2, setting_map_height - 2);
  j_fun_determineGraphicIdsForWater(0, 0, setting_map_width - 1, setting_map_height - 1);
  j_fun_determineGraphicIdsForEarthquakeLines(0, 0, setting_map_width - 1, setting_map_height - 1);
  */
	TerrainGraphics_updateAllRocks();
  /*
  sub_402400(0);
  */
	TerrainGraphics_updateRegionEmptyLand(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateRegionMeadow(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
	TerrainGraphics_updateAllRoads();
	TerrainGraphics_updateRegionPlazas(0, 0, Data_Settings_Map.width - 1, Data_Settings_Map.height - 1);
/*
  sub_402BA8();
  sub_4014F1(0, 0, setting_map_width - 1, setting_map_height - 1, 0);
  j_fun_scenarioLoad_setupNatives();
	*/

	CityView_checkCameraBoundaries();

	Routing_clearLandTypeCitizen();
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();
	Routing_determineWalls();
}

static void initGrids()
{
	int lenByte = GRID_SIZE * GRID_SIZE;
	int lenShort = 2 * lenByte;
	memset(Data_Grid_graphicIds, 0, lenShort);
	memset(Data_Grid_edge, 0, lenByte);
	memset(Data_Grid_buildingIds, 0, lenShort);
	memset(Data_Grid_terrain, 0, lenShort);
	memset(Data_Grid_aqueducts, 0, lenByte);
	memset(Data_Grid_walkerIds, 0, lenShort);
	memset(Data_Grid_bitfields, 0, lenByte);
	memset(Data_Grid_spriteOffsets, 0, lenByte);
	memset(Data_Grid_random, 0, lenByte);
	memset(Data_Grid_desirability, 0, lenByte);
	memset(Data_Grid_elevation, 0, lenByte);
	memset(Data_Grid_buildingDamage, 0, lenByte);
	memset(Data_Grid_rubbleBuildingType, 0, lenByte);
	// TODO
	//memset(Data_Grid_romanSoldierConcentration, 0, lenByte);
	//memset(Data_Grid_byte_91C920, 0, lenByte);
	//memset(Data_Grid_byte_8ADF60, 0, lenByte);
	/* TODO
	j_fun_setByte14Of16Zero(byte_5F2250, 48);
	j_fun_setByte14Of16Zero(byte_5F2550, 48);
	j_fun_setByte14Of16Zero(byte_5F2850, 10);
	j_fun_setByte14Of16Zero(byte_5F28F0, 14);
	j_fun_setByte14Of16Zero(byte_5F29D0, 17);
	j_fun_setByte14Of16Zero(byte_5F2AE0, 17);
	j_fun_setByte14Of16Zero(byte_5F2BF0, 17);
	j_fun_setByte14Of16Zero(byte_5F2D00, 48);
	j_fun_setByte14Of16Zero(byte_5F3000, 16);
	*/
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
			Random_generateNext();
			Data_Grid_random[gridOffset] = Data_Random.random1_15bit;
		}
	}
}

static void initGridGraphicIds()
{
	int gridOffset = Data_Settings_Map.gridStartOffset;
	int graphicId = GraphicId(ID_Graphic_TerrainUglyGrass);
	for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize) {
		for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++) {
			Data_Grid_graphicIds[gridOffset] = graphicId + (Data_Grid_random[gridOffset] & 7);
			if (Data_Grid_random[gridOffset] & 1) {
				Data_Grid_bitfields[gridOffset] |= Bitfield_AlternateTerrain;
			}
		}
	}
}
