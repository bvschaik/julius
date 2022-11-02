#include "file_editor.h"

#include "building/construction.h"
#include "building/menu.h"
#include "building/storage.h"
#include "city/data.h"
#include "city/message.h"
#include "city/victory.h"
#include "city/view.h"
#include "core/image.h"
#include "core/image_group_editor.h"
#include "empire/empire.h"
#include "empire/object.h"
#include "figure/enemy_army.h"
#include "figure/figure.h"
#include "figure/formation.h"
#include "figure/name.h"
#include "figure/route.h"
#include "figure/trader.h"
#include "figuretype/editor.h"
#include "figuretype/water.h"
#include "game/animation.h"
#include "game/file_io.h"
#include "game/state.h"
#include "game/time.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/elevation.h"
#include "map/figure.h"
#include "map/image.h"
#include "map/image_context.h"
#include "map/natives.h"
#include "map/property.h"
#include "map/random.h"
#include "map/road_network.h"
#include "map/routing_terrain.h"
#include "map/soldier_strength.h"
#include "map/sprite.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "scenario/distant_battle.h"
#include "scenario/editor.h"
#include "scenario/empire.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/city.h"
#include "sound/music.h"

void game_file_editor_clear_data(void)
{
    city_victory_reset();
    building_construction_clear_type();
    city_data_init();
    city_data_init_scenario();
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
    figure_route_clear_all();
    traders_clear();
    game_time_init(2098);
    scenario_invasion_clear();
}

static void clear_map_data(void)
{
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
    map_terrain_init_outside_map();
    map_random_init();
    map_property_init_alternate_terrain();
}

static void create_blank_map(int size)
{
    scenario_editor_create(size);
    scenario_map_init();
    clear_map_data();
    map_image_init_edges();
    city_view_set_scale(100);
    city_view_set_camera(76, 152);
    city_view_reset_orientation();
}

static void prepare_map_for_editing(void)
{
    image_load_climate(scenario_property_climate(), 1, 0, 0);

    int empire_id = scenario_empire_id();
    empire_load(1, empire_id);
    empire_object_init_cities(empire_id);

    figure_init_scenario();
    figure_create_editor_flags();
    figure_create_flotsam();

    map_tiles_update_all_elevation();
    map_tiles_update_all_water();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_roads();
    map_tiles_update_all_highways();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);
    map_natives_init_editor();
    map_routing_update_all();

    city_view_init();
    game_state_unpause();
}

void game_file_editor_create_scenario(int size)
{
    create_blank_map(size);
    prepare_map_for_editing();
}

int game_file_editor_load_scenario(const char *scenario_file)
{
    clear_map_data();
    if (!game_file_io_read_scenario(scenario_file)) {
        return 0;
    }
    scenario_map_init();

    prepare_map_for_editing();
    return 1;
}

int game_file_editor_write_scenario(const char *scenario_file)
{
    scenario_editor_set_native_images(
        image_group(GROUP_EDITOR_BUILDING_NATIVE),
        image_group(GROUP_EDITOR_BUILDING_NATIVE) + 2,
        image_group(GROUP_EDITOR_BUILDING_CROPS)
    );
    scenario_distant_battle_set_roman_travel_months();
    scenario_distant_battle_set_enemy_travel_months();

    return game_file_io_write_scenario(scenario_file);
}
