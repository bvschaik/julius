#include "GameFile.h"

#include "Data/Grid.h"
#include "Data/Random.h"
#include "Data/Scenario.h"
#include "Data/Settings.h"
#include "Data/Message.h"
#include "Data/Empire.h"
#include "Data/CityInfo.h"
#include "Data/Walker.h"
#include "Data/Tutorial.h"
#include "Data/Sound.h"

#include "Zip.h"

#include <stdio.h>

#define SCENARIO_PARTS 12
#define SAVEGAME_PARTS 300
#define COMPRESS_BUFFER_SIZE 600000
#define UNCOMPRESSED 0x80000000

struct GameFilePart {
	int compressed;
	void *data;
	int lengthInBytes;
};

static const int savegame_version = 0x66;
static int savegame_file_version;

static char compressBuffer[COMPRESS_BUFFER_SIZE]; // TODO use global malloc'ed scratchpad buffer

static char tmp[COMPRESS_BUFFER_SIZE]; // TODO remove when all savegame fields are known

static int endMarker = 0;

static char missionSavedGames[][32] = {
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
	{0, &Data_Tutorial_tutorial1.fire, 4},
	{0, &Data_Settings_Map.camera.x, 4},
	{0, &Data_Settings_Map.camera.y, 4},
	{0, &Data_Scenario, 1720},
};

static GameFilePart saveGameParts[SAVEGAME_PARTS] = {
	{0, &Data_Settings.saveGameMissionId, 4},
	{0, &savegame_file_version, 4},
	{1, &Data_Grid_graphicIds, 52488},
	{1, &Data_Grid_edge, 26244},
	{1, &Data_Grid_buildingIds, 52488},
	{1, &Data_Grid_terrain, 52488},
	{1, &Data_Grid_aqueducts, 26244},
	{1, &Data_Grid_walkerIds, 52488},
	{1, &Data_Grid_bitfields, 26244},
	{1, &Data_Grid_animation, 26244},
	{0, &Data_Grid_random, 26244},
	{1, &Data_Grid_desirability, 26244},
	{1, &Data_Grid_elevation, 26244},
	{1, &Data_Grid_buildingDamage, 26244},
	{1, &tmp, 26244}, //{1, &undo_grid_aqueducts, 26244},
	{1, &tmp, 26244}, //{1, &undo_grid_animation, 26244},
	{1, &tmp, 128000}, //{1, &walker_inUse, 0x1F400},
	{1, &tmp, 1200}, //{1, &destinationpath_index, 0x4B0},
	{1, &tmp, 300000}, //{1, &destinationpath_data, 0x493E0},
	{1, &tmp, 6400}, //{1, &formation_start_inUse, 0x1900},
	{0, &tmp, 4}, //{0, &formationId_lastInUse, 4},
	{0, &tmp, 4}, //{0, &formationId_lastFort, 4},
	{0, &tmp, 4}, //{0, &numForts, 4},
	{1, &Data_CityInfo, 36136},
	{0, &tmp, 2}, //{0, &byte_658DCC, 2},
	{0, &tmp, 64}, //{0, &save_player0name, 0x40},
	{0, &tmp, 4}, //{0, &ciid, 4},
	{1, &tmp, 256000}, //{1, &building_inUse, 0x3E800},
	{0, &tmp, 4}, //{0, &mapOrientation, 4},
	{0, &tmp, 4}, //{0, &game_ticks, 4},
	{0, &tmp, 4}, //{0, &game_weeks, 4},
	{0, &Data_CityInfo_Extra.gameTimeMonth, 4},
	{0, &Data_CityInfo_Extra.gameTimeYear, 4},
	{0, &Data_CityInfo_Extra.gameTimeTotalWeeks, 4},
	{0, &tmp, 4}, //{0, &buildingId_highestInUseEver, 4},
	{0, &tmp, 4}, //{0, &dword_98C480, 4},
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
	{0, &tmp, 4}, //{0, &unk_650060, 4},
	{0, &tmp, 4}, //{0, &event_emperorChange_gameYear, 4},
	{0, &tmp, 4}, //{0, &event_emperorChange_month, 4},
	{0, &Data_Empire.scrollX, 4},
	{0, &Data_Empire.scrollY, 4},
	{0, &Data_Empire.selectedObject, 4},
	{1, &Data_Empire_Cities, 2706},
	{0, &Data_CityInfo_Buildings.industry.total, 64},
	{0, &Data_CityInfo_Buildings.industry.working, 64},
	{0, &tmp, 128}, //{0, &tradeprices_buy, 0x80},
	{0, &Data_Walker_NameSequence.citizen, 4},
	{0, &Data_Walker_NameSequence.patrician, 4},
	{0, &Data_Walker_NameSequence.female, 4},
	{0, &Data_Walker_NameSequence.taxCollector, 4},
	{0, &Data_Walker_NameSequence.engineer, 4},
	{0, &Data_Walker_NameSequence.prefect, 4},
	{0, &Data_Walker_NameSequence.javelinThrower, 4},
	{0, &Data_Walker_NameSequence.cavalry, 4},
	{0, &Data_Walker_NameSequence.legionary, 4},
	{0, &Data_Walker_NameSequence.actor, 4},
	{0, &Data_Walker_NameSequence.gladiator, 4},
	{0, &Data_Walker_NameSequence.lionTamer, 4},
	{0, &Data_Walker_NameSequence.charioteer, 4},
	{0, &Data_Walker_NameSequence.barbarian, 4},
	{0, &Data_Walker_NameSequence.greek, 4},
	{0, &Data_Walker_NameSequence.egyptian, 4},
	{0, &Data_Walker_NameSequence.arabian, 4},
	{0, &Data_Walker_NameSequence.trader, 4},
	{0, &Data_Walker_NameSequence.tradeShip, 4},
	{0, &Data_Walker_NameSequence.unknown1, 4},
	{0, &Data_Walker_NameSequence.unknown2, 4},
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
	{0, &Data_CityInfo_CultureCoverage.oracles, 4},
	{0, &Data_CityInfo_CultureCoverage.school, 4},
	{0, &Data_CityInfo_CultureCoverage.library, 4},
	{0, &Data_CityInfo_CultureCoverage.academy, 4},
	{0, &Data_CityInfo_CultureCoverage.hospital, 4},
	{0, &Data_Scenario, 1720},
	{0, &tmp, 4}, //{0, &timeLimit_maxGameYear, 4},
	{0, &tmp, 4}, //{0, &event_earthquake_gameYear, 4},
	{0, &tmp, 4}, //{0, &event_earthquake_month, 4},
	{0, &tmp, 4}, //{0, &event_earthquake_state, 4},
	{0, &tmp, 4}, //{0, &dword_89AA8C, 4},
	{0, &tmp, 4}, //{0, &event_earthquake_maxDuration, 4},
	{0, &tmp, 4}, //{0, &event_earthquake_maxDamage, 4},
	{0, &tmp, 4}, //{0, &dword_89AB08, 4},
	{0, &tmp, 32}, //{0, &dword_929660, 0x20},
	{0, &tmp, 4}, //{0, &event_emperorChange_state, 4},
	{1, &Data_Message.messages, 16000},
	{0, &Data_Message.nextMessageId, 4},
	{0, &Data_Message.totalMessages, 4},
	{0, &Data_Message.currentMessageId, 4},
	{0, &Data_Message.populationMessagesShown, 0xA},
	{0, &tmp, 80}, //{0, &dword_65DE20, 0x50},
	{0, &tmp, 80}, //{0, &dword_65DE80, 0x50},
	{0, &tmp, 4}, //{0, &dword_98BF18, 4},
	{0, &tmp, 4}, //{0, &dword_98C020, 4},
	{0, &tmp, 4}, //{0, &dword_607FC8, 4},
	{0, &tmp, 4}, //{0, &startingFavor, 4},
	{0, &tmp, 4}, //{0, &personalSavings_lastMission, 4},
	{0, &tmp, 4}, //{0, &setting_currentMissionId, 4},
	{1, &tmp, 3232}, //{1, &byte_98EF40, 0xCA0},
	{0, &tmp, 4}, //{0, &setting_isCustomScenario, 4},
	{0, &Data_Sound_City, 8960},
	{0, &tmp, 4}, //{0, &buildingId_highestInUse, 4},
	{0, &tmp, 4800}, //{0, &dword_993F60, 0x12C0},
	{0, &tmp, 4}, //{0, &dword_990CD8, 4},
	{1, &tmp, 1000}, //{1, &word_98C080, 0x3E8},
	{1, &tmp, 1000}, //{1, &word_949F00, 0x3E8},
	{1, &tmp, 4000}, //{1, &gametick_tmpBuildingList, 0xFA0},
	{0, &Data_Tutorial_tutorial1.fire, 4},
	{0, &Data_Tutorial_tutorial1.crime, 4},
	{0, &Data_Tutorial_tutorial1.prefectureBuilt, 4},
	{0, &Data_Tutorial_tutorial2.granaryBuilt, 4},
	{0, &Data_Tutorial_tutorial2.population250Reached, 4},
	{0, &Data_Tutorial_tutorial1.senateEngineerBuilt, 4},
	{0, &Data_Tutorial_tutorial2.population450Reached, 4},
	{0, &Data_Tutorial_tutorial2.potteryMade, 4},
	{0, &Data_CityInfo_Buildings.militaryAcademy.total, 4},
	{0, &Data_CityInfo_Buildings.militaryAcademy.working, 4},
	{0, &Data_CityInfo_Buildings.barracks.total, 4},
	{0, &Data_CityInfo_Buildings.barracks.working, 4},
	{0, &tmp, 4}, //{0, &dword_819848, 4},
	{0, &tmp, 4}, //{0, &dword_7FA234, 4},
	{0, &tmp, 4}, //{0, &dword_7F87A8, 4},
	{0, &tmp, 4}, //{0, &dword_7F87AC, 4},
	{0, &tmp, 4}, //{0, &dword_863318, 4},
	{0, &tmp, 6400}, //{0, &storage_start, 0x1900},
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
	{0, &Data_Tutorial_tutorial2.potteryMadeYear, 4},
	{0, &tmp, 4}, //{0, &event_gladiatorRevolt_gameYear, 4},
	{0, &tmp, 4}, //{0, &event_gladiatorRevolt_month, 4},
	{0, &tmp, 4}, //{0, &event_gladiatorRevold_endMonth, 4},
	{0, &tmp, 4}, //{0, &event_gladiatorRevolt_state, 4},
	{1, &Data_Empire_Trade.maxPerYear, 1280},
	{1, &Data_Empire_Trade.tradedThisYear, 1280},
	{0, &tmp, 4}, //{0, &dword_7FA224, 4},
	{0, &tmp, 4}, //{0, &buildingId_placedSequence, 4},
	{0, &tmp, 4}, //{0, &unk_634474, 4},
	{0, &tmp, 4}, //{0, &dword_614158, 4},
	{0, &tmp, 4}, //{0, &dword_634468, 4},
	{0, &tmp, 4}, //{0, &unk_634470, 4},
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
	{0, &tmp, 100}, //{0, &dword_7FA2C0, 0x64},
	{0, &tmp, 100}, //{0, &dword_862DE0, 0x64},
	{0, &tmp, 100}, //{0, &dword_862D60, 0x64},
	{0, &tmp, 100}, //{0, &dword_819860, 0x64},
	{0, &tmp, 100}, //{0, &dword_819760, 0x64},
	{0, &tmp, 100}, //{0, &dword_8197E0, 0x64},
	{0, &tmp, 100}, //{0, &dword_7FA1C0, 0x64},
	{0, &tmp, 100}, //{0, &dword_8198E0, 0x64},
	{0, &tmp, 100}, //{0, &dword_7FA240, 0x64},
	{0, &tmp, 4}, //{0, &dword_607F94, 4},
	{0, &tmp, 4}, //{0, &dword_607F98, 4},
	{0, &tmp, 4}, //{0, &dword_607F9C, 4},
	{0, &tmp, 4}, //{0, &dword_607FA0, 4},
	{0, &tmp, 2}, //{0, &lastInvasionInternalId, 2},
	{0, &tmp, 4}, //{0, &dword_98C064, 4},
	{0, &tmp, 4}, //{0, &dword_94A30C, 4},
	{0, &tmp, 65}, //{0, &currentScenarioFilename, 0x41},
	{0, &tmp, 32}, //{0, &mapBookmarks_x, 0x20},
	{0, &tmp, 4}, //{0, &tutorial2_disease, 4},
	{0, &tmp, 4}, //{0, &dword_8E7B24, 4},
	{0, &tmp, 4}, //{0, &dword_89AA64, 4},
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

static int writeCompressedChunk(FILE *fp, const void *buffer, int bytesToWrite);
static int readCompressedChunk(FILE *fp, void *buffer, int bytesToRead);


/*
int GameFile_deleteSavedGame(const char *filename)
{
	int ok = remove(filename);
	if (ok == -1) {
		// Error
		return 1;
	}
	// Refresh file list
	FileSystem_findFiles("*.sav");
	///
    filelist_numFiles = findfiles_numFiles;
    filelist_selectedIndex = 0;
    if ( filelist_scrollPosition + 12 >= filelist_numFiles )
      --filelist_scrollPosition;
    if ( filelist_scrollPosition < 0 )
      filelist_scrollPosition = 0;
	return 0;
}

signed int __cdecl fun_writeSavedGame(const char *filename)
{
  signed int result; // eax@2
  signed int i; // [sp+4Ch] [bp-Ch]@3
  int length; // [sp+50h] [bp-8h]@7
  int fd; // [sp+54h] [bp-4h]@1

  savedgame_version = savedgame_fileversion;
  strcpy(save_playername, setting_player_name);
  j_fun_chdirHome();
  fd = _open(filename, 0x8301u);
  if ( fd == -1 )
  {
    result = 1;
  }
  else
  {
    for ( i = 0; i < 300 && savedgame_fields[i].length; ++i )
    {
      length = savedgame_fields[i].length;
      if ( savedgame_fields[i].compressed )
        j_fun_writeCompressedChunk(fd, savedgame_fields[i].offset, length);
      else
        _write(fd, savedgame_fields[i].offset, length);
    }
    _close(fd);
    result = 0;
  }
  return result;
}
*/

int GameFile_loadSavedGame(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		return 1;
	}
	// TODO sub_403116(0);
	for (int i = 0; i < 300 && saveGameParts[i].lengthInBytes; i++) {
		if (saveGameParts[i].compressed) {
			readCompressedChunk(fp, saveGameParts[i].data, saveGameParts[i].lengthInBytes);
		} else {
			fread(saveGameParts[i].data, 1, saveGameParts[i].lengthInBytes, fp);
		}
	}
	fclose(fp);
	// TODO setup
	return 0;
}
/*
//----- (004D6CC0) --------------------------------------------------------
signed int __cdecl fun_loadSavedGame(const char *filename)
{
  signed int result; // eax@2
  signed int i; // [sp+50h] [bp-Ch]@3
  signed int j; // [sp+50h] [bp-Ch]@6
  int length; // [sp+54h] [bp-8h]@10
  int fd; // [sp+58h] [bp-4h]@1

  j_fun_chdirHome();
  byte_6A10F8 = 0;
  fd = _open(filename, 0x8000u);
  if ( fd == -1 )
  {
    result = 1;
  }
  else
  {
    sub_403116(0);
    for ( i = 0; i < 2; ++i )
      _read(fd, savedgame_fields[i].offset, savedgame_fields[i].length);
    for ( j = 2; j < 300 && savedgame_fields[j].length; ++j )
    {
      length = savedgame_fields[j].length;
      if ( savedgame_fields[j].compressed )
        j_fun_readCompressedChunk(fd, savedgame_fields[j].offset, length);
      else
        _read(fd, savedgame_fields[j].offset, length);
    }
    _close(fd);
    byte_6A10F8 = 1;
    j_fun_setupFromSavedGame();
    j_fun_setStorageBuildingIds();
    strcpy(setting_player_name, save_playername);
    result = 0;
  }
  return result;
}
// 53F440: using guessed type _DWORD __cdecl _close(_DWORD);
// 6A10F8: using guessed type char byte_6A10F8;

static void fun_setupFromSavedGame()
{
  dword_990608 = scn_empire;
  sub_40323D();
  if ( setting_isCustomScenario )
    j_fun_readEmpireFile(1);
  else
    j_fun_readEmpireFile(0);
  j_fun_calculateDistantBattleRomanTravelTime(1);
  j_fun_calculateDistantBattleEnemyTravelTime(1);
  setting_map_width = scn_sizeX;
  setting_map_height = scn_sizeY;
  setting_map_startGridOffset = scn_gridFirstElement;
  setting_map_gridBorderSize = scn_gridBorderSize;
  if ( mapOrientation >= 0 )
  {
    if ( mapOrientation > 6 )
      mapOrientation = 0;
  }
  else
  {
    mapOrientation = 0;
  }
  mapCurrentOrientation = mapOrientation;
  sub_4017CB(mapOrientation);
  sub_40230B();
  j_fun_clearGroundTypeGrid();
  j_fun_determineGroundType();
  sub_4019D8();
  sub_401D84();
  sub_402B26();
  sub_402AC2();
  j_fun_garbageCollectDestinationPaths();
  sub_401320();
  sub_4033FA();
  sub_403472();
  j_fun_enableBuildingMenuItems();
  j_fun_enableSidebarButtons();
  sub_4017FD();
  sub_401613();
  j_fun_resetBackgroundMusic();
  dword_8E1484 = 0;
  currentOverlay = 0;
  previousOverlay = 0;
  dword_9DA7B0 = 1;
  dword_654644[4517 * ciid] = 1;
  dword_654640[4517 * ciid] = 1;
  j_fun_loadClimateGraphics((unsigned __int8)scn_climate);
  j_fun_loadEnemyGraphics(scn_enemy);
  j_fun_determineDistantBattleCity();
  sub_401E97();
  dword_658DD0 = 0;
  dword_659C18 = 0;
  setting_game_paused = 0;
}
// 4017CB: using guessed type _DWORD __cdecl sub_4017CB(_DWORD);
// 40227A: using guessed type _DWORD __cdecl j_fun_readEmpireFile(_DWORD);
// 402AC2: using guessed type int sub_402AC2(void);
// 608074: using guessed type int previousOverlay;
// 654640: using guessed type int dword_654640[];
// 654644: using guessed type int dword_654644[];
// 65DEF8: using guessed type int mapOrientation;
// 65E708: using guessed type char setting_game_paused;
// 65E764: using guessed type int setting_isCustomScenario;
// 8C79F8: using guessed type int setting_map_width;
// 8C79FC: using guessed type int setting_map_height;
// 8C7A00: using guessed type int setting_map_startGridOffset;
// 8C7A04: using guessed type int setting_map_gridBorderSize;
// 98E884: using guessed type __int16 scn_empire;
// 98E9FC: using guessed type __int16 scn_enemy;
// 98EA04: using guessed type int scn_sizeX;
// 98EA08: using guessed type int scn_sizeY;
// 98EA0C: using guessed type int scn_gridFirstElement;
// 98EA10: using guessed type int scn_gridBorderSize;
// 98EF28: using guessed type char scn_climate;
// 990608: using guessed type int dword_990608;
// 9DA7B0: using guessed type int dword_9DA7B0;
*/
/*
void GameFile fun_writeMissionSavedGame(int missionId)
{
  signed int v0; // [sp+4Ch] [bp-4h]@1

  v0 = setting_currentMissionId;
  if ( setting_currentMissionId > 11 )
    v0 = 11;
  if ( !dword_65463C[4517 * ciid] )
  {
    dword_65463C[4517 * ciid] = 1;
    if ( !j_fun_fileExistsHome(&aCitizen_sav[40 * v0]) )
      j_fun_writeSavedGame(&aCitizen_sav[40 * v0]);
  }
}
// 65463C: using guessed type int dword_65463C[];
// 65E760: using guessed type int setting_currentMissionId;
*/
//----- (004D7150) --------------------------------------------------------
int GameFile_saveScenario(const char *filename)
{
	//map_graphic_nativeHut = graphic_nativeBuilding;
	//map_graphic_nativeMeeting = graphic_nativeBuilding + 2;
	//map_graphic_nativeCrops = graphic_nativeCrops;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		return 1;
	}
	for (int i = 0; i < SCENARIO_PARTS && scenarioParts[i].lengthInBytes > 0; i++) {
		fwrite(scenarioParts[i].data, 1, scenarioParts[i].lengthInBytes, fp);
	}
	fclose(fp);
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
    //dword_990608 = map_empire;
    //j_fun_readEmpireFile(1);
    //j_fun_calculateDistantBattleRomanTravelTime(1);
    //j_fun_calculateDistantBattleEnemyTravelTime(1);
    return 0;
}

/*
//----- (004D7350) --------------------------------------------------------
signed int __cdecl fun_loadSavedGameFromMissionPack(int a1)
{
  signed int result; // eax@2
  signed int i; // [sp+4Ch] [bp-10h]@7
  signed int j; // [sp+4Ch] [bp-10h]@10
  int v4; // [sp+50h] [bp-Ch]@14
  int v5; // [sp+54h] [bp-8h]@5
  LONG Buffer; // [sp+58h] [bp-4h]@1

  j_fun_chdirHome();
  if ( j_fun_readDataFromFilename(&aMission1_pak[20 * a1], &Buffer, 4u, 4 * savedgame_missionId) )
  {
    if ( Buffer > 0 )
    {
      v5 = _open(&aMission1_pak[20 * a1], 32768);
      if ( v5 == -1 )
      {
        result = 0;
      }
      else
      {
        _lseek(v5, Buffer, 0);
        for ( i = 0; i < 2; ++i )
          _read(v5, savedgame_fields[i].offset, savedgame_fields[i].length);
        for ( j = 2; j < 300 && savedgame_fields[j].length; ++j )
        {
          v4 = savedgame_fields[j].length;
          if ( savedgame_fields[j].compressed )
            j_fun_readCompressedChunk(v5, savedgame_fields[j].offset, v4);
          else
            _read(v5, savedgame_fields[j].offset, v4);
        }
        _close(v5);
        j_fun_setupFromSavedGame();
        result = 1;
      }
    }
    else
    {
      result = 0;
    }
  }
  else
  {
    result = 0;
  }
  return result;
}
// 53F440: using guessed type _DWORD __cdecl _close(_DWORD);
// 65E750: using guessed type int savedgame_missionId;
*/

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
