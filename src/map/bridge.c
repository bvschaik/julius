#include "bridge.h"

#include "city/view.h"
#include "core/direction.h"
#include "map/data.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/sprite.h"
#include "map/terrain.h"

static struct {
    int end_grid_offset;
    int length;
    int direction;
    int direction_grid_delta;
} bridge;

int map_bridge_building_length()
{
    return bridge.length;
}

void map_bridge_reset_building_length()
{
    bridge.length = 0;
}

int map_bridge_calculate_length_direction(int x, int y, int *length, int *direction)
{
    int grid_offset = map_grid_offset(x, y);
    bridge.end_grid_offset = 0;
    bridge.direction_grid_delta = 0;
    bridge.length = *length = 0;
    bridge.direction = *direction = 0;

    if (!map_terrain_is(grid_offset, TERRAIN_WATER)) {
        return 0;
    }
    if (map_terrain_is(grid_offset, TERRAIN_ROAD | TERRAIN_BUILDING)) {
        return 0;
    }
    if (map_terrain_count_directly_adjacent_with_type(grid_offset, TERRAIN_WATER) != 3) {
        return 0;
    }
    if (!map_terrain_is(grid_offset + map_grid_delta(0, -1), TERRAIN_WATER)) {
        bridge.direction_grid_delta = map_grid_delta(0, 1);
        bridge.direction = DIR_4_BOTTOM;
    } else if (!map_terrain_is(grid_offset + map_grid_delta(1, 0), TERRAIN_WATER)) {
        bridge.direction_grid_delta = map_grid_delta(-1, 0);
        bridge.direction = DIR_6_LEFT;
    } else if (!map_terrain_is(grid_offset + map_grid_delta(0, 1), TERRAIN_WATER)) {
        bridge.direction_grid_delta = map_grid_delta(0, -1);
        bridge.direction = DIR_0_TOP;
    } else if (!map_terrain_is(grid_offset + map_grid_delta(-1, 0), TERRAIN_WATER)) {
        bridge.direction_grid_delta = map_grid_delta(1, 0);
        bridge.direction = DIR_2_RIGHT;
    } else {
        return 0;
    }
    *direction = bridge.direction;
    bridge.length = 1;
    for (int i = 0; i < 40; i++) {
        grid_offset += bridge.direction_grid_delta;
        bridge.length++;
        int next_offset = grid_offset + bridge.direction_grid_delta;
        if (map_terrain_is(next_offset, TERRAIN_TREE)) {
            break;
        }
        if (!map_terrain_is(next_offset, TERRAIN_WATER)) {
            bridge.end_grid_offset = grid_offset;
            if (map_terrain_count_directly_adjacent_with_type(grid_offset, TERRAIN_WATER) != 3) {
                bridge.end_grid_offset = 0;
            }
            *length = bridge.length;
            return bridge.end_grid_offset;
        }
        if (map_terrain_is(next_offset, TERRAIN_ROAD | TERRAIN_BUILDING)) {
            break;
        }
        if (map_terrain_count_diagonally_adjacent_with_type(grid_offset, TERRAIN_WATER) != 4) {
            break;
        }
    }
    // invalid bridge
    *length = bridge.length;
    return 0;
}

static int get_pillar_distance(int length)
{
    switch (bridge.length) {
        case  9:
        case 10:
            return 4;
        case 11:
        case 12:
            return 5;
        case 13:
        case 14:
            return 6;
        case 15:
        case 16:
            return 7;
        default:
            return 8;
    }
}

int map_bridge_get_sprite_id(int index, int length, int direction, int is_ship_bridge)
{
    int pillar_distance = get_pillar_distance(length);
    if (is_ship_bridge) {
        if (index == 1 || index == length - 2) {
            // platform after ramp
            return 0;
        } else if (index == 0) {
            // ramp at start
            switch (direction) {
                case DIR_0_TOP:
                    return 7;
                case DIR_2_RIGHT:
                    return 8;
                case DIR_4_BOTTOM:
                    return 9;
                case DIR_6_LEFT:
                    return 10;
            }
        } else if (index == length - 1) {
            // ramp at end
            switch (direction) {
                case DIR_0_TOP:
                    return 9;
                case DIR_2_RIGHT:
                    return 10;
                case DIR_4_BOTTOM:
                    return 7;
                case DIR_6_LEFT:
                    return 8;
            }
        } else if (index == pillar_distance) {
            if (direction == DIR_0_TOP || direction == DIR_4_BOTTOM) {
                return 14;
            } else {
                return 15;
            }
        } else {
            // middle of the bridge
            if (direction == DIR_0_TOP || direction == DIR_4_BOTTOM) {
                return 11;
            } else {
                return 12;
            }
        }
    } else {
        if (index == 0) {
            // ramp at start
            switch (direction) {
                case DIR_0_TOP:
                    return 1;
                case DIR_2_RIGHT:
                    return 2;
                case DIR_4_BOTTOM:
                    return 3;
                case DIR_6_LEFT:
                    return 4;
            }
        } else if (index == length - 1) {
            // ramp at end
            switch (direction) {
                case DIR_0_TOP:
                    return 3;
                case DIR_2_RIGHT:
                    return 4;
                case DIR_4_BOTTOM:
                    return 1;
                case DIR_6_LEFT:
                    return 2;
            }
        } else {
            // middle part
            if (direction == DIR_0_TOP || direction == DIR_4_BOTTOM) {
                return 5;
            } else {
                return 6;
            }
        }
    }
    return 0;
}

int map_bridge_add(int x, int y, int is_ship_bridge)
{
    int min_length = is_ship_bridge ? 5 : 2;
    if (bridge.end_grid_offset <= 0 || bridge.length < min_length) {
        bridge.length = 0;
        return bridge.length;
    }

    bridge.direction -= city_view_orientation();
    if (bridge.direction < 0) {
        bridge.direction += 8;
    }

    int grid_offset = map_grid_offset(x, y);
    for (int i = 0; i < bridge.length; i++) {
        map_terrain_add(grid_offset, TERRAIN_ROAD);
        int value = map_bridge_get_sprite_id(i, bridge.length, bridge.direction, is_ship_bridge);
        map_sprite_bridge_set(grid_offset, value);
        grid_offset += bridge.direction_grid_delta;
    }

    map_routing_update_land();
    map_routing_update_water();

    return bridge.length;
}

int map_is_bridge(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_WATER) && map_sprite_bridge_at(grid_offset);
}

static int get_y_bridge_tiles(int grid_offset)
{
    int tiles = 0;
    if (map_is_bridge(grid_offset + map_grid_delta(0, -1))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(0, -2))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(0, 1))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(0, 2))) {
        tiles++;
    }
    return tiles;
}

static int get_x_bridge_tiles(int grid_offset)
{
    int tiles = 0;
    if (map_is_bridge(grid_offset + map_grid_delta(-1, 0))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(-2, 0))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(1, 0))) {
        tiles++;
    }
    if (map_is_bridge(grid_offset + map_grid_delta(2, 0))) {
        tiles++;
    }
    return tiles;
}

void map_bridge_remove(int grid_offset, int mark_deleted)
{
    if (!map_is_bridge(grid_offset)) {
        return;
    }

    int tiles_x = get_x_bridge_tiles(grid_offset);
    int tiles_y = get_y_bridge_tiles(grid_offset);

    int offset_up = tiles_x > tiles_y ? map_grid_delta(1, 0) : map_grid_delta(0, 1);
    // find lower end of the bridge
    while (map_is_bridge(grid_offset - offset_up)) {
        grid_offset -= offset_up;
    }

    if (mark_deleted) {
        map_property_mark_deleted(grid_offset);
    } else {
        map_sprite_clear_tile(grid_offset);
        map_terrain_remove(grid_offset, TERRAIN_ROAD);
    }
    while (map_is_bridge(grid_offset + offset_up)) {
        grid_offset += offset_up;
        if (mark_deleted) {
            map_property_mark_deleted(grid_offset);
        } else {
            map_sprite_clear_tile(grid_offset);
            map_terrain_remove(grid_offset, TERRAIN_ROAD);
        }
    }
}

int map_bridge_count_figures(int grid_offset)
{
    if (!map_is_bridge(grid_offset)) {
        return 0;
    }
    int tiles_x = get_x_bridge_tiles(grid_offset);
    int tiles_y = get_y_bridge_tiles(grid_offset);

    int offset_up = tiles_x > tiles_y ? map_grid_delta(1, 0) : map_grid_delta(0, 1);
    // find lower end of the bridge
    while (map_is_bridge(grid_offset - offset_up)) {
        grid_offset -= offset_up;
    }

    int figures = 0;
    if (map_has_figure_at(grid_offset)) {
        figures = 1;
    }
    map_property_clear_deleted(grid_offset);
    while (map_is_bridge(grid_offset + offset_up)) {
        grid_offset += offset_up;
        map_property_clear_deleted(grid_offset);
        if (map_has_figure_at(grid_offset)) {
            figures++;
        }
    }
    return figures;
}

void map_bridge_update_after_rotate(int counter_clockwise)
{
    int grid_offset = map_data.start_offset;
    for (int y = 0; y < map_data.height; y++, grid_offset += map_data.border_size) {
        for (int x = 0; x < map_data.width; x++, grid_offset++) {
            if (map_is_bridge(grid_offset)) {
                int new_value;
                switch (map_sprite_bridge_at(grid_offset)) {
                    case 1: new_value = counter_clockwise ? 2 : 4; break;
                    case 2: new_value = counter_clockwise ? 3 : 1; break;
                    case 3: new_value = counter_clockwise ? 4 : 2; break;
                    case 4: new_value = counter_clockwise ? 1 : 3; break;
                    case 5: new_value = 6; break;
                    case 6: new_value = 5; break;
                    case 7: new_value = counter_clockwise ? 8 : 10; break;
                    case 8: new_value = counter_clockwise ? 9 : 7; break;
                    case 9: new_value = counter_clockwise ? 10 : 8; break;
                    case 10: new_value = counter_clockwise ? 7 : 9; break;
                    case 11: new_value = 12; break;
                    case 12: new_value = 11; break;
                    case 13: new_value = 13; break;
                    case 14: new_value = 15; break;
                    case 15: new_value = 14; break;
                    default: new_value = map_sprite_bridge_at(grid_offset);
                }
                map_sprite_bridge_set(grid_offset, new_value);
            }
        }
    }
}

int map_bridge_height(int grid_offset)
{
    int sprite = map_sprite_bridge_at(grid_offset);
    if (sprite <= 6) {
        // low bridge
        switch (sprite) {
            case 1:
            case 4:
                return 10;
            case 2:
            case 3:
                return 16;
            default:
                return 20;
        }
    } else {
        // ship bridge
        switch (sprite) {
            case 7:
            case 8:
            case 9:
            case 10:
                return 14;
            case 13:
                return 30;
            default:
                return 36;
        }
    }
}
