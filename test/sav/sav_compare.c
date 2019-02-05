#include "../src/core/zip.h"

#include <stdio.h>
#include <string.h>

#define SAVEGAME_PARTS 300
#define COMPRESS_BUFFER_SIZE 600000
#define UNCOMPRESSED 0x80000000

struct game_file_part {
    int compressed;
    int length_in_bytes;
    char name[100];
    int record_length;
};

static struct game_file_part save_game_parts[] = {
    {0, 4, "saveGameMissionId"},
    {0, 4, "savegameFileVersion"},
    {1, 52488, "Data_Grid_graphicIds", 2},
    {1, 26244, "Data_Grid_edge"},
    {1, 52488, "Data_Grid_buildingIds", 2},
    {1, 52488, "Data_Grid_terrain", 2},
    {1, 26244, "Data_Grid_aqueducts"},
    {1, 52488, "Data_Grid_walkerIds", 2},
    {1, 26244, "Data_Grid_bitfields"},
    {1, 26244, "Data_Grid_spriteOffsets"},
    {0, 26244, "Data_Grid_random"},
    {1, 26244, "Data_Grid_desirability"},
    {1, 26244, "Data_Grid_elevation"},
    {1, 26244, "Data_Grid_buildingDamage"},
    {1, 26244, "Data_Grid_Undo_aqueducts"},
    {1, 26244, "Data_Grid_Undo_spriteOffsets"},
    {1, 128000, "Data_Walkers", 128},
    {1, 1200, "Data_Routes.walkerIds"},
    {1, 300000, "Data_Routes.directionPaths", 500},
    {1, 6400, "Data_Formations", 128},
    {0, 4, "Data_Formation_Extra.idLastInUse"},
    {0, 4, "Data_Formation_Extra.idLastLegion"},
    {0, 4, "Data_Formation_Extra.numLegions"},
    {1, 36136, "Data_CityInfo", 17908},
    {0, 2, "Data_CityInfo_Extra.unknownBytes"},
    {0, 64, "playerNames"},
    {0, 4, "Data_CityInfo_Extra.ciid"},
    {1, 256000, "Data_Buildings", 128},
    {0, 4, "Data_Settings_Map.orientation"},
    {0, 4, "Data_CityInfo_Extra.gameTimeTick"},
    {0, 4, "Data_CityInfo_Extra.gameTimeDay"},
    {0, 4, "Data_CityInfo_Extra.gameTimeMonth"},
    {0, 4, "Data_CityInfo_Extra.gameTimeYear"},
    {0, 4, "Data_CityInfo_Extra.gameTimeTotalDays"},
    {0, 4, "Data_Buildings_Extra.highestBuildingIdEver"},
    {0, 4, "Data_Debug.maxConnectsEver"},
    {0, 4, "Data_Random.iv1"},
    {0, 4, "Data_Random.iv2"},
    {0, 4, "Data_Settings_Map.camera.x"},
    {0, 4, "Data_Settings_Map.camera.y"},
    {0, 4, "Data_CityInfo_Buildings.theater.total"},
    {0, 4, "Data_CityInfo_Buildings.theater.working"},
    {0, 4, "Data_CityInfo_Buildings.amphitheater.total"},
    {0, 4, "Data_CityInfo_Buildings.amphitheater.working"},
    {0, 4, "Data_CityInfo_Buildings.colosseum.total"},
    {0, 4, "Data_CityInfo_Buildings.colosseum.working"},
    {0, 4, "Data_CityInfo_Buildings.hippodrome.total"},
    {0, 4, "Data_CityInfo_Buildings.hippodrome.working"},
    {0, 4, "Data_CityInfo_Buildings.school.total"},
    {0, 4, "Data_CityInfo_Buildings.school.working"},
    {0, 4, "Data_CityInfo_Buildings.library.total"},
    {0, 4, "Data_CityInfo_Buildings.library.working"},
    {0, 4, "Data_CityInfo_Buildings.academy.total"},
    {0, 4, "Data_CityInfo_Buildings.academy.working"},
    {0, 4, "Data_CityInfo_Buildings.barber.total"},
    {0, 4, "Data_CityInfo_Buildings.barber.working"},
    {0, 4, "Data_CityInfo_Buildings.bathhouse.total"},
    {0, 4, "Data_CityInfo_Buildings.bathhouse.working"},
    {0, 4, "Data_CityInfo_Buildings.clinic.total"},
    {0, 4, "Data_CityInfo_Buildings.clinic.working"},
    {0, 4, "Data_CityInfo_Buildings.hospital.total"},
    {0, 4, "Data_CityInfo_Buildings.hospital.working"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleCeres.total"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleNeptune.total"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleMercury.total"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleMars.total"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleVenus.total"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleCeres.total"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleNeptune.total"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleMercury.total"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleMars.total"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleVenus.total"},
    {0, 4, "Data_CityInfo_Buildings.oracle.total"},
    {0, 4, "Data_CityInfo_Extra.populationGraphOrder"},
    {0, 4, "Data_CityInfo_Extra.unknownOrder"},
    {0, 4, "Data_Event.emperorChange.gameYear"},
    {0, 4, "Data_Event.emperorChange.month"},
    {0, 4, "Data_Empire.scrollX"},
    {0, 4, "Data_Empire.scrollY"},
    {0, 4, "Data_Empire.selectedObject"},
    {1, 2706, "Data_Empire_Cities"},
    {0, 64, "Data_CityInfo_Buildings.industry.total"},
    {0, 64, "Data_CityInfo_Buildings.industry.working"},
    {0, 128, "Data_TradePrices"},
    {0, 4, "Data_Walker_NameSequence.citizenMale"},
    {0, 4, "Data_Walker_NameSequence.patrician"},
    {0, 4, "Data_Walker_NameSequence.citizenFemale"},
    {0, 4, "Data_Walker_NameSequence.taxCollector"},
    {0, 4, "Data_Walker_NameSequence.engineer"},
    {0, 4, "Data_Walker_NameSequence.prefect"},
    {0, 4, "Data_Walker_NameSequence.javelinThrower"},
    {0, 4, "Data_Walker_NameSequence.cavalry"},
    {0, 4, "Data_Walker_NameSequence.legionary"},
    {0, 4, "Data_Walker_NameSequence.actor"},
    {0, 4, "Data_Walker_NameSequence.gladiator"},
    {0, 4, "Data_Walker_NameSequence.lionTamer"},
    {0, 4, "Data_Walker_NameSequence.charioteer"},
    {0, 4, "Data_Walker_NameSequence.barbarian"},
    {0, 4, "Data_Walker_NameSequence.enemyGreek"},
    {0, 4, "Data_Walker_NameSequence.enemyEgyptian"},
    {0, 4, "Data_Walker_NameSequence.enemyArabian"},
    {0, 4, "Data_Walker_NameSequence.trader"},
    {0, 4, "Data_Walker_NameSequence.tradeShip"},
    {0, 4, "Data_Walker_NameSequence.warShip"},
    {0, 4, "Data_Walker_NameSequence.enemyShip"},
    {0, 4, "Data_CityInfo_CultureCoverage.theater"},
    {0, 4, "Data_CityInfo_CultureCoverage.amphitheater"},
    {0, 4, "Data_CityInfo_CultureCoverage.colosseum"},
    {0, 4, "Data_CityInfo_CultureCoverage.hospital"},
    {0, 4, "Data_CityInfo_CultureCoverage.hippodrome"},
    {0, 4, "Data_CityInfo_CultureCoverage.religionCeres"},
    {0, 4, "Data_CityInfo_CultureCoverage.religionNeptune"},
    {0, 4, "Data_CityInfo_CultureCoverage.religionMercury"},
    {0, 4, "Data_CityInfo_CultureCoverage.religionMars"},
    {0, 4, "Data_CityInfo_CultureCoverage.religionVenus"},
    {0, 4, "Data_CityInfo_CultureCoverage.oracle"},
    {0, 4, "Data_CityInfo_CultureCoverage.school"},
    {0, 4, "Data_CityInfo_CultureCoverage.library"},
    {0, 4, "Data_CityInfo_CultureCoverage.academy"},
    {0, 4, "Data_CityInfo_CultureCoverage.hospital"},
    {0, 1720, "Data_Scenario"},
    {0, 4, "Data_Event.timeLimitMaxGameYear"},
    {0, 4, "Data_Event.earthquake.gameYear"},
    {0, 4, "Data_Event.earthquake.month"},
    {0, 4, "Data_Event.earthquake.state"},
    {0, 4, "Data_Event.earthquake.duration"},
    {0, 4, "Data_Event.earthquake.maxDuration"},
    {0, 4, "Data_Event.earthquake.maxDelay"},
    {0, 4, "Data_Event.earthquake.delay"},
    {0, 32, "Data_Event.earthquake.expand"},
    {0, 4, "Data_Event.emperorChange.state"},
    {1, 16000, "Data_Message.messages"},
    {0, 4, "Data_Message.nextMessageSequence"},
    {0, 4, "Data_Message.totalMessages"},
    {0, 4, "Data_Message.currentMessageId"},
    {0, 10, "Data_Message.populationMessagesShown"},
    {0, 80, "Data_Message.messageCategoryCount"},
    {0, 80, "Data_Message.messageDelay"},
    {0, 4, "Data_BuildingList.burning.totalBurning"},
    {0, 4, "Data_BuildingList.burning.index"},
    {0, 4, "Data_Walker_Extra.createdSequence"},
    {0, 4, "Data_Settings.startingFavor"},
    {0, 4, "Data_Settings.personalSavingsLastMission"},
    {0, 4, "Data_Settings.currentMissionId"},
    {1, 3232, "Data_InvasionWarnings"},
    {0, 4, "Data_Settings.isCustomScenario"},
    {0, 8960, "Data_Sound_City", 128},
    {0, 4, "Data_Buildings_Extra.highestBuildingIdInUse"},
    {0, 4800, "Data_Walker_Traders"},
    {0, 4, "Data_Walker_Extra.nextTraderId"},
    {1, 1000, "Data_BuildingList.burning.items"},
    {1, 1000, "Data_BuildingList.small.items"},
    {1, 4000, "Data_BuildingList.large.items"},
    {0, 4, "Data_Tutorial.tutorial1.fire"},
    {0, 4, "Data_Tutorial.tutorial1.crime"},
    {0, 4, "Data_Tutorial.tutorial1.collapse"},
    {0, 4, "Data_Tutorial.tutorial2.granaryBuilt"},
    {0, 4, "Data_Tutorial.tutorial2.population250Reached"},
    {0, 4, "Data_Tutorial.tutorial1.senateBuilt"},
    {0, 4, "Data_Tutorial.tutorial2.population450Reached"},
    {0, 4, "Data_Tutorial.tutorial2.potteryMade"},
    {0, 4, "Data_CityInfo_Buildings.militaryAcademy.total"},
    {0, 4, "Data_CityInfo_Buildings.militaryAcademy.working"},
    {0, 4, "Data_CityInfo_Buildings.barracks.total"},
    {0, 4, "Data_CityInfo_Buildings.barracks.working"},
    {0, 4, "Data_Formation_Extra.numEnemyFormations"},
    {0, 4, "Data_Formation_Extra.numEnemySoldierStrength"},
    {0, 4, "Data_Formation_Extra.numLegionFormations"},
    {0, 4, "Data_Formation_Extra.numLegionSoldierStrength"},
    {0, 4, "Data_Formation_Extra.daysSinceRomanSoldierConcentration"},
    {0, 6400, "Data_Building_Storages"},
    {0, 4, "Data_CityInfo_Buildings.actorColony.total"},
    {0, 4, "Data_CityInfo_Buildings.actorColony.working"},
    {0, 4, "Data_CityInfo_Buildings.gladiatorSchool.total"},
    {0, 4, "Data_CityInfo_Buildings.gladiatorSchool.working"},
    {0, 4, "Data_CityInfo_Buildings.lionHouse.total"},
    {0, 4, "Data_CityInfo_Buildings.lionHouse.working"},
    {0, 4, "Data_CityInfo_Buildings.chariotMaker.total"},
    {0, 4, "Data_CityInfo_Buildings.chariotMaker.working"},
    {0, 4, "Data_CityInfo_Buildings.market.total"},
    {0, 4, "Data_CityInfo_Buildings.market.working"},
    {0, 4, "Data_CityInfo_Buildings.reservoir.total"},
    {0, 4, "Data_CityInfo_Buildings.reservoir.working"},
    {0, 4, "Data_CityInfo_Buildings.fountain.total"},
    {0, 4, "Data_CityInfo_Buildings.fountain.working"},
    {0, 4, "Data_Tutorial.tutorial2.potteryMadeYear"},
    {0, 4, "Data_Event.gladiatorRevolt.gameYear"},
    {0, 4, "Data_Event.gladiatorRevolt.month"},
    {0, 4, "Data_Event.gladiatorRevolt.endMonth"},
    {0, 4, "Data_Event.gladiatorRevolt.state"},
    {1, 1280, "Data_Empire_Trade.maxPerYear"},
    {1, 1280, "Data_Empire_Trade.tradedThisYear"},
    {0, 4, "Data_Buildings_Extra.barracksTowerSentryRequested"},
    {0, 4, "Data_Buildings_Extra.createdSequence"},
    {0, 4, "Data_Routes.unknown1RoutesCalculated"},
    {0, 4, "Data_Routes.enemyRoutesCalculated"},
    {0, 4, "Data_Routes.totalRoutesCalculated"},
    {0, 4, "Data_Routes.unknown1RoutesCalculated"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleCeres.working"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleNeptune.working"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleMercury.working"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleMars.working"},
    {0, 4, "Data_CityInfo_Buildings.smallTempleVenus.working"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleCeres.working"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleNeptune.working"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleMercury.working"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleMars.working"},
    {0, 4, "Data_CityInfo_Buildings.largeTempleVenus.working"},
    {0, 100, "Data_Formation_Invasion.formationId"},
    {0, 100, "Data_Formation_Invasion.homeX"},
    {0, 100, "Data_Formation_Invasion.homeY"},
    {0, 100, "Data_Formation_Invasion.layout"},
    {0, 100, "Data_Formation_Invasion.destinationX"},
    {0, 100, "Data_Formation_Invasion.destinationY"},
    {0, 100, "Data_Formation_Invasion.destinationBuildingId"},
    {0, 100, "Data_Formation_Invasion.numLegions"},
    {0, 100, "Data_Formation_Invasion.ignoreRomanSoldiers"},
    {0, 4, "Data_CityInfo_Extra.entryPointFlag.x"},
    {0, 4, "Data_CityInfo_Extra.entryPointFlag.y"},
    {0, 4, "Data_CityInfo_Extra.exitPointFlag.x"},
    {0, 4, "Data_CityInfo_Extra.exitPointFlag.y"},
    {0, 2, "Data_Event.lastInternalInvasionId"},
    {0, 4, "Data_Debug.incorrectHousePositions"},
    {0, 4, "Data_Debug.unfixableHousePositions"},
    {0, 65, "Data_FileList.selectedScenario"},
    {0, 32, "Data_CityInfo_Extra.bookmarks"},
    {0, 4, "Data_Tutorial.tutorial3.disease"},
    {0, 4, "Data_CityInfo_Extra.entryPointFlag.gridOffset"},
    {0, 4, "Data_CityInfo_Extra.exitPointFlag.gridOffset"},
    {0, 0, ""},
};

static char compress_buffer[COMPRESS_BUFFER_SIZE];
static unsigned char file1_data[1300000];
static unsigned char file2_data[1300000];

static unsigned int to_uint(const unsigned char *buffer)
{
    return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

static unsigned int to_ushort(const unsigned char *buffer)
{
    return buffer[0] | (buffer[1] << 8);
}

static int index_of_part(const char *part_name)
{
    for (int i = 0; save_game_parts[i].length_in_bytes; i++) {
        if (strcmp(part_name, save_game_parts[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

static int read_compressed_chunk(FILE *fp, void *buffer, int bytes_to_read)
{
    if (bytes_to_read > COMPRESS_BUFFER_SIZE) {
        return 0;
    }
    unsigned int input_size = bytes_to_read;
    unsigned char intbuf[4];
    if (fread(&intbuf, 1, 4, fp) == 4) {
        input_size = to_uint(intbuf);
    }
    if (input_size == UNCOMPRESSED) {
        fread(buffer, 1, bytes_to_read, fp);
    } else {
        fread(compress_buffer, 1, input_size, fp);
        if (!zip_decompress(compress_buffer, input_size, buffer, &bytes_to_read)) {
            return 0;
        }
    }
    return 1;
}

static int unpack(const char *filename, unsigned char *buffer)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Unable to open file %s\n", filename);
        return 0;
    }
    int offset = 0;
    for (int i = 0; save_game_parts[i].length_in_bytes; i++) {
        if (save_game_parts[i].compressed) {
            read_compressed_chunk(fp, &buffer[offset], save_game_parts[i].length_in_bytes);
        } else {
            fread(&buffer[offset], 1, save_game_parts[i].length_in_bytes, fp);
        }
        offset += save_game_parts[i].length_in_bytes;
    }
    fclose(fp);
    return offset;
}

static int is_between(unsigned int value, unsigned int range_from, unsigned int range_to)
{
    return value >= range_from && value <= range_to;
}

static int both_between(unsigned int value1, unsigned int value2, unsigned int range_from, unsigned int range_to)
{
    return is_between(value1, range_from, range_to) && is_between(value2, range_from, range_to);
}

static int is_exception_cityinfo(int global_offset, int part_offset)
{
    if (part_offset == 35160) {
        // Bug fixed compared to C3: caesar invasion and barbarian invasion
        // influence on peace rating are switched
        if (file1_data[global_offset] == 7 && file2_data[global_offset] == 8) {
            return 1;
        }
        if (file1_data[global_offset] == 8 && file2_data[global_offset] == 7) {
            return 1;
        }
    }
    return 0;
}

static int is_exception_image_grid(int global_offset)
{
    unsigned int v1 = to_ushort(&file1_data[global_offset & ~1]);
    unsigned int v2 = to_ushort(&file2_data[global_offset & ~1]);
    // water: depends on animation timer
    if (both_between(v1, v2, 364, 369)) {
        return 1;
    }
    // burning tent: fix in julius to use its own graphic
    if ((v1 == 734 && is_between(v2, 743, 770)) || (v2 == 734 && is_between(v1, 743, 770))) {
        return 1;
    }
    return 0;
}

static int is_exception_buildings(int global_offset, int part_offset)
{
    int building_offset = (global_offset - part_offset) + 128 * (part_offset / 128);
    int difference_offset = global_offset - building_offset;
    int type = to_ushort(&file1_data[building_offset + 10]);
    if (type == 99 && is_between(difference_offset, 0x4A, 0x73)) { // burning ruin extra data
        return 1;
    }
    return 0;
}

static int is_exception(int index, int global_offset, int part_offset)
{
    if (index == 2) { // Data_Grid_graphicIds
        return is_exception_image_grid(global_offset);
    }
    if (index == 9) { // sprite offsets
        // don't care about sprite + building = animation
        int building_offset = global_offset - 8 * 162 * 162 + part_offset;
        if (file1_data[building_offset] || file1_data[building_offset + 1]) {
            return 1;
        }
    }
    if (index == index_of_part("Data_Grid_Undo_spriteOffsets")) {
        return 1;
    }
    if (index == index_of_part("Data_Settings_Map.camera.x") || index == index_of_part("Data_Settings_Map.camera.y")) {
        return 1;
    }
    if (index == index_of_part("Data_CityInfo")) {
        return is_exception_cityinfo(global_offset, part_offset);
    }
    if (index == index_of_part("Data_Buildings")) {
        return is_exception_buildings(global_offset, part_offset);
    }
    if (index == index_of_part("Data_BuildingList.burning.index")) {
        // We use it for burning size in Julius, while C3 writes the index used to loop over the buildings,
        // which is either 0 (no prefects in the city) or the burning size
        // So: we ignore this variable if one of them is zero
        if (to_uint(&file1_data[global_offset - part_offset]) == 0 || to_uint(&file2_data[global_offset - part_offset]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int compare_part(int index, int offset)
{
    int different = 0;
    int length = save_game_parts[index].length_in_bytes;
    for (int i = 0; i < length; i++) {
        if (file1_data[offset + i] != file2_data[offset + i] && !is_exception(index, offset + i, i)) {
            different = 1;
            printf("Part %d [%s] (%d) ", index, save_game_parts[index].name, i);
            if (save_game_parts[index].record_length) {
                printf("record %d offset 0x%X", i / save_game_parts[index].record_length, i % save_game_parts[index].record_length);
                if (index == index_of_part("Data_Buildings")) {
                    int record_length = save_game_parts[index].record_length;
                    int type_offset = (i / record_length) * record_length + 10;
                    printf(" (type: %d)", to_ushort(&file1_data[offset + type_offset]));
                }
            } else {
                printf("offset %d", i);
            }
            printf(": %d <-> %d\n", file1_data[offset + i], file2_data[offset + i]);
        }
    }
    return different;
}

static void print_game_time(unsigned char *data)
{
    int offset_tick = 1200222;
    unsigned int tick = to_uint(&data[offset_tick]);
    unsigned int day = to_uint(&data[offset_tick + 4]);
    unsigned int month = to_uint(&data[offset_tick + 8]);
    int year = (int) to_uint(&data[offset_tick + 12]);
    unsigned int total_days = to_uint(&data[offset_tick + 16]);

    printf("%d.%u.%u.%u (%u)\n", year, month, day, tick, total_days);
}

static void compare_game_time(void)
{
    int offset_tick = 1200222;
    int offset_days = offset_tick + 16;
    unsigned int ticks1 = to_uint(&file1_data[offset_tick]) + 50 * to_uint(&file1_data[offset_days]);
    unsigned int ticks2 = to_uint(&file2_data[offset_tick]) + 50 * to_uint(&file2_data[offset_days]);
    if (ticks1 != ticks2) {
        printf("WARN: ticks not in sync: %u <--> %u (%d)\n", ticks1, ticks2, ticks1 - ticks2);
        printf("File 1: ");
        print_game_time(file1_data);
        printf("File 2: ");
        print_game_time(file2_data);
    }
}

static int compare(void)
{
    compare_game_time();
    int offset = 0;
    int different = 0;
    for (int i = 0; save_game_parts[i].length_in_bytes; i++) {
        if (i != 146) { // skip city sounds
            different |= compare_part(i, offset);
        }
        offset += save_game_parts[i].length_in_bytes;
    }
    return different;
}

int compare_files(const char *file1, const char *file2)
{
    int length1 = unpack(file1, file1_data);
    int length2 = unpack(file2, file2_data);
    if (length1 && length1 == length2) {
        return compare();
    } else {
        printf("WARN: lengths are different: %d <--> %d\n", length1, length2);
        return 1;
    }
}
