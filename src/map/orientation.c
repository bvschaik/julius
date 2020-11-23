#include "orientation.h"

#include "building/rotation.h"
#include "building/properties.h"
#include "city/view.h"
#include "core/direction.h"
#include "core/image.h"
#include "figuretype/animal.h"
#include "figuretype/wall.h"
#include "game/undo.h"
#include "map/bridge.h"
#include "map/building_tiles.h"
#include "map/data.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"
#include "map/tiles.h"
#include "map/water.h"
#include "mods/mods.h"

#include <math.h>
#include <stdlib.h>


static void determine_leftmost_tile(void)
{
    int orientation = city_view_orientation();
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            int size = map_property_multi_tile_size(grid_offset);
            if (size == 1) {
                map_property_mark_draw_tile(grid_offset);
                continue;
            }
            map_property_clear_draw_tile(grid_offset);
            int dx = orientation == DIR_4_BOTTOM || orientation == DIR_6_LEFT ? size - 1 : 0;
            int dy = orientation == DIR_0_TOP || orientation == DIR_6_LEFT ? size - 1 : 0;
            if (map_property_is_multi_tile_xy(grid_offset, dx, dy)) {
                map_property_mark_draw_tile(grid_offset);
            }
        }
    }
}

void map_orientation_change(int counter_clockwise)
{
    map_tiles_remove_entry_exit_flags();
    game_undo_disable();
    determine_leftmost_tile();

    map_tiles_update_all_elevation();
    map_tiles_update_all_water();
    map_tiles_update_all_earthquake();
    map_tiles_update_all_rocks();
    map_tiles_update_all_gardens();

    map_tiles_add_entry_exit_flags();

    map_tiles_update_all_empty_land();
    map_tiles_update_all_meadow();
    map_tiles_update_all_rubble();
    map_tiles_update_all_roads();
    map_tiles_update_all_plazas();
    map_tiles_update_all_walls();
    map_tiles_update_all_aqueducts(0);
    map_tiles_update_all_hedges();


    map_orientation_update_buildings();
    map_bridge_update_after_rotate(counter_clockwise);
    map_routing_update_walls();

    figure_tower_sentry_reroute();
    figure_hippodrome_horse_reroute();
}

int map_orientation_for_gatehouse(int x, int y)
{
    switch (city_view_orientation()) {
        case DIR_2_RIGHT: x--; break;
        case DIR_4_BOTTOM: x--; y--; break;
        case DIR_6_LEFT: y--; break;
    }
    int grid_offset = map_grid_offset(x, y);
    int num_road_tiles_within = 0;
    int road_tiles_within_flags = 0;
    // tiles within gate, flags:
    // 1  2
    // 4  8
    if (map_terrain_is(map_grid_offset(x, y), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 1;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 2;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 4;
        num_road_tiles_within++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, 1), TERRAIN_ROAD)) {
        road_tiles_within_flags |= 8;
        num_road_tiles_within++;
    }

    if (num_road_tiles_within != 2 && num_road_tiles_within != 4) {
        return 0;
    }
    if (num_road_tiles_within == 2) {
        if (road_tiles_within_flags == 6 || road_tiles_within_flags == 9) { // diagonals
            return 0;
        }
        if (road_tiles_within_flags == 5 || road_tiles_within_flags == 10) { // top to bottom
            return 1;
        }
        if (road_tiles_within_flags == 3 || road_tiles_within_flags == 12) { // left to right
            return 2;
        }
        return 0;
    }
    // all 4 tiles are road: check adjacent roads
    int num_road_tiles_top = 0;
    int num_road_tiles_right = 0;
    int num_road_tiles_bottom = 0;
    int num_road_tiles_left = 0;
    // top
    if (map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_ROAD)) {
        num_road_tiles_top++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, -1), TERRAIN_ROAD)) {
        num_road_tiles_top++;
    }
    // bottom
    if (map_terrain_is(grid_offset + map_grid_delta(0, 2), TERRAIN_ROAD)) {
        num_road_tiles_bottom++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(1, 2), TERRAIN_ROAD)) {
        num_road_tiles_bottom++;
    }
    // left
    if (map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_ROAD)) {
        num_road_tiles_left++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(-1, 1), TERRAIN_ROAD)) {
        num_road_tiles_left++;
    }
    // right
    if (map_terrain_is(grid_offset + map_grid_delta(2, 0), TERRAIN_ROAD)) {
        num_road_tiles_right++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(2, 1), TERRAIN_ROAD)) {
        num_road_tiles_right++;
    }
    // determine direction
    if (num_road_tiles_top || num_road_tiles_bottom) {
        if (num_road_tiles_left || num_road_tiles_right) {
            return 0;
        }
        return 1;
    } else if (num_road_tiles_left || num_road_tiles_right) {
        return 2;
    }
    return 0;
}

int map_orientation_for_triumphal_arch(int x, int y)
{
    switch (city_view_orientation()) {
        case DIR_2_RIGHT: x -= 2; break;
        case DIR_4_BOTTOM: x -= 2; y -= 2; break;
        case DIR_6_LEFT: y -= 2; break;
    }
    int num_road_tiles_top_bottom = 0;
    int num_road_tiles_left_right = 0;
    int num_blocked_tiles = 0;

    int grid_offset = map_grid_offset(x, y);
    // check corner tiles
    if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(2, 0), TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(0, 2), TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    if (map_terrain_is(grid_offset + map_grid_delta(2, 2), TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    // road tiles top to bottom
    int top_offset = grid_offset + map_grid_delta(1, 0);
    if ((map_terrain_get(top_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        num_road_tiles_top_bottom++;
    } else if (map_terrain_is(top_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    int bottom_offset = grid_offset + map_grid_delta(1, 2);
    if ((map_terrain_get(bottom_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        num_road_tiles_top_bottom++;
    } else if (map_terrain_is(bottom_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    // road tiles left to right
    int left_offset = grid_offset + map_grid_delta(0, 1);
    if ((map_terrain_get(left_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        num_road_tiles_left_right++;
    } else if (map_terrain_is(left_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    int right_offset = grid_offset + map_grid_delta(2, 1);
    if ((map_terrain_get(right_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        num_road_tiles_left_right++;
    } else if (map_terrain_is(right_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    // center tile
    int center_offset = grid_offset + map_grid_delta(2, 1);
    if ((map_terrain_get(center_offset) & TERRAIN_NOT_CLEAR) == TERRAIN_ROAD) {
        // do nothing
    } else if (map_terrain_is(center_offset, TERRAIN_NOT_CLEAR)) {
        num_blocked_tiles++;
    }
    // judgement time
    if (num_blocked_tiles) {
        return 0;
    }
    if (!num_road_tiles_left_right && !num_road_tiles_top_bottom) {
        return 0; // no road: can't determine direction
    }
    if (num_road_tiles_top_bottom == 2 && !num_road_tiles_left_right) {
        return 1;
    }
    if (num_road_tiles_left_right == 2 && !num_road_tiles_top_bottom) {
        return 2;
    }
    return 0;
}

void map_orientation_update_buildings(void)
{
    int map_orientation = city_view_orientation();
    int orientation_is_top_bottom = map_orientation == DIR_0_TOP || map_orientation == DIR_4_BOTTOM;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building *b = building_get(i);
        if (b->state == BUILDING_STATE_UNUSED) {
            continue;
        }
        int image_id;
        int image_offset;
        switch (b->type) {
            case BUILDING_GATEHOUSE:
                if (b->subtype.orientation == 1) {
                    if (orientation_is_top_bottom) {
                        image_id = image_group(GROUP_BUILDING_TOWER) + 1;
                    } else {
                        image_id = image_group(GROUP_BUILDING_TOWER) + 2;
                    }
                } else {
                    if (orientation_is_top_bottom) {
                        image_id = image_group(GROUP_BUILDING_TOWER) + 2;
                    } else {
                        image_id = image_group(GROUP_BUILDING_TOWER) + 1;
                    }
                }
                map_building_tiles_add(i, b->x, b->y, b->size, image_id, TERRAIN_GATEHOUSE | TERRAIN_BUILDING);
                map_terrain_add_gatehouse_roads(b->x, b->y, 0);
                break;
            case BUILDING_TRIUMPHAL_ARCH:
                if (b->subtype.orientation == 1) {
                    if (orientation_is_top_bottom) {
                        image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                    } else {
                        image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                    }
                } else {
                    if (orientation_is_top_bottom) {
                        image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH) + 2;
                    } else {
                        image_id = image_group(GROUP_BUILDING_TRIUMPHAL_ARCH);
                    }
                }
                map_building_tiles_add(i, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
                map_terrain_add_triumphal_arch_roads(b->x, b->y, b->subtype.orientation);
                break;
            case BUILDING_HIPPODROME:
            {
                int phase = b->data.monument.monument_phase;
                int phase_offset = 6;
                int image1 = 0;
                int image2 = 0;

                if (phase == -1) {
                    image1 = image_group(GROUP_BUILDING_HIPPODROME_1);
                    image2 = image_group(GROUP_BUILDING_HIPPODROME_2);
                }
                else {
                    image1 = mods_get_image_id(mods_get_group_id("Areldir", "Circus"), "Circus NESW 01") + ((phase - 1) * phase_offset);
                    image2 = mods_get_image_id(mods_get_group_id("Areldir", "Circus"), "Circus NWSE 01") + ((phase - 1) * phase_offset);
                }

                int building_part;
                if(b->prev_part_building_id == 0){
                    building_part = 0; // part 1, no previous building
                } else if(b->next_part_building_id == 0){
                    building_part = 2; // part 3, no next building
                } else {
                    building_part = 1; // part 2
                }
                map_orientation = building_rotation_get_building_orientation(b->subtype.orientation);
                if (map_orientation == DIR_0_TOP) {
                    image_id = image2;
                    switch (building_part) {
                        case 0: image_id += 0; break; // part 1
                        case 1: image_id += 2; break; // part 2
                        case 2: image_id += 4; break; // part 3, same for switch cases below
                    }
                } else if (map_orientation == DIR_4_BOTTOM) {
                    image_id = image2;
                    switch (building_part) {
                        case 0: image_id += 4; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 0; break;
                    }
                } else if (map_orientation == DIR_6_LEFT) {
                    image_id = image1;
                    switch (building_part) {
                        case 0: image_id += 0; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 4; break;
                    }
                } else { // DIR_2_RIGHT
                    image_id = image1;
                    switch (building_part) {
                        case 0: image_id += 4; break;
                        case 1: image_id += 2; break;
                        case 2: image_id += 0; break;
                    }
                }
                map_building_tiles_add(i, b->x, b->y, b->size, image_id, TERRAIN_BUILDING);
                break;
            }
            case BUILDING_SHIPYARD:
                image_offset = (4 + b->data.industry.orientation - map_orientation / 2) % 4;
                image_id = image_group(GROUP_BUILDING_SHIPYARD) + image_offset;
                map_water_add_building(i, b->x, b->y, 2, image_id);
                break;
            case BUILDING_WHARF:
                image_offset = (4 + b->data.industry.orientation - map_orientation / 2) % 4;
                image_id = image_group(GROUP_BUILDING_WHARF) + image_offset;
                map_water_add_building(i, b->x, b->y, 2, image_id);
                break;
            case BUILDING_DOCK:
                image_offset = (4 + b->data.dock.orientation - map_orientation / 2) % 4;
                switch (image_offset) {
                    case 0: image_id = image_group(GROUP_BUILDING_DOCK_1); break;
                    case 1: image_id = image_group(GROUP_BUILDING_DOCK_2); break;
                    case 2: image_id = image_group(GROUP_BUILDING_DOCK_3); break;
                    default:image_id = image_group(GROUP_BUILDING_DOCK_4); break;
                }
                map_water_add_building(i, b->x, b->y, 3, image_id);
                break;
        }
        if (b->type >= BUILDING_PINE_PATH && b->type <= BUILDING_DATE_PATH) {
            image_id = mods_get_group_id("Areldir", "Aesthetics") + (b->type - BUILDING_PINE_TREE) + (abs((b->subtype.orientation - (map_orientation / 2) % 2)) * PATH_ROTATE_OFFSET);
            map_building_tiles_add(i, b->x, b->y, 1, image_id, TERRAIN_BUILDING);
        }
        if (b->type >= BUILDING_SMALL_STATUE_ALT && b->type <= BUILDING_SMALL_STATUE_ALT_B) {
            int rotation_offset = building_properties_for_type(b->type)->rotation_offset;
            image_id = mods_get_image_id(mods_get_group_id("Areldir", "Aesthetics"), "sml statue 2") + (b->type - BUILDING_SMALL_STATUE_ALT)
                + (abs((b->subtype.orientation - (map_orientation / 2) % 2)) * rotation_offset);
            map_building_tiles_add(i, b->x, b->y, 1, image_id, TERRAIN_BUILDING);
        }
        if (b->type == BUILDING_LEGION_STATUE) {
            image_id = mods_get_image_id(mods_get_group_id("Areldir", "Aesthetics"), "legio statue") + (abs((b->subtype.orientation - (map_orientation / 2) % 2)));
            map_building_tiles_add(i, b->x, b->y, 2, image_id, TERRAIN_BUILDING);
        }
        if (b->type == BUILDING_DECORATIVE_COLUMN) {
            image_id = mods_get_image_id(mods_get_group_id("Areldir", "Aesthetics"), "sml col B") + (abs((b->subtype.orientation - (map_orientation / 2) % 2)));
            map_building_tiles_add(i, b->x, b->y, 2, image_id, TERRAIN_BUILDING);
        }
    }
}
