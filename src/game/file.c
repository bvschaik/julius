#include "file.h"

#include "building/construction.h"
#include "building/granary.h"
#include "building/maintenance.h"
#include "building/menu.h"
#include "building/monument.h"
#include "building/storage.h"
#include "city/data.h"
#include "city/emperor.h"
#include "city/map.h"
#include "city/message.h"
#include "city/military.h"
#include "city/mission.h"
#include "city/victory.h"
#include "city/view.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image.h"
#include "core/io.h"
#include "core/lang.h"
#include "core/string.h"
#include "empire/empire.h"
#include "empire/trade_prices.h"
#include "figure/enemy_army.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figuretype/animal.h"
#include "figuretype/water.h"
#include "game/animation.h"
#include "game/difficulty.h"
#include "game/file_io.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/time.h"
#include "game/tutorial.h"
#include "game/undo.h"
#include "map/aqueduct.h"
#include "map/bookmark.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/image_context.h"
#include "map/natives.h"
#include "map/orientation.h"
#include "map/property.h"
#include "map/random.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/soldier_strength.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/criteria.h"
#include "scenario/demand_change.h"
#include "scenario/distant_battle.h"
#include "scenario/earthquake.h"
#include "scenario/emperor_change.h"
#include "scenario/empire.h"
#include "scenario/gladiator_revolt.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/price_change.h"
#include "scenario/property.h"
#include "scenario/request.h"
#include "scenario/scenario.h"
#include "sound/city.h"
#include "sound/music.h"

#include <string.h>

static const char MISSION_PACK_FILE[] = "mission1.pak";

static const char MISSION_SAVED_GAMES[][32] = {
    "Citizen.sav",
    "Clerk.sav",
    "Engineer.sav",
    "Architect.sav",
    "Quaestor.sav",
    "Procurator.sav",
    "Aedile.sav",
    "Praetor.sav",
    "Consul.sav",
    "Proconsul.sav",
    "Caesar.sav",
    "Caesar2.sav"
};

static void clear_scenario_data(void)
{
    // clear data
    city_victory_reset();
    building_construction_clear_type();
    city_data_init();
    city_message_init_scenario();
    game_state_init();
    game_animation_init();
    sound_city_init();
    building_menu_enable_all();
    building_clear_all();
    building_storage_clear_all();
    figure_init_scenario();
    enemy_armies_clear();
    figure_name_init();
    formations_clear();
    building_monument_initialize_deliveries();
    figure_route_clear_all();

    game_time_init(2098);

    // clear grids
    map_image_clear();
    map_building_clear();
    map_terrain_clear();
    map_aqueduct_clear();
    map_figure_clear();
    map_property_clear();
    map_sprite_clear();
    map_random_clear();
    map_desirability_clear();
    map_elevation_clear();
    map_soldier_strength_clear();
    map_road_network_clear();

    map_image_context_init();
    map_random_init();
}

static void initialize_scenario_data(const uint8_t *scenario_name)
{
    scenario_set_name(scenario_name);
    scenario_map_init();

    // initialize grids
    map_tiles_init();
    map_tiles_update_all_elevation();
    map_tiles_update_all_water();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    map_tiles_add_entry_exit_flags();
    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_roads();
    map_tiles_update_all_highways();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);

    map_natives_init();

    city_view_init();

    figure_create_fishing_points();
    figure_create_herds();
    figure_create_flotsam();

    map_routing_update_all();

    scenario_map_init_entry_exit();

    map_point entry = scenario_map_entry();
    map_point exit = scenario_map_exit();

    city_map_set_entry_point(entry.x, entry.y);
    city_map_set_exit_point(exit.x, exit.y);

    game_time_init(scenario_property_start_year());

    // set up events
    scenario_earthquake_init();
    scenario_gladiator_revolt_init();
    scenario_emperor_change_init();
    scenario_criteria_init_max_year();

    empire_init_scenario();
    traders_clear();
    scenario_invasion_init();
    city_military_determine_distant_battle_city();
    scenario_request_init();
    scenario_demand_change_init();
    scenario_price_change_init();
    building_menu_update();
    image_load_climate(scenario_property_climate(), 0, 0, 0);
    image_load_enemy(scenario_property_enemy());

    city_data_init_scenario();

    setting_set_default_game_speed();
    game_state_unpause();
}

static void load_empire_data(int is_custom_scenario, int empire_id)
{
    empire_load(is_custom_scenario, empire_id);
    scenario_distant_battle_set_roman_travel_months();
    scenario_distant_battle_set_enemy_travel_months();
}

static int load_scenario_data(const char *scenario_file)
{
    if (!game_file_io_read_scenario(scenario_file)) {
        return 0;
    }

    trade_prices_reset();
    load_empire_data(1, scenario_empire_id());
    city_view_reset_orientation();
    return 1;
}

static int load_custom_scenario(const uint8_t *scenario_name, const char *scenario_file)
{
    if (!file_exists(scenario_file, NOT_LOCALIZED)) {
        return 0;
    }

    clear_scenario_data();
    if (!load_scenario_data(scenario_file)) {
        return 0;
    }
    initialize_scenario_data(scenario_name);
    return 1;
}

/**
 * search for hippodrome buildings, all three pieces should have the same subtype.orientation
 */
static void check_hippodrome_compatibility(building *b)
{
// if we got the middle part of the hippodrome
    if (b->next_part_building_id && b->prev_part_building_id) {
        building *next = building_get(b->next_part_building_id);
        building *prev = building_get(b->prev_part_building_id);
        // if orientation is different, it means that rotation was not available yet in augustus, so it should be set to 0
        if (b->subtype.orientation != next->subtype.orientation || b->subtype.orientation != prev->subtype.orientation) {
            prev->subtype.orientation = 0;
            b->subtype.orientation = 0;
            next->subtype.orientation = 0;
        }
    }
}

static void check_backward_compatibility(void)
{
    for (building *b = building_first_of_type(BUILDING_HIPPODROME); b; b = b->next_of_type) {
        check_hippodrome_compatibility(b);
    }
}

static void initialize_saved_game(void)
{
    load_empire_data(scenario_is_custom(), scenario_empire_id());

    scenario_map_init();

    city_view_init();

    map_routing_update_all();

    map_orientation_update_buildings();
    figure_route_clean();
    map_road_network_update();
    building_maintenance_check_rome_access();
    building_granaries_calculate_stocks();
    building_menu_update();
    city_message_init_problem_areas();

    sound_city_init();

    building_construction_clear_type();
    game_undo_disable();
    game_state_reset_overlay();

    city_mission_tutorial_set_fire_message_shown(1);
    city_mission_tutorial_set_disease_message_shown(1);

    image_load_climate(scenario_property_climate(), 0, 0, 0);
    image_load_enemy(scenario_property_enemy());
    city_military_determine_distant_battle_city();
    map_tiles_determine_gardens();

    map_natives_check_land(0);

    city_message_clear_scroll();

    setting_set_default_game_speed();

    game_state_unpause();
}

static int get_campaign_mission_offset(int mission_id)
{
    uint8_t offset_data[4];
    buffer buf;
    buffer_init(&buf, offset_data, 4);
    if (!io_read_file_part_into_buffer(MISSION_PACK_FILE, NOT_LOCALIZED, offset_data, 4, 4 * mission_id)) {
        return 0;
    }
    return buffer_read_i32(&buf);
}

static int load_campaign_mission(int mission_id)
{
    int offset = get_campaign_mission_offset(mission_id);
    if (offset <= 0) {
        return 0;
    }
    if (!game_file_io_read_saved_game(MISSION_PACK_FILE, offset)) {
        return 0;
    }

    if (mission_id == 0) {
        scenario_set_player_name(setting_player_name());
    } else {
        scenario_restore_campaign_player_name();
    }
    initialize_saved_game();
    scenario_fix_patch_trade(mission_id);

    city_data_init_campaign_mission();
    return 1;
}

static int start_scenario(const uint8_t *scenario_name, const char *scenario_file)
{
    int mission = scenario_campaign_mission();
    int rank = scenario_campaign_rank();
    map_bookmarks_clear();
    if (scenario_is_custom()) {
        if (!load_custom_scenario(scenario_name, scenario_file)) {
            return 0;
        }
        scenario_set_player_name(setting_player_name());
    } else {
        if (!load_campaign_mission(mission)) {
            return 0;
        }
    }
    scenario_set_campaign_mission(mission);
    scenario_set_campaign_rank(rank);

    if (scenario_is_tutorial_1()) {
        setting_set_personal_savings_for_mission(0, 0);
    }

    scenario_settings_init_mission();

    city_emperor_init_scenario(rank);

    tutorial_init();

    building_menu_update();
    city_message_init_scenario();
    return 1;
}

static const char *get_scenario_filename(const uint8_t *scenario_name, int decomposed)
{
    static char filename[FILE_NAME_MAX];
    encoding_to_utf8(scenario_name, filename, FILE_NAME_MAX, decomposed);
    if (!file_has_extension(filename, "map")) {
        file_append_extension(filename, "map");
    }
    return filename;
}

int game_file_start_scenario_by_name(const uint8_t *scenario_name)
{
    if (start_scenario(scenario_name, get_scenario_filename(scenario_name, 0))) {
        return 1;
    } else {
        return start_scenario(scenario_name, get_scenario_filename(scenario_name, 1));
    }
}

int game_file_start_scenario(const char *scenario_file)
{
    uint8_t scenario_name[FILE_NAME_MAX];
    encoding_from_utf8(scenario_file, scenario_name, FILE_NAME_MAX);
    file_remove_extension((char *) scenario_name);
    scenario_set_custom(2);
    return start_scenario(scenario_name, scenario_file);
}

int game_file_load_saved_game(const char *filename)
{
    int result = game_file_io_read_saved_game(filename, 0);
    if (result != 1) {
        return result;
    }
    check_backward_compatibility();
    initialize_saved_game();
    building_storage_reset_building_ids();

    sound_music_update(1);
    return 1;
}

int game_file_write_saved_game(const char *filename)
{
    return game_file_io_write_saved_game(filename);
}

int game_file_delete_saved_game(const char *filename)
{
    return game_file_io_delete_saved_game(filename);
}

void game_file_write_mission_saved_game(void)
{
    int rank = scenario_campaign_rank();
    if (rank < 0) {
        rank = 0;
    } else if (rank > 11) {
        rank = 11;
    }
    const char *filename = MISSION_SAVED_GAMES[rank];
    char localized_filename[FILE_NAME_MAX];
    if (locale_translate_rank_autosaves()) {
        encoding_to_utf8(lang_get_string(32, rank), localized_filename, FILE_NAME_MAX,
            encoding_system_uses_decomposed());
        strcat(localized_filename, ".svx");
        filename = localized_filename;
    }
    if (city_mission_should_save_start() && !file_exists(filename, NOT_LOCALIZED)) {
        game_file_io_write_saved_game(filename);
    }
}
