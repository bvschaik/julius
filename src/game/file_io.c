#include "file_io.h"

#include "building/barracks.h"
#include "building/count.h"
#include "building/granary.h"
#include "building/list.h"
#include "building/monument.h"
#include "building/storage.h"
#include "city/culture.h"
#include "city/data.h"
#include "city/message.h"
#include "city/view.h"
#include "core/file.h"
#include "core/log.h"
#include "core/dir.h"
#include "core/random.h"
#include "core/string.h"
#include "core/zip.h"
#include "core/zlib_helper.h"
#include "empire/city.h"
#include "empire/empire.h"
#include "empire/empire_xml.h"
#include "empire/object.h"
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
#include "map/tiles.h"
#include "scenario/criteria.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/empire.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "widget/minimap.h"

#include "zlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPRESS_BUFFER_SIZE 3000000
#define UNCOMPRESSED 0x80000000
#define PIECE_SIZE_DYNAMIC 0

static const int SAVE_GAME_CURRENT_VERSION = 0x8e;

static const int SAVE_GAME_LAST_ORIGINAL_LIMITS_VERSION = 0x66;
static const int SAVE_GAME_LAST_SMALLER_IMAGE_ID_VERSION = 0x76;
static const int SAVE_GAME_LAST_NO_DELIVERIES_VERSION = 0x77;
static const int SAVE_GAME_LAST_STATIC_VERSION = 0x78;
static const int SAVE_GAME_LAST_JOINED_IMPORT_EXPORT_VERSION = 0x79;
static const int SAVE_GAME_LAST_STATIC_BUILDING_COUNT_VERSION = 0x80;
static const int SAVE_GAME_LAST_STATIC_MONUMENT_DELIVERIES_VERSION = 0x81;
static const int SAVE_GAME_LAST_STORED_IMAGE_IDS = 0x83;
// SAVE_GAME_INCREASE_GRANARY_CAPACITY shall be updated if we decide to change granary capacity again.
static const int SAVE_GAME_INCREASE_GRANARY_CAPACITY = 0x85;
// static const int SAVE_GAME_ROADBLOCK_DATA_MOVED_FROM_SUBTYPE = 0x86; This define is unneeded for now
static const int SAVE_GAME_LAST_ORIGINAL_TERRAIN_DATA_SIZE_VERSION = 0x86;
static const int SAVE_GAME_LAST_CARAVANSERAI_WRONG_OFFSET = 0x87;
static const int SAVE_GAME_LAST_ZIP_COMPRESSION = 0x88;
static const int SAVE_GAME_LAST_ENEMY_ARMIES_BUFFER_BUG = 0x89;
static const int SAVE_GAME_LAST_BARRACKS_TOWER_SENTRY_REQUEST = 0x8a;
// static const int SAVE_GAME_LAST_WITHOUT_HIGHWAYS = 0x8b; no actual changes to how games are saved. Crudelios just wants this here
static const int SAVE_GAME_LAST_UNVERSIONED_SCENARIOS = 0x8c;
static const int SAVE_GAME_LAST_EMPIRE_RESOURCES_ALWAYS_WRITE = 0x8d;

static char compress_buffer[COMPRESS_BUFFER_SIZE];

typedef struct {
    buffer buf;
    int compressed;
    int dynamic;
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
    buffer *empire;
    buffer *end_marker;
} scenario_state;

static struct {
    int version;
    int num_pieces;
    file_piece pieces[11];
    scenario_state state;
} scenario_data;

typedef struct {
    buffer *scenario_campaign_mission;
    buffer *file_version;
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
    buffer *city_data;
    buffer *city_faction_unknown;
    buffer *player_name;
    buffer *city_faction;
    buffer *buildings;
    buffer *city_view_orientation;
    buffer *game_time;
    buffer *building_extra_highest_id_ever;
    buffer *random_iv;
    buffer *city_view_camera;
    buffer *building_count_culture1;
    buffer *city_graph_order;
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
    buffer *city_entry_exit_xy;
    buffer *last_invasion_id;
    buffer *building_extra_corrupt_houses;
    buffer *scenario_name;
    buffer *bookmarks;
    buffer *tutorial_part3;
    buffer *city_entry_exit_grid_offset;
    buffer *end_marker;
    buffer *deliveries;
    buffer *custom_empire;
} savegame_state;

typedef struct {
    struct {
        int burning_totals;
        int image_grid;
        int terrain_grid;
        int figures;
        int route_figures;
        int route_paths;
        int formations;
        int buildings;
        int building_list_burning;
        int building_list_small;
        int building_list_large;
        int building_storages;
        int monument_deliveries;
        int enemy_armies;
        int scenario;
    } piece_sizes;
    struct {
        int culture1;
        int culture2;
        int culture3;
        int military;
        int industry;
        int support;
    } building_counts;
    int has_image_grid;
    int has_monument_deliveries;
    int has_barracks_tower_sentry_request;
    int has_custom_empires;
} savegame_version_data;

static struct {
    int num_pieces;
    file_piece pieces[100];
    savegame_state state;
} savegame_data;

static struct {
    minimap_functions functions;
    int version;
    int city_width;
    int city_height;
    int caravanserai_id;
    scenario_climate climate;
} minimap_data;

static void init_file_piece(file_piece *piece, int size, int compressed)
{
    piece->compressed = compressed;
    piece->dynamic = size == PIECE_SIZE_DYNAMIC;
    if (piece->dynamic) {
        buffer_init(&piece->buf, 0, 0);
    } else {
        void *data = malloc(size);
        memset(data, 0, size);
        buffer_init(&piece->buf, data, size);
    }
}

static buffer *create_scenario_piece(int size, int compressed)
{
    file_piece *piece = &scenario_data.pieces[scenario_data.num_pieces++];
    init_file_piece(piece, size, compressed);
    return &piece->buf;
}

static buffer *create_savegame_piece(int size, int compressed)
{
    file_piece *piece = &savegame_data.pieces[savegame_data.num_pieces++];
    init_file_piece(piece, size, compressed);
    return &piece->buf;
}

static void clear_savegame_pieces(void)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        buffer_reset(&savegame_data.pieces[i].buf);
        free(savegame_data.pieces[i].buf.data);
    }
    savegame_data.num_pieces = 0;
}

static void clear_scenario_pieces(void)
{
    scenario_data.version = 0;
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        buffer_reset(&scenario_data.pieces[i].buf);
        free(scenario_data.pieces[i].buf.data);
    }
    scenario_data.num_pieces = 0;
}

static void init_scenario_data(int version)
{
    clear_scenario_pieces();
    scenario_data.version = version;
    scenario_state *state = &scenario_data.state;
    state->graphic_ids = create_scenario_piece(52488, 0);
    state->edge = create_scenario_piece(26244, 0);
    state->terrain = create_scenario_piece(52488, 0);
    state->bitfields = create_scenario_piece(26244, 0);
    state->random = create_scenario_piece(26244, 0);
    state->elevation = create_scenario_piece(26244, 0);
    state->random_iv = create_scenario_piece(8, 0);
    state->camera = create_scenario_piece(8, 0);
    if (version <= SCENARIO_LAST_UNVERSIONED) {
        state->scenario = create_scenario_piece(1720, 0);
    } else if(version <= SCENARIO_LAST_NO_WAGE_LIMITS) {
        state->scenario = create_scenario_piece(1830, 0);
    } else {
        state->scenario = create_scenario_piece(1838, 0);
    }
    if (version > SCENARIO_LAST_UNVERSIONED) {
        state->empire = create_scenario_piece(PIECE_SIZE_DYNAMIC, 1);
    }
    state->end_marker = create_scenario_piece(4, 0);
}

static void get_version_data(savegame_version_data *version_data, int version)
{
    int multiplier = 1;
    int count_multiplier = 1;
    version_data->piece_sizes.burning_totals = 8;
    if (version > SAVE_GAME_LAST_ORIGINAL_LIMITS_VERSION) {
        multiplier = 5;
    }
    if (version > SAVE_GAME_LAST_STATIC_VERSION) {
        multiplier = PIECE_SIZE_DYNAMIC;
        version_data->piece_sizes.burning_totals = 4;
    }

    if (version > SAVE_GAME_LAST_STATIC_BUILDING_COUNT_VERSION) {
        count_multiplier = PIECE_SIZE_DYNAMIC;
    }

    version_data->piece_sizes.image_grid = 52488 * (version > SAVE_GAME_LAST_SMALLER_IMAGE_ID_VERSION ? 2 : 1);
    version_data->piece_sizes.terrain_grid = 52488 * (version > SAVE_GAME_LAST_ORIGINAL_TERRAIN_DATA_SIZE_VERSION ? 2 : 1);
    version_data->piece_sizes.figures = 128000 * multiplier;
    version_data->piece_sizes.route_figures = 1200 * multiplier;
    version_data->piece_sizes.route_paths = 300000 * multiplier;
    version_data->piece_sizes.formations = 6400 * multiplier;
    version_data->piece_sizes.buildings = 256000 * multiplier;
    version_data->piece_sizes.building_list_burning = 1000 * multiplier;
    version_data->piece_sizes.building_list_small = 1000 * multiplier;
    version_data->piece_sizes.building_list_large = 4000 * multiplier;
    version_data->piece_sizes.building_storages = 6400 * multiplier;
    version_data->piece_sizes.monument_deliveries = version > SAVE_GAME_LAST_STATIC_MONUMENT_DELIVERIES_VERSION ? PIECE_SIZE_DYNAMIC : 3200;
    version_data->piece_sizes.enemy_armies = version > SAVE_GAME_LAST_ENEMY_ARMIES_BUFFER_BUG ? (MAX_ENEMY_ARMIES * sizeof(int) * 9) : 900;

    version_data->building_counts.culture1 = 132 * count_multiplier;
    version_data->building_counts.culture2 = 32 * count_multiplier;
    version_data->building_counts.culture3 = 40 * count_multiplier;
    version_data->building_counts.military = 16 * count_multiplier;
    version_data->building_counts.industry = 128 * count_multiplier;
    version_data->building_counts.support = 24 * count_multiplier;

    version_data->has_image_grid = version <= SAVE_GAME_LAST_STORED_IMAGE_IDS;
    version_data->has_monument_deliveries = version > SAVE_GAME_LAST_NO_DELIVERIES_VERSION;
    version_data->has_barracks_tower_sentry_request = version <= SAVE_GAME_LAST_BARRACKS_TOWER_SENTRY_REQUEST;

    if (version <= SAVE_GAME_LAST_UNVERSIONED_SCENARIOS) {
        version_data->piece_sizes.scenario = 1720;
        version_data->has_custom_empires = 0;
    } else {
        version_data->piece_sizes.scenario = 1838;
        version_data->has_custom_empires = 1;
    }
}

static void init_savegame_data(int version)
{
    clear_savegame_pieces();

    savegame_version_data version_data;
    get_version_data(&version_data, version);

    savegame_state *state = &savegame_data.state;
    state->scenario_campaign_mission = create_savegame_piece(4, 0);
    state->file_version = create_savegame_piece(4, 0);
    if (version_data.has_image_grid) {
        state->image_grid = create_savegame_piece(version_data.piece_sizes.image_grid, 1);
    }
    state->edge_grid = create_savegame_piece(26244, 1);
    state->building_grid = create_savegame_piece(52488, 1);
    state->terrain_grid = create_savegame_piece(version_data.piece_sizes.terrain_grid, 1);
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
    state->figures = create_savegame_piece(version_data.piece_sizes.figures, 1);
    state->route_figures = create_savegame_piece(version_data.piece_sizes.route_figures, 1);
    state->route_paths = create_savegame_piece(version_data.piece_sizes.route_paths, 1);
    state->formations = create_savegame_piece(version_data.piece_sizes.formations, 1);
    state->formation_totals = create_savegame_piece(12, 0);
    state->city_data = create_savegame_piece(36136, 1);
    state->city_faction_unknown = create_savegame_piece(2, 0);
    state->player_name = create_savegame_piece(64, 0);
    state->city_faction = create_savegame_piece(4, 0);
    state->buildings = create_savegame_piece(version_data.piece_sizes.buildings, 1);
    state->city_view_orientation = create_savegame_piece(4, 0);
    state->game_time = create_savegame_piece(20, 0);
    state->building_extra_highest_id_ever = create_savegame_piece(8, 0);
    state->random_iv = create_savegame_piece(8, 0);
    state->city_view_camera = create_savegame_piece(8, 0);
    state->building_count_culture1 = create_savegame_piece(version_data.building_counts.culture1, 0);
    state->city_graph_order = create_savegame_piece(8, 0);
    state->emperor_change_time = create_savegame_piece(8, 0);
    state->empire = create_savegame_piece(12, 0);
    state->empire_cities = create_savegame_piece(2706, 1);
    state->building_count_industry = create_savegame_piece(version_data.building_counts.industry, 0);
    state->trade_prices = create_savegame_piece(128, 0);
    state->figure_names = create_savegame_piece(84, 0);
    state->culture_coverage = create_savegame_piece(60, 0);
    state->scenario = create_savegame_piece(version_data.piece_sizes.scenario, 0);
    state->max_game_year = create_savegame_piece(4, 0);
    state->earthquake = create_savegame_piece(60, 0);
    state->emperor_change_state = create_savegame_piece(4, 0);
    state->messages = create_savegame_piece(16000, 1);
    state->message_extra = create_savegame_piece(12, 0);
    state->population_messages = create_savegame_piece(10, 0);
    state->message_counts = create_savegame_piece(80, 0);
    state->message_delays = create_savegame_piece(80, 0);
    state->building_list_burning_totals = create_savegame_piece(version_data.piece_sizes.burning_totals, 0);
    state->figure_sequence = create_savegame_piece(4, 0);
    state->scenario_settings = create_savegame_piece(12, 0);
    state->invasion_warnings = create_savegame_piece(3232, 1);
    state->scenario_is_custom = create_savegame_piece(4, 0);
    state->city_sounds = create_savegame_piece(8960, 0);
    state->building_extra_highest_id = create_savegame_piece(4, 0);
    state->figure_traders = create_savegame_piece(4804, 0);
    state->building_list_burning = create_savegame_piece(version_data.piece_sizes.building_list_burning, 1);
    state->building_list_small = create_savegame_piece(version_data.piece_sizes.building_list_small, 1);
    state->building_list_large = create_savegame_piece(version_data.piece_sizes.building_list_large, 1);
    state->tutorial_part1 = create_savegame_piece(32, 0);
    state->building_count_military = create_savegame_piece(version_data.building_counts.military, 0);
    state->enemy_army_totals = create_savegame_piece(20, 0);
    state->building_storages = create_savegame_piece(version_data.piece_sizes.building_storages, 0);
    state->building_count_culture2 = create_savegame_piece(version_data.building_counts.culture2, 0);
    state->building_count_support = create_savegame_piece(version_data.building_counts.support, 0);
    state->tutorial_part2 = create_savegame_piece(4, 0);
    state->gladiator_revolt = create_savegame_piece(16, 0);
    state->trade_route_limit = create_savegame_piece(1280, 1);
    state->trade_route_traded = create_savegame_piece(1280, 1);
    if (version_data.has_barracks_tower_sentry_request) {
        state->building_barracks_tower_sentry = create_savegame_piece(4, 0);
    }
    state->building_extra_sequence = create_savegame_piece(4, 0);
    state->routing_counters = create_savegame_piece(16, 0);
    state->building_count_culture3 = create_savegame_piece(version_data.building_counts.culture3, 0);
    state->enemy_armies = create_savegame_piece(version_data.piece_sizes.enemy_armies, 0);
    state->city_entry_exit_xy = create_savegame_piece(16, 0);
    state->last_invasion_id = create_savegame_piece(2, 0);
    state->building_extra_corrupt_houses = create_savegame_piece(8, 0);
    state->scenario_name = create_savegame_piece(65, 0);
    state->bookmarks = create_savegame_piece(32, 0);
    state->tutorial_part3 = create_savegame_piece(4, 0);
    state->city_entry_exit_grid_offset = create_savegame_piece(8, 0);
    state->end_marker = create_savegame_piece(284, 0); // 71x 4-bytes emptiness
    if (version_data.has_monument_deliveries) {
        state->deliveries = create_savegame_piece(version_data.piece_sizes.monument_deliveries, 0);
    }
    if (version_data.has_custom_empires) {
        state->custom_empire = create_savegame_piece(PIECE_SIZE_DYNAMIC, 1);
    }
}

static void scenario_load_from_state(scenario_state *file, int version)
{
    map_image_load_state(file->graphic_ids);
    map_terrain_load_state(file->terrain, 0, file->graphic_ids, 1);
    map_property_load_state(file->bitfields, file->edge);
    map_random_load_state(file->random);
    map_elevation_load_state(file->elevation);
    city_view_load_scenario_state(file->camera);
    random_load_state(file->random_iv);
    scenario_load_state(file->scenario, version);
    if (version > SCENARIO_LAST_UNVERSIONED) {
        empire_object_load(file->empire, version);
    }
    buffer_skip(file->end_marker, 4);
}

static void scenario_save_to_state(scenario_state *file)
{
    map_image_save_state(file->graphic_ids);
    map_terrain_save_state_legacy(file->terrain);
    map_property_save_state(file->bitfields, file->edge);
    map_random_save_state(file->random);
    map_elevation_save_state(file->elevation);
    city_view_save_scenario_state(file->camera);
    random_save_state(file->random_iv);
    scenario_save_state(file->scenario);
    empire_object_save(file->empire);
    buffer_skip(file->end_marker, 4);
}

static int save_version_to_scenario_version(int save_version) {
    if (save_version <= SAVE_GAME_LAST_UNVERSIONED_SCENARIOS) {
        return SCENARIO_LAST_UNVERSIONED;
    }
    if (save_version <= SAVE_GAME_LAST_EMPIRE_RESOURCES_ALWAYS_WRITE) {
        return SCENARIO_LAST_EMPIRE_RESOURCES_ALWAYS_WRITE;
    }
    return SCENARIO_CURRENT_VERSION;
}

static void savegame_load_from_state(savegame_state *state, int version)
{
    scenario_settings_load_state(state->scenario_campaign_mission,
        state->scenario_settings,
        state->scenario_is_custom,
        state->player_name,
        state->scenario_name);

    int scenario_version = save_version_to_scenario_version(version);
    scenario_load_state(state->scenario, scenario_version);
    scenario_map_init();

    map_building_load_state(state->building_grid, state->building_damage_grid);
    map_terrain_load_state(state->terrain_grid, version > SAVE_GAME_LAST_ORIGINAL_TERRAIN_DATA_SIZE_VERSION,
        version <= SAVE_GAME_LAST_STORED_IMAGE_IDS ? state->image_grid : 0,
        version <= SAVE_GAME_LAST_SMALLER_IMAGE_ID_VERSION);
    map_aqueduct_load_state(state->aqueduct_grid, state->aqueduct_backup_grid);
    map_figure_load_state(state->figure_grid);
    map_sprite_load_state(state->sprite_grid, state->sprite_backup_grid);
    map_property_load_state(state->bitfields_grid, state->edge_grid);
    map_random_load_state(state->random_grid);
    map_desirability_load_state(state->desirability_grid);
    map_elevation_load_state(state->elevation_grid);
    figure_load_state(state->figures, state->figure_sequence, version > SAVE_GAME_LAST_STATIC_VERSION);
    figure_route_load_state(state->route_figures, state->route_paths);
    formations_load_state(state->formations, state->formation_totals, version);

    city_data_load_state(state->city_data,
        state->city_faction,
        state->city_faction_unknown,
        state->city_graph_order,
        state->city_entry_exit_xy,
        state->city_entry_exit_grid_offset,
        version > SAVE_GAME_LAST_JOINED_IMPORT_EXPORT_VERSION);

    building_load_state(state->buildings,
        state->building_extra_sequence,
        state->building_extra_corrupt_houses,
        version > SAVE_GAME_LAST_STATIC_VERSION,
        version);
    city_view_load_state(state->city_view_orientation, state->city_view_camera);
    game_time_load_state(state->game_time);
    random_load_state(state->random_iv);
    building_count_load_state(state->building_count_industry,
        state->building_count_culture1,
        state->building_count_culture2,
        state->building_count_culture3,
        state->building_count_military,
        state->building_count_support,
        version > SAVE_GAME_LAST_STATIC_BUILDING_COUNT_VERSION);
    if (version < SAVE_GAME_INCREASE_GRANARY_CAPACITY) {
        building_granary_update_built_granaries_capacity();
    }

    scenario_emperor_change_load_state(state->emperor_change_time, state->emperor_change_state);
    empire_load_state(state->empire);
    empire_city_load_state(state->empire_cities);
    trade_prices_load_state(state->trade_prices);
    figure_name_load_state(state->figure_names);
    city_culture_load_state(state->culture_coverage);

    scenario_criteria_load_state(state->max_game_year);
    scenario_earthquake_load_state(state->earthquake);
    city_message_load_state(state->messages, state->message_extra,
        state->message_counts, state->message_delays,
        state->population_messages);
    sound_city_load_state(state->city_sounds);
    traders_load_state(state->figure_traders);

    building_list_load_state(state->building_list_small, state->building_list_large,
        state->building_list_burning, state->building_list_burning_totals,
        version > SAVE_GAME_LAST_STATIC_VERSION);

    tutorial_load_state(state->tutorial_part1, state->tutorial_part2, state->tutorial_part3);

    building_storage_load_state(state->building_storages, version > SAVE_GAME_LAST_STATIC_VERSION);
    scenario_gladiator_revolt_load_state(state->gladiator_revolt);
    trade_routes_load_state(state->trade_route_limit, state->trade_route_traded);
    map_routing_load_state(state->routing_counters);
    enemy_armies_load_state(state->enemy_armies, state->enemy_army_totals);
    scenario_invasion_load_state(state->last_invasion_id, state->invasion_warnings);
    map_bookmark_load_state(state->bookmarks);

    buffer_skip(state->end_marker, 284);
    if (state) {
        buffer_skip(state->end_marker, 8);
    }
    if (version <= SAVE_GAME_LAST_NO_DELIVERIES_VERSION) {
        building_monument_initialize_deliveries();
    } else {
        building_monument_delivery_load_state(state->deliveries,
            version > SAVE_GAME_LAST_STATIC_MONUMENT_DELIVERIES_VERSION);
    }
    if (version > SAVE_GAME_LAST_UNVERSIONED_SCENARIOS) {
        empire_object_load(state->custom_empire, scenario_version);
    }
    map_image_clear();
    map_image_update_all();
}

static void savegame_save_to_state(savegame_state *state)
{
    buffer_write_i32(state->file_version, SAVE_GAME_CURRENT_VERSION);

    scenario_settings_save_state(state->scenario_campaign_mission,
        state->scenario_settings,
        state->scenario_is_custom,
        state->player_name,
        state->scenario_name);

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

    city_data_save_state(state->city_data,
        state->city_faction,
        state->city_faction_unknown,
        state->city_graph_order,
        state->city_entry_exit_xy,
        state->city_entry_exit_grid_offset);

    building_save_state(state->buildings,
        state->building_extra_highest_id,
        state->building_extra_highest_id_ever,
        state->building_extra_sequence,
        state->building_extra_corrupt_houses);
    city_view_save_state(state->city_view_orientation, state->city_view_camera);
    game_time_save_state(state->game_time);
    random_save_state(state->random_iv);
    building_count_save_state(state->building_count_industry,
        state->building_count_culture1,
        state->building_count_culture2,
        state->building_count_culture3,
        state->building_count_military,
        state->building_count_support);

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
    scenario_invasion_save_state(state->last_invasion_id, state->invasion_warnings);
    map_bookmark_save_state(state->bookmarks);

    buffer_skip(state->end_marker, 284);

    building_monument_delivery_save_state(state->deliveries);
    empire_object_save(state->custom_empire);
}

static int get_scenario_version(FILE *fp)
{
    char version_magic[8];
    int read = fread(version_magic, 1, 8, fp);
    if (read != sizeof(version_magic)) {
        log_error("Unable to read version header from file", 0, 0);
        return 0;
    }
    if (strcmp(version_magic, "VERSION") != 0) {
        rewind(fp);
        return SCENARIO_LAST_UNVERSIONED;
    }

    buffer buf;
    uint8_t version_data[4];
    buffer_init(&buf, version_data, 4);
    read = fread(version_data, 1, 4, fp);
    if (read != sizeof(version_data)) {
        log_error("Unable to read version number from file", 0, 0);
        return 0;
    }
    return buffer_read_i32(&buf);
}

static int read_int32(FILE *fp)
{
    uint8_t data[4];
    if (fread(&data, 1, 4, fp) != 4) {
        return 0;
    }
    buffer buf;
    buffer_init(&buf, data, 4);
    return buffer_read_i32(&buf);
}

static void write_int32(FILE *fp, int value)
{
    uint8_t data[4];
    buffer buf;
    buffer_init(&buf, data, 4);
    buffer_write_i32(&buf, value);
    fwrite(&data, 1, 4, fp);
}

static int read_compressed_chunk(FILE *fp, void *buffer, int bytes_to_read, int read_as_zlib)
{
    if (bytes_to_read > COMPRESS_BUFFER_SIZE) {
        return 0;
    }
    int input_size = read_int32(fp);
    if ((unsigned int)input_size == UNCOMPRESSED) {
        return fread(buffer, 1, bytes_to_read, fp) == bytes_to_read;
    } else {
        if (fread(compress_buffer, 1, input_size, fp) != input_size) {
            return 0;
        }

        if (!read_as_zlib) {
            return zip_decompress(compress_buffer, input_size, buffer, &bytes_to_read);
        } else {
            int output_size = 0;
            return zlib_helper_decompress(compress_buffer, input_size, buffer, bytes_to_read, &output_size);
        }
    }
}

static int read_compressed_savegame_chunk(FILE *fp, void *buffer, int bytes_to_read, int version)
{
    int read_as_zlib = version > SAVE_GAME_LAST_ZIP_COMPRESSION;
    return read_compressed_chunk(fp, buffer, bytes_to_read, read_as_zlib);
}

static int write_compressed_chunk(FILE *fp, void *buffer, int bytes_to_write)
{
    if (bytes_to_write > COMPRESS_BUFFER_SIZE) {
        return 0;
    }
    int output_size = 0;
    if (zlib_helper_compress(buffer, bytes_to_write, compress_buffer, COMPRESS_BUFFER_SIZE, &output_size)) {
        write_int32(fp, output_size);
        fwrite(compress_buffer, 1, output_size, fp);
    } else {
        // unable to compress: write uncompressed
        write_int32(fp, UNCOMPRESSED);
        fwrite(buffer, 1, bytes_to_write, fp);
    }
    return 1;
}

static int prepare_dynamic_piece(FILE *fp, file_piece *piece)
{
    if (piece->dynamic) {
        int size = read_int32(fp);
        if (!size) {
            return 0;
        }
        uint8_t *data = malloc(size);
        memset(data, 0, size);
        buffer_init(&piece->buf, data, size);
    }
    return 1;
}

static int load_scenario_to_buffers(const char *filename, int *version)
{
    FILE *fp = file_open(dir_get_file(filename, NOT_LOCALIZED), "rb");
    if (!fp) {
        return 0;
    }
    *version = get_scenario_version(fp);
    init_scenario_data(*version);
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        file_piece *piece = &scenario_data.pieces[i];
        int result = 0;
        if (!prepare_dynamic_piece(fp, piece)) {
            continue;
        }
        if (piece->compressed) {
            result = read_compressed_chunk(fp, piece->buf.data, piece->buf.size, 1);
        } else {
            int bytes_read = fread(piece->buf.data, 1, piece->buf.size, fp);
            result = bytes_read == piece->buf.size;
        }
        if (!result) {
            log_info("Incorrect buffer size, got", 0, result);
            log_info("Incorrect buffer size, expected", 0, piece->buf.size);
            file_close(fp);
            return 0;
        }
    }
    file_close(fp);
    return 1;
}

int game_file_io_read_scenario(const char *filename)
{
    log_info("Loading scenario", filename, 0);
    int version = 0;
    if (!load_scenario_to_buffers(filename, &version)) {
        return 0;
    }
    scenario_load_from_state(&scenario_data.state, scenario_data.version);
    return 1;
}

static int scenario_terrain_at(int grid_offset)
{
    return map_terrain_get_from_buffer_16(scenario_data.state.terrain, grid_offset);
}

static int scenario_tile_size_at(int grid_offset)
{
    return map_property_multi_tile_size_from_buffer(scenario_data.state.bitfields, grid_offset);
}

static int scenario_is_draw_tile_at(int grid_offset)
{
    return map_property_is_draw_tile_from_buffer(scenario_data.state.edge, grid_offset);
}

static int scenario_random_at(int grid_offset)
{
    return map_random_get_from_buffer(scenario_data.state.random, grid_offset);
}

static scenario_climate get_climate(void)
{
    return minimap_data.climate;
}

static int map_width(void)
{
    return minimap_data.city_width;
}

static int map_height(void)
{
    return minimap_data.city_height;
}

static void set_viewport(int *x, int *y, int *width, int *height)
{
    *x = 0;
    *y = 0;
    *width = minimap_data.city_width;
    *height = minimap_data.city_height;
}

int game_file_io_read_scenario_info(const char *filename, scenario_info *info)
{
    int version = 0;
    if (!load_scenario_to_buffers(filename, &version)) {
        return 0;
    }

    const scenario_state *state = &scenario_data.state;

    scenario_description_from_buffer(state->scenario, info->description);
    info->image_id = scenario_image_id_from_buffer(state->scenario);
    info->climate = scenario_climate_from_buffer(state->scenario, version);
    info->total_invasions = scenario_invasions_from_buffer(state->scenario);
    info->player_rank = scenario_rank_from_buffer(state->scenario);
    info->start_year = scenario_start_year_from_buffer(state->scenario);
    scenario_open_play_info_from_buffer(state->scenario, version, &info->is_open_play, &info->open_play_id);

    if (!info->is_open_play) {
        scenario_objectives_from_buffer(state->scenario, version, &info->win_criteria);
    }
    int grid_start;
    int grid_border_size;

    scenario_map_data_from_buffer(state->scenario, &minimap_data.city_width, &minimap_data.city_height,
        &grid_start, &grid_border_size);
    minimap_data.version = 0;
    minimap_data.climate = info->climate;
    minimap_data.functions.building = 0;
    minimap_data.functions.climate = get_climate;
    minimap_data.functions.map.width = map_width;
    minimap_data.functions.map.height = map_height;
    minimap_data.functions.viewport = set_viewport;
    minimap_data.functions.offset.building_id = 0;
    minimap_data.functions.offset.figure = 0;
    minimap_data.functions.offset.is_draw_tile = scenario_is_draw_tile_at;
    minimap_data.functions.offset.random = scenario_random_at;
    minimap_data.functions.offset.terrain = scenario_terrain_at;
    minimap_data.functions.offset.tile_size = scenario_tile_size_at;

    city_view_set_custom_lookup(grid_start, minimap_data.city_width, minimap_data.city_height, grid_border_size);
    widget_minimap_update(&minimap_data.functions);
    city_view_restore_lookup();

    clear_scenario_pieces();

    return 1;
}

int game_file_io_write_scenario(const char *filename)
{
    log_info("Saving scenario", filename, 0);
    init_scenario_data(SCENARIO_CURRENT_VERSION);
    scenario_save_to_state(&scenario_data.state);

    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        log_error("Unable to save scenario", 0, 0);
        return 0;
    }
    uint8_t header[8];
    string_copy(string_from_ascii("VERSION"), header, sizeof(header));
    fwrite(header, 1, 8, fp);
    write_int32(fp, SCENARIO_CURRENT_VERSION);
    for (int i = 0; i < scenario_data.num_pieces; i++) {
        file_piece *piece = &scenario_data.pieces[i];
        if (piece->dynamic) {
            write_int32(fp, piece->buf.size);
            if (!piece->buf.size) {
                continue;
            }
        }
        if (piece->compressed) {
            write_compressed_chunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fwrite(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
    file_close(fp);
    return 1;
}

static int savegame_read_from_file(FILE *fp, int version)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        int result = 0;
        if (!prepare_dynamic_piece(fp, piece)) {
            continue;
        }
        if (piece->compressed) {
            result = read_compressed_savegame_chunk(fp, piece->buf.data, piece->buf.size, version);
        } else {
            result = fread(piece->buf.data, 1, piece->buf.size, fp) == piece->buf.size;
        }
        // The last piece may be smaller than buf.size
        if (!result && i != (savegame_data.num_pieces - 1)) {
            log_info("Incorrect buffer size, got", 0, result);
            log_info("Incorrect buffer size, expected", 0, piece->buf.size);
            return 0;
        }
    }
    return 1;
}

static void savegame_write_to_file(FILE *fp)
{
    for (int i = 0; i < savegame_data.num_pieces; i++) {
        file_piece *piece = &savegame_data.pieces[i];
        if (piece->dynamic) {
            write_int32(fp, piece->buf.size);
            if (!piece->buf.size) {
                continue;
            }
        }
        if (piece->compressed) {
            write_compressed_chunk(fp, piece->buf.data, piece->buf.size);
        } else {
            fwrite(piece->buf.data, 1, piece->buf.size, fp);
        }
    }
}

static int get_savegame_version(FILE *fp)
{
    buffer buf;
    uint8_t data[4];
    buffer_init(&buf, data, 4);
    if (fseek(fp, 4, SEEK_CUR) ||
        fread(data, 1, 4, fp) != 4 ||
        fseek(fp, -8, SEEK_CUR)) {
        return 0;
    }
    return buffer_read_i32(&buf);
}

int game_file_io_read_saved_game(const char *filename, int offset)
{
    log_info("Loading saved game", filename, 0);
    FILE *fp = file_open(dir_get_file(filename, NOT_LOCALIZED), "rb");
    if (!fp) {
        log_error("Unable to load game, unable to open file.", 0, 0);
        return 0;
    }
    if (offset) {
        fseek(fp, offset, SEEK_SET);
    }
    int result = 0;
    int version = get_savegame_version(fp);
    if (version) {
        if (version > SAVE_GAME_CURRENT_VERSION) {
            log_error("Newer save game version than supported. Please update your Augustus. Version:", 0, version);
            return -1;
        }
        log_info("Savegame version", 0, version);
        init_savegame_data(version);
        result = savegame_read_from_file(fp, version);
    }
    file_close(fp);
    if (!result) {
        log_error("Unable to load game, unable to read savefile.", 0, 0);
        return 0;
    }
    savegame_load_from_state(&savegame_data.state, version);
    return 1;
}

static int skip_piece(FILE *fp, int size, int compressed)
{
    if (size == PIECE_SIZE_DYNAMIC) {
        size = read_int32(fp);
        if (!size) {
            return 1;
        }
    }
    if (!compressed) {
        return fseek(fp, size, SEEK_CUR) == 0;
    }
    int input_size = read_int32(fp);
    if ((unsigned int) input_size == UNCOMPRESSED) {
        return fseek(fp, size, SEEK_CUR) == 0;
    }
    return fseek(fp, input_size, SEEK_CUR) == 0;
}

static void free_file_piece(file_piece *piece)
{
    buffer_reset(&piece->buf);
    free(piece->buf.data);
}

static int savegame_terrain_at(int grid_offset)
{
    if (minimap_data.version <= SAVE_GAME_LAST_ORIGINAL_TERRAIN_DATA_SIZE_VERSION) {
        return map_terrain_get_from_buffer_16(savegame_data.state.terrain_grid, grid_offset);
    } else {
        return map_terrain_get_from_buffer_32(savegame_data.state.terrain_grid, grid_offset);
    }
}

static int savegame_tile_size_at(int grid_offset)
{
    return map_property_multi_tile_size_from_buffer(savegame_data.state.bitfields_grid, grid_offset);
}

static int savegame_is_draw_tile_at(int grid_offset)
{
    return map_property_is_draw_tile_from_buffer(savegame_data.state.edge_grid, grid_offset);
}

static int savegame_random_at(int grid_offset)
{
    return map_random_get_from_buffer(savegame_data.state.random_grid, grid_offset);
}

static int savegame_get_building_id(int grid_offset)
{
    return map_building_from_buffer(savegame_data.state.building_grid, grid_offset);
}

static building *savegame_building(int id)
{
    static building b;
    // Old savegame versions had a bug where the caravanserai's building save data size was one byte too small, so all
    // buildings saved after the caravanserai need to have their offset pushed back by 1
    int offset = minimap_data.version <= SAVE_GAME_LAST_CARAVANSERAI_WRONG_OFFSET && minimap_data.caravanserai_id &&
        id > minimap_data.caravanserai_id ? -1 : 0;
    building_get_from_buffer(savegame_data.state.buildings, id, &b,
        minimap_data.version > SAVE_GAME_LAST_STATIC_VERSION, minimap_data.version, offset);
    return &b;
}

static int savegame_read_file_info(FILE *fp, saved_game_info *info, int version)
{
    clear_savegame_pieces();

    savegame_version_data version_data;
    get_version_data(&version_data, version);
    int scenario_version = save_version_to_scenario_version(version);

    file_piece city_data, game_time, terrain_grid, random_grid, scenario;
    file_piece bitfields_grid, edge_grid, building_grid, buildings;

    savegame_state *state = &savegame_data.state;

    init_file_piece(&city_data, 36136, 0);
    init_file_piece(&game_time, 20, 0);
    init_file_piece(&terrain_grid, version_data.piece_sizes.terrain_grid, 0);
    init_file_piece(&random_grid, 26244, 0);
    init_file_piece(&edge_grid, 26244, 0);
    init_file_piece(&bitfields_grid, 26244, 0);
    init_file_piece(&scenario, version_data.piece_sizes.scenario, 0);
    init_file_piece(&building_grid, 52488, 0);
    init_file_piece(&buildings, version_data.piece_sizes.buildings, 0);

    state->terrain_grid = &terrain_grid.buf;
    state->random_grid = &random_grid.buf;
    state->edge_grid = &edge_grid.buf;
    state->bitfields_grid = &bitfields_grid.buf;
    state->building_grid = &building_grid.buf;
    state->buildings = &buildings.buf;

    info->mission = read_int32(fp);

    skip_piece(fp, 4, 0);
    if (version_data.has_image_grid) {
        skip_piece(fp, version_data.piece_sizes.image_grid, 1);
    }

    if (!read_compressed_savegame_chunk(fp, edge_grid.buf.data, edge_grid.buf.size, version)) {
        return 0;
    }

    if (!read_compressed_savegame_chunk(fp, building_grid.buf.data, building_grid.buf.size, version)) {
        return 0;
    }

    if (!read_compressed_savegame_chunk(fp, terrain_grid.buf.data, terrain_grid.buf.size, version)) {
        return 0;
    }

    skip_piece(fp, 26244, 1);
    skip_piece(fp, 52488, 1);

    if (!read_compressed_savegame_chunk(fp, bitfields_grid.buf.data, bitfields_grid.buf.size, version)) {
        return 0;
    }

    skip_piece(fp, 26244, 1);

    if (fread(random_grid.buf.data, 1, random_grid.buf.size, fp) != random_grid.buf.size) {
        return 0;
    }

    skip_piece(fp, 26244, 1);
    skip_piece(fp, 26244, 1);
    skip_piece(fp, 26244, 1);
    skip_piece(fp, 26244, 1);
    skip_piece(fp, 26244, 1);
    skip_piece(fp, version_data.piece_sizes.figures, 1);
    skip_piece(fp, version_data.piece_sizes.route_figures, 1);
    skip_piece(fp, version_data.piece_sizes.route_paths, 1);
    skip_piece(fp, version_data.piece_sizes.formations, 1);
    skip_piece(fp, 12, 0);

    if (!read_compressed_savegame_chunk(fp, city_data.buf.data, city_data.buf.size, version)) {
        return 0;
    }

    skip_piece(fp, 2, 0);
    skip_piece(fp, 64, 0);
    skip_piece(fp, 4, 0);

    if (!prepare_dynamic_piece(fp, &buildings) || !read_compressed_savegame_chunk(fp, buildings.buf.data, buildings.buf.size, version)) {
        return 0;
    }

    skip_piece(fp, 4, 0);

    if (fread(game_time.buf.data, 1, game_time.buf.size, fp) != game_time.buf.size) {
        return 0;
    }

    skip_piece(fp, 8, 0);
    skip_piece(fp, 8, 0);
    skip_piece(fp, 8, 0);
    skip_piece(fp, version_data.building_counts.culture1, 0);
    skip_piece(fp, 8, 0);
    skip_piece(fp, 8, 0);
    skip_piece(fp, 12, 0);
    skip_piece(fp, 2706, 1);
    skip_piece(fp, version_data.building_counts.industry, 0);
    skip_piece(fp, 128, 0);
    skip_piece(fp, 84, 0);
    skip_piece(fp, 60, 0);

    if (fread(scenario.buf.data, 1, scenario.buf.size, fp) != scenario.buf.size) {
        return 0;
    }

    skip_piece(fp, 4, 0);
    skip_piece(fp, 60, 0);
    skip_piece(fp, 4, 0);
    skip_piece(fp, 16000, 1);
    skip_piece(fp, 12, 0);
    skip_piece(fp, 10, 0);
    skip_piece(fp, 80, 0);
    skip_piece(fp, 80, 0);
    skip_piece(fp, version_data.piece_sizes.burning_totals, 0);
    skip_piece(fp, 4, 0);
    skip_piece(fp, 12, 0);
    skip_piece(fp, 3232, 1);

    info->custom_mission = read_int32(fp);

    city_data_load_basic_info(&city_data.buf, &info->population, &info->treasury, &minimap_data.caravanserai_id);
    game_time_load_basic_info(&game_time.buf, &info->month, &info->year);

    int grid_start;
    int grid_border_size;

    minimap_data.version = version;
    scenario_map_data_from_buffer(&scenario.buf, &minimap_data.city_width, &minimap_data.city_height,
        &grid_start, &grid_border_size);
    minimap_data.climate = scenario_climate_from_buffer(&scenario.buf, scenario_version);
    minimap_data.functions.building = savegame_building;
    minimap_data.functions.climate = get_climate;
    minimap_data.functions.map.width = map_width;
    minimap_data.functions.map.height = map_height;
    minimap_data.functions.viewport = set_viewport;
    minimap_data.functions.offset.building_id = savegame_get_building_id;
    minimap_data.functions.offset.figure = 0;
    minimap_data.functions.offset.is_draw_tile = savegame_is_draw_tile_at;
    minimap_data.functions.offset.random = savegame_random_at;
    minimap_data.functions.offset.terrain = savegame_terrain_at;
    minimap_data.functions.offset.tile_size = savegame_tile_size_at;

    city_view_set_custom_lookup(grid_start, minimap_data.city_width, minimap_data.city_height, grid_border_size);
    widget_minimap_update(&minimap_data.functions);
    city_view_restore_lookup();

    free_file_piece(&city_data);
    free_file_piece(&game_time);
    free_file_piece(&terrain_grid);
    free_file_piece(&scenario);
    free_file_piece(&random_grid);
    free_file_piece(&edge_grid);
    free_file_piece(&bitfields_grid);
    free_file_piece(&building_grid);
    free_file_piece(&buildings);

    return 1;
}

int game_file_io_read_saved_game_info(const char *filename, saved_game_info *info)
{
    FILE *fp = file_open(dir_get_file(filename, NOT_LOCALIZED), "rb");
    if (!fp) {
        return 0;
    }
    int result = 0;
    int version = get_savegame_version(fp);
    if (version && version <= SAVE_GAME_CURRENT_VERSION) {
        result = savegame_read_file_info(fp, info, version);
    }
    file_close(fp);
    return result;
}

int game_file_io_write_saved_game(const char *filename)
{
    init_savegame_data(SAVE_GAME_CURRENT_VERSION);

    log_info("Saving game", filename, 0);
    savegame_save_to_state(&savegame_data.state);

    FILE *fp = file_open(filename, "wb");
    if (!fp) {
        log_error("Unable to save game", 0, 0);
        return 0;
    }
    savegame_write_to_file(fp);
    file_close(fp);
    return 1;
}

int game_file_io_delete_saved_game(const char *filename)
{
    log_info("Deleting game", filename, 0);
    int result = file_remove(filename);
    if (!result) {
        log_error("Unable to delete game", 0, 0);
    }
    return result;
}
