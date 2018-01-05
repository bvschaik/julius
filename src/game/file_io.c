#include "file_io.h"

#include "Data/CityInfo.h"

#include "building/barracks.h"
#include "building/count.h"
#include "building/list.h"
#include "building/storage.h"
#include "city/culture.h"
#include "city/message.h"
#include "city/view.h"
#include "core/random.h"
#include "core/zip.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "map/aqueduct.h"
#include "map/bookmark.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/routing.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "scenario/criteria.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/scenario.h"
#include "sound/city.h"

#include <stdio.h>
#include <stdlib.h>

#define COMPRESS_BUFFER_SIZE 600000
#define UNCOMPRESSED 0x80000000

static const int SAVE_GAME_VERSION = 0x66;

static char compress_buffer[COMPRESS_BUFFER_SIZE];

static int savegame_version;

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

static struct {
    int num_pieces;
    file_piece pieces[9];
    scenario_state state;
} scenario_data = {0};

typedef struct {
    buffer *scenario_campaign_mission;
    buffer *savegameFileVersion;
    buffer *image_grid;
    buffer *edge_grid;
    buffer *building_grid;
    buffer *terrain_grid;
    buffer *aqueduct_grid;
    buffer *figure_grid;
    buffer *bitfields_grid;
    buffer *sprite_grid;
    buffer *random_grid;
    buffer *desirability_grid;
    buffer *elevation_grid;
    buffer *building_damage_grid;
    buffer *aqueduct_backup_grid;
    buffer *sprite_backup_grid;
    buffer *figures;
    buffer *route_figures;
    buffer *route_paths;
    buffer *formations;
    buffer *formation_totals;
    buffer *Data_CityInfo;
    buffer *Data_CityInfo_Extra_unknownBytes;
    buffer *player_name;
    buffer *Data_CityInfo_Extra_ciid;
    buffer *buildings;
    buffer *city_view_orientation;
    buffer *game_time;
    buffer *building_extra_highest_id_ever;
    buffer *random_iv;
    buffer *city_view_camera;
    buffer *building_count_culture1;
    buffer *Data_CityInfo_Extra_populationGraphOrder;
    buffer *Data_CityInfo_Extra_unknownOrder;
    buffer *emperor_change_time;
    buffer *empire;
    buffer *empire_cities;
    buffer *building_count_industry;
    buffer *trade_prices;
    buffer *figure_names;
    buffer *culture_coverage;
    buffer *scenario;
    buffer *max_game_year;
    buffer *earthquake;
    buffer *emperor_change_state;
    buffer *messages;
    buffer *message_extra;
    buffer *population_messages;
    buffer *message_counts;
    buffer *message_delays;
    buffer *building_list_burning_totals;
    buffer *figure_sequence;
    buffer *scenario_settings;
    buffer *invasion_warnings;
    buffer *scenario_is_custom;
    buffer *city_sounds;
    buffer *building_extra_highest_id;
    buffer *figure_traders;
    buffer *building_list_burning;
    buffer *building_list_small;
    buffer *building_list_large;
    buffer *tutorial_part1;
    buffer *building_count_military;
    buffer *enemy_army_totals;
    buffer *building_storages;
    buffer *building_count_culture2;
    buffer *building_count_support;
    buffer *tutorial_part2;
    buffer *gladiator_revolt;
    buffer *trade_route_limit;
    buffer *trade_route_traded;
    buffer *building_barracks_tower_sentry;
    buffer *building_extra_sequence;
    buffer *routing_counters;
    buffer *building_count_culture3;
    buffer *enemy_armies;
    buffer *Data_CityInfo_Extra_entryPointFlag_x;
    buffer *Data_CityInfo_Extra_entryPointFlag_y;
    buffer *Data_CityInfo_Extra_exitPointFlag_x;
    buffer *Data_CityInfo_Extra_exitPointFlag_y;
    buffer *last_invasion_id;
    buffer *building_extra_corrupt_houses;
    buffer *scenario_name;
    buffer *bookmarks;
    buffer *tutorial_part3;
    buffer *Data_CityInfo_Extra_entryPointFlag_gridOffset;
    buffer *Data_CityInfo_Extra_exitPointFlag_gridOffset;
    buffer *endMarker;
} savegame_state;

static struct {
    int num_pieces;
    file_piece pieces[100];
    savegame_state state;
} savegame_data = {0};

static void init_file_piece(file_piece *piece, int size, int compressed)
{
    piece->compressed = compressed;
    void *data = malloc(size);
    buffer_init(&piece->buf, data, size);
}

static buffer *create_scenario_piece(int size)
{
    file_piece *piece = &scenario_data.pieces[scenario_data.num_pieces++];
    init_file_piece(piece, size, 0);
    return &piece->buf;
}

static buffer *create_savegame_piece(int size, int compressed)
{
    file_piece *piece = &savegame_data.pieces[savegame_data.num_pieces++];
    init_file_piece(piece, size, compressed);
    return &piece->buf;
}

static void init_scenario_data()
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

static void init_savegame_data()
{
    if (savegame_data.num_pieces > 0) {
        for (int i = 0; i < savegame_data.num_pieces; i++) {
            buffer_reset(&savegame_data.pieces[i].buf);
        }
        return;
    }
    savegame_state *state = &savegame_data.state;
    state->scenario_campaign_mission = create_savegame_piece(4, 0);
    state->savegameFileVersion = create_savegame_piece(4, 0);
    state->image_grid = create_savegame_piece(52488, 1);
    state->edge_grid = create_savegame_piece(26244, 1);
    state->building_grid = create_savegame_piece(52488, 1);
    state->terrain_grid = create_savegame_piece(52488, 1);
    state->aqueduct_grid = create_savegame_piece(26244, 1);
    state->figure_grid = create_savegame_piece(52488, 1);
    state->bitfields_grid = create_savegame_piece(26244, 1);
    state->sprite_grid = create_savegame_piece(26244, 1);
    state->random_grid = create_savegame_piece(26244, 0);
    state->desirability_grid = create_savegame_piece(26244, 1);
    state->elevation_grid = create_savegame_piece(26244, 1);
    state->building_damage_grid = create_savegame_piece(26244, 1);
    state->aqueduct_backup_grid = create_savegame_piece(26244, 1);
    state->sprite_backup_grid = create_savegame_piece(26244, 1);
    state->figures = create_savegame_piece(128000, 1);
    state->route_figures = create_savegame_piece(1200, 1);
    state->route_paths = create_savegame_piece(300000, 1);
    state->formations = create_savegame_piece(6400, 1);
    state->formation_totals = create_savegame_piece(12, 0);
    state->Data_CityInfo = create_savegame_piece(36136, 1);
    state->Data_CityInfo_Extra_unknownBytes = create_savegame_piece(2, 0);
    state->player_name = create_savegame_piece(64, 0);
    state->Data_CityInfo_Extra_ciid = create_savegame_piece(4, 0);
    state->buildings = create_savegame_piece(256000, 1);
    state->city_view_orientation = create_savegame_piece(4, 0);
    state->game_time = create_savegame_piece(20, 0);
    state->building_extra_highest_id_ever = create_savegame_piece(8, 0);
    state->random_iv = create_savegame_piece(8, 0);
    state->city_view_camera = create_savegame_piece(8, 0);
    state->building_count_culture1 = create_savegame_piece(132, 0);
    state->Data_CityInfo_Extra_populationGraphOrder = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_unknownOrder = create_savegame_piece(4, 0);
    state->emperor_change_time = create_savegame_piece(8, 0);
    state->empire = create_savegame_piece(12, 0);
    state->empire_cities = create_savegame_piece(2706, 1);
    state->building_count_industry = create_savegame_piece(128, 0);
    state->trade_prices = create_savegame_piece(128, 0);
    state->figure_names = create_savegame_piece(84, 0);
    state->culture_coverage = create_savegame_piece(60, 0);
    state->scenario = create_savegame_piece(1720, 0);
    state->max_game_year = create_savegame_piece(4, 0);
    state->earthquake = create_savegame_piece(60, 0);
    state->emperor_change_state = create_savegame_piece(4, 0);
    state->messages = create_savegame_piece(16000, 1);
    state->message_extra = create_savegame_piece(12, 0);
    state->population_messages = create_savegame_piece(10, 0);
    state->message_counts = create_savegame_piece(80, 0);
    state->message_delays = create_savegame_piece(80, 0);
    state->building_list_burning_totals = create_savegame_piece(8, 0);
    state->figure_sequence = create_savegame_piece(4, 0);
    state->scenario_settings = create_savegame_piece(12, 0);
    state->invasion_warnings = create_savegame_piece(3232, 1);
    state->scenario_is_custom = create_savegame_piece(4, 0);
    state->city_sounds = create_savegame_piece(8960, 0);
    state->building_extra_highest_id = create_savegame_piece(4, 0);
    state->figure_traders = create_savegame_piece(4804, 0);
    state->building_list_burning = create_savegame_piece(1000, 1);
    state->building_list_small = create_savegame_piece(1000, 1);
    state->building_list_large = create_savegame_piece(4000, 1);
    state->tutorial_part1 = create_savegame_piece(32, 0);
    state->building_count_military = create_savegame_piece(16, 0);
    state->enemy_army_totals = create_savegame_piece(20, 0);
    state->building_storages = create_savegame_piece(6400, 0);
    state->building_count_culture2 = create_savegame_piece(32, 0);
    state->building_count_support = create_savegame_piece(24, 0);
    state->tutorial_part2 = create_savegame_piece(4, 0);
    state->gladiator_revolt = create_savegame_piece(16, 0);
    state->trade_route_limit = create_savegame_piece(1280, 1);
    state->trade_route_traded = create_savegame_piece(1280, 1);
    state->building_barracks_tower_sentry = create_savegame_piece(4, 0);
    state->building_extra_sequence = create_savegame_piece(4, 0);
    state->routing_counters = create_savegame_piece(16, 0);
    state->building_count_culture3 = create_savegame_piece(40, 0);
    state->enemy_armies = create_savegame_piece(900, 0);
    state->Data_CityInfo_Extra_entryPointFlag_x = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_entryPointFlag_y = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_x = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_y = create_savegame_piece(4, 0);
    state->last_invasion_id = create_savegame_piece(2, 0);
    state->building_extra_corrupt_houses = create_savegame_piece(8, 0);
    state->scenario_name = create_savegame_piece(65, 0);
    state->bookmarks = create_savegame_piece(32, 0);
    state->tutorial_part3 = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_entryPointFlag_gridOffset = create_savegame_piece(4, 0);
    state->Data_CityInfo_Extra_exitPointFlag_gridOffset = create_savegame_piece(4, 0);
    state->endMarker = create_savegame_piece(284, 0); // 71x 4-bytes emptiness
}

// TODO temporary until all data pieces have been migrated
static void read_all_from_buffer(buffer *buf, void *data)
{
    buffer_read_raw(buf, data, buf->size);
}

// TODO temporary until all data pieces have been migrated
static void write_all_to_buffer(buffer *buf, void *data)
{
    buffer_write_raw(buf, data, buf->size);
}

static void scenario_load_from_state(scenario_state *file)
{
    map_image_load_state(file->graphic_ids);
    map_terrain_load_state(file->terrain);
    map_property_load_state(file->bitfields, file->edge);
    map_random_load_state(file->random);
    map_elevation_load_state(file->elevation);
    city_view_load_scenario_state(file->camera);
    
    random_load_state(file->random_iv);

    scenario_load_state(file->scenario);
    
    // check if all buffers are empty
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        buffer *buf = &scenario_data.pieces[i].buf;
        if (buf->index != buf->size) {
            printf("ERR: buffer %d not empty: %d of %d bytes used\n", i, buf->index, buf->size);
        } else if (buf->overflow) {
            printf("ERR: buffer %d overflowed\n", i);
        }
    }
}

static void savegame_load_from_state(savegame_state *state)
{
    scenario_settings_load_state(state->scenario_campaign_mission,
                                 state->scenario_settings,
                                 state->scenario_is_custom,
                                 state->player_name,
                                 state->scenario_name);

    read_all_from_buffer(state->savegameFileVersion, &savegame_version);

    map_image_load_state(state->image_grid);
    map_building_load_state(state->building_grid, state->building_damage_grid);
    map_terrain_load_state(state->terrain_grid);
    map_aqueduct_load_state(state->aqueduct_grid, state->aqueduct_backup_grid);
    map_figure_load_state(state->figure_grid);
    map_sprite_load_state(state->sprite_grid, state->sprite_backup_grid);
    map_property_load_state(state->bitfields_grid, state->edge_grid);
    map_random_load_state(state->random_grid);
    map_desirability_load_state(state->desirability_grid);
    map_elevation_load_state(state->elevation_grid);

    figure_load_state(state->figures, state->figure_sequence);
    figure_route_load_state(state->route_figures, state->route_paths);
    formations_load_state(state->formations, state->formation_totals);
    
    read_all_from_buffer(state->Data_CityInfo, &Data_CityInfo);
    read_all_from_buffer(state->Data_CityInfo_Extra_unknownBytes, &Data_CityInfo_Extra.unknownBytes);
    read_all_from_buffer(state->Data_CityInfo_Extra_ciid, &Data_CityInfo_Extra.ciid);

    building_load_state(state->buildings,
                        state->building_extra_highest_id,
                        state->building_extra_highest_id_ever,
                        state->building_extra_sequence,
                        state->building_extra_corrupt_houses);
    building_barracks_load_state(state->building_barracks_tower_sentry);
    city_view_load_state(state->city_view_orientation, state->city_view_camera);
    game_time_load_state(state->game_time);
    random_load_state(state->random_iv);
    building_count_load_state(state->building_count_industry,
                              state->building_count_culture1,
                              state->building_count_culture2,
                              state->building_count_culture3,
                              state->building_count_military,
                              state->building_count_support);

    read_all_from_buffer(state->Data_CityInfo_Extra_populationGraphOrder, &Data_CityInfo_Extra.populationGraphOrder);
    read_all_from_buffer(state->Data_CityInfo_Extra_unknownOrder, &Data_CityInfo_Extra.unknownOrder);

    scenario_emperor_change_load_state(state->emperor_change_time, state->emperor_change_state);

    empire_load_state(state->empire);
    empire_city_load_state(state->empire_cities);
    trade_prices_load_state(state->trade_prices);
    figure_name_load_state(state->figure_names);
    city_culture_load_state(state->culture_coverage);

    scenario_load_state(state->scenario);
    scenario_criteria_load_state(state->max_game_year);
    scenario_earthquake_load_state(state->earthquake);
    city_message_load_state(state->messages, state->message_extra,
                            state->message_counts, state->message_delays,
                            state->population_messages);
    sound_city_load_state(state->city_sounds);
    traders_load_state(state->figure_traders);
    
    building_list_load_state(state->building_list_small, state->building_list_large,
                             state->building_list_burning, state->building_list_burning_totals);
    
    tutorial_load_state(state->tutorial_part1, state->tutorial_part2, state->tutorial_part3);

    building_storage_load_state(state->building_storages);
    scenario_gladiator_revolt_load_state(state->gladiator_revolt);
    trade_routes_load_state(state->trade_route_limit, state->trade_route_traded);
    map_routing_load_state(state->routing_counters);
    enemy_armies_load_state(state->enemy_armies, state->enemy_army_totals);

    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_x, &Data_CityInfo_Extra.entryPointFlag.x);
    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_y, &Data_CityInfo_Extra.entryPointFlag.y);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_x, &Data_CityInfo_Extra.exitPointFlag.x);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_y, &Data_CityInfo_Extra.exitPointFlag.y);

    scenario_invasion_load_state(state->last_invasion_id, state->invasion_warnings);
    map_bookmark_load_state(state->bookmarks);

    read_all_from_buffer(state->Data_CityInfo_Extra_entryPointFlag_gridOffset, &Data_CityInfo_Extra.entryPointFlag.gridOffset);
    read_all_from_buffer(state->Data_CityInfo_Extra_exitPointFlag_gridOffset, &Data_CityInfo_Extra.exitPointFlag.gridOffset);

    buffer_skip(state->endMarker, 284);

    // check if all buffers are empty
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        buffer *buf = &savegame_data.pieces[i].buf;
        if (buf->index != buf->size) {
            printf("ERR: buffer %d not empty: %d of %d bytes used\n", i, buf->index, buf->size);
        } else if (buf->overflow) {
            printf("ERR: buffer %d overflowed\n", i);
        }
    }
}

static void savegame_save_to_state(savegame_state *state)
{
    scenario_settings_save_state(state->scenario_campaign_mission,
                                 state->scenario_settings,
                                 state->scenario_is_custom,
                                 state->player_name,
                                 state->scenario_name);

    write_all_to_buffer(state->savegameFileVersion, &savegame_version);

    map_image_save_state(state->image_grid);
    map_building_save_state(state->building_grid, state->building_damage_grid);
    map_terrain_save_state(state->terrain_grid);
    map_aqueduct_save_state(state->aqueduct_grid, state->aqueduct_backup_grid);
    map_figure_save_state(state->figure_grid);
    map_sprite_save_state(state->sprite_grid, state->sprite_backup_grid);
    map_property_save_state(state->bitfields_grid, state->edge_grid);
    map_random_save_state(state->random_grid);
    map_desirability_save_state(state->desirability_grid);
    map_elevation_save_state(state->elevation_grid);

    figure_save_state(state->figures, state->figure_sequence);
    figure_route_save_state(state->route_figures, state->route_paths);
    formations_save_state(state->formations, state->formation_totals);
    
    write_all_to_buffer(state->Data_CityInfo, &Data_CityInfo);
    write_all_to_buffer(state->Data_CityInfo_Extra_unknownBytes, &Data_CityInfo_Extra.unknownBytes);
    write_all_to_buffer(state->Data_CityInfo_Extra_ciid, &Data_CityInfo_Extra.ciid);

    building_save_state(state->buildings,
                        state->building_extra_highest_id,
                        state->building_extra_highest_id_ever,
                        state->building_extra_sequence,
                        state->building_extra_corrupt_houses);
    building_barracks_save_state(state->building_barracks_tower_sentry);
    city_view_save_state(state->city_view_orientation, state->city_view_camera);
    game_time_save_state(state->game_time);
    random_save_state(state->random_iv);
    building_count_save_state(state->building_count_industry,
                              state->building_count_culture1,
                              state->building_count_culture2,
                              state->building_count_culture3,
                              state->building_count_military,
                              state->building_count_support);

    write_all_to_buffer(state->Data_CityInfo_Extra_populationGraphOrder, &Data_CityInfo_Extra.populationGraphOrder);
    write_all_to_buffer(state->Data_CityInfo_Extra_unknownOrder, &Data_CityInfo_Extra.unknownOrder);

    scenario_emperor_change_save_state(state->emperor_change_time, state->emperor_change_state);
    empire_save_state(state->empire);
    empire_city_save_state(state->empire_cities);
    trade_prices_save_state(state->trade_prices);
    figure_name_save_state(state->figure_names);
    city_culture_save_state(state->culture_coverage);

    scenario_save_state(state->scenario);

    scenario_criteria_save_state(state->max_game_year);
    scenario_earthquake_save_state(state->earthquake);
    city_message_save_state(state->messages, state->message_extra,
                            state->message_counts, state->message_delays,
                            state->population_messages);
    sound_city_save_state(state->city_sounds);
    traders_save_state(state->figure_traders);

    building_list_save_state(state->building_list_small, state->building_list_large,
                             state->building_list_burning, state->building_list_burning_totals);

    tutorial_save_state(state->tutorial_part1, state->tutorial_part2, state->tutorial_part3);

    building_storage_save_state(state->building_storages);
    scenario_gladiator_revolt_save_state(state->gladiator_revolt);
    trade_routes_save_state(state->trade_route_limit, state->trade_route_traded);
    map_routing_save_state(state->routing_counters);
    enemy_armies_save_state(state->enemy_armies, state->enemy_army_totals);

    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_x, &Data_CityInfo_Extra.entryPointFlag.x);
    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_y, &Data_CityInfo_Extra.entryPointFlag.y);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_x, &Data_CityInfo_Extra.exitPointFlag.x);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_y, &Data_CityInfo_Extra.exitPointFlag.y);

    scenario_invasion_save_state(state->last_invasion_id, state->invasion_warnings);
    map_bookmark_save_state(state->bookmarks);

    write_all_to_buffer(state->Data_CityInfo_Extra_entryPointFlag_gridOffset, &Data_CityInfo_Extra.entryPointFlag.gridOffset);
    write_all_to_buffer(state->Data_CityInfo_Extra_exitPointFlag_gridOffset, &Data_CityInfo_Extra.exitPointFlag.gridOffset);

    buffer_skip(state->endMarker, 284);

    // check if all buffers are empty
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        buffer *buf = &savegame_data.pieces[i].buf;
        if (buf->index != buf->size) {
            printf("ERR: buffer %d not empty: %d of %d bytes used\n", i, buf->index, buf->size);
        } else if (buf->overflow) {
            printf("ERR: buffer %d overflowed\n", i);
        }
    }
}

int game_file_io_read_scenario(const char *filename)
{
    printf("Loading scenario: %s\n", filename);
    init_scenario_data();
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        fread(scenario_data.pieces[i].buf.data, 1, scenario_data.pieces[i].buf.size, fp);
    }
    fclose(fp);
    
    scenario_load_from_state(&scenario_data.state);
    return 1;
}

static int read_compressed_chunk(FILE *fp, void *buffer, int bytesToRead)
{
    if (bytesToRead > COMPRESS_BUFFER_SIZE) {
        return 0;
    }
    int inputSize = bytesToRead;
    fread(&inputSize, 4, 1, fp);
    if ((unsigned int) inputSize == UNCOMPRESSED) {
        fread(buffer, 1, bytesToRead, fp);
    } else {
        fread(compress_buffer, 1, inputSize, fp);
        if (!zip_decompress(compress_buffer, inputSize, buffer, &bytesToRead)) {
            return 0;
        }
    }
    return 1;
}

static int write_compressed_chunk(FILE *fp, const void *buffer, int bytesToWrite)
{
    if (bytesToWrite > COMPRESS_BUFFER_SIZE) {
        return 0;
    }
    int outputSize = COMPRESS_BUFFER_SIZE;
    if (zip_compress(buffer, bytesToWrite, compress_buffer, &outputSize)) {
        fwrite(&outputSize, 4, 1, fp);
        fwrite(compress_buffer, 1, outputSize, fp);
    } else {
        // unable to compress: write uncompressed
        outputSize = UNCOMPRESSED;
        fwrite(&outputSize, 4, 1, fp);
        fwrite(buffer, 1, bytesToWrite, fp);
    }
    return 1;
}

static void savegame_read_from_file(FILE *fp)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        if (piece->compressed) {
            read_compressed_chunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fread(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
}

static void savegame_write_to_file(FILE *fp)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        if (piece->compressed) {
            write_compressed_chunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fwrite(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
}

int game_file_io_read_saved_game(const char *filename, int offset)
{
    init_savegame_data();

    printf("Loading saved game: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }
    if (offset) {
        fseek(fp, offset, SEEK_SET);
    }
    savegame_read_from_file(fp);
    fclose(fp);
    
    savegame_load_from_state(&savegame_data.state);
    return 1;
}

int game_file_io_write_saved_game(const char *filename)
{
    init_savegame_data();

    printf("Saving game: %s\n", filename);
    savegame_version = SAVE_GAME_VERSION;
    savegame_save_to_state(&savegame_data.state);

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return 0;
    }
    savegame_write_to_file(fp);
    fclose(fp);
    return 1;
}

int game_file_io_delete_saved_game(const char *filename)
{
    return remove(filename) == 0;
}
