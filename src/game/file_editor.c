#include "file_editor.h"

#include "city/view.h"
#include "empire/empire.h"
#include "figure/figure.h"
#include "figuretype/editor.h"
#include "figuretype/water.h"
#include "game/file_io.h"
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
#include "scenario/empire.h"

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

int game_file_editor_load_scenario(const char *scenario_file)
{
    clear_map_data();
    if (!game_file_io_read_scenario(scenario_file)) {
        return 0;
    }
    empire_load(1, scenario_empire_id());

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
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);
    map_natives_init();
    city_view_init();
    map_routing_update_all();
    return 1;
}

int game_file_editor_write_scenario(const char *scenario_file)
{
    return game_file_io_write_scenario(scenario_file);
}
