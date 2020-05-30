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
    {0, 4, "scenario_campaign_mission"},
    {0, 4, "file_version"},
    {1, 52488, "image_grid", 2},
    {1, 26244, "edge_grid"},
    {1, 52488, "building_grid", 2},
    {1, 52488, "terrain_grid", 2},
    {1, 26244, "aqueduct_grid"},
    {1, 52488, "figure_grid", 2},
    {1, 26244, "bitfields_grid"},
    {1, 26244, "sprite_grid"},
    {0, 26244, "random_grid"},
    {1, 26244, "desirability_grid"},
    {1, 26244, "elevation_grid"},
    {1, 26244, "building_damage_grid"},
    {1, 26244, "aqueduct_backup_grid"},
    {1, 26244, "sprite_backup_grid"},
    {1, 128000, "figures", 128},
    {1, 1200, "route_figures", 2},
    {1, 300000, "route_paths", 500},
    {1, 6400, "formations", 128},
    {0, 12, "formation_totals"},
    {1, 36136, "city_data", 17908},
    {0, 2, "city_faction_unknown"},
    {0, 64, "player_name"},
    {0, 4, "city_faction"},
    {1, 256000, "buildings", 128},
    {0, 4, "city_view_orientation"},
    {0, 4, "game_time.tick"},
    {0, 4, "game_time.day"},
    {0, 4, "game_time.month"},
    {0, 4, "game_time.year"},
    {0, 4, "game_time.total_days"},
    {0, 8, "building_extra_highest_id_ever"},
    {0, 8, "random_iv"},
    {0, 8, "camera"},
    {0, 4, "building_count.theater.total"},
    {0, 4, "building_count.theater.working"},
    {0, 4, "building_count.amphitheater.total"},
    {0, 4, "building_count.amphitheater.working"},
    {0, 4, "building_count.colosseum.total"},
    {0, 4, "building_count.colosseum.working"},
    {0, 4, "building_count.hippodrome.total"},
    {0, 4, "building_count.hippodrome.working"},
    {0, 4, "building_count.school.total"},
    {0, 4, "building_count.school.working"},
    {0, 4, "building_count.library.total"},
    {0, 4, "building_count.library.working"},
    {0, 4, "building_count.academy.total"},
    {0, 4, "building_count.academy.working"},
    {0, 4, "building_count.barber.total"},
    {0, 4, "building_count.barber.working"},
    {0, 4, "building_count.bathhouse.total"},
    {0, 4, "building_count.bathhouse.working"},
    {0, 4, "building_count.clinic.total"},
    {0, 4, "building_count.clinic.working"},
    {0, 4, "building_count.hospital.total"},
    {0, 4, "building_count.hospital.working"},
    {0, 4, "building_count.smallTempleCeres.total"},
    {0, 4, "building_count.smallTempleNeptune.total"},
    {0, 4, "building_count.smallTempleMercury.total"},
    {0, 4, "building_count.smallTempleMars.total"},
    {0, 4, "building_count.smallTempleVenus.total"},
    {0, 4, "building_count.largeTempleCeres.total"},
    {0, 4, "building_count.largeTempleNeptune.total"},
    {0, 4, "building_count.largeTempleMercury.total"},
    {0, 4, "building_count.largeTempleMars.total"},
    {0, 4, "building_count.largeTempleVenus.total"},
    {0, 4, "building_count.oracle.total"},
    {0, 8, "city_graph_order"},
    {0, 8, "emperor_change_time"},
    {0, 4, "empire.scroll_x"},
    {0, 4, "empire.scroll_y"},
    {0, 4, "empire.selected_object"},
    {1, 2706, "empire_cities"},
    {0, 64, "building_count_industry.total"},
    {0, 64, "building_count_industry.active"},
    {0, 128, "trade_prices"},
    {0, 4, "figure_names.citizen_male"},
    {0, 4, "figure_names.patrician"},
    {0, 4, "figure_names.citizen_female"},
    {0, 4, "figure_names.tax_collector"},
    {0, 4, "figure_names.engineer"},
    {0, 4, "figure_names.prefect"},
    {0, 4, "figure_names.javelin_thrower"},
    {0, 4, "figure_names.cavalry"},
    {0, 4, "figure_names.legionary"},
    {0, 4, "figure_names.actor"},
    {0, 4, "figure_names.gladiator"},
    {0, 4, "figure_names.lion_tamer"},
    {0, 4, "figure_names.charioteer"},
    {0, 4, "figure_names.barbarian"},
    {0, 4, "figure_names.enemy_greek"},
    {0, 4, "figure_names.enemy_egyptian"},
    {0, 4, "figure_names.enemy_arabian"},
    {0, 4, "figure_names.trader"},
    {0, 4, "figure_names.ship"},
    {0, 4, "figure_names.warship"},
    {0, 4, "figure_names.enemy_warship"},
    {0, 4, "culture_coverage.theater"},
    {0, 4, "culture_coverage.amphitheater"},
    {0, 4, "culture_coverage.colosseum"},
    {0, 4, "culture_coverage.hospital"},
    {0, 4, "culture_coverage.hippodrome"},
    {0, 4, "culture_coverage.religion_ceres"},
    {0, 4, "culture_coverage.religion_neptune"},
    {0, 4, "culture_coverage.religion_mercury"},
    {0, 4, "culture_coverage.religion_mars"},
    {0, 4, "culture_coverage.religion_venus"},
    {0, 4, "culture_coverage.oracle"},
    {0, 4, "culture_coverage.school"},
    {0, 4, "culture_coverage.library"},
    {0, 4, "culture_coverage.academy"},
    {0, 4, "culture_coverage.hospital"},
    {0, 1720, "scenario"},
    {0, 4, "max_game_year"},
    {0, 4, "earthquake.game_year"},
    {0, 4, "earthquake.month"},
    {0, 4, "earthquake.state"},
    {0, 4, "earthquake.duration"},
    {0, 4, "earthquake.max_duration"},
    {0, 4, "earthquake.max_delay"},
    {0, 4, "earthquake.delay"},
    {0, 32, "earthquake.expand"},
    {0, 4, "emperor_change_state"},
    {1, 16000, "messages"},
    {0, 4, "message_extra.next_message_sequence"},
    {0, 4, "message_extra.total_messages"},
    {0, 4, "message_extra.current_message_id"},
    {0, 10, "population_messages"},
    {0, 80, "message_counts"},
    {0, 80, "message_delays"},
    {0, 4, "building_list_burning_totals.total"},
    {0, 4, "building_list_burning_totals.size"},
    {0, 4, "figure_sequence"},
    {0, 12, "scenario_settings"},
    {1, 3232, "invasion_warnings"},
    {0, 4, "scenario_is_custom"},
    {0, 8960, "city_sounds", 128},
    {0, 4, "building_extra_highest_id"},
    {0, 4800, "figure_traders"},
    {0, 4, "figure_traders.next_trader_id"},
    {1, 1000, "building_list_burning"},
    {1, 1000, "building_list_small"},
    {1, 4000, "building_list_large"},
    {0, 4, "tutorial_part1.tutorial1.fire"},
    {0, 4, "tutorial_part1.tutorial1.crime"},
    {0, 4, "tutorial_part1.tutorial1.collapse"},
    {0, 4, "tutorial_part1.tutorial2.granaryBuilt"},
    {0, 4, "tutorial_part1.tutorial2.population250Reached"},
    {0, 4, "tutorial_part1.tutorial1.senateBuilt"},
    {0, 4, "tutorial_part1.tutorial2.population450Reached"},
    {0, 4, "tutorial_part1.tutorial2.potteryMade"},
    {0, 4, "building_count.militaryAcademy.total"},
    {0, 4, "building_count.militaryAcademy.working"},
    {0, 4, "building_count.barracks.total"},
    {0, 4, "building_count.barracks.working"},
    {0, 4, "enemy_army_totals.enemy_formations"},
    {0, 4, "enemy_army_totals.enemy_strength"},
    {0, 4, "enemy_army_totals.legion_formations"},
    {0, 4, "enemy_army_totals.legion_strength"},
    {0, 4, "enemy_army_totals.days_since_roman_influence_calculation"},
    {0, 6400, "building_storages"},
    {0, 4, "building_count.actorColony.total"},
    {0, 4, "building_count.actorColony.working"},
    {0, 4, "building_count.gladiatorSchool.total"},
    {0, 4, "building_count.gladiatorSchool.working"},
    {0, 4, "building_count.lionHouse.total"},
    {0, 4, "building_count.lionHouse.working"},
    {0, 4, "building_count.chariotMaker.total"},
    {0, 4, "building_count.chariotMaker.working"},
    {0, 4, "building_count.market.total"},
    {0, 4, "building_count.market.working"},
    {0, 4, "building_count.reservoir.total"},
    {0, 4, "building_count.reservoir.working"},
    {0, 4, "building_count.fountain.total"},
    {0, 4, "building_count.fountain.working"},
    {0, 4, "tutorial_part2"},
    {0, 4, "gladiator_revolt.gameYear"},
    {0, 4, "gladiator_revolt.month"},
    {0, 4, "gladiator_revolt.endMonth"},
    {0, 4, "gladiator_revolt.state"},
    {1, 1280, "trade_route_limit"},
    {1, 1280, "trade_route_traded"},
    {0, 4, "building_barracks_tower_sentry"},
    {0, 4, "building_extra_sequence"},
    {0, 4, "routing_counters.unknown"},
    {0, 4, "routing_counters.enemy_routes_calculated"},
    {0, 4, "routing_counters.total_routes_calculated"},
    {0, 4, "routing_counters.unknown"},
    {0, 4, "building_count.smallTempleCeres.working"},
    {0, 4, "building_count.smallTempleNeptune.working"},
    {0, 4, "building_count.smallTempleMercury.working"},
    {0, 4, "building_count.smallTempleMars.working"},
    {0, 4, "building_count.smallTempleVenus.working"},
    {0, 4, "building_count.largeTempleCeres.working"},
    {0, 4, "building_count.largeTempleNeptune.working"},
    {0, 4, "building_count.largeTempleMercury.working"},
    {0, 4, "building_count.largeTempleMars.working"},
    {0, 4, "building_count.largeTempleVenus.working"},
    {0, 100, "enemy_armies.formation_id"},
    {0, 100, "enemy_armies.home_x"},
    {0, 100, "enemy_armies.home_y"},
    {0, 100, "enemy_armies.layout"},
    {0, 100, "enemy_armies.destination_x"},
    {0, 100, "enemy_armies.destination_y"},
    {0, 100, "enemy_armies.destination_building_id"},
    {0, 100, "enemy_armies.num_legions"},
    {0, 100, "enemy_armies.ignore_roman_soldiers"},
    {0, 16, "city_entry_exit_xy"},
    {0, 2, "last_invasion_id"},
    {0, 8, "building_extra_corrupt_houses"},
    {0, 65, "scenario_name"},
    {0, 32, "bookmarks"},
    {0, 4, "tutorial_part3"},
    {0, 8, "city_entry_exit_grid_offset"},
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
    printf("WARN: part %s not found\n", part_name);
    return -1;
}

static int offset_of_part(const char *part_name)
{
    int total_offset = 0;
    for (int i = 0; save_game_parts[i].length_in_bytes; i++) {
        if (strcmp(part_name, save_game_parts[i].name) == 0) {
            return total_offset;
        }
        total_offset += save_game_parts[i].length_in_bytes;
    }
    printf("WARN: part %s not found\n", part_name);
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
        if (fread(buffer, 1, bytes_to_read, fp) != bytes_to_read) {
            return 0;
        }
    } else {
        if (fread(compress_buffer, 1, input_size, fp) != input_size || !zip_decompress(compress_buffer, input_size, buffer, &bytes_to_read)) {
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
        int result = 0;
        if (save_game_parts[i].compressed) {
            result = read_compressed_chunk(fp, &buffer[offset], save_game_parts[i].length_in_bytes);
        } else {
            result = fread(&buffer[offset], 1, save_game_parts[i].length_in_bytes, fp) == save_game_parts[i].length_in_bytes;
        }
        offset += save_game_parts[i].length_in_bytes;
        if (!result) {
            printf("Error while loading file %s\n", filename);
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return offset;
}

static int has_adjacent_building_type(int part_offset, int building_type)
{
    int grid_offset = part_offset / 2;
    const int adjacent_tiles[] = { -162, 1, 162, -1 };

    for (int i = 0; i < 4; ++i) {
        int adjacent_offset = grid_offset + adjacent_tiles[i];
        if (adjacent_offset < 0 || adjacent_offset >= 162 * 162) {
            continue;
        }
        int building_id = to_ushort(&file1_data[offset_of_part("building_grid") + adjacent_offset * 2]);
        int building_offset = offset_of_part("buildings") + building_id * 128;
        int type = to_ushort(&file1_data[building_offset + 10]);
        if (type == building_type) {
            return 1;
        }
    }
    return 0;
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

static int is_exception_image_grid(int global_offset, int part_offset)
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
    // Exception for roads next to a granary: in julius the dirt roads and paved roads lead
    // into the granary, while in Caesar 3 they do not. Therefore we do not check roads that
    // are adjacent to a granary (building type 71).
    if (both_between(v1, v2, 591, 657) && has_adjacent_building_type(part_offset, 71)) {
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

static int is_exception_building_grid(int global_offset, int part_offset)
{
    int grid_offset = part_offset / 2;
    // Exception for earthquake tiles: Caesar 3 does not clear the building ID when
    // a building gets destroyed by an earthquake, resulting in visual artifacts;
    // Julius does clear the building ID from the grid.
    // Earthquake tile is defined as:
    // - 0x80 bit is set in bitfields_grid
    // - 0x0002 bit is set in terrain_grid
    int is_earthquake = (file1_data[offset_of_part("bitfields_grid") + grid_offset] & 0x80) &&
            (file1_data[offset_of_part("terrain_grid") + 2 * grid_offset] & 0x02);
    if (is_earthquake) {
        unsigned int v1 = to_ushort(&file1_data[global_offset & ~1]);
        unsigned int v2 = to_ushort(&file2_data[global_offset & ~1]);
        if (v1 == 0 || v2 == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_exception(int index, int global_offset, int part_offset)
{
    if (index == index_of_part("city_sounds")) {
        return 1;
    }
    if (index == index_of_part("sprite_backup_grid")) {
        return 1;
    }
    if (index == index_of_part("camera")) {
        return 1;
    }
    if (index == index_of_part("image_grid")) {
        return is_exception_image_grid(global_offset, part_offset);
    }
    if (index == index_of_part("sprite_grid")) {
        // don't care about sprite + building = animation
        int building_offset = global_offset - 8 * 162 * 162 + part_offset;
        if (file1_data[building_offset] || file1_data[building_offset + 1]) {
            return 1;
        }
    }
    if (index == index_of_part("city_data")) {
        return is_exception_cityinfo(global_offset, part_offset);
    }
    if (index == index_of_part("buildings")) {
        return is_exception_buildings(global_offset, part_offset);
    }
    if (index == index_of_part("building_grid")) {
        return is_exception_building_grid(global_offset, part_offset);
    }
    if (index == index_of_part("building_list_burning_totals.size")) {
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
                if (index == index_of_part("buildings")) {
                    int record_length = save_game_parts[index].record_length;
                    int type_offset = (i / record_length) * record_length + 10;
                    printf(" (type: %d)", to_ushort(&file1_data[offset + type_offset]));
                } else if (index == index_of_part("figures")) {
                    int record_length = save_game_parts[index].record_length;
                    int type_offset = (i / record_length) * record_length + 10;
                    printf(" (type: %d)", file1_data[offset + type_offset]);
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
        different |= compare_part(i, offset);
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
