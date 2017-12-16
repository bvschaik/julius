#include "bridge.h"

#include "core/direction.h"
#include "map/figure.h"
#include "map/grid.h"
#include "map/property.h"
#include "map/routing_terrain.h"
#include "map/terrain.h"

#include "Data/State.h"
#include "../Terrain.h"

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

int map_bridge_calculate_length_direction(int x, int y, int is_ship_bridge, int *length, int *direction)
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
    if (Terrain_countTerrainTypeDirectlyAdjacentTo(grid_offset, TERRAIN_WATER) != 3) {
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
            if (Terrain_countTerrainTypeDirectlyAdjacentTo(grid_offset, TERRAIN_WATER) != 3) {
                bridge.end_grid_offset = 0;
            }
            *length = bridge.length;
            return bridge.end_grid_offset;
        }
        if (map_terrain_is(next_offset, TERRAIN_ROAD | TERRAIN_BUILDING)) {
            break;
        }
        if (Terrain_countTerrainTypeDiagonallyAdjacentTo(grid_offset, TERRAIN_WATER) != 4) {
            break;
        }
    }
    // invalid bridge
    *length = bridge.length;
    return 0;
}

int map_bridge_add(int x, int y, int is_ship_bridge)
{
    int min_length = is_ship_bridge ? 5 : 2;
    if (bridge.end_grid_offset <= 0 || bridge.length < min_length) {
        bridge.length = 0;
        return bridge.length;
    }

    int pillar_distance = 0;
    bridge.direction -= Data_State.map.orientation;
    if (bridge.direction < 0) bridge.direction += 8;
    if (is_ship_bridge) {
        switch (bridge.length) {
            case  9: case 10: pillar_distance = 4; break;
            case 11: case 12: pillar_distance = 5; break;
            case 13: case 14: pillar_distance = 6; break;
            case 15: case 16: pillar_distance = 7; break;
            default: pillar_distance = 8; break;
        }
    }

    int grid_offset = map_grid_offset(x, y);
    for (int i = 0; i < bridge.length; i++) {
        map_terrain_add(grid_offset, TERRAIN_ROAD);
        int value = 0;
        if (is_ship_bridge) {
            if (i == 1 || i == bridge.length - 2) {
                // platform after ramp
                value = 13;
            } else if (i == 0) {
                // ramp at start
                switch (bridge.direction) {
                    case 0: value = 7; break;
                    case 2: value = 8; break;
                    case 4: value = 9; break;
                    case 6: value = 10; break;
                }
            } else if (i == bridge.length - 1) {
                // ramp at end
                switch (bridge.direction) {
                    case 0: value = 9; break;
                    case 2: value = 10; break;
                    case 4: value = 7; break;
                    case 6: value = 8; break;
                }
            } else if (i == pillar_distance) {
                switch (bridge.direction) {
                    case 0: value = 14; break;
                    case 2: value = 15; break;
                    case 4: value = 14; break;
                    case 6: value = 15; break;
                }
            } else {
                // middle of the bridge
                switch (bridge.direction) {
                    case 0: value = 11; break;
                    case 2: value = 12; break;
                    case 4: value = 11; break;
                    case 6: value = 12; break;
                }
            }
        } else {
            if (i == 0) {
                // ramp at start
                switch (bridge.direction) {
                case 0: value = 1; break;
                case 2: value = 2; break;
                case 4: value = 3; break;
                case 6: value = 4; break;
                }
            } else if (i == bridge.length - 1) {
                // ramp at end
                switch (bridge.direction) {
                case 0: value = 3; break;
                case 2: value = 4; break;
                case 4: value = 1; break;
                case 6: value = 2; break;
                }
            } else {
                // middle part
                switch (bridge.direction) {
                case 0: value = 5; break;
                case 2: value = 6; break;
                case 4: value = 5; break;
                case 6: value = 6; break;
                }
            }
        }
        Data_Grid_spriteOffsets[grid_offset] = value;
        grid_offset += bridge.direction_grid_delta;
    }

    map_routing_update_land();
    map_routing_update_water();

    return bridge.length;
}

static int is_bridge(int grid_offset)
{
    return map_terrain_is(grid_offset, TERRAIN_WATER) && Data_Grid_spriteOffsets[grid_offset];
}

static int get_y_bridge_tiles(int grid_offset)
{
    int tiles = 0;
    if (is_bridge(grid_offset + map_grid_delta(0, -1))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(0, -2))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(0, 1))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(0, 2))) {
        tiles++;
    }
    return tiles;
}

static int get_x_bridge_tiles(int grid_offset)
{
    int tiles = 0;
    if (is_bridge(grid_offset + map_grid_delta(-1, 0))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(-2, 0))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(1, 0))) {
        tiles++;
    }
    if (is_bridge(grid_offset + map_grid_delta(2, 0))) {
        tiles++;
    }
    return tiles;
}

void map_bridge_remove(int grid_offset, int mark_deleted)
{
    if (!is_bridge(grid_offset)) {
        return;
    }

    int tiles_x = get_x_bridge_tiles(grid_offset);
    int tiles_y = get_y_bridge_tiles(grid_offset);

    int offset_up = tiles_x > tiles_y ? map_grid_delta(1, 0) : map_grid_delta(0, 1);
    // find lower end of the bridge
    while (is_bridge(grid_offset - offset_up)) {
        grid_offset -= offset_up;
    }

    if (mark_deleted) {
        map_property_mark_deleted(grid_offset);
    } else {
        Data_Grid_spriteOffsets[grid_offset] = 0;
        map_terrain_remove(grid_offset, TERRAIN_ROAD);
    }
    while (is_bridge(grid_offset + offset_up)) {
        grid_offset += offset_up;
        if (mark_deleted) {
            map_property_mark_deleted(grid_offset);
        } else {
            Data_Grid_spriteOffsets[grid_offset] = 0;
            map_terrain_remove(grid_offset, TERRAIN_ROAD);
        }
    }
}

int map_bridge_count_figures(int grid_offset)
{
    if (!is_bridge(grid_offset)) {
        return 0;
    }
    int tiles_x = get_x_bridge_tiles(grid_offset);
    int tiles_y = get_y_bridge_tiles(grid_offset);

    int offset_up = tiles_x > tiles_y ? map_grid_delta(1, 0) : map_grid_delta(0, 1);
    // find lower end of the bridge
    while (is_bridge(grid_offset - offset_up)) {
        grid_offset -= offset_up;
    }

    int figures = 0;
    if (map_has_figure_at(grid_offset)) {
        figures = 1;
    }
    map_property_clear_deleted(grid_offset);
    while (is_bridge(grid_offset + offset_up)) {
        grid_offset += offset_up;
        map_property_clear_deleted(grid_offset);
        if (map_has_figure_at(grid_offset)) {
            figures++;
        }
    }
    return figures;
}

void map_bridge_update_after_rotate(int ccw)
{
    for (int y = 0; y < Data_State.map.height; y++) {
        for (int x = 0; x < Data_State.map.width; x++) {
            int grid_offset = map_grid_offset(x, y);
            if (is_bridge(grid_offset)) {
                int new_value;
                switch (Data_Grid_spriteOffsets[grid_offset]) {
                    case 1: new_value = ccw ? 2 : 4; break;
                    case 2: new_value = ccw ? 3 : 1; break;
                    case 3: new_value = ccw ? 4 : 2; break;
                    case 4: new_value = ccw ? 1 : 3; break;
                    case 5: new_value = 6; break;
                    case 6: new_value = 5; break;
                    case 7: new_value = ccw ? 8 : 10; break;
                    case 8: new_value = ccw ? 9 : 7; break;
                    case 9: new_value = ccw ? 10 : 8; break;
                    case 10: new_value = ccw ? 7 : 9; break;
                    case 11: new_value = 12; break;
                    case 12: new_value = 11; break;
                    case 13: new_value = 13; break;
                    case 14: new_value = 15; break;
                    case 15: new_value = 14; break;
                    default: new_value = Data_Grid_spriteOffsets[grid_offset];
                }
                Data_Grid_spriteOffsets[grid_offset] = new_value;
            }
        }
    }
}
