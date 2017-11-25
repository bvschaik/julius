#include "scenario.h"

#include "animation.h"
#include "building.h"
#include "core/calc.h"
#include "cityinfo.h"
#include "cityview.h"
#include "empire/empire.h"
#include "figure.h"
#include "formation.h"
#include "gamefile.h"
#include "grid.h"
#include "loader.h"
#include "natives.h"
#include "city/message.h"
#include "routing.h"
#include "sidebarmenu.h"
#include "terrain.h"
#include "terraingraphics.h"

#include "ui/window.h"
#include "game/tutorial.h"
#include "building/storage.h"

#include <sound>
#include <data>
#include <scenario>

#include "core/file.h"
#include "core/io.h"
#include "core/random.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/trader.h"
#include "game/difficulty.h"
#include "game/settings.h"
#include "game/time.h"
#include "graphics/image.h"

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
    if (Data_Settings.isCustomScenario)
    {
        if (!mapFileExists(scenarioName))
        {
            UI_Window_goTo(Window_City);
            return;
        }
        initCustomScenario(scenarioName);
    }
    else
    {
        if (!GameFile_loadSavedGameFromMissionPack(saveMissionId))
        {
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
    if (Data_Settings.isCustomScenario)
    {
        Data_CityInfo.personalSavings = 0;
        Data_CityInfo.playerRank = scenario_property_player_rank();
        Data_CityInfo.salaryRank = scenario_property_player_rank();
    }
    if (Data_CityInfo.salaryRank > 10)
    {
        Data_CityInfo.salaryRank = 10;
    }
    Data_CityInfo.salaryAmount = Constant_SalaryForRank[Data_CityInfo.salaryRank];

    Tutorial::init();

    if (IsTutorial1())
    {
        setting_set_personal_savings_for_mission(0, 0);
        Data_Settings.personalSavingsLastMission = 0;
        Data_CityInfo.personalSavings = 0;
    }

    SidebarMenu_enableBuildingMenuItemsAndButtons();
    city_message_init_scenario();
}

static void clearBookmarks()
{
    for (int i = 0; i < 4; i++)
    {
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
    Animation::resetTimers();
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
            Data_Scenario.riverExitPoint.x != -1)
    {
        hasWaterEntry = 1;
    }
    Figure_createFishingPoints();
    Figure_createHerds();
    Figure_createFlotsam();

    Routing_determineLandCitizen();
    Routing_determineLandNonCitizen();
    Routing_determineWater();
    Routing_determineWalls();

    scenario_map_init_entry_exit();

    map_point entry = scenario_map_entry();
    map_point exit = scenario_map_exit();
    Data_CityInfo.entryPointX = entry.x;
    Data_CityInfo.entryPointY =  entry.y;
    Data_CityInfo.entryPointGridOffset = GridOffset(Data_CityInfo.entryPointX, Data_CityInfo.entryPointY);

    Data_CityInfo.exitPointX = exit.x;
    Data_CityInfo.exitPointY = exit.y;
    Data_CityInfo.exitPointGridOffset = GridOffset(Data_CityInfo.exitPointX, Data_CityInfo.exitPointY);
    Data_CityInfo.treasury = difficulty_adjust_money(scenario_initial_funds());
    Data_CityInfo.financeBalanceLastYear = Data_CityInfo.treasury;
    game_time_init(scenario_property_start_year());

    // set up events
    // earthquake
    scenario_earthquake_init();
    // gladiator revolt
    scenario_gladiator_revolt_init();
    // emperor change
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

static void readScenarioAndInitGraphics()
{
    initGrids();
    file_remove_extension(Data_FileList.selectedScenario);
    file_append_extension(Data_FileList.selectedScenario, "map");
    GameFile_loadScenario(Data_FileList.selectedScenario);
    file_remove_extension(Data_FileList.selectedScenario);

    scenario_map_init();

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
    for (int y = 0; y < Data_Settings_Map.height; y++)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            if (x < (GRID_SIZE - Data_Settings_Map.width) / 2 ||
                    x >= (GRID_SIZE - Data_Settings_Map.width) / 2 + Data_Settings_Map.width)
            {
                Data_Grid_terrain[gridOffset] = Terrain_OutsideMap;
            }
            if (y < (GRID_SIZE - Data_Settings_Map.height) / 2 ||
                    y >= (GRID_SIZE - Data_Settings_Map.height) / 2 + Data_Settings_Map.height)
            {
                Data_Grid_terrain[gridOffset] = Terrain_OutsideMap;
            }
        }
    }
}

static void initGridRandom()
{
    int gridOffset = 0;
    for (int y = 0; y < Data_Settings_Map.height; y++)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            random_generate_next();
            Data_Grid_random[gridOffset] = random_short();
        }
    }
}

static void initGridGraphicIds()
{
    int gridOffset = Data_Settings_Map.gridStartOffset;
    int graphicId = image_group(ID_Graphic_TerrainUglyGrass);
    for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            Data_Grid_graphicIds[gridOffset] = graphicId + (Data_Grid_random[gridOffset] & 7);
            if (Data_Grid_random[gridOffset] & 1)
            {
                Data_Grid_bitfields[gridOffset] |= Bitfield_AlternateTerrain;
            }
        }
    }
}
