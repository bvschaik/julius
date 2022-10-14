#include "construction_building.h"

#include "assets/assets.h"
#include "building/building.h"
#include "building/building_variant.h"
#include "building/construction.h"
#include "building/construction_warning.h"
#include "building/count.h"
#include "building/distribution.h"
#include "building/dock.h"
#include "building/image.h"
#include "building/menu.h"
#include "building/monument.h"
#include "building/properties.h"
#include "building/rotation.h"
#include "building/storage.h"
#include "city/buildings.h"
#include "city/view.h"
#include "city/warning.h"
#include "core/config.h"
#include "core/image.h"
#include "core/random.h"
#include "empire/city.h"
#include "figure/formation_legion.h"
#include "game/undo.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/orientation.h"
#include "map/routing.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "scenario/property.h"

static void add_fort(int type, building *fort)
{
    fort->prev_part_building_id = 0;
    map_building_tiles_add(fort->id, fort->x, fort->y, fort->size, building_image_get(fort), TERRAIN_BUILDING);
    if (type == BUILDING_FORT_LEGIONARIES) {
        fort->subtype.fort_figure_type = FIGURE_FORT_LEGIONARY;
    } else if (type == BUILDING_FORT_JAVELIN) {
        fort->subtype.fort_figure_type = FIGURE_FORT_JAVELIN;
    } else if (type == BUILDING_FORT_MOUNTED) {
        fort->subtype.fort_figure_type = FIGURE_FORT_MOUNTED;
    }

    // create parade ground
    const int offsets_x[] = { 3, -1, -4, 0 };
    const int offsets_y[] = { -1, -4, 0, 3 };
    int id = fort->id;
    building *ground = building_create(BUILDING_FORT_GROUND, fort->x + offsets_x[building_rotation_get_rotation()], fort->y + offsets_y[building_rotation_get_rotation()]);
    game_undo_add_building(ground);
    fort = building_get(id);
    ground->prev_part_building_id = fort->id;
    fort->next_part_building_id = ground->id;
    ground->next_part_building_id = 0;
    map_building_tiles_add(ground->id, fort->x + offsets_x[building_rotation_get_rotation()], fort->y + offsets_y[building_rotation_get_rotation()], 4,
        image_group(GROUP_BUILDING_FORT) + 1, TERRAIN_BUILDING);

    fort->formation_id = formation_legion_create_for_fort(fort);
    ground->formation_id = fort->formation_id;
}


static void add_hippodrome(building *b)
{
    building *part1 = b;

    int x_offset, y_offset;
    building_rotation_get_offset_with_rotation(5, building_rotation_get_rotation(), &x_offset, &y_offset);
    building *part2 = building_create(BUILDING_HIPPODROME, part1->x + x_offset, part1->y + y_offset);
    game_undo_add_building(part2);

    building_rotation_get_offset_with_rotation(10, building_rotation_get_rotation(), &x_offset, &y_offset);
    building *part3 = building_create(BUILDING_HIPPODROME, part1->x + x_offset, part1->y + y_offset);
    game_undo_add_building(part3);

    part1->prev_part_building_id = 0;
    part1->next_part_building_id = part2->id;
    part2->prev_part_building_id = part1->id;
    part2->next_part_building_id = part3->id;
    part3->prev_part_building_id = part2->id;
    part3->next_part_building_id = 0;
}

static int add_warehouse_space(int x, int y, int prev_id)
{
    building *b = building_create(BUILDING_WAREHOUSE_SPACE, x, y);
    game_undo_add_building(b);
    building *prev = building_get(prev_id);
    b->prev_part_building_id = prev->id;
    prev->next_part_building_id = b->id;
    map_building_tiles_add(b->id, x, y, 1,
        image_group(GROUP_BUILDING_WAREHOUSE_STORAGE_EMPTY), TERRAIN_BUILDING);
    return b->id;
}

static void add_warehouse(building *b)
{
    int x_offset[9] = { 0, 0, 1, 1, 0, 2, 1, 2, 2 };
    int y_offset[9] = { 0, 1, 0, 1, 2, 0, 2, 1, 2 };
    int corner = building_rotation_get_corner(2 * building_rotation_get_rotation());

    b->storage_id = building_storage_create();
    if (config_get(CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT)) {
        building_storage_accept_none(b->storage_id);
    }
    b->prev_part_building_id = 0;
    map_building_tiles_add(b->id, b->x + x_offset[corner], b->y + y_offset[corner], 1,
        image_group(GROUP_BUILDING_WAREHOUSE), TERRAIN_BUILDING);

    int id = b->id;
    int prev = id;
    for (int i = 0; i < 9; i++) {
        if (i == corner) {
            continue;
        }
        prev = add_warehouse_space(b->x + x_offset[i], b->y + y_offset[i], prev);
    }
    b = building_get(id);
    // adjust BUILDING_WAREHOUSE
    b->x = b->x + x_offset[corner];
    b->y = b->y + y_offset[corner];
    b->grid_offset = map_grid_offset(b->x, b->y);
    game_undo_adjust_building(b);

    building_get(prev)->next_part_building_id = 0;
}

static void add_building(building *b)
{
    int image_id = building_image_get(b);
    if (image_id) {
        map_building_tiles_add(b->id, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
    }
}

static void add_to_map(int type, building *b, int size,
    int orientation, int waterside_orientation_abs, int waterside_orientation_rel)
{
    if (building_variant_has_variants(b->type)) {
        b->variant = building_rotation_get_rotation_with_limit(building_variant_get_number_of_variants(b->type));
    }
    switch (type) {
        default:
            add_building(b);
            break;
        // entertainment
        case BUILDING_COLOSSEUM:
            map_tiles_update_area_roads(b->x, b->y, 5);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        // farms
        case BUILDING_WHEAT_FARM:
        case BUILDING_VEGETABLE_FARM:
        case BUILDING_FRUIT_FARM:
        case BUILDING_OLIVE_FARM:
        case BUILDING_VINES_FARM:
        case BUILDING_PIG_FARM:
            map_building_tiles_add_farm(b->id, b->x, b->y,
                image_group(GROUP_BUILDING_FARM_CROPS) + 5 * (b->output_resource_id - 1), 0);
            break;
        // distribution
        case BUILDING_GRANARY:
            b->storage_id = building_storage_create();
            if (config_get(CONFIG_GP_CH_WAREHOUSES_DONT_ACCEPT)) {
                building_storage_accept_none(b->storage_id);
            }
            add_building(b);
            map_tiles_update_area_roads(b->x, b->y, 5);
            break;
        // Don't autodistribute wine for new Venus temples
        case BUILDING_SMALL_TEMPLE_VENUS:
            add_building(b);
            building_distribution_unaccept_all_goods(b);
            break;
        case BUILDING_LARGE_TEMPLE_VENUS:
            map_tiles_update_area_roads(b->x, b->y, 5);
            building_monument_set_phase(b, MONUMENT_START);
            building_distribution_unaccept_all_goods(b);
            break;
        case BUILDING_LARGE_TEMPLE_CERES:
        case BUILDING_LARGE_TEMPLE_NEPTUNE:
        case BUILDING_LARGE_TEMPLE_MERCURY:
        case BUILDING_LARGE_TEMPLE_MARS:
            map_tiles_update_area_roads(b->x, b->y, 5);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_ORACLE:
            map_tiles_update_area_roads(b->x, b->y, 2);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_ROADBLOCK:
            add_building(b);
            map_terrain_add_roadblock_road(b->x, b->y);
            map_tiles_update_area_roads(b->x, b->y, 5);
            map_tiles_update_all_plazas();
            break;
        case BUILDING_SHIPYARD:
        case BUILDING_WHARF:
            b->data.industry.orientation = waterside_orientation_abs;
            map_water_add_building(b->id, b->x, b->y, 2);
            break;
        case BUILDING_DOCK:
            city_buildings_add_dock();
            b->data.dock.orientation = waterside_orientation_abs;
            map_water_add_building(b->id, b->x, b->y, size);
            break;
        case BUILDING_TOWER:
            map_terrain_remove_with_radius(b->x, b->y, 2, 0, TERRAIN_WALL);
            map_building_tiles_add(b->id, b->x, b->y, size, building_image_get(b),
                TERRAIN_BUILDING | TERRAIN_GATEHOUSE);
            map_tiles_update_area_walls(b->x, b->y, 5);
            break;
        case BUILDING_GATEHOUSE:
            b->subtype.orientation = orientation;
            map_building_tiles_add_remove(b->id, b->x, b->y, size,
                building_image_get(b), TERRAIN_BUILDING | TERRAIN_GATEHOUSE, TERRAIN_CLEARABLE & ~TERRAIN_HIGHWAY);
            map_orientation_update_buildings();
            map_terrain_add_gatehouse_roads(b->x, b->y, orientation);
            map_tiles_update_area_roads(b->x, b->y, 5);
            map_tiles_update_area_highways(b->x, b->y, 3);
            map_tiles_update_all_plazas();
            map_tiles_update_area_walls(b->x, b->y, 5);
            break;
        case BUILDING_TRIUMPHAL_ARCH:
            b->subtype.orientation = orientation;
            add_building(b);
            map_orientation_update_buildings();
            map_terrain_add_triumphal_arch_roads(b->x, b->y, orientation);
            map_tiles_update_area_roads(b->x, b->y, 5);
            map_tiles_update_all_plazas();
            city_buildings_build_triumphal_arch();
            building_menu_update();
            building_construction_clear_type();
            break;
        case BUILDING_SENATE_UPGRADED:
            add_building(b);
            city_buildings_add_senate(b);
            break;
        case BUILDING_BARRACKS:
            add_building(b);
            city_buildings_add_barracks(b);
            break;
        case BUILDING_WAREHOUSE:
            add_warehouse(b);
            break;
        case BUILDING_HIPPODROME:
            add_hippodrome(b);
            building_monument_set_phase(b, MONUMENT_START);
            building *b2 = building_get(b->next_part_building_id);
            building_monument_set_phase(b2, MONUMENT_START);
            building *b3 = building_get(b2->next_part_building_id);
            building_monument_set_phase(b3, MONUMENT_START);
            break;
        case BUILDING_FORT_LEGIONARIES:
        case BUILDING_FORT_JAVELIN:
        case BUILDING_FORT_MOUNTED:
            add_fort(type, b);
            break;
        // distribution center (also unused)
        case BUILDING_DISTRIBUTION_CENTER_UNUSED:
            city_buildings_add_distribution_center(b);
            break;
        case BUILDING_GRAND_TEMPLE_CERES:
        case BUILDING_GRAND_TEMPLE_NEPTUNE:
        case BUILDING_GRAND_TEMPLE_MERCURY:
        case BUILDING_GRAND_TEMPLE_MARS:
        case BUILDING_GRAND_TEMPLE_VENUS:
        case BUILDING_PANTHEON:
            map_tiles_update_area_roads(b->x, b->y, 9);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_LIGHTHOUSE:
            map_tiles_update_area_roads(b->x, b->y, 5);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_MESS_HALL:
            b->data.market.is_mess_hall = 1;
            city_buildings_add_mess_hall(b);
            add_building(b);
            break;
        case BUILDING_SMALL_STATUE:
        case BUILDING_MEDIUM_STATUE:
        case BUILDING_HORSE_STATUE:
        case BUILDING_LEGION_STATUE:
        case BUILDING_GLADIATOR_STATUE:
            b->subtype.orientation = building_rotation_get_rotation();
            add_building(b);
            break;
        case BUILDING_SMALL_MAUSOLEUM:
            b->subtype.orientation = building_rotation_get_rotation();
            map_tiles_update_area_roads(b->x, b->y, 4);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_LARGE_MAUSOLEUM:
        case BUILDING_NYMPHAEUM:
            map_tiles_update_area_roads(b->x, b->y, 5);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_CARAVANSERAI:
            city_buildings_add_caravanserai(b);
            map_tiles_update_area_roads(b->x, b->y, 4);
            building_monument_set_phase(b, MONUMENT_START);
            break;
        case BUILDING_HIGHWAY:
            add_building(b);
            break;
    }
    map_routing_update_land();
    map_routing_update_walls();
}

int building_construction_place_building(building_type type, int x, int y)
{
    int terrain_mask = TERRAIN_ALL;
    if (type == BUILDING_GATEHOUSE || type == BUILDING_TRIUMPHAL_ARCH || building_type_is_roadblock(type)) {
        terrain_mask = ~TERRAIN_ROAD & ~TERRAIN_HIGHWAY;
    } else if (type == BUILDING_TOWER) {
        terrain_mask = ~TERRAIN_WALL;
    }
    int size = building_properties_for_type(type)->size;
    if (type == BUILDING_WAREHOUSE) {
        size = 3;
    }
    int building_orientation = 0;
    if (type == BUILDING_GATEHOUSE) {
        building_orientation = map_orientation_for_gatehouse(x, y);
    } else if (type == BUILDING_TRIUMPHAL_ARCH) {
        building_orientation = map_orientation_for_triumphal_arch(x, y);
    }
    building_construction_offset_start_from_orientation(&x, &y, size);
    // extra checks
    if (type == BUILDING_GATEHOUSE) {
        if (!map_tiles_are_clear(x, y, size, terrain_mask)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
        if (!building_orientation) {
            if (building_rotation_get_road_orientation() == 1) {
                building_orientation = 1;
            } else {
                building_orientation = 2;
            }
        }
    }
    if (type == BUILDING_ROADBLOCK) {
        if (map_tiles_are_clear(x, y, size, TERRAIN_ROAD)) {
            return 0;
        }
    }
    if (type == BUILDING_TRIUMPHAL_ARCH) {
        if (!map_tiles_are_clear(x, y, size, terrain_mask)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
        if (!building_orientation) {
            if (building_rotation_get_road_orientation() == 1) {
                building_orientation = 1;
            } else {
                building_orientation = 3;
            }
        }
    }
    int waterside_orientation_abs = 0, waterside_orientation_rel = 0;
    if (type == BUILDING_SHIPYARD || type == BUILDING_WHARF) {
        if (map_water_determine_orientation_size2(
            x, y, 0, &waterside_orientation_abs, &waterside_orientation_rel)) {
            city_warning_show(WARNING_SHORE_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
    } else if (type == BUILDING_DOCK) {
        if (map_water_determine_orientation_size3(
            x, y, 0, &waterside_orientation_abs, &waterside_orientation_rel)) {
            city_warning_show(WARNING_SHORE_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
        if (!building_dock_is_connected_to_open_water(x, y)) {
            city_warning_show(WARNING_DOCK_OPEN_WATER_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
    } else {
        if (!map_tiles_are_clear(x, y, size, terrain_mask)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
        int warning_id;
        if (!building_construction_can_place_on_terrain(x, y, &warning_id)) {
            city_warning_show(warning_id, NEW_WARNING_SLOT);
            return 0;
        }
    }
    if (building_is_fort(type)) {
        const int offsets_x[] = { 3, -1, -4, 0 };
        const int offsets_y[] = { -1, -4, 0, 3 };
        int orient_index = building_rotation_get_rotation();
        int x_offset = offsets_x[orient_index];
        int y_offset = offsets_y[orient_index];
        if (!map_tiles_are_clear(x + x_offset, y + y_offset, 4, terrain_mask)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
        if (formation_get_num_legions_cached() >= formation_get_max_legions()) {
            city_warning_show(WARNING_MAX_LEGIONS_REACHED, NEW_WARNING_SLOT);
            return 0;
        }
        if (!city_buildings_has_mess_hall()) {
            city_warning_show(WARNING_NO_MESS_HALL, NEW_WARNING_SLOT);
            return 0;
        }
    }

    if (building_monument_type_is_monument(type)) {
        if (!empire_has_access_to_resource(RESOURCE_CLAY) ||
            !empire_has_access_to_resource(RESOURCE_TIMBER) ||
            !empire_has_access_to_resource(RESOURCE_MARBLE)) {
            city_warning_show(WARNING_RESOURCES_NOT_AVAILABLE, NEW_WARNING_SLOT);
            return 0;
        }
    }

    if (building_monument_get_id(type) && !building_monument_type_is_mini_monument(type)) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
        return 0;
    }

    if (building_monument_is_grand_temple(type) &&
        building_monument_count_grand_temples() >= config_get(CONFIG_GP_CH_MAX_GRAND_TEMPLES)) {
        city_warning_show(WARNING_MAX_GRAND_TEMPLES, NEW_WARNING_SLOT);
        return 0;
    }
    if (type == BUILDING_COLOSSEUM) {
        if (building_count_colosseum()) {
            city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
            return 0;
        }
    }
    if (type == BUILDING_HIPPODROME) {
        if (city_buildings_has_hippodrome()) {
            city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
            return 0;
        }
        int x_offset_1, y_offset_1;
        building_rotation_get_offset_with_rotation(5, building_rotation_get_rotation(), &x_offset_1, &y_offset_1);
        int x_offset_2, y_offset_2;
        building_rotation_get_offset_with_rotation(10, building_rotation_get_rotation(), &x_offset_2, &y_offset_2);
        if (!map_tiles_are_clear(x + x_offset_1, y + y_offset_1, 5, terrain_mask) ||
            !map_tiles_are_clear(x + x_offset_2, y + y_offset_2, 5, terrain_mask)) {
            city_warning_show(WARNING_CLEAR_LAND_NEEDED, NEW_WARNING_SLOT);
            return 0;
        }
    }
    if (type == BUILDING_SENATE_UPGRADED && city_buildings_has_senate()) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
        return 0;
    }
    if (type == BUILDING_CARAVANSERAI && city_buildings_has_caravanserai()) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
        return 0;
    }
    if (type == BUILDING_BARRACKS && city_buildings_has_barracks() && !config_get(CONFIG_GP_CH_MULTIPLE_BARRACKS)) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
        return 0;
    }
    if (type == BUILDING_MESS_HALL && city_buildings_has_mess_hall()) {
        city_warning_show(WARNING_ONE_BUILDING_OF_TYPE, NEW_WARNING_SLOT);
        return 0;
    }
    building_construction_warning_check_all(type, x, y, size);


    // phew, checks done!
    building *b;
    if (building_is_fort(type)) {
        b = building_create(BUILDING_FORT, x, y);
    } else {
        b = building_create(type, x, y);
    }
    game_undo_add_building(b);
    if (b->id <= 0) {
        return 0;
    }
    add_to_map(type, b, size, building_orientation, waterside_orientation_abs, waterside_orientation_rel);
    return 1;
}
