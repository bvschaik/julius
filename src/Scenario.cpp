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
#include "Grid.h"
#include "Loader.h"
#include "Natives.h"
#include "PlayerMessage.h"
#include "Random.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "Terrain.h"
#include "TerrainGraphics.h"
#include "Trader.h"
#include "Walker.h"

#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Constants.h"
#include "Data/Empire.h"
#include "Data/Event.h"
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
		Data_CityInfo.treasury = Calc_adjustWithPercentage(Data_CityInfo.treasury,
			Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
	}
	Data_Settings.saveGameMissionId = saveMissionId;
	Data_Settings.currentMissionId = curMissionId;

	Data_CityInfo_Extra.startingFavor = Data_Model_Difficulty.startingFavor[Data_Settings.difficulty];
	Data_Settings.personalSavingsLastMission = Data_Settings.personalSavingsPerMission[curMissionId];
	
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

	setTutorialFlags(curMissionId);

	if (IsTutorial1()) {
		Data_Settings.personalSavingsPerMission[0] = 0;
		Data_Settings.personalSavingsLastMission = 0;
		Data_CityInfo.personalSavings = 0;
	}

	SidebarMenu_enableBuildingMenuItemsAndButtons();
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
	Data_CityInfo_Extra.ciid = 1;
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

	Data_CityInfo_Extra.ciid = 1;
	Data_CityInfo.__unknown_00a2 = 1;
	Data_CityInfo.__unknown_00a3 = 1;
	Empire_determineDistantBattleCity();
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
	Walker_createFishingPoints();
	Walker_createHerds();
	Walker_createFlotsam(Data_Scenario.riverEntryPoint.x, Data_Scenario.riverEntryPoint.y, hasWaterEntry);

	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();
	Routing_determineWalls();

	if (Data_Scenario.entryPoint.x == -1 || Data_Scenario.entryPoint.y == -1) {
		Data_Scenario.entryPoint.x = Data_Settings_Map.width - 1;
		Data_Scenario.entryPoint.y = Data_Settings_Map.height / 2;
	}
	Data_CityInfo.entryPointX = (char) Data_Scenario.entryPoint.x;
	Data_CityInfo.entryPointY = (char) Data_Scenario.entryPoint.y;
	Data_CityInfo.entryPointGridOffset = GridOffset(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);

	if (Data_Scenario.exitPoint.x == -1 || Data_Scenario.exitPoint.y == -1) {
		Data_Scenario.exitPoint.x = Data_Scenario.entryPoint.x;
		Data_Scenario.exitPoint.y = Data_Scenario.entryPoint.y;
	}
	Data_CityInfo.exitPointX = (char) Data_Scenario.exitPoint.x;
	Data_CityInfo.exitPointY = (char) Data_Scenario.exitPoint.y;
	Data_CityInfo.exitPointGridOffset = GridOffset(Data_CityInfo.exitPointX, Data_CityInfo.exitPointY);
	Data_CityInfo.treasury = Calc_adjustWithPercentage(Data_Scenario.startFunds,
		Data_Model_Difficulty.moneyPercentage[Data_Settings.difficulty]);
	Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
	Data_CityInfo_Extra.gameTimeYear = Data_Scenario.startYear;

	// set up events
	// earthquake
	Data_Event.earthquake.gameYear = Data_Scenario.startYear + Data_Scenario.earthquakeYear;
	Data_Event.earthquake.month = 2 + (Data_Random.random1_7bit & 7);
	switch (Data_Scenario.earthquakeSeverity) {
		default:
			Data_Event.earthquake.maxDuration = 0;
			Data_Event.earthquake.maxDelay = 0;
			break;
		case 1:
			Data_Event.earthquake.maxDuration = 25 + (Data_Random.random1_7bit & 0x1f);
			Data_Event.earthquake.maxDelay = 10;
			break;
		case 2:
			Data_Event.earthquake.maxDuration = 100 + (Data_Random.random1_7bit & 0x3f);
			Data_Event.earthquake.maxDelay = 8;
			break;
		case 3:
			Data_Event.earthquake.maxDuration = 250 + Data_Random.random1_7bit;
			Data_Event.earthquake.maxDelay = 6;
			break;
	}
	Data_Event.earthquake.state = 0;
	for (int i = 0; i < 4; i++) {
		Data_Event.earthquake.expand[i].x = Data_Scenario.earthquakePoint.x;
		Data_Event.earthquake.expand[i].y = Data_Scenario.earthquakePoint.y;
	}
	// gladiator revolt
	Data_Event.gladiatorRevolt.gameYear = Data_Scenario.startYear + Data_Scenario.gladiatorRevolt.year;
	Data_Event.gladiatorRevolt.month = 3 + (Data_Random.random1_7bit & 3);
	Data_Event.gladiatorRevolt.endMonth = 3 + Data_Event.gladiatorRevolt.month;
	Data_Event.gladiatorRevolt.state = GladiatorRevolt_NotStarted;
	// emperor change
	Data_Event.emperorChange.gameYear = Data_Scenario.startYear + Data_Scenario.emperorChange.year;
	Data_Event.emperorChange.month = 1 + (Data_Random.random1_7bit & 7);
	Data_Event.emperorChange.state = 0;
	// time limit
	if (Data_Scenario.winCriteria.timeLimitYearsEnabled) {
		Data_Event.timeLimitMaxGameYear = Data_Scenario.startYear + Data_Scenario.winCriteria.timeLimitYears;
	} else if (Data_Scenario.winCriteria.survivalYearsEnabled) {
		Data_Event.timeLimitMaxGameYear = Data_Scenario.startYear + Data_Scenario.winCriteria.survivalYears;
	} else {
		Data_Event.timeLimitMaxGameYear = 1000000 + Data_Scenario.startYear;
	}

	Empire_load(1, Data_Scenario.empireId);
	Empire_initCities();
	Trader_clearList();
	Event_initInvasions();
	Empire_determineDistantBattleCity();
	Event_initRequests();
	Event_initDemandChanges();
	Event_initPriceChanges();
	Data_Empire.scrollX = Data_Empire_Index[Data_Scenario.empireId].initialScrollX;
	Data_Empire.scrollY = Data_Empire_Index[Data_Scenario.empireId].initialScrollY;
	Empire_checkScrollBoundaries();
	SidebarMenu_enableBuildingMenuItemsAndButtons();
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

	Empire_initTradeAmountCodes();
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
	Grid_clearShortGrid(Data_Grid_walkerIds);
	Grid_clearUByteGrid(Data_Grid_bitfields);
	Grid_clearUByteGrid(Data_Grid_spriteOffsets);
	Grid_clearUByteGrid(Data_Grid_random);
	Grid_clearByteGrid(Data_Grid_desirability);
	Grid_clearUByteGrid(Data_Grid_elevation);
	Grid_clearUByteGrid(Data_Grid_buildingDamage);
	Grid_clearUByteGrid(Data_Grid_rubbleBuildingType);
	Grid_clearUByteGrid(Data_Grid_romanSoldierConcentration);
	Grid_clearUByteGrid(Data_Grid_roadNetworks);
	//Grid_clearUByteGrid(Data_Grid_byte_8ADF60);

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
