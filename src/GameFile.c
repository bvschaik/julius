#include "GameFile.h"

#include "Building.h"
#include "CityView.h"
#include "Empire.h"
#include "Event.h"
#include "Figure.h"
#include "Loader.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "UtilityManagement.h"

#include "Data/Building.h"
#include "Data/CityInfo.h"
#include "Data/Debug.h"
#include "Data/Empire.h"
#include "Data/Event.h"
#include "Data/FileList.h"
#include "Data/Formation.h"
#include "Data/Grid.h"
#include "Data/Invasion.h"
#include "Data/Message.h"
#include "Data/Routes.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Sound.h"
#include "Data/State.h"
#include "Data/Trade.h"
#include "Data/Tutorial.h"
#include "Data/Figure.h"
#include "Data/Figure.h"

#include "core/buffer.h"
#include "core/file.h"
#include "core/io.h"
#include "core/random.h"
#include "core/zip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCENARIO_PARTS 12
#define SAVEGAME_PARTS 300
#define COMPRESS_BUFFER_SIZE 600000
#define UNCOMPRESSED 0x80000000

struct GameFilePart {
	int compressed;
	void *data;
	int lengthInBytes;
};

static const int savegameVersion = 0x66;

static int savegameFileVersion;
static char playerNames[2][32];

static char compressBuffer[COMPRESS_BUFFER_SIZE];

static int endMarker = 0;

static const char missionPackFile[] = "mission1.pak";

static const char missionSavedGames[][32] = {
	"Citizen.sav",
	"Clerk.sav",
	"Engineer.sav",
	"Architect.sav",
	"Quaestor.sav",
	"Procurator.sav",
	"Aedile.sav",
	"Praetor.sav",
	"Consul.sav",
	"Proconsul.sav"
	"Caesar.sav",
	"Caesar2.sav"
};

typedef struct {
    buffer buf;
    int compressed;
} file_piece;

typedef struct {
    buffer *graphic_ids;
    buffer *edge;
    buffer *terrain;
    buffer *bitfields;
    buffer *random;
    buffer *elevation;
    buffer *random_iv;
    buffer *camera;
    buffer *scenario;
} scenario_state;

struct {
    int num_pieces;
    file_piece pieces[9];
    scenario_state state;
} scenario_data = {0};


typedef struct {
    buffer *Data_Settings_saveGameMissionId;
    buffer *savegameFileVersion;
    buffer *Data_Grid_graphicIds;
    buffer *Data_Grid_edge;
    buffer *Data_Grid_buildingIds;
    buffer *Data_Grid_terrain;
    buffer *Data_Grid_aqueducts;
    buffer *Data_Grid_figureIds;
    buffer *Data_Grid_bitfields;
    buffer *Data_Grid_spriteOffsets;
    buffer *Data_Grid_random;
    buffer *Data_Grid_desirability;
    buffer *Data_Grid_elevation;
    buffer *Data_Grid_buildingDamage;
    buffer *Data_Grid_Undo_aqueducts;
    buffer *Data_Grid_Undo_spriteOffsets;
    buffer *Data_Figures;
    buffer *Data_Routes_figureIds;
    buffer *Data_Routes_directionPaths;
    buffer *Data_Formations;
    buffer *Data_Formation_Extra_idLastInUse;
    buffer *Data_Formation_Extra_idLastLegion;
    buffer *Data_Formation_Extra_numForts;
    buffer *Data_CityInfo;
    buffer *Data_CityInfo_Extra_unknownBytes;
    buffer *playerNames;
    buffer *Data_CityInfo_Extra_ciid;
    buffer *Data_Buildings;
    buffer *Data_Settings_Map_orientation;
    buffer *Data_CityInfo_Extra_gameTimeTick;
    buffer *Data_CityInfo_Extra_gameTimeDay;
    buffer *Data_CityInfo_Extra_gameTimeMonth;
    buffer *Data_CityInfo_Extra_gameTimeYear;
    buffer *Data_CityInfo_Extra_gameTimeTotalDays;
    buffer *Data_Buildings_Extra_highestBuildingIdEver;
    buffer *Data_Debug_maxConnectsEver;
    buffer *random_iv;
    buffer *Data_Settings_Map_camera_x;
    buffer *Data_Settings_Map_camera_y;
    buffer *Data_CityInfo_Buildings_theater_total;
    buffer *Data_CityInfo_Buildings_theater_working;
    buffer *Data_CityInfo_Buildings_amphitheater_total;
    buffer *Data_CityInfo_Buildings_amphitheater_working;
    buffer *Data_CityInfo_Buildings_colosseum_total;
    buffer *Data_CityInfo_Buildings_colosseum_working;
    buffer *Data_CityInfo_Buildings_hippodrome_total;
    buffer *Data_CityInfo_Buildings_hippodrome_working;
    buffer *Data_CityInfo_Buildings_school_total;
    buffer *Data_CityInfo_Buildings_school_working;
    buffer *Data_CityInfo_Buildings_library_total;
    buffer *Data_CityInfo_Buildings_library_working;
    buffer *Data_CityInfo_Buildings_academy_total;
    buffer *Data_CityInfo_Buildings_academy_working;
    buffer *Data_CityInfo_Buildings_barber_total;
    buffer *Data_CityInfo_Buildings_barber_working;
    buffer *Data_CityInfo_Buildings_bathhouse_total;
    buffer *Data_CityInfo_Buildings_bathhouse_working;
    buffer *Data_CityInfo_Buildings_clinic_total;
    buffer *Data_CityInfo_Buildings_clinic_working;
    buffer *Data_CityInfo_Buildings_hospital_total;
    buffer *Data_CityInfo_Buildings_hospital_working;
    buffer *Data_CityInfo_Buildings_smallTempleCeres_total;
    buffer *Data_CityInfo_Buildings_smallTempleNeptune_total;
    buffer *Data_CityInfo_Buildings_smallTempleMercury_total;
    buffer *Data_CityInfo_Buildings_smallTempleMars_total;
    buffer *Data_CityInfo_Buildings_smallTempleVenus_total;
    buffer *Data_CityInfo_Buildings_largeTempleCeres_total;
    buffer *Data_CityInfo_Buildings_largeTempleNeptune_total;
    buffer *Data_CityInfo_Buildings_largeTempleMercury_total;
    buffer *Data_CityInfo_Buildings_largeTempleMars_total;
    buffer *Data_CityInfo_Buildings_largeTempleVenus_total;
    buffer *Data_CityInfo_Buildings_oracle_total;
    buffer *Data_CityInfo_Extra_populationGraphOrder;
    buffer *Data_CityInfo_Extra_unknownOrder;
    buffer *Data_Event_emperorChange_gameYear;
    buffer *Data_Event_emperorChange_month;
    buffer *Data_Empire_scrollX;
    buffer *Data_Empire_scrollY;
    buffer *Data_Empire_selectedObject;
    buffer *Data_Empire_Cities;
    buffer *Data_CityInfo_Buildings_industry_total;
    buffer *Data_CityInfo_Buildings_industry_working;
    buffer *Data_TradePrices;
    buffer *Data_Figure_NameSequence_citizenMale;
    buffer *Data_Figure_NameSequence_patrician;
    buffer *Data_Figure_NameSequence_citizenFemale;
    buffer *Data_Figure_NameSequence_taxCollector;
    buffer *Data_Figure_NameSequence_engineer;
    buffer *Data_Figure_NameSequence_prefect;
    buffer *Data_Figure_NameSequence_javelinThrower;
    buffer *Data_Figure_NameSequence_cavalry;
    buffer *Data_Figure_NameSequence_legionary;
    buffer *Data_Figure_NameSequence_actor;
    buffer *Data_Figure_NameSequence_gladiator;
    buffer *Data_Figure_NameSequence_lionTamer;
    buffer *Data_Figure_NameSequence_charioteer;
    buffer *Data_Figure_NameSequence_barbarian;
    buffer *Data_Figure_NameSequence_enemyGreek;
    buffer *Data_Figure_NameSequence_enemyEgyptian;
    buffer *Data_Figure_NameSequence_enemyArabian;
    buffer *Data_Figure_NameSequence_trader;
    buffer *Data_Figure_NameSequence_tradeShip;
    buffer *Data_Figure_NameSequence_warShip;
    buffer *Data_Figure_NameSequence_enemyShip;
    buffer *Data_CityInfo_CultureCoverage_theater;
    buffer *Data_CityInfo_CultureCoverage_amphitheater;
    buffer *Data_CityInfo_CultureCoverage_colosseum;
    buffer *Data_CityInfo_CultureCoverage_hospital1;
    buffer *Data_CityInfo_CultureCoverage_hippodrome;
    buffer *Data_CityInfo_CultureCoverage_religionCeres;
    buffer *Data_CityInfo_CultureCoverage_religionNeptune;
    buffer *Data_CityInfo_CultureCoverage_religionMercury;
    buffer *Data_CityInfo_CultureCoverage_religionMars;
    buffer *Data_CityInfo_CultureCoverage_religionVenus;
    buffer *Data_CityInfo_CultureCoverage_oracle;
    buffer *Data_CityInfo_CultureCoverage_school;
    buffer *Data_CityInfo_CultureCoverage_library;
    buffer *Data_CityInfo_CultureCoverage_academy;
    buffer *Data_CityInfo_CultureCoverage_hospital;
    buffer *Data_Scenario;
    buffer *Data_Event_timeLimitMaxGameYear;
    buffer *Data_Event_earthquake_gameYear;
    buffer *Data_Event_earthquake_month;
    buffer *Data_Event_earthquake_state;
    buffer *Data_Event_earthquake_duration;
    buffer *Data_Event_earthquake_maxDuration;
    buffer *Data_Event_earthquake_maxDelay;
    buffer *Data_Event_earthquake_delay;
    buffer *Data_Event_earthquake_expand;
    buffer *Data_Event_emperorChange_state;
    buffer *Data_Message_messages;
    buffer *Data_Message_nextMessageSequence;
    buffer *Data_Message_totalMessages;
    buffer *Data_Message_currentMessageId;
    buffer *Data_Message_populationMessagesShown;
    buffer *Data_Message_messageCategoryCount;
    buffer *Data_Message_messageDelay;
    buffer *Data_BuildingList_burning_totalBurning;
    buffer *Data_BuildingList_burning_index;
    buffer *Data_Figure_Extra_createdSequence;
    buffer *Data_Settings_startingFavor;
    buffer *Data_Settings_personalSavingsLastMission;
    buffer *Data_Settings_currentMissionId;
    buffer *Data_InvasionWarnings;
    buffer *Data_Settings_isCustomScenario;
    buffer *Data_Sound_City;
    buffer *Data_Buildings_Extra_highestBuildingIdInUse;
    buffer *Data_Figure_Traders;
    buffer *Data_Figure_Extra_nextTraderId;
    buffer *Data_BuildingList_burning_items;
    buffer *Data_BuildingList_small_items;
    buffer *Data_BuildingList_large_items;
    buffer *Data_Tutorial_tutorial1_fire;
    buffer *Data_Tutorial_tutorial1_crime;
    buffer *Data_Tutorial_tutorial1_collapse;
    buffer *Data_Tutorial_tutorial2_granaryBuilt;
    buffer *Data_Tutorial_tutorial2_population250Reached;
    buffer *Data_Tutorial_tutorial1_senateBuilt;
    buffer *Data_Tutorial_tutorial2_population450Reached;
    buffer *Data_Tutorial_tutorial2_potteryMade;
    buffer *Data_CityInfo_Buildings_militaryAcademy_total;
    buffer *Data_CityInfo_Buildings_militaryAcademy_working;
    buffer *Data_CityInfo_Buildings_barracks_total;
    buffer *Data_CityInfo_Buildings_barracks_working;
    buffer *Data_Formation_Extra_numEnemyFormations;
    buffer *Data_Formation_Extra_numEnemySoldierStrength;
    buffer *Data_Formation_Extra_numLegionFormations;
    buffer *Data_Formation_Extra_numLegionSoldierStrength;
    buffer *Data_Formation_Extra_daysSinceRomanSoldierConcentration;
    buffer *Data_Building_Storages;
    buffer *Data_CityInfo_Buildings_actorColony_total;
    buffer *Data_CityInfo_Buildings_actorColony_working;
    buffer *Data_CityInfo_Buildings_gladiatorSchool_total;
    buffer *Data_CityInfo_Buildings_gladiatorSchool_working;
    buffer *Data_CityInfo_Buildings_lionHouse_total;
    buffer *Data_CityInfo_Buildings_lionHouse_working;
    buffer *Data_CityInfo_Buildings_chariotMaker_total;
    buffer *Data_CityInfo_Buildings_chariotMaker_working;
    buffer *Data_CityInfo_Buildings_market_total;
    buffer *Data_CityInfo_Buildings_market_working;
    buffer *Data_CityInfo_Buildings_reservoir_total;
    buffer *Data_CityInfo_Buildings_reservoir_working;
    buffer *Data_CityInfo_Buildings_fountain_total;
    buffer *Data_CityInfo_Buildings_fountain_working;
    buffer *Data_Tutorial_tutorial2_potteryMadeYear;
    buffer *Data_Event_gladiatorRevolt_gameYear;
    buffer *Data_Event_gladiatorRevolt_month;
    buffer *Data_Event_gladiatorRevolt_endMonth;
    buffer *Data_Event_gladiatorRevolt_state;
    buffer *Data_Empire_Trade_maxPerYear;
    buffer *Data_Empire_Trade_tradedThisYear;
    buffer *Data_Buildings_Extra_barracksTowerSentryRequested;
    buffer *Data_Buildings_Extra_createdSequence;
    buffer *Data_Routes_unknown1RoutesCalculated;
    buffer *Data_Routes_enemyRoutesCalculated;
    buffer *Data_Routes_totalRoutesCalculated;
    buffer *Data_Routes_unknown2RoutesCalculated;
    buffer *Data_CityInfo_Buildings_smallTempleCeres_working;
    buffer *Data_CityInfo_Buildings_smallTempleNeptune_working;
    buffer *Data_CityInfo_Buildings_smallTempleMercury_working;
    buffer *Data_CityInfo_Buildings_smallTempleMars_working;
    buffer *Data_CityInfo_Buildings_smallTempleVenus_working;
    buffer *Data_CityInfo_Buildings_largeTempleCeres_working;
    buffer *Data_CityInfo_Buildings_largeTempleNeptune_working;
    buffer *Data_CityInfo_Buildings_largeTempleMercury_working;
    buffer *Data_CityInfo_Buildings_largeTempleMars_working;
    buffer *Data_CityInfo_Buildings_largeTempleVenus_working;
    buffer *Data_Formation_Invasion_formationId;
    buffer *Data_Formation_Invasion_homeX;
    buffer *Data_Formation_Invasion_homeY;
    buffer *Data_Formation_Invasion_layout;
    buffer *Data_Formation_Invasion_destinationX;
    buffer *Data_Formation_Invasion_destinationY;
    buffer *Data_Formation_Invasion_destinationBuildingId;
    buffer *Data_Formation_Invasion_numLegions;
    buffer *Data_Formation_Invasion_ignoreRomanSoldiers;
    buffer *Data_CityInfo_Extra_entryPointFlag_x;
    buffer *Data_CityInfo_Extra_entryPointFlag_y;
    buffer *Data_CityInfo_Extra_exitPointFlag_x;
    buffer *Data_CityInfo_Extra_exitPointFlag_y;
    buffer *Data_Event_lastInternalInvasionId;
    buffer *Data_Debug_incorrectHousePositions;
    buffer *Data_Debug_unfixableHousePositions;
    buffer *Data_FileList_selectedScenario;
    buffer *Data_CityInfo_Extra_bookmarks;
    buffer *Data_Tutorial_tutorial3_disease;
    buffer *Data_CityInfo_Extra_entryPointFlag_gridOffset;
    buffer *Data_CityInfo_Extra_exitPointFlag_gridOffset;
    buffer *endMarker;
} savegame_state;
struct {
    int num_pieces;
    file_piece pieces[300];
    savegame_state state;
} savegame_data;

void init_file_piece(file_piece *piece, int size, int compressed)
{
    piece->compressed = compressed;
    void *data = malloc(size);
    buffer_init(&piece->buf, data, size);
}

buffer *create_scenario_piece(int size)
{
    file_piece *piece = &scenario_data.pieces[scenario_data.num_pieces++];
    init_file_piece(piece, size, 0);
    return &piece->buf;
}

buffer *create_savegame_piece(int size, int compressed)
{
    file_piece *piece = &savegame_data.pieces[savegame_data.num_pieces++];
    init_file_piece(piece, size, compressed);
    return &piece->buf;
}

void init_scenario_data()
{
    if (scenario_data.num_pieces > 0) {
        for (int i = 0; i < scenario_data.num_pieces; i++) {
            buffer_reset(&scenario_data.pieces[i].buf);
        }
        return;
    }
    scenario_state *state = &scenario_data.state;
    state->graphic_ids = create_scenario_piece(52488);
    state->edge = create_scenario_piece(26244);
    state->terrain = create_scenario_piece(52488);
    state->bitfields = create_scenario_piece(26244);
    state->random = create_scenario_piece(26244);
    state->elevation = create_scenario_piece(26244);
    state->random_iv = create_scenario_piece(8);
    state->camera = create_scenario_piece(8);
    state->scenario = create_scenario_piece(1720);
}

void init_savegame_data()
{
    if (savegame_data.num_pieces > 0) {
        for (int i = 0; i < savegame_data.num_pieces; i++) {
            buffer_reset(&savegame_data.pieces[i].buf);
        }
        return;
    }
    savegame_state *state = &savegame_data.state;
    state->Data_Settings_saveGameMissionId = create_savegame_piece(4, 0);
    state->savegameFileVersion = create_savegame_piece(4, 0);
    state->Data_Grid_graphicIds = create_savegame_piece(52488, 1);
    state->Data_Grid_edge = create_savegame_piece(26244, 1);
    state->Data_Grid_buildingIds = create_savegame_piece(52488, 1);
    state->Data_Grid_terrain = create_savegame_piece(52488, 1);
    state->Data_Grid_aqueducts = create_savegame_piece(26244, 1);
    state->Data_Grid_figureIds = create_savegame_piece(52488, 1);
    state->Data_Grid_bitfields = create_savegame_piece(26244, 1);
    state->Data_Grid_spriteOffsets = create_savegame_piece(26244, 1);
    state->Data_Grid_random = create_savegame_piece(26244, 0);
    state->Data_Grid_desirability = create_savegame_piece(26244, 1);
    state->Data_Grid_elevation = create_savegame_piece(26244, 1);
    state->Data_Grid_buildingDamage = create_savegame_piece(26244, 1);
    state->Data_Grid_Undo_aqueducts = create_savegame_piece(26244, 1);
    state->Data_Grid_Undo_spriteOffsets = create_savegame_piece(26244, 1);
    state->Data_Figures = create_savegame_piece(128000, 1);
    state->Data_Routes_figureIds = create_savegame_piece(1200, 1);
    state->Data_Routes_directionPaths = create_savegame_piece(300000, 1);
    state->Data_Formations = create_savegame_piece(6400, 1);
    state->Data_Formation_Extra_idLastInUse = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_idLastLegion = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_numForts = create_savegame_piece(4, 0);
    state->Data_CityInfo = create_savegame_piece(36136, 1);
    state->Data_CityInfo_Extra_unknownBytes = create_savegame_piece(2, 0);
    state->playerNames = create_savegame_piece(64, 0);
    state->Data_CityInfo_Extra_ciid = create_savegame_piece(4, 0);
    state->Data_Buildings = create_savegame_piece(256000, 1);
    state->Data_Settings_Map_orientation = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_gameTimeTick = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_gameTimeDay = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_gameTimeMonth = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_gameTimeYear = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_gameTimeTotalDays = create_savegame_piece(4, 0);
    state->Data_Buildings_Extra_highestBuildingIdEver = create_savegame_piece(4, 0);
    state->Data_Debug_maxConnectsEver = create_savegame_piece(4, 0);
    state->random_iv = create_savegame_piece(8, 0);
    state->Data_Settings_Map_camera_x = create_savegame_piece(4, 0);
    state->Data_Settings_Map_camera_y = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_theater_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_theater_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_amphitheater_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_amphitheater_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_colosseum_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_colosseum_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_hippodrome_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_hippodrome_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_school_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_school_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_library_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_library_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_academy_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_academy_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_barber_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_barber_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_bathhouse_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_bathhouse_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_clinic_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_clinic_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_hospital_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_hospital_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleCeres_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleNeptune_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleMercury_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleMars_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleVenus_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleCeres_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleNeptune_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleMercury_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleMars_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleVenus_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_oracle_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_populationGraphOrder = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_unknownOrder = create_savegame_piece(4, 0);
    state->Data_Event_emperorChange_gameYear = create_savegame_piece(4, 0);
    state->Data_Event_emperorChange_month = create_savegame_piece(4, 0);
    state->Data_Empire_scrollX = create_savegame_piece(4, 0);
    state->Data_Empire_scrollY = create_savegame_piece(4, 0);
    state->Data_Empire_selectedObject = create_savegame_piece(4, 0);
    state->Data_Empire_Cities = create_savegame_piece(2706, 1);
    state->Data_CityInfo_Buildings_industry_total = create_savegame_piece(64, 0);
    state->Data_CityInfo_Buildings_industry_working = create_savegame_piece(64, 0);
    state->Data_TradePrices = create_savegame_piece(128, 0);
    state->Data_Figure_NameSequence_citizenMale = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_patrician = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_citizenFemale = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_taxCollector = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_engineer = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_prefect = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_javelinThrower = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_cavalry = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_legionary = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_actor = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_gladiator = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_lionTamer = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_charioteer = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_barbarian = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_enemyGreek = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_enemyEgyptian = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_enemyArabian = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_trader = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_tradeShip = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_warShip = create_savegame_piece(4, 0);
    state->Data_Figure_NameSequence_enemyShip = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_theater = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_amphitheater = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_colosseum = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_hospital1 = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_hippodrome = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_religionCeres = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_religionNeptune = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_religionMercury = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_religionMars = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_religionVenus = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_oracle = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_school = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_library = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_academy = create_savegame_piece(4, 0);
    state->Data_CityInfo_CultureCoverage_hospital = create_savegame_piece(4, 0);
    state->Data_Scenario = create_savegame_piece(1720, 0);
    state->Data_Event_timeLimitMaxGameYear = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_gameYear = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_month = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_state = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_duration = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_maxDuration = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_maxDelay = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_delay = create_savegame_piece(4, 0);
    state->Data_Event_earthquake_expand = create_savegame_piece(32, 0);
    state->Data_Event_emperorChange_state = create_savegame_piece(4, 0);
    state->Data_Message_messages = create_savegame_piece(16000, 1);
    state->Data_Message_nextMessageSequence = create_savegame_piece(4, 0);
    state->Data_Message_totalMessages = create_savegame_piece(4, 0);
    state->Data_Message_currentMessageId = create_savegame_piece(4, 0);
    state->Data_Message_populationMessagesShown = create_savegame_piece(10, 0);
    state->Data_Message_messageCategoryCount = create_savegame_piece(80, 0);
    state->Data_Message_messageDelay = create_savegame_piece(80, 0);
    state->Data_BuildingList_burning_totalBurning = create_savegame_piece(4, 0);
    state->Data_BuildingList_burning_index = create_savegame_piece(4, 0);
    state->Data_Figure_Extra_createdSequence = create_savegame_piece(4, 0);
    state->Data_Settings_startingFavor = create_savegame_piece(4, 0);
    state->Data_Settings_personalSavingsLastMission = create_savegame_piece(4, 0);
    state->Data_Settings_currentMissionId = create_savegame_piece(4, 0);
    state->Data_InvasionWarnings = create_savegame_piece(3232, 1);
    state->Data_Settings_isCustomScenario = create_savegame_piece(4, 0);
    state->Data_Sound_City = create_savegame_piece(8960, 0);
    state->Data_Buildings_Extra_highestBuildingIdInUse = create_savegame_piece(4, 0);
    state->Data_Figure_Traders = create_savegame_piece(4800, 0);
    state->Data_Figure_Extra_nextTraderId = create_savegame_piece(4, 0);
    state->Data_BuildingList_burning_items = create_savegame_piece(1000, 1);
    state->Data_BuildingList_small_items = create_savegame_piece(1000, 1);
    state->Data_BuildingList_large_items = create_savegame_piece(4000, 1);
    state->Data_Tutorial_tutorial1_fire = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial1_crime = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial1_collapse = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial2_granaryBuilt = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial2_population250Reached = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial1_senateBuilt = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial2_population450Reached = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial2_potteryMade = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_militaryAcademy_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_militaryAcademy_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_barracks_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_barracks_working = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_numEnemyFormations = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_numEnemySoldierStrength = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_numLegionFormations = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_numLegionSoldierStrength = create_savegame_piece(4, 0);
    state->Data_Formation_Extra_daysSinceRomanSoldierConcentration = create_savegame_piece(4, 0);
    state->Data_Building_Storages = create_savegame_piece(6400, 0);
    state->Data_CityInfo_Buildings_actorColony_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_actorColony_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_gladiatorSchool_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_gladiatorSchool_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_lionHouse_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_lionHouse_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_chariotMaker_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_chariotMaker_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_market_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_market_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_reservoir_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_reservoir_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_fountain_total = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_fountain_working = create_savegame_piece(4, 0);
    state->Data_Tutorial_tutorial2_potteryMadeYear = create_savegame_piece(4, 0);
    state->Data_Event_gladiatorRevolt_gameYear = create_savegame_piece(4, 0);
    state->Data_Event_gladiatorRevolt_month = create_savegame_piece(4, 0);
    state->Data_Event_gladiatorRevolt_endMonth = create_savegame_piece(4, 0);
    state->Data_Event_gladiatorRevolt_state = create_savegame_piece(4, 0);
    state->Data_Empire_Trade_maxPerYear = create_savegame_piece(1280, 1);
    state->Data_Empire_Trade_tradedThisYear = create_savegame_piece(1280, 1);
    state->Data_Buildings_Extra_barracksTowerSentryRequested = create_savegame_piece(4, 0);
    state->Data_Buildings_Extra_createdSequence = create_savegame_piece(4, 0);
    state->Data_Routes_unknown1RoutesCalculated = create_savegame_piece(4, 0); //state->unk_634474 = create_savegame_piece(4, 0); not referenced
    state->Data_Routes_enemyRoutesCalculated = create_savegame_piece(4, 0);
    state->Data_Routes_totalRoutesCalculated = create_savegame_piece(4, 0);
    state->Data_Routes_unknown2RoutesCalculated = create_savegame_piece(4, 0); //state->unk_634470 = create_savegame_piece(4, 0); not referenced
    state->Data_CityInfo_Buildings_smallTempleCeres_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleNeptune_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleMercury_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleMars_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_smallTempleVenus_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleCeres_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleNeptune_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleMercury_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleMars_working = create_savegame_piece(4, 0);
    state->Data_CityInfo_Buildings_largeTempleVenus_working = create_savegame_piece(4, 0);
    state->Data_Formation_Invasion_formationId = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_homeX = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_homeY = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_layout = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_destinationX = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_destinationY = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_destinationBuildingId = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_numLegions = create_savegame_piece(100, 0);
    state->Data_Formation_Invasion_ignoreRomanSoldiers = create_savegame_piece(100, 0);
    state->Data_CityInfo_Extra_entryPointFlag_x = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_entryPointFlag_y = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_x = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_y = create_savegame_piece(4, 0);
    state->Data_Event_lastInternalInvasionId = create_savegame_piece(2, 0);
    state->Data_Debug_incorrectHousePositions = create_savegame_piece(4, 0);
    state->Data_Debug_unfixableHousePositions = create_savegame_piece(4, 0);
    state->Data_FileList_selectedScenario = create_savegame_piece(65, 0);
    state->Data_CityInfo_Extra_bookmarks = create_savegame_piece(32, 0);
    state->Data_Tutorial_tutorial3_disease = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_entryPointFlag_gridOffset = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_gridOffset = create_savegame_piece(4, 0);
    state->endMarker = create_savegame_piece(284, 0); // 71x 4-bytes emptiness
}

void init_all()
{
    if (!savegame_data.num_pieces) {
        init_scenario_data();
        init_savegame_data();
    }
}

void read_all_from_buffer(buffer *buf, void *data)
{
    buffer_read_raw(buf, data, buf->size);
}

void write_all_to_buffer(buffer *buf, void *data)
{
    buffer_write_raw(buf, data, buf->size);
}

void scenario_deserialize(scenario_state *file)
{
    read_all_from_buffer(file->graphic_ids, &Data_Grid_graphicIds);
    read_all_from_buffer(file->edge, &Data_Grid_edge);
    read_all_from_buffer(file->terrain, &Data_Grid_terrain);
    read_all_from_buffer(file->bitfields, &Data_Grid_bitfields);
    read_all_from_buffer(file->random, &Data_Grid_random);
    read_all_from_buffer(file->elevation, &Data_Grid_elevation);
    
    Data_Settings_Map.camera.x = buffer_read_i32(file->camera);
    Data_Settings_Map.camera.y = buffer_read_i32(file->camera);
    
    random_load_state(file->random_iv);

    read_all_from_buffer(file->scenario, &Data_Scenario);
}

static void savegame_deserialize(savegame_state *state)
{
    read_all_from_buffer(state->Data_Settings_saveGameMissionId, &Data_Settings.saveGameMissionId);
    read_all_from_buffer(state->savegameFileVersion, &savegameFileVersion);
    read_all_from_buffer(state->Data_Grid_graphicIds, &Data_Grid_graphicIds);
    read_all_from_buffer(state->Data_Grid_edge, &Data_Grid_edge);
    read_all_from_buffer(state->Data_Grid_buildingIds, &Data_Grid_buildingIds);
    read_all_from_buffer(state->Data_Grid_terrain, &Data_Grid_terrain);
    read_all_from_buffer(state->Data_Grid_aqueducts, &Data_Grid_aqueducts);
    read_all_from_buffer(state->Data_Grid_figureIds, &Data_Grid_figureIds);
    read_all_from_buffer(state->Data_Grid_bitfields, &Data_Grid_bitfields);
    read_all_from_buffer(state->Data_Grid_spriteOffsets, &Data_Grid_spriteOffsets);
    read_all_from_buffer(state->Data_Grid_random, &Data_Grid_random);
    read_all_from_buffer(state->Data_Grid_desirability, &Data_Grid_desirability);
    read_all_from_buffer(state->Data_Grid_elevation, &Data_Grid_elevation);
    read_all_from_buffer(state->Data_Grid_buildingDamage, &Data_Grid_buildingDamage);
    read_all_from_buffer(state->Data_Grid_Undo_aqueducts, &Data_Grid_Undo_aqueducts);
    read_all_from_buffer(state->Data_Grid_Undo_spriteOffsets, &Data_Grid_Undo_spriteOffsets);
    read_all_from_buffer(state->Data_Figures, &Data_Figures);
    read_all_from_buffer(state->Data_Routes_figureIds, &Data_Routes.figureIds);
    read_all_from_buffer(state->Data_Routes_directionPaths, &Data_Routes.directionPaths);
    read_all_from_buffer(state->Data_Formations, &Data_Formations);
    read_all_from_buffer(state->Data_Formation_Extra_idLastInUse, &Data_Formation_Extra.idLastInUse);
    read_all_from_buffer(state->Data_Formation_Extra_idLastLegion, &Data_Formation_Extra.idLastLegion);
    read_all_from_buffer(state->Data_Formation_Extra_numForts, &Data_Formation_Extra.numForts);
    read_all_from_buffer(state->Data_CityInfo, &Data_CityInfo);
    read_all_from_buffer(state->Data_CityInfo_Extra_unknownBytes, &Data_CityInfo_Extra.unknownBytes);
    read_all_from_buffer(state->playerNames, &playerNames);
    read_all_from_buffer(state->Data_CityInfo_Extra_ciid, &Data_CityInfo_Extra.ciid);
    read_all_from_buffer(state->Data_Buildings, &Data_Buildings);
    read_all_from_buffer(state->Data_Settings_Map_orientation, &Data_Settings_Map.orientation);
    read_all_from_buffer(state->Data_CityInfo_Extra_gameTimeTick, &Data_CityInfo_Extra.gameTimeTick);
    read_all_from_buffer(state->Data_CityInfo_Extra_gameTimeDay, &Data_CityInfo_Extra.gameTimeDay);
    read_all_from_buffer(state->Data_CityInfo_Extra_gameTimeMonth, &Data_CityInfo_Extra.gameTimeMonth);
    read_all_from_buffer(state->Data_CityInfo_Extra_gameTimeYear, &Data_CityInfo_Extra.gameTimeYear);
    read_all_from_buffer(state->Data_CityInfo_Extra_gameTimeTotalDays, &Data_CityInfo_Extra.gameTimeTotalDays);
    read_all_from_buffer(state->Data_Buildings_Extra_highestBuildingIdEver, &Data_Buildings_Extra.highestBuildingIdEver);
    read_all_from_buffer(state->Data_Debug_maxConnectsEver, &Data_Debug.maxConnectsEver);
    
    random_load_state(state->random_iv);

    read_all_from_buffer(state->Data_Settings_Map_camera_x, &Data_Settings_Map.camera.x);
    read_all_from_buffer(state->Data_Settings_Map_camera_y, &Data_Settings_Map.camera.y);
    read_all_from_buffer(state->Data_CityInfo_Buildings_theater_total, &Data_CityInfo_Buildings.theater.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_theater_working, &Data_CityInfo_Buildings.theater.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_amphitheater_total, &Data_CityInfo_Buildings.amphitheater.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_amphitheater_working, &Data_CityInfo_Buildings.amphitheater.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_colosseum_total, &Data_CityInfo_Buildings.colosseum.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_colosseum_working, &Data_CityInfo_Buildings.colosseum.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_hippodrome_total, &Data_CityInfo_Buildings.hippodrome.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_hippodrome_working, &Data_CityInfo_Buildings.hippodrome.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_school_total, &Data_CityInfo_Buildings.school.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_school_working, &Data_CityInfo_Buildings.school.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_library_total, &Data_CityInfo_Buildings.library.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_library_working, &Data_CityInfo_Buildings.library.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_academy_total, &Data_CityInfo_Buildings.academy.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_academy_working, &Data_CityInfo_Buildings.academy.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_barber_total, &Data_CityInfo_Buildings.barber.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_barber_working, &Data_CityInfo_Buildings.barber.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_bathhouse_total, &Data_CityInfo_Buildings.bathhouse.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_bathhouse_working, &Data_CityInfo_Buildings.bathhouse.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_clinic_total, &Data_CityInfo_Buildings.clinic.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_clinic_working, &Data_CityInfo_Buildings.clinic.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_hospital_total, &Data_CityInfo_Buildings.hospital.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_hospital_working, &Data_CityInfo_Buildings.hospital.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleCeres_total, &Data_CityInfo_Buildings.smallTempleCeres.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleNeptune_total, &Data_CityInfo_Buildings.smallTempleNeptune.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleMercury_total, &Data_CityInfo_Buildings.smallTempleMercury.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleMars_total, &Data_CityInfo_Buildings.smallTempleMars.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleVenus_total, &Data_CityInfo_Buildings.smallTempleVenus.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleCeres_total, &Data_CityInfo_Buildings.largeTempleCeres.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleNeptune_total, &Data_CityInfo_Buildings.largeTempleNeptune.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleMercury_total, &Data_CityInfo_Buildings.largeTempleMercury.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleMars_total, &Data_CityInfo_Buildings.largeTempleMars.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleVenus_total, &Data_CityInfo_Buildings.largeTempleVenus.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_oracle_total, &Data_CityInfo_Buildings.oracle.total);
    read_all_from_buffer(state->Data_CityInfo_Extra_populationGraphOrder, &Data_CityInfo_Extra.populationGraphOrder);
    read_all_from_buffer(state->Data_CityInfo_Extra_unknownOrder, &Data_CityInfo_Extra.unknownOrder);
    read_all_from_buffer(state->Data_Event_emperorChange_gameYear, &Data_Event.emperorChange.gameYear);
    read_all_from_buffer(state->Data_Event_emperorChange_month, &Data_Event.emperorChange.month);
    read_all_from_buffer(state->Data_Empire_scrollX, &Data_Empire.scrollX);
    read_all_from_buffer(state->Data_Empire_scrollY, &Data_Empire.scrollY);
    read_all_from_buffer(state->Data_Empire_selectedObject, &Data_Empire.selectedObject);
    read_all_from_buffer(state->Data_Empire_Cities, &Data_Empire_Cities);
    read_all_from_buffer(state->Data_CityInfo_Buildings_industry_total, &Data_CityInfo_Buildings.industry.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_industry_working, &Data_CityInfo_Buildings.industry.working);
    read_all_from_buffer(state->Data_TradePrices, &Data_TradePrices);
    read_all_from_buffer(state->Data_Figure_NameSequence_citizenMale, &Data_Figure_NameSequence.citizenMale);
    read_all_from_buffer(state->Data_Figure_NameSequence_patrician, &Data_Figure_NameSequence.patrician);
    read_all_from_buffer(state->Data_Figure_NameSequence_citizenFemale, &Data_Figure_NameSequence.citizenFemale);
    read_all_from_buffer(state->Data_Figure_NameSequence_taxCollector, &Data_Figure_NameSequence.taxCollector);
    read_all_from_buffer(state->Data_Figure_NameSequence_engineer, &Data_Figure_NameSequence.engineer);
    read_all_from_buffer(state->Data_Figure_NameSequence_prefect, &Data_Figure_NameSequence.prefect);
    read_all_from_buffer(state->Data_Figure_NameSequence_javelinThrower, &Data_Figure_NameSequence.javelinThrower);
    read_all_from_buffer(state->Data_Figure_NameSequence_cavalry, &Data_Figure_NameSequence.cavalry);
    read_all_from_buffer(state->Data_Figure_NameSequence_legionary, &Data_Figure_NameSequence.legionary);
    read_all_from_buffer(state->Data_Figure_NameSequence_actor, &Data_Figure_NameSequence.actor);
    read_all_from_buffer(state->Data_Figure_NameSequence_gladiator, &Data_Figure_NameSequence.gladiator);
    read_all_from_buffer(state->Data_Figure_NameSequence_lionTamer, &Data_Figure_NameSequence.lionTamer);
    read_all_from_buffer(state->Data_Figure_NameSequence_charioteer, &Data_Figure_NameSequence.charioteer);
    read_all_from_buffer(state->Data_Figure_NameSequence_barbarian, &Data_Figure_NameSequence.barbarian);
    read_all_from_buffer(state->Data_Figure_NameSequence_enemyGreek, &Data_Figure_NameSequence.enemyGreek);
    read_all_from_buffer(state->Data_Figure_NameSequence_enemyEgyptian, &Data_Figure_NameSequence.enemyEgyptian);
    read_all_from_buffer(state->Data_Figure_NameSequence_enemyArabian, &Data_Figure_NameSequence.enemyArabian);
    read_all_from_buffer(state->Data_Figure_NameSequence_trader, &Data_Figure_NameSequence.trader);
    read_all_from_buffer(state->Data_Figure_NameSequence_tradeShip, &Data_Figure_NameSequence.tradeShip);
    read_all_from_buffer(state->Data_Figure_NameSequence_warShip, &Data_Figure_NameSequence.warShip);
    read_all_from_buffer(state->Data_Figure_NameSequence_enemyShip, &Data_Figure_NameSequence.enemyShip);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_theater, &Data_CityInfo_CultureCoverage.theater);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_amphitheater, &Data_CityInfo_CultureCoverage.amphitheater);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_colosseum, &Data_CityInfo_CultureCoverage.colosseum);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_hospital1, &Data_CityInfo_CultureCoverage.hospital);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_hippodrome, &Data_CityInfo_CultureCoverage.hippodrome);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_religionCeres, &Data_CityInfo_CultureCoverage.religionCeres);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_religionNeptune, &Data_CityInfo_CultureCoverage.religionNeptune);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_religionMercury, &Data_CityInfo_CultureCoverage.religionMercury);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_religionMars, &Data_CityInfo_CultureCoverage.religionMars);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_religionVenus, &Data_CityInfo_CultureCoverage.religionVenus);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_oracle, &Data_CityInfo_CultureCoverage.oracle);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_school, &Data_CityInfo_CultureCoverage.school);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_library, &Data_CityInfo_CultureCoverage.library);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_academy, &Data_CityInfo_CultureCoverage.academy);
    read_all_from_buffer(state->Data_CityInfo_CultureCoverage_hospital, &Data_CityInfo_CultureCoverage.hospital);
    read_all_from_buffer(state->Data_Scenario, &Data_Scenario);
    read_all_from_buffer(state->Data_Event_timeLimitMaxGameYear, &Data_Event.timeLimitMaxGameYear);
    read_all_from_buffer(state->Data_Event_earthquake_gameYear, &Data_Event.earthquake.gameYear);
    read_all_from_buffer(state->Data_Event_earthquake_month, &Data_Event.earthquake.month);
    read_all_from_buffer(state->Data_Event_earthquake_state, &Data_Event.earthquake.state);
    read_all_from_buffer(state->Data_Event_earthquake_duration, &Data_Event.earthquake.duration);
    read_all_from_buffer(state->Data_Event_earthquake_maxDuration, &Data_Event.earthquake.maxDuration);
    read_all_from_buffer(state->Data_Event_earthquake_maxDelay, &Data_Event.earthquake.maxDelay);
    read_all_from_buffer(state->Data_Event_earthquake_delay, &Data_Event.earthquake.delay);
    read_all_from_buffer(state->Data_Event_earthquake_expand, &Data_Event.earthquake.expand);
    read_all_from_buffer(state->Data_Event_emperorChange_state, &Data_Event.emperorChange.state);
    read_all_from_buffer(state->Data_Message_messages, &Data_Message.messages);
    read_all_from_buffer(state->Data_Message_nextMessageSequence, &Data_Message.nextMessageSequence);
    read_all_from_buffer(state->Data_Message_totalMessages, &Data_Message.totalMessages);
    read_all_from_buffer(state->Data_Message_currentMessageId, &Data_Message.currentMessageId);
    read_all_from_buffer(state->Data_Message_populationMessagesShown, &Data_Message.populationMessagesShown);
    read_all_from_buffer(state->Data_Message_messageCategoryCount, &Data_Message.messageCategoryCount);
    read_all_from_buffer(state->Data_Message_messageDelay, &Data_Message.messageDelay);
    read_all_from_buffer(state->Data_BuildingList_burning_totalBurning, &Data_BuildingList.burning.totalBurning);
    read_all_from_buffer(state->Data_BuildingList_burning_index, &Data_BuildingList.burning.index);
    read_all_from_buffer(state->Data_Figure_Extra_createdSequence, &Data_Figure_Extra.createdSequence);
    read_all_from_buffer(state->Data_Settings_startingFavor, &Data_Settings.startingFavor);
    read_all_from_buffer(state->Data_Settings_personalSavingsLastMission, &Data_Settings.personalSavingsLastMission);
    read_all_from_buffer(state->Data_Settings_currentMissionId, &Data_Settings.currentMissionId);
    read_all_from_buffer(state->Data_InvasionWarnings, &Data_InvasionWarnings);
    read_all_from_buffer(state->Data_Settings_isCustomScenario, &Data_Settings.isCustomScenario);
    read_all_from_buffer(state->Data_Sound_City, &Data_Sound_City);
    read_all_from_buffer(state->Data_Buildings_Extra_highestBuildingIdInUse, &Data_Buildings_Extra.highestBuildingIdInUse);
    read_all_from_buffer(state->Data_Figure_Traders, &Data_Figure_Traders);
    read_all_from_buffer(state->Data_Figure_Extra_nextTraderId, &Data_Figure_Extra.nextTraderId);
    read_all_from_buffer(state->Data_BuildingList_burning_items, &Data_BuildingList.burning.items);
    read_all_from_buffer(state->Data_BuildingList_small_items, &Data_BuildingList.small.items);
    read_all_from_buffer(state->Data_BuildingList_large_items, &Data_BuildingList.large.items);
    read_all_from_buffer(state->Data_Tutorial_tutorial1_fire, &Data_Tutorial.tutorial1.fire);
    read_all_from_buffer(state->Data_Tutorial_tutorial1_crime, &Data_Tutorial.tutorial1.crime);
    read_all_from_buffer(state->Data_Tutorial_tutorial1_collapse, &Data_Tutorial.tutorial1.collapse);
    read_all_from_buffer(state->Data_Tutorial_tutorial2_granaryBuilt, &Data_Tutorial.tutorial2.granaryBuilt);
    read_all_from_buffer(state->Data_Tutorial_tutorial2_population250Reached, &Data_Tutorial.tutorial2.population250Reached);
    read_all_from_buffer(state->Data_Tutorial_tutorial1_senateBuilt, &Data_Tutorial.tutorial1.senateBuilt);
    read_all_from_buffer(state->Data_Tutorial_tutorial2_population450Reached, &Data_Tutorial.tutorial2.population450Reached);
    read_all_from_buffer(state->Data_Tutorial_tutorial2_potteryMade, &Data_Tutorial.tutorial2.potteryMade);
    read_all_from_buffer(state->Data_CityInfo_Buildings_militaryAcademy_total, &Data_CityInfo_Buildings.militaryAcademy.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_militaryAcademy_working, &Data_CityInfo_Buildings.militaryAcademy.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_barracks_total, &Data_CityInfo_Buildings.barracks.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_barracks_working, &Data_CityInfo_Buildings.barracks.working);
    read_all_from_buffer(state->Data_Formation_Extra_numEnemyFormations, &Data_Formation_Extra.numEnemyFormations);
    read_all_from_buffer(state->Data_Formation_Extra_numEnemySoldierStrength, &Data_Formation_Extra.numEnemySoldierStrength);
    read_all_from_buffer(state->Data_Formation_Extra_numLegionFormations, &Data_Formation_Extra.numLegionFormations);
    read_all_from_buffer(state->Data_Formation_Extra_numLegionSoldierStrength, &Data_Formation_Extra.numLegionSoldierStrength);
    read_all_from_buffer(state->Data_Formation_Extra_daysSinceRomanSoldierConcentration, &Data_Formation_Extra.daysSinceRomanSoldierConcentration);
    read_all_from_buffer(state->Data_Building_Storages, &Data_Building_Storages);
    read_all_from_buffer(state->Data_CityInfo_Buildings_actorColony_total, &Data_CityInfo_Buildings.actorColony.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_actorColony_working, &Data_CityInfo_Buildings.actorColony.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_gladiatorSchool_total, &Data_CityInfo_Buildings.gladiatorSchool.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_gladiatorSchool_working, &Data_CityInfo_Buildings.gladiatorSchool.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_lionHouse_total, &Data_CityInfo_Buildings.lionHouse.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_lionHouse_working, &Data_CityInfo_Buildings.lionHouse.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_chariotMaker_total, &Data_CityInfo_Buildings.chariotMaker.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_chariotMaker_working, &Data_CityInfo_Buildings.chariotMaker.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_market_total, &Data_CityInfo_Buildings.market.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_market_working, &Data_CityInfo_Buildings.market.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_reservoir_total, &Data_CityInfo_Buildings.reservoir.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_reservoir_working, &Data_CityInfo_Buildings.reservoir.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_fountain_total, &Data_CityInfo_Buildings.fountain.total);
    read_all_from_buffer(state->Data_CityInfo_Buildings_fountain_working, &Data_CityInfo_Buildings.fountain.working);
    read_all_from_buffer(state->Data_Tutorial_tutorial2_potteryMadeYear, &Data_Tutorial.tutorial2.potteryMadeYear);
    read_all_from_buffer(state->Data_Event_gladiatorRevolt_gameYear, &Data_Event.gladiatorRevolt.gameYear);
    read_all_from_buffer(state->Data_Event_gladiatorRevolt_month, &Data_Event.gladiatorRevolt.month);
    read_all_from_buffer(state->Data_Event_gladiatorRevolt_endMonth, &Data_Event.gladiatorRevolt.endMonth);
    read_all_from_buffer(state->Data_Event_gladiatorRevolt_state, &Data_Event.gladiatorRevolt.state);
    read_all_from_buffer(state->Data_Empire_Trade_maxPerYear, &Data_Empire_Trade.maxPerYear);
    read_all_from_buffer(state->Data_Empire_Trade_tradedThisYear, &Data_Empire_Trade.tradedThisYear);
    read_all_from_buffer(state->Data_Buildings_Extra_barracksTowerSentryRequested, &Data_Buildings_Extra.barracksTowerSentryRequested);
    read_all_from_buffer(state->Data_Buildings_Extra_createdSequence, &Data_Buildings_Extra.createdSequence);
    read_all_from_buffer(state->Data_Routes_unknown1RoutesCalculated, &Data_Routes.unknown1RoutesCalculated); //read_all_from_buffer(state->unk_634474, &unk_634474); not referenced
    read_all_from_buffer(state->Data_Routes_enemyRoutesCalculated, &Data_Routes.enemyRoutesCalculated);
    read_all_from_buffer(state->Data_Routes_totalRoutesCalculated, &Data_Routes.totalRoutesCalculated);
    read_all_from_buffer(state->Data_Routes_unknown2RoutesCalculated, &Data_Routes.unknown2RoutesCalculated); //read_all_from_buffer(state->unk_634470, &unk_634470); not referenced
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleCeres_working, &Data_CityInfo_Buildings.smallTempleCeres.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleNeptune_working, &Data_CityInfo_Buildings.smallTempleNeptune.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleMercury_working, &Data_CityInfo_Buildings.smallTempleMercury.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleMars_working, &Data_CityInfo_Buildings.smallTempleMars.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_smallTempleVenus_working, &Data_CityInfo_Buildings.smallTempleVenus.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleCeres_working, &Data_CityInfo_Buildings.largeTempleCeres.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleNeptune_working, &Data_CityInfo_Buildings.largeTempleNeptune.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleMercury_working, &Data_CityInfo_Buildings.largeTempleMercury.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleMars_working, &Data_CityInfo_Buildings.largeTempleMars.working);
    read_all_from_buffer(state->Data_CityInfo_Buildings_largeTempleVenus_working, &Data_CityInfo_Buildings.largeTempleVenus.working);
    read_all_from_buffer(state->Data_Formation_Invasion_formationId, &Data_Formation_Invasion.formationId);
    read_all_from_buffer(state->Data_Formation_Invasion_homeX, &Data_Formation_Invasion.homeX);
    read_all_from_buffer(state->Data_Formation_Invasion_homeY, &Data_Formation_Invasion.homeY);
    read_all_from_buffer(state->Data_Formation_Invasion_layout, &Data_Formation_Invasion.layout);
    read_all_from_buffer(state->Data_Formation_Invasion_destinationX, &Data_Formation_Invasion.destinationX);
    read_all_from_buffer(state->Data_Formation_Invasion_destinationY, &Data_Formation_Invasion.destinationY);
    read_all_from_buffer(state->Data_Formation_Invasion_destinationBuildingId, &Data_Formation_Invasion.destinationBuildingId);
    read_all_from_buffer(state->Data_Formation_Invasion_numLegions, &Data_Formation_Invasion.numLegions);
    read_all_from_buffer(state->Data_Formation_Invasion_ignoreRomanSoldiers, &Data_Formation_Invasion.ignoreRomanSoldiers);
    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_x, &Data_CityInfo_Extra.entryPointFlag.x);
    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_y, &Data_CityInfo_Extra.entryPointFlag.y);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_x, &Data_CityInfo_Extra.exitPointFlag.x);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_y, &Data_CityInfo_Extra.exitPointFlag.y);
    read_all_from_buffer(state->Data_Event_lastInternalInvasionId, &Data_Event.lastInternalInvasionId);
    read_all_from_buffer(state->Data_Debug_incorrectHousePositions, &Data_Debug.incorrectHousePositions);
    read_all_from_buffer(state->Data_Debug_unfixableHousePositions, &Data_Debug.unfixableHousePositions);
    read_all_from_buffer(state->Data_FileList_selectedScenario, &Data_FileList.selectedScenario);
    read_all_from_buffer(state->Data_CityInfo_Extra_bookmarks, &Data_CityInfo_Extra.bookmarks);
    read_all_from_buffer(state->Data_Tutorial_tutorial3_disease, &Data_Tutorial.tutorial3.disease);
    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_gridOffset, &Data_CityInfo_Extra.entryPointFlag.gridOffset);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_gridOffset, &Data_CityInfo_Extra.exitPointFlag.gridOffset);
    read_all_from_buffer(state->endMarker, &endMarker);
}

static void savegame_serialize(savegame_state *state)
{
    write_all_to_buffer(state->Data_Settings_saveGameMissionId, &Data_Settings.saveGameMissionId);
    write_all_to_buffer(state->savegameFileVersion, &savegameFileVersion);
    write_all_to_buffer(state->Data_Grid_graphicIds, &Data_Grid_graphicIds);
    write_all_to_buffer(state->Data_Grid_edge, &Data_Grid_edge);
    write_all_to_buffer(state->Data_Grid_buildingIds, &Data_Grid_buildingIds);
    write_all_to_buffer(state->Data_Grid_terrain, &Data_Grid_terrain);
    write_all_to_buffer(state->Data_Grid_aqueducts, &Data_Grid_aqueducts);
    write_all_to_buffer(state->Data_Grid_figureIds, &Data_Grid_figureIds);
    write_all_to_buffer(state->Data_Grid_bitfields, &Data_Grid_bitfields);
    write_all_to_buffer(state->Data_Grid_spriteOffsets, &Data_Grid_spriteOffsets);
    write_all_to_buffer(state->Data_Grid_random, &Data_Grid_random);
    write_all_to_buffer(state->Data_Grid_desirability, &Data_Grid_desirability);
    write_all_to_buffer(state->Data_Grid_elevation, &Data_Grid_elevation);
    write_all_to_buffer(state->Data_Grid_buildingDamage, &Data_Grid_buildingDamage);
    write_all_to_buffer(state->Data_Grid_Undo_aqueducts, &Data_Grid_Undo_aqueducts);
    write_all_to_buffer(state->Data_Grid_Undo_spriteOffsets, &Data_Grid_Undo_spriteOffsets);
    write_all_to_buffer(state->Data_Figures, &Data_Figures);
    write_all_to_buffer(state->Data_Routes_figureIds, &Data_Routes.figureIds);
    write_all_to_buffer(state->Data_Routes_directionPaths, &Data_Routes.directionPaths);
    write_all_to_buffer(state->Data_Formations, &Data_Formations);
    write_all_to_buffer(state->Data_Formation_Extra_idLastInUse, &Data_Formation_Extra.idLastInUse);
    write_all_to_buffer(state->Data_Formation_Extra_idLastLegion, &Data_Formation_Extra.idLastLegion);
    write_all_to_buffer(state->Data_Formation_Extra_numForts, &Data_Formation_Extra.numForts);
    write_all_to_buffer(state->Data_CityInfo, &Data_CityInfo);
    write_all_to_buffer(state->Data_CityInfo_Extra_unknownBytes, &Data_CityInfo_Extra.unknownBytes);
    write_all_to_buffer(state->playerNames, &playerNames);
    write_all_to_buffer(state->Data_CityInfo_Extra_ciid, &Data_CityInfo_Extra.ciid);
    write_all_to_buffer(state->Data_Buildings, &Data_Buildings);
    write_all_to_buffer(state->Data_Settings_Map_orientation, &Data_Settings_Map.orientation);
    write_all_to_buffer(state->Data_CityInfo_Extra_gameTimeTick, &Data_CityInfo_Extra.gameTimeTick);
    write_all_to_buffer(state->Data_CityInfo_Extra_gameTimeDay, &Data_CityInfo_Extra.gameTimeDay);
    write_all_to_buffer(state->Data_CityInfo_Extra_gameTimeMonth, &Data_CityInfo_Extra.gameTimeMonth);
    write_all_to_buffer(state->Data_CityInfo_Extra_gameTimeYear, &Data_CityInfo_Extra.gameTimeYear);
    write_all_to_buffer(state->Data_CityInfo_Extra_gameTimeTotalDays, &Data_CityInfo_Extra.gameTimeTotalDays);
    write_all_to_buffer(state->Data_Buildings_Extra_highestBuildingIdEver, &Data_Buildings_Extra.highestBuildingIdEver);
    write_all_to_buffer(state->Data_Debug_maxConnectsEver, &Data_Debug.maxConnectsEver);
    
    random_save_state(state->random_iv);

    write_all_to_buffer(state->Data_Settings_Map_camera_x, &Data_Settings_Map.camera.x);
    write_all_to_buffer(state->Data_Settings_Map_camera_y, &Data_Settings_Map.camera.y);
    write_all_to_buffer(state->Data_CityInfo_Buildings_theater_total, &Data_CityInfo_Buildings.theater.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_theater_working, &Data_CityInfo_Buildings.theater.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_amphitheater_total, &Data_CityInfo_Buildings.amphitheater.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_amphitheater_working, &Data_CityInfo_Buildings.amphitheater.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_colosseum_total, &Data_CityInfo_Buildings.colosseum.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_colosseum_working, &Data_CityInfo_Buildings.colosseum.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_hippodrome_total, &Data_CityInfo_Buildings.hippodrome.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_hippodrome_working, &Data_CityInfo_Buildings.hippodrome.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_school_total, &Data_CityInfo_Buildings.school.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_school_working, &Data_CityInfo_Buildings.school.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_library_total, &Data_CityInfo_Buildings.library.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_library_working, &Data_CityInfo_Buildings.library.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_academy_total, &Data_CityInfo_Buildings.academy.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_academy_working, &Data_CityInfo_Buildings.academy.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_barber_total, &Data_CityInfo_Buildings.barber.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_barber_working, &Data_CityInfo_Buildings.barber.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_bathhouse_total, &Data_CityInfo_Buildings.bathhouse.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_bathhouse_working, &Data_CityInfo_Buildings.bathhouse.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_clinic_total, &Data_CityInfo_Buildings.clinic.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_clinic_working, &Data_CityInfo_Buildings.clinic.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_hospital_total, &Data_CityInfo_Buildings.hospital.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_hospital_working, &Data_CityInfo_Buildings.hospital.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleCeres_total, &Data_CityInfo_Buildings.smallTempleCeres.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleNeptune_total, &Data_CityInfo_Buildings.smallTempleNeptune.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleMercury_total, &Data_CityInfo_Buildings.smallTempleMercury.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleMars_total, &Data_CityInfo_Buildings.smallTempleMars.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleVenus_total, &Data_CityInfo_Buildings.smallTempleVenus.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleCeres_total, &Data_CityInfo_Buildings.largeTempleCeres.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleNeptune_total, &Data_CityInfo_Buildings.largeTempleNeptune.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleMercury_total, &Data_CityInfo_Buildings.largeTempleMercury.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleMars_total, &Data_CityInfo_Buildings.largeTempleMars.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleVenus_total, &Data_CityInfo_Buildings.largeTempleVenus.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_oracle_total, &Data_CityInfo_Buildings.oracle.total);
    write_all_to_buffer(state->Data_CityInfo_Extra_populationGraphOrder, &Data_CityInfo_Extra.populationGraphOrder);
    write_all_to_buffer(state->Data_CityInfo_Extra_unknownOrder, &Data_CityInfo_Extra.unknownOrder);
    write_all_to_buffer(state->Data_Event_emperorChange_gameYear, &Data_Event.emperorChange.gameYear);
    write_all_to_buffer(state->Data_Event_emperorChange_month, &Data_Event.emperorChange.month);
    write_all_to_buffer(state->Data_Empire_scrollX, &Data_Empire.scrollX);
    write_all_to_buffer(state->Data_Empire_scrollY, &Data_Empire.scrollY);
    write_all_to_buffer(state->Data_Empire_selectedObject, &Data_Empire.selectedObject);
    write_all_to_buffer(state->Data_Empire_Cities, &Data_Empire_Cities);
    write_all_to_buffer(state->Data_CityInfo_Buildings_industry_total, &Data_CityInfo_Buildings.industry.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_industry_working, &Data_CityInfo_Buildings.industry.working);
    write_all_to_buffer(state->Data_TradePrices, &Data_TradePrices);
    write_all_to_buffer(state->Data_Figure_NameSequence_citizenMale, &Data_Figure_NameSequence.citizenMale);
    write_all_to_buffer(state->Data_Figure_NameSequence_patrician, &Data_Figure_NameSequence.patrician);
    write_all_to_buffer(state->Data_Figure_NameSequence_citizenFemale, &Data_Figure_NameSequence.citizenFemale);
    write_all_to_buffer(state->Data_Figure_NameSequence_taxCollector, &Data_Figure_NameSequence.taxCollector);
    write_all_to_buffer(state->Data_Figure_NameSequence_engineer, &Data_Figure_NameSequence.engineer);
    write_all_to_buffer(state->Data_Figure_NameSequence_prefect, &Data_Figure_NameSequence.prefect);
    write_all_to_buffer(state->Data_Figure_NameSequence_javelinThrower, &Data_Figure_NameSequence.javelinThrower);
    write_all_to_buffer(state->Data_Figure_NameSequence_cavalry, &Data_Figure_NameSequence.cavalry);
    write_all_to_buffer(state->Data_Figure_NameSequence_legionary, &Data_Figure_NameSequence.legionary);
    write_all_to_buffer(state->Data_Figure_NameSequence_actor, &Data_Figure_NameSequence.actor);
    write_all_to_buffer(state->Data_Figure_NameSequence_gladiator, &Data_Figure_NameSequence.gladiator);
    write_all_to_buffer(state->Data_Figure_NameSequence_lionTamer, &Data_Figure_NameSequence.lionTamer);
    write_all_to_buffer(state->Data_Figure_NameSequence_charioteer, &Data_Figure_NameSequence.charioteer);
    write_all_to_buffer(state->Data_Figure_NameSequence_barbarian, &Data_Figure_NameSequence.barbarian);
    write_all_to_buffer(state->Data_Figure_NameSequence_enemyGreek, &Data_Figure_NameSequence.enemyGreek);
    write_all_to_buffer(state->Data_Figure_NameSequence_enemyEgyptian, &Data_Figure_NameSequence.enemyEgyptian);
    write_all_to_buffer(state->Data_Figure_NameSequence_enemyArabian, &Data_Figure_NameSequence.enemyArabian);
    write_all_to_buffer(state->Data_Figure_NameSequence_trader, &Data_Figure_NameSequence.trader);
    write_all_to_buffer(state->Data_Figure_NameSequence_tradeShip, &Data_Figure_NameSequence.tradeShip);
    write_all_to_buffer(state->Data_Figure_NameSequence_warShip, &Data_Figure_NameSequence.warShip);
    write_all_to_buffer(state->Data_Figure_NameSequence_enemyShip, &Data_Figure_NameSequence.enemyShip);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_theater, &Data_CityInfo_CultureCoverage.theater);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_amphitheater, &Data_CityInfo_CultureCoverage.amphitheater);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_colosseum, &Data_CityInfo_CultureCoverage.colosseum);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_hospital1, &Data_CityInfo_CultureCoverage.hospital);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_hippodrome, &Data_CityInfo_CultureCoverage.hippodrome);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_religionCeres, &Data_CityInfo_CultureCoverage.religionCeres);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_religionNeptune, &Data_CityInfo_CultureCoverage.religionNeptune);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_religionMercury, &Data_CityInfo_CultureCoverage.religionMercury);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_religionMars, &Data_CityInfo_CultureCoverage.religionMars);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_religionVenus, &Data_CityInfo_CultureCoverage.religionVenus);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_oracle, &Data_CityInfo_CultureCoverage.oracle);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_school, &Data_CityInfo_CultureCoverage.school);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_library, &Data_CityInfo_CultureCoverage.library);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_academy, &Data_CityInfo_CultureCoverage.academy);
    write_all_to_buffer(state->Data_CityInfo_CultureCoverage_hospital, &Data_CityInfo_CultureCoverage.hospital);
    write_all_to_buffer(state->Data_Scenario, &Data_Scenario);
    write_all_to_buffer(state->Data_Event_timeLimitMaxGameYear, &Data_Event.timeLimitMaxGameYear);
    write_all_to_buffer(state->Data_Event_earthquake_gameYear, &Data_Event.earthquake.gameYear);
    write_all_to_buffer(state->Data_Event_earthquake_month, &Data_Event.earthquake.month);
    write_all_to_buffer(state->Data_Event_earthquake_state, &Data_Event.earthquake.state);
    write_all_to_buffer(state->Data_Event_earthquake_duration, &Data_Event.earthquake.duration);
    write_all_to_buffer(state->Data_Event_earthquake_maxDuration, &Data_Event.earthquake.maxDuration);
    write_all_to_buffer(state->Data_Event_earthquake_maxDelay, &Data_Event.earthquake.maxDelay);
    write_all_to_buffer(state->Data_Event_earthquake_delay, &Data_Event.earthquake.delay);
    write_all_to_buffer(state->Data_Event_earthquake_expand, &Data_Event.earthquake.expand);
    write_all_to_buffer(state->Data_Event_emperorChange_state, &Data_Event.emperorChange.state);
    write_all_to_buffer(state->Data_Message_messages, &Data_Message.messages);
    write_all_to_buffer(state->Data_Message_nextMessageSequence, &Data_Message.nextMessageSequence);
    write_all_to_buffer(state->Data_Message_totalMessages, &Data_Message.totalMessages);
    write_all_to_buffer(state->Data_Message_currentMessageId, &Data_Message.currentMessageId);
    write_all_to_buffer(state->Data_Message_populationMessagesShown, &Data_Message.populationMessagesShown);
    write_all_to_buffer(state->Data_Message_messageCategoryCount, &Data_Message.messageCategoryCount);
    write_all_to_buffer(state->Data_Message_messageDelay, &Data_Message.messageDelay);
    write_all_to_buffer(state->Data_BuildingList_burning_totalBurning, &Data_BuildingList.burning.totalBurning);
    write_all_to_buffer(state->Data_BuildingList_burning_index, &Data_BuildingList.burning.index);
    write_all_to_buffer(state->Data_Figure_Extra_createdSequence, &Data_Figure_Extra.createdSequence);
    write_all_to_buffer(state->Data_Settings_startingFavor, &Data_Settings.startingFavor);
    write_all_to_buffer(state->Data_Settings_personalSavingsLastMission, &Data_Settings.personalSavingsLastMission);
    write_all_to_buffer(state->Data_Settings_currentMissionId, &Data_Settings.currentMissionId);
    write_all_to_buffer(state->Data_InvasionWarnings, &Data_InvasionWarnings);
    write_all_to_buffer(state->Data_Settings_isCustomScenario, &Data_Settings.isCustomScenario);
    write_all_to_buffer(state->Data_Sound_City, &Data_Sound_City);
    write_all_to_buffer(state->Data_Buildings_Extra_highestBuildingIdInUse, &Data_Buildings_Extra.highestBuildingIdInUse);
    write_all_to_buffer(state->Data_Figure_Traders, &Data_Figure_Traders);
    write_all_to_buffer(state->Data_Figure_Extra_nextTraderId, &Data_Figure_Extra.nextTraderId);
    write_all_to_buffer(state->Data_BuildingList_burning_items, &Data_BuildingList.burning.items);
    write_all_to_buffer(state->Data_BuildingList_small_items, &Data_BuildingList.small.items);
    write_all_to_buffer(state->Data_BuildingList_large_items, &Data_BuildingList.large.items);
    write_all_to_buffer(state->Data_Tutorial_tutorial1_fire, &Data_Tutorial.tutorial1.fire);
    write_all_to_buffer(state->Data_Tutorial_tutorial1_crime, &Data_Tutorial.tutorial1.crime);
    write_all_to_buffer(state->Data_Tutorial_tutorial1_collapse, &Data_Tutorial.tutorial1.collapse);
    write_all_to_buffer(state->Data_Tutorial_tutorial2_granaryBuilt, &Data_Tutorial.tutorial2.granaryBuilt);
    write_all_to_buffer(state->Data_Tutorial_tutorial2_population250Reached, &Data_Tutorial.tutorial2.population250Reached);
    write_all_to_buffer(state->Data_Tutorial_tutorial1_senateBuilt, &Data_Tutorial.tutorial1.senateBuilt);
    write_all_to_buffer(state->Data_Tutorial_tutorial2_population450Reached, &Data_Tutorial.tutorial2.population450Reached);
    write_all_to_buffer(state->Data_Tutorial_tutorial2_potteryMade, &Data_Tutorial.tutorial2.potteryMade);
    write_all_to_buffer(state->Data_CityInfo_Buildings_militaryAcademy_total, &Data_CityInfo_Buildings.militaryAcademy.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_militaryAcademy_working, &Data_CityInfo_Buildings.militaryAcademy.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_barracks_total, &Data_CityInfo_Buildings.barracks.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_barracks_working, &Data_CityInfo_Buildings.barracks.working);
    write_all_to_buffer(state->Data_Formation_Extra_numEnemyFormations, &Data_Formation_Extra.numEnemyFormations);
    write_all_to_buffer(state->Data_Formation_Extra_numEnemySoldierStrength, &Data_Formation_Extra.numEnemySoldierStrength);
    write_all_to_buffer(state->Data_Formation_Extra_numLegionFormations, &Data_Formation_Extra.numLegionFormations);
    write_all_to_buffer(state->Data_Formation_Extra_numLegionSoldierStrength, &Data_Formation_Extra.numLegionSoldierStrength);
    write_all_to_buffer(state->Data_Formation_Extra_daysSinceRomanSoldierConcentration, &Data_Formation_Extra.daysSinceRomanSoldierConcentration);
    write_all_to_buffer(state->Data_Building_Storages, &Data_Building_Storages);
    write_all_to_buffer(state->Data_CityInfo_Buildings_actorColony_total, &Data_CityInfo_Buildings.actorColony.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_actorColony_working, &Data_CityInfo_Buildings.actorColony.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_gladiatorSchool_total, &Data_CityInfo_Buildings.gladiatorSchool.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_gladiatorSchool_working, &Data_CityInfo_Buildings.gladiatorSchool.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_lionHouse_total, &Data_CityInfo_Buildings.lionHouse.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_lionHouse_working, &Data_CityInfo_Buildings.lionHouse.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_chariotMaker_total, &Data_CityInfo_Buildings.chariotMaker.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_chariotMaker_working, &Data_CityInfo_Buildings.chariotMaker.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_market_total, &Data_CityInfo_Buildings.market.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_market_working, &Data_CityInfo_Buildings.market.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_reservoir_total, &Data_CityInfo_Buildings.reservoir.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_reservoir_working, &Data_CityInfo_Buildings.reservoir.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_fountain_total, &Data_CityInfo_Buildings.fountain.total);
    write_all_to_buffer(state->Data_CityInfo_Buildings_fountain_working, &Data_CityInfo_Buildings.fountain.working);
    write_all_to_buffer(state->Data_Tutorial_tutorial2_potteryMadeYear, &Data_Tutorial.tutorial2.potteryMadeYear);
    write_all_to_buffer(state->Data_Event_gladiatorRevolt_gameYear, &Data_Event.gladiatorRevolt.gameYear);
    write_all_to_buffer(state->Data_Event_gladiatorRevolt_month, &Data_Event.gladiatorRevolt.month);
    write_all_to_buffer(state->Data_Event_gladiatorRevolt_endMonth, &Data_Event.gladiatorRevolt.endMonth);
    write_all_to_buffer(state->Data_Event_gladiatorRevolt_state, &Data_Event.gladiatorRevolt.state);
    write_all_to_buffer(state->Data_Empire_Trade_maxPerYear, &Data_Empire_Trade.maxPerYear);
    write_all_to_buffer(state->Data_Empire_Trade_tradedThisYear, &Data_Empire_Trade.tradedThisYear);
    write_all_to_buffer(state->Data_Buildings_Extra_barracksTowerSentryRequested, &Data_Buildings_Extra.barracksTowerSentryRequested);
    write_all_to_buffer(state->Data_Buildings_Extra_createdSequence, &Data_Buildings_Extra.createdSequence);
    write_all_to_buffer(state->Data_Routes_unknown1RoutesCalculated, &Data_Routes.unknown1RoutesCalculated); //write_all_to_buffer(state->unk_634474, &unk_634474); not referenced
    write_all_to_buffer(state->Data_Routes_enemyRoutesCalculated, &Data_Routes.enemyRoutesCalculated);
    write_all_to_buffer(state->Data_Routes_totalRoutesCalculated, &Data_Routes.totalRoutesCalculated);
    write_all_to_buffer(state->Data_Routes_unknown2RoutesCalculated, &Data_Routes.unknown2RoutesCalculated); //write_all_to_buffer(state->unk_634470, &unk_634470); not referenced
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleCeres_working, &Data_CityInfo_Buildings.smallTempleCeres.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleNeptune_working, &Data_CityInfo_Buildings.smallTempleNeptune.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleMercury_working, &Data_CityInfo_Buildings.smallTempleMercury.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleMars_working, &Data_CityInfo_Buildings.smallTempleMars.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_smallTempleVenus_working, &Data_CityInfo_Buildings.smallTempleVenus.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleCeres_working, &Data_CityInfo_Buildings.largeTempleCeres.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleNeptune_working, &Data_CityInfo_Buildings.largeTempleNeptune.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleMercury_working, &Data_CityInfo_Buildings.largeTempleMercury.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleMars_working, &Data_CityInfo_Buildings.largeTempleMars.working);
    write_all_to_buffer(state->Data_CityInfo_Buildings_largeTempleVenus_working, &Data_CityInfo_Buildings.largeTempleVenus.working);
    write_all_to_buffer(state->Data_Formation_Invasion_formationId, &Data_Formation_Invasion.formationId);
    write_all_to_buffer(state->Data_Formation_Invasion_homeX, &Data_Formation_Invasion.homeX);
    write_all_to_buffer(state->Data_Formation_Invasion_homeY, &Data_Formation_Invasion.homeY);
    write_all_to_buffer(state->Data_Formation_Invasion_layout, &Data_Formation_Invasion.layout);
    write_all_to_buffer(state->Data_Formation_Invasion_destinationX, &Data_Formation_Invasion.destinationX);
    write_all_to_buffer(state->Data_Formation_Invasion_destinationY, &Data_Formation_Invasion.destinationY);
    write_all_to_buffer(state->Data_Formation_Invasion_destinationBuildingId, &Data_Formation_Invasion.destinationBuildingId);
    write_all_to_buffer(state->Data_Formation_Invasion_numLegions, &Data_Formation_Invasion.numLegions);
    write_all_to_buffer(state->Data_Formation_Invasion_ignoreRomanSoldiers, &Data_Formation_Invasion.ignoreRomanSoldiers);
    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_x, &Data_CityInfo_Extra.entryPointFlag.x);
    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_y, &Data_CityInfo_Extra.entryPointFlag.y);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_x, &Data_CityInfo_Extra.exitPointFlag.x);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_y, &Data_CityInfo_Extra.exitPointFlag.y);
    write_all_to_buffer(state->Data_Event_lastInternalInvasionId, &Data_Event.lastInternalInvasionId);
    write_all_to_buffer(state->Data_Debug_incorrectHousePositions, &Data_Debug.incorrectHousePositions);
    write_all_to_buffer(state->Data_Debug_unfixableHousePositions, &Data_Debug.unfixableHousePositions);
    write_all_to_buffer(state->Data_FileList_selectedScenario, &Data_FileList.selectedScenario);
    write_all_to_buffer(state->Data_CityInfo_Extra_bookmarks, &Data_CityInfo_Extra.bookmarks);
    write_all_to_buffer(state->Data_Tutorial_tutorial3_disease, &Data_Tutorial.tutorial3.disease);
    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_gridOffset, &Data_CityInfo_Extra.entryPointFlag.gridOffset);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_gridOffset, &Data_CityInfo_Extra.exitPointFlag.gridOffset);
    write_all_to_buffer(state->endMarker, &endMarker);
}

static void setupFromSavedGame();
static int writeCompressedChunk(FILE *fp, const void *buffer, int bytesToWrite);
static int readCompressedChunk(FILE *fp, void *buffer, int bytesToRead);

static void savegame_read_from_file(FILE *fp)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        if (piece->compressed) {
            readCompressedChunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fread(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
}

static void savegame_write_to_file(FILE *fp)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        printf("Writing piece %d (size %d)\n", i, piece->buf.size);
        if (piece->compressed) {
            writeCompressedChunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fwrite(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
}

int GameFile_loadSavedGame(const char *filename)
{
    init_savegame_data();
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 0;
	}
	Sound_stopMusic();
    savegame_read_from_file(fp);
	fclose(fp);
    
    savegame_deserialize(&savegame_data.state);
	
	setupFromSavedGame();
	BuildingStorage_resetBuildingIds();
	strcpy((char*)Data_Settings.playerName, playerNames[1]);
	return 1;
}

int GameFile_loadSavedGameFromMissionPack(int missionId)
{
    init_savegame_data();
	int offset;
	if (!io_read_file_part_into_buffer(missionPackFile, &offset, 4, 4 * missionId)) {
		return 0;
	}
	if (offset <= 0) {
		return 0;
	}
	FILE *fp = fopen(missionPackFile, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, offset, SEEK_SET);
    savegame_read_from_file(fp);
	fclose(fp);

    savegame_deserialize(&savegame_data.state);

    setupFromSavedGame();
	return 1;
}

static void debug()
{
	printf("TIME: y %d m %d d %d t %d\n",
		Data_CityInfo_Extra.gameTimeYear,
		Data_CityInfo_Extra.gameTimeMonth,
		Data_CityInfo_Extra.gameTimeDay,
		Data_CityInfo_Extra.gameTimeTick);
	for (int i = 1; i < MAX_BUILDINGS; i++) {
		struct Data_Building *b = &Data_Buildings[i];
		if (b->state != BuildingState_Unused || b->type) {
			printf("Building %d type %d inUse %d x %d y %d emp %d w %d ls %d hc %d\n",
				i, b->type, b->state, b->x, b->y, b->numWorkers, b->figureId, b->figureId2, b->housesCovered);
		}
	}
	/**/
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Figure *f = &Data_Figures[i];
		if (f->state == FigureState_Alive) {
			printf("Figure %d type %d as %d wt %d mt %d\n",
				i, f->type, f->actionState, f->waitTicks, f->waitTicksMissile);
		}
	}
}

static void setupFromSavedGame()
{
	debug();
	Empire_load(Data_Settings.isCustomScenario, Data_Scenario.empireId);
	Event_calculateDistantBattleRomanTravelTime();
	Event_calculateDistantBattleEnemyTravelTime();

	Data_Settings_Map.width = Data_Scenario.mapSizeX;
	Data_Settings_Map.height = Data_Scenario.mapSizeY;
	Data_Settings_Map.gridStartOffset = Data_Scenario.gridFirstElement;
	Data_Settings_Map.gridBorderSize = Data_Scenario.gridBorderSize;

	if (Data_Settings_Map.orientation >= 0 && Data_Settings_Map.orientation <= 6) {
		// ensure even number
		Data_Settings_Map.orientation = 2 * (Data_Settings_Map.orientation / 2);
	} else {
		Data_Settings_Map.orientation = 0;
	}

	CityView_calculateLookup();
	CityView_checkCameraBoundaries();

	Routing_clearLandTypeCitizen();
	Routing_determineLandCitizen();
	Routing_determineLandNonCitizen();
	Routing_determineWater();
	Routing_determineWalls();

	Building_determineGraphicIdsForOrientedBuildings();
	FigureRoute_clean();
	UtilityManagement_determineRoadNetworks();
	Building_GameTick_checkAccessToRome();
	Resource_gatherGranaryGettingInfo();
	SidebarMenu_enableBuildingMenuItemsAndButtons();
	PlayerMessage_initProblemArea();

	Sound_City_init();
	Sound_Music_reset();

	Data_State.undoAvailable = 0;
	Data_State.currentOverlay = 0;
	Data_State.previousOverlay = 0;
	Data_State.missionBriefingShown = 1;

	Data_CityInfo.tutorial1FireMessageShown = 1;
	Data_CityInfo.tutorial3DiseaseMessageShown = 1;

	Loader_Graphics_loadMainGraphics(Data_Scenario.climate);
	Loader_Graphics_loadEnemyGraphics(Data_Scenario.enemyId);
	Empire_determineDistantBattleCity();
	TerrainGraphics_determineGardensFromGraphicIds();

	Data_Message.maxScrollPosition = 0;
	Data_Message.scrollPosition = 0;

	Data_Settings.gamePaused = 0;
}

void GameFile_writeMissionSavedGameIfNeeded()
{
	int missionId = Data_Settings.currentMissionId;
	if (missionId < 0) {
		missionId = 0;
	} else if (missionId > 11) {
		missionId = 11;
	}
	if (!Data_CityInfo.missionSavedGameWritten) {
		Data_CityInfo.missionSavedGameWritten = 1;
		if (!file_exists(missionSavedGames[missionId])) {
			GameFile_writeSavedGame(missionSavedGames[missionId]);
		}
	}
}

int GameFile_writeSavedGame(const char *filename)
{
    init_savegame_data();
	printf("GameFile: Saving game to %s\n", filename);
	savegameFileVersion = savegameVersion;
	strcpy(playerNames[1], (char*)Data_Settings.playerName);
    savegame_serialize(&savegame_data.state);

	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return 0;
	}
	savegame_write_to_file(fp);
	fclose(fp);
	return 1;
}

int GameFile_deleteSavedGame(const char *filename)
{
	if (remove(filename) == 0) {
		return 1;
	}
	return 0;
}

int GameFile_loadScenario(const char *filename)
{
    init_scenario_data();
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 1;
	}

	for (int i = 0; i < scenario_data.num_pieces; i++) {
		fread(scenario_data.pieces[i].buf.data, 1, scenario_data.pieces[i].buf.size, fp);
	}
	fclose(fp);
    
    scenario_deserialize(&scenario_data.state);
    
    // check if all buffers are empty
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        buffer *buf = &scenario_data.pieces[i].buf;
        if (buf->index != buf->size) {
            printf("ERR: buffer %d not empty: %d of %d bytes used\n", i, buf->index, buf->size);
        }
    }

	Empire_load(1, Data_Scenario.empireId);
	Event_calculateDistantBattleRomanTravelTime();
	Event_calculateDistantBattleEnemyTravelTime();
	return 0;
}

static int writeCompressedChunk(FILE *fp, const void *buffer, int bytesToWrite)
{
	if (bytesToWrite > COMPRESS_BUFFER_SIZE) {
		return 0;
	}
	int outputSize = COMPRESS_BUFFER_SIZE;
	if (zip_compress(buffer, bytesToWrite, compressBuffer, &outputSize)) {
		fwrite(&outputSize, 4, 1, fp);
		fwrite(compressBuffer, 1, outputSize, fp);
	} else {
		// unable to compress: write uncompressed
		outputSize = UNCOMPRESSED;
		fwrite(&outputSize, 4, 1, fp);
		fwrite(buffer, 1, bytesToWrite, fp);
	}
	return 1;
}

static int readCompressedChunk(FILE *fp, void *buffer, int bytesToRead)
{
	if (bytesToRead > COMPRESS_BUFFER_SIZE) {
		return 0;
	}
	int inputSize = bytesToRead;
	fread(&inputSize, 4, 1, fp);
	if ((unsigned int) inputSize == UNCOMPRESSED) {
		fread(buffer, 1, bytesToRead, fp);
	} else {
		fread(compressBuffer, 1, inputSize, fp);
		if (!zip_decompress(compressBuffer, inputSize, buffer, &bytesToRead)) {
			return 0;
		}
	}
	return 1;
}
