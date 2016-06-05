#include "GameFile.h"

#include "Building.h"
#include "CityView.h"
#include "Empire.h"
#include "Event.h"
#include "FileSystem.h"
#include "Loader.h"
#include "PlayerMessage.h"
#include "Resource.h"
#include "Routing.h"
#include "SidebarMenu.h"
#include "Sound.h"
#include "TerrainGraphics.h"
#include "UtilityManagement.h"
#include "Walker.h"
#include "Zip.h"

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
#include "Data/Random.h"
#include "Data/Routes.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Sound.h"
#include "Data/State.h"
#include "Data/Trade.h"
#include "Data/Tutorial.h"
#include "Data/Figure.h"
#include "Data/Figure.h"

#include <stdio.h>
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

static GameFilePart scenarioParts[SCENARIO_PARTS] = {
	{0, &Data_Grid_graphicIds, 52488},
	{0, &Data_Grid_edge, 26244},
	{0, &Data_Grid_terrain, 52488},
	{0, &Data_Grid_bitfields, 26244},
	{0, &Data_Grid_random, 26244},
	{0, &Data_Grid_elevation, 26244},
	{0, &Data_Random.iv1, 4},
	{0, &Data_Random.iv2, 4},
	{0, &Data_Settings_Map.camera.x, 4},
	{0, &Data_Settings_Map.camera.y, 4},
	{0, &Data_Scenario, 1720},
};

static GameFilePart saveGameParts[SAVEGAME_PARTS] = {
	{0, &Data_Settings.saveGameMissionId, 4},
	{0, &savegameFileVersion, 4},
	{1, &Data_Grid_graphicIds, 52488},
	{1, &Data_Grid_edge, 26244},
	{1, &Data_Grid_buildingIds, 52488},
	{1, &Data_Grid_terrain, 52488},
	{1, &Data_Grid_aqueducts, 26244},
	{1, &Data_Grid_figureIds, 52488},
	{1, &Data_Grid_bitfields, 26244},
	{1, &Data_Grid_spriteOffsets, 26244},
	{0, &Data_Grid_random, 26244},
	{1, &Data_Grid_desirability, 26244},
	{1, &Data_Grid_elevation, 26244},
	{1, &Data_Grid_buildingDamage, 26244},
	{1, &Data_Grid_Undo_aqueducts, 26244},
	{1, &Data_Grid_Undo_spriteOffsets, 26244},
	{1, &Data_Walkers, 128000},
	{1, &Data_Routes.figureIds, 1200},
	{1, &Data_Routes.directionPaths, 300000},
	{1, &Data_Formations, 6400},
	{0, &Data_Formation_Extra.idLastInUse, 4},
	{0, &Data_Formation_Extra.idLastLegion, 4},
	{0, &Data_Formation_Extra.numForts, 4},
	{1, &Data_CityInfo, 36136},
	{0, &Data_CityInfo_Extra.unknownBytes, 2},
	{0, &playerNames, 64},
	{0, &Data_CityInfo_Extra.ciid, 4},
	{1, &Data_Buildings, 256000},
	{0, &Data_Settings_Map.orientation, 4},
	{0, &Data_CityInfo_Extra.gameTimeTick, 4},
	{0, &Data_CityInfo_Extra.gameTimeDay, 4},
	{0, &Data_CityInfo_Extra.gameTimeMonth, 4},
	{0, &Data_CityInfo_Extra.gameTimeYear, 4},
	{0, &Data_CityInfo_Extra.gameTimeTotalDays, 4},
	{0, &Data_Buildings_Extra.highestBuildingIdEver, 4},
	{0, &Data_Debug.maxConnectsEver, 4},
	{0, &Data_Random.iv1, 4},
	{0, &Data_Random.iv2, 4},
	{0, &Data_Settings_Map.camera.x, 4},
	{0, &Data_Settings_Map.camera.y, 4},
	{0, &Data_CityInfo_Buildings.theater.total, 4},
	{0, &Data_CityInfo_Buildings.theater.working, 4},
	{0, &Data_CityInfo_Buildings.amphitheater.total, 4},
	{0, &Data_CityInfo_Buildings.amphitheater.working, 4},
	{0, &Data_CityInfo_Buildings.colosseum.total, 4},
	{0, &Data_CityInfo_Buildings.colosseum.working, 4},
	{0, &Data_CityInfo_Buildings.hippodrome.total, 4},
	{0, &Data_CityInfo_Buildings.hippodrome.working, 4},
	{0, &Data_CityInfo_Buildings.school.total, 4},
	{0, &Data_CityInfo_Buildings.school.working, 4},
	{0, &Data_CityInfo_Buildings.library.total, 4},
	{0, &Data_CityInfo_Buildings.library.working, 4},
	{0, &Data_CityInfo_Buildings.academy.total, 4},
	{0, &Data_CityInfo_Buildings.academy.working, 4},
	{0, &Data_CityInfo_Buildings.barber.total, 4},
	{0, &Data_CityInfo_Buildings.barber.working, 4},
	{0, &Data_CityInfo_Buildings.bathhouse.total, 4},
	{0, &Data_CityInfo_Buildings.bathhouse.working, 4},
	{0, &Data_CityInfo_Buildings.clinic.total, 4},
	{0, &Data_CityInfo_Buildings.clinic.working, 4},
	{0, &Data_CityInfo_Buildings.hospital.total, 4},
	{0, &Data_CityInfo_Buildings.hospital.working, 4},
	{0, &Data_CityInfo_Buildings.smallTempleCeres.total, 4},
	{0, &Data_CityInfo_Buildings.smallTempleNeptune.total, 4},
	{0, &Data_CityInfo_Buildings.smallTempleMercury.total, 4},
	{0, &Data_CityInfo_Buildings.smallTempleMars.total, 4},
	{0, &Data_CityInfo_Buildings.smallTempleVenus.total, 4},
	{0, &Data_CityInfo_Buildings.largeTempleCeres.total, 4},
	{0, &Data_CityInfo_Buildings.largeTempleNeptune.total, 4},
	{0, &Data_CityInfo_Buildings.largeTempleMercury.total, 4},
	{0, &Data_CityInfo_Buildings.largeTempleMars.total, 4},
	{0, &Data_CityInfo_Buildings.largeTempleVenus.total, 4},
	{0, &Data_CityInfo_Buildings.oracle.total, 4},
	{0, &Data_CityInfo_Extra.populationGraphOrder, 4},
	{0, &Data_CityInfo_Extra.unknownOrder, 4},
	{0, &Data_Event.emperorChange.gameYear, 4},
	{0, &Data_Event.emperorChange.month, 4},
	{0, &Data_Empire.scrollX, 4},
	{0, &Data_Empire.scrollY, 4},
	{0, &Data_Empire.selectedObject, 4},
	{1, &Data_Empire_Cities, 2706},
	{0, &Data_CityInfo_Buildings.industry.total, 64},
	{0, &Data_CityInfo_Buildings.industry.working, 64},
	{0, &Data_TradePrices, 128},
	{0, &Data_Figure_NameSequence.citizenMale, 4},
	{0, &Data_Figure_NameSequence.patrician, 4},
	{0, &Data_Figure_NameSequence.citizenFemale, 4},
	{0, &Data_Figure_NameSequence.taxCollector, 4},
	{0, &Data_Figure_NameSequence.engineer, 4},
	{0, &Data_Figure_NameSequence.prefect, 4},
	{0, &Data_Figure_NameSequence.javelinThrower, 4},
	{0, &Data_Figure_NameSequence.cavalry, 4},
	{0, &Data_Figure_NameSequence.legionary, 4},
	{0, &Data_Figure_NameSequence.actor, 4},
	{0, &Data_Figure_NameSequence.gladiator, 4},
	{0, &Data_Figure_NameSequence.lionTamer, 4},
	{0, &Data_Figure_NameSequence.charioteer, 4},
	{0, &Data_Figure_NameSequence.barbarian, 4},
	{0, &Data_Figure_NameSequence.enemyGreek, 4},
	{0, &Data_Figure_NameSequence.enemyEgyptian, 4},
	{0, &Data_Figure_NameSequence.enemyArabian, 4},
	{0, &Data_Figure_NameSequence.trader, 4},
	{0, &Data_Figure_NameSequence.tradeShip, 4},
	{0, &Data_Figure_NameSequence.warShip, 4},
	{0, &Data_Figure_NameSequence.enemyShip, 4},
	{0, &Data_CityInfo_CultureCoverage.theater, 4},
	{0, &Data_CityInfo_CultureCoverage.amphitheater, 4},
	{0, &Data_CityInfo_CultureCoverage.colosseum, 4},
	{0, &Data_CityInfo_CultureCoverage.hospital, 4},
	{0, &Data_CityInfo_CultureCoverage.hippodrome, 4},
	{0, &Data_CityInfo_CultureCoverage.religionCeres, 4},
	{0, &Data_CityInfo_CultureCoverage.religionNeptune, 4},
	{0, &Data_CityInfo_CultureCoverage.religionMercury, 4},
	{0, &Data_CityInfo_CultureCoverage.religionMars, 4},
	{0, &Data_CityInfo_CultureCoverage.religionVenus, 4},
	{0, &Data_CityInfo_CultureCoverage.oracle, 4},
	{0, &Data_CityInfo_CultureCoverage.school, 4},
	{0, &Data_CityInfo_CultureCoverage.library, 4},
	{0, &Data_CityInfo_CultureCoverage.academy, 4},
	{0, &Data_CityInfo_CultureCoverage.hospital, 4},
	{0, &Data_Scenario, 1720},
	{0, &Data_Event.timeLimitMaxGameYear, 4},
	{0, &Data_Event.earthquake.gameYear, 4},
	{0, &Data_Event.earthquake.month, 4},
	{0, &Data_Event.earthquake.state, 4},
	{0, &Data_Event.earthquake.duration, 4},
	{0, &Data_Event.earthquake.maxDuration, 4},
	{0, &Data_Event.earthquake.maxDelay, 4},
	{0, &Data_Event.earthquake.delay, 4},
	{0, &Data_Event.earthquake.expand, 32},
	{0, &Data_Event.emperorChange.state, 4},
	{1, &Data_Message.messages, 16000},
	{0, &Data_Message.nextMessageSequence, 4},
	{0, &Data_Message.totalMessages, 4},
	{0, &Data_Message.currentMessageId, 4},
	{0, &Data_Message.populationMessagesShown, 0xA},
	{0, &Data_Message.messageCategoryCount, 80},
	{0, &Data_Message.messageDelay, 80},
	{0, &Data_BuildingList.burning.totalBurning, 4},
	{0, &Data_BuildingList.burning.index, 4},
	{0, &Data_Figure_Extra.createdSequence, 4},
	{0, &Data_Settings.startingFavor, 4},
	{0, &Data_Settings.personalSavingsLastMission, 4},
	{0, &Data_Settings.currentMissionId, 4},
	{1, &Data_InvasionWarnings, 3232},
	{0, &Data_Settings.isCustomScenario, 4},
	{0, &Data_Sound_City, 8960},
	{0, &Data_Buildings_Extra.highestBuildingIdInUse, 4},
	{0, &Data_Figure_Traders, 4800},
	{0, &Data_Figure_Extra.nextTraderId, 4},
	{1, &Data_BuildingList.burning.items, 1000},
	{1, &Data_BuildingList.small.items, 1000},
	{1, &Data_BuildingList.large.items, 4000},
	{0, &Data_Tutorial.tutorial1.fire, 4},
	{0, &Data_Tutorial.tutorial1.crime, 4},
	{0, &Data_Tutorial.tutorial1.collapse, 4},
	{0, &Data_Tutorial.tutorial2.granaryBuilt, 4},
	{0, &Data_Tutorial.tutorial2.population250Reached, 4},
	{0, &Data_Tutorial.tutorial1.senateBuilt, 4},
	{0, &Data_Tutorial.tutorial2.population450Reached, 4},
	{0, &Data_Tutorial.tutorial2.potteryMade, 4},
	{0, &Data_CityInfo_Buildings.militaryAcademy.total, 4},
	{0, &Data_CityInfo_Buildings.militaryAcademy.working, 4},
	{0, &Data_CityInfo_Buildings.barracks.total, 4},
	{0, &Data_CityInfo_Buildings.barracks.working, 4},
	{0, &Data_Formation_Extra.numEnemyFormations, 4},
	{0, &Data_Formation_Extra.numEnemySoldierStrength, 4},
	{0, &Data_Formation_Extra.numLegionFormations, 4},
	{0, &Data_Formation_Extra.numLegionSoldierStrength, 4},
	{0, &Data_Formation_Extra.daysSinceRomanSoldierConcentration, 4},
	{0, &Data_Building_Storages, 6400},
	{0, &Data_CityInfo_Buildings.actorColony.total, 4},
	{0, &Data_CityInfo_Buildings.actorColony.working, 4},
	{0, &Data_CityInfo_Buildings.gladiatorSchool.total, 4},
	{0, &Data_CityInfo_Buildings.gladiatorSchool.working, 4},
	{0, &Data_CityInfo_Buildings.lionHouse.total, 4},
	{0, &Data_CityInfo_Buildings.lionHouse.working, 4},
	{0, &Data_CityInfo_Buildings.chariotMaker.total, 4},
	{0, &Data_CityInfo_Buildings.chariotMaker.working, 4},
	{0, &Data_CityInfo_Buildings.market.total, 4},
	{0, &Data_CityInfo_Buildings.market.working, 4},
	{0, &Data_CityInfo_Buildings.reservoir.total, 4},
	{0, &Data_CityInfo_Buildings.reservoir.working, 4},
	{0, &Data_CityInfo_Buildings.fountain.total, 4},
	{0, &Data_CityInfo_Buildings.fountain.working, 4},
	{0, &Data_Tutorial.tutorial2.potteryMadeYear, 4},
	{0, &Data_Event.gladiatorRevolt.gameYear, 4},
	{0, &Data_Event.gladiatorRevolt.month, 4},
	{0, &Data_Event.gladiatorRevolt.endMonth, 4},
	{0, &Data_Event.gladiatorRevolt.state, 4},
	{1, &Data_Empire_Trade.maxPerYear, 1280},
	{1, &Data_Empire_Trade.tradedThisYear, 1280},
	{0, &Data_Buildings_Extra.barracksTowerSentryRequested, 4},
	{0, &Data_Buildings_Extra.createdSequence, 4},
	{0, &Data_Routes.unknown1RoutesCalculated, 4}, //{0, &unk_634474, 4}, not referenced
	{0, &Data_Routes.enemyRoutesCalculated, 4},
	{0, &Data_Routes.totalRoutesCalculated, 4},
	{0, &Data_Routes.unknown2RoutesCalculated, 4}, //{0, &unk_634470, 4}, not referenced
	{0, &Data_CityInfo_Buildings.smallTempleCeres.working, 4},
	{0, &Data_CityInfo_Buildings.smallTempleNeptune.working, 4},
	{0, &Data_CityInfo_Buildings.smallTempleMercury.working, 4},
	{0, &Data_CityInfo_Buildings.smallTempleMars.working, 4},
	{0, &Data_CityInfo_Buildings.smallTempleVenus.working, 4},
	{0, &Data_CityInfo_Buildings.largeTempleCeres.working, 4},
	{0, &Data_CityInfo_Buildings.largeTempleNeptune.working, 4},
	{0, &Data_CityInfo_Buildings.largeTempleMercury.working, 4},
	{0, &Data_CityInfo_Buildings.largeTempleMars.working, 4},
	{0, &Data_CityInfo_Buildings.largeTempleVenus.working, 4},
	{0, &Data_Formation_Invasion.formationId, 100},
	{0, &Data_Formation_Invasion.homeX, 100},
	{0, &Data_Formation_Invasion.homeY, 100},
	{0, &Data_Formation_Invasion.layout, 100},
	{0, &Data_Formation_Invasion.destinationX, 100},
	{0, &Data_Formation_Invasion.destinationY, 100},
	{0, &Data_Formation_Invasion.destinationBuildingId, 100},
	{0, &Data_Formation_Invasion.numLegions, 100},
	{0, &Data_Formation_Invasion.ignoreRomanSoldiers, 100},
	{0, &Data_CityInfo_Extra.entryPointFlag.x, 4},
	{0, &Data_CityInfo_Extra.entryPointFlag.y, 4},
	{0, &Data_CityInfo_Extra.exitPointFlag.x, 4},
	{0, &Data_CityInfo_Extra.exitPointFlag.y, 4},
	{0, &Data_Event.lastInternalInvasionId, 2},
	{0, &Data_Debug.incorrectHousePositions, 4},
	{0, &Data_Debug.unfixableHousePositions, 4},
	{0, &Data_FileList.selectedScenario, 65},
	{0, &Data_CityInfo_Extra.bookmarks, 32},
	{0, &Data_Tutorial.tutorial3.disease, 4},
	{0, &Data_CityInfo_Extra.entryPointFlag.gridOffset, 4},
	{0, &Data_CityInfo_Extra.exitPointFlag.gridOffset, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
	{0, &endMarker, 4},
};

static void setupFromSavedGame();
static int writeCompressedChunk(FILE *fp, const void *buffer, int bytesToWrite);
static int readCompressedChunk(FILE *fp, void *buffer, int bytesToRead);

int GameFile_loadSavedGame(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 0;
	}
	Sound_stopMusic();
	for (int i = 0; i < 300 && saveGameParts[i].lengthInBytes; i++) {
		if (saveGameParts[i].compressed) {
			readCompressedChunk(fp, saveGameParts[i].data, saveGameParts[i].lengthInBytes);
		} else {
			fread(saveGameParts[i].data, 1, saveGameParts[i].lengthInBytes, fp);
		}
	}
	fclose(fp);
	
	setupFromSavedGame();
	BuildingStorage_resetBuildingIds();
	strcpy(Data_Settings.playerName, playerNames[1]);
	return 1;
}

int GameFile_loadSavedGameFromMissionPack(int missionId)
{
	int offset;
	if (!FileSystem_readFilePartIntoBuffer(missionPackFile, &offset, 4, 4 * missionId)) {
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
	for (int i = 0; i < 300 && saveGameParts[i].lengthInBytes; i++) {
		if (saveGameParts[i].compressed) {
			readCompressedChunk(fp, saveGameParts[i].data, saveGameParts[i].lengthInBytes);
		} else {
			fread(saveGameParts[i].data, 1, saveGameParts[i].lengthInBytes, fp);
		}
	}
	fclose(fp);

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
				i, b->type, b->state, b->x, b->y, b->numWorkers, b->walkerId, b->walkerId2, b->housesCovered);
		}
	}
	/**/
	for (int i = 1; i < MAX_FIGURES; i++) {
		struct Data_Walker *w = &Data_Walkers[i];
		if (w->state == FigureState_Alive) {
			printf("Walker %d type %d as %d wt %d mt %d\n",
				i, w->type, w->actionState, w->waitTicks, w->waitTicksMissile);
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
		if (!FileSystem_fileExists(missionSavedGames[missionId])) {
			GameFile_writeSavedGame(missionSavedGames[missionId]);
		}
	}
}

int GameFile_writeSavedGame(const char *filename)
{
	printf("GameFile: Saving game to %s\n", filename);
	savegameFileVersion = savegameVersion;
	strcpy(playerNames[1], Data_Settings.playerName);

	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return 0;
	}
	for (int i = 0; i < 300 && saveGameParts[i].lengthInBytes; i++) {
		if (saveGameParts[i].compressed) {
			writeCompressedChunk(fp, saveGameParts[i].data, saveGameParts[i].lengthInBytes);
		} else {
			fwrite(saveGameParts[i].data, 1, saveGameParts[i].lengthInBytes, fp);
		}
	}
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
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 1;
	}

	for (int i = 0; i < SCENARIO_PARTS && scenarioParts[i].lengthInBytes > 0; i++) {
		fread(scenarioParts[i].data, 1, scenarioParts[i].lengthInBytes, fp);
	}
	fclose(fp);

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
	if (Zip_compress(buffer, bytesToWrite, compressBuffer, &outputSize)) {
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
		if (!Zip_decompress(compressBuffer, inputSize, buffer, &bytesToRead)) {
			return 0;
		}
	}
	return 1;
}
