#include "map_editor_tool.h"

#include "building/properties.h"
#include "editor/tool.h"
#include "graphics/image.h"
#include "input/scroll.h"
#include "map/figure.h"
#include "map/terrain.h"

#define MAX_TILES 4

static const int X_VIEW_OFFSETS[MAX_TILES] = {
    0, -30, 30, 0
};

static const int Y_VIEW_OFFSETS[MAX_TILES] = {
    0, 15, 15, 30
};

static const int TILE_GRID_OFFSETS[MAX_TILES] = {
    0, 162, 1, 163
};

static void draw_flat_tile(int x, int y, color_t color_mask)
{
    image_draw_blend(image_group(GROUP_TERRAIN_FLAT_TILE), x, y, color_mask);
}

static void draw_partially_blocked(int x, int y, int num_tiles, int *blocked_tiles)
{
    for (int i = 0; i < num_tiles; i++) {
        int x_offset = x + X_VIEW_OFFSETS[i];
        int y_offset = y + Y_VIEW_OFFSETS[i];
        if (blocked_tiles[i]) {
            draw_flat_tile(x_offset, y_offset, COLOR_MASK_RED);
        } else {
            draw_flat_tile(x_offset, y_offset, COLOR_MASK_GREEN);
        }
    }
}

static void draw_building_image(int image_id, int x, int y)
{
    image_draw_isometric_footprint(image_id, x, y, COLOR_MASK_GREEN);
    image_draw_isometric_top(image_id, x, y, COLOR_MASK_GREEN);
}

static void draw_building(const map_tile *tile, int x_view, int y_view, building_type type)
{
    const building_properties *props = building_properties_for_type(type);
    int grid_offset = tile->grid_offset;

    int blocked = 0;
    int num_tiles = props->size * props->size;
    int blocked_tiles[MAX_TILES];
    for (int i = 0; i < num_tiles; i++) {
        int tile_offset = grid_offset + TILE_GRID_OFFSETS[i];
        int forbidden_terrain = map_terrain_get(tile_offset) & TERRAIN_NOT_CLEAR;
        if (forbidden_terrain || map_has_figure_at(tile_offset)) {
            blocked_tiles[i] = blocked = 1;
        } else {
            blocked_tiles[i] = 0;
        }
    }
    if (blocked) {
        draw_partially_blocked(x_view, y_view, num_tiles, blocked_tiles);
    } else {
        int image_id = image_group(props->image_group) + props->image_offset;
        draw_building_image(image_id, x_view, y_view);
    }
}

static void draw_road(const map_tile *tile, int x, int y)
{
    int grid_offset = tile->grid_offset;
    int blocked = 0;
    int image_id = 0;
    if (map_terrain_is(grid_offset, TERRAIN_NOT_CLEAR)) {
        blocked = 1;
    } else {
        image_id = image_group(GROUP_TERRAIN_ROAD);
        if (!map_terrain_has_adjacent_x_with_type(grid_offset, TERRAIN_ROAD) &&
            map_terrain_has_adjacent_y_with_type(grid_offset, TERRAIN_ROAD)) {
            image_id++;
        }
    }
    if (blocked) {
        draw_flat_tile(x, y, COLOR_MASK_RED);
    } else {
        draw_building_image(image_id, x, y);
    }
}

static void draw_brush(const map_tile *tile, int x, int y)
{
    int brush_size = editor_tool_brush_size();
    for (int dy = -brush_size + 1; dy < brush_size; dy++) {
        for (int dx = -brush_size + 1; dx < brush_size; dx++) {
            int steps = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
            if (steps < brush_size) {
                draw_flat_tile(x + (dx - dy) * 30, y + (dx + dy) * 15, COLOR_MASK_GREEN);
            }
        }
    }
}

static void draw_land_flag_point(const map_tile *tile, int x, int y)
{
    int blocked = map_terrain_is(tile->grid_offset, TERRAIN_NOT_CLEAR);
    draw_flat_tile(x, y, blocked ? COLOR_MASK_RED : COLOR_MASK_GREEN);
}

static void draw_water_flag_point(const map_tile *tile, int x, int y)
{
    int blocked = !map_terrain_is(tile->grid_offset, TERRAIN_WATER);
    draw_flat_tile(x, y, blocked ? COLOR_MASK_RED : COLOR_MASK_GREEN);
}

void map_editor_tool_draw(const map_tile *tile)
{
    if (!tile->grid_offset || scroll_in_progress() || !editor_tool_is_active()) {
        return;
    }

    tool_type type = editor_tool_type();
    
    int x, y;
    city_view_get_selected_tile_pixels(&x, &y);
    switch (type) {
        case TOOL_NATIVE_CENTER:
            draw_building(tile, x, y, BUILDING_NATIVE_MEETING);
            break;
        case TOOL_NATIVE_HUT:
            draw_building(tile, x, y, BUILDING_NATIVE_HUT);
            break;
        case TOOL_NATIVE_FIELD:
            draw_building(tile, x, y, BUILDING_NATIVE_CROPS);
            break;

        case TOOL_EARTHQUAKE_POINT:
        case TOOL_HERD_POINT:
            draw_land_flag_point(tile, x, y);
            break;

        case TOOL_ENTRY_POINT:
        case TOOL_EXIT_POINT:
        case TOOL_INVASION_POINT:
            // TODO draw_land_edge_flag_point(tile, x, y);
            break;
            
        case TOOL_FISHING_POINT:
            draw_water_flag_point(tile, x, y);
            break;

        case TOOL_RIVER_ENTRY_POINT:
        case TOOL_RIVER_EXIT_POINT:
            // TODO draw_water_edge_flag_point(tile, x, y);
            break;

        case TOOL_ACCESS_RAMP:
            // TODO draw_access_ramp(tile, x, y);
            break;
            
        case TOOL_GRASS:
        case TOOL_MEADOW:
        case TOOL_ROCKS:
        case TOOL_SCRUB:
        case TOOL_TREES:
        case TOOL_WATER:
        case TOOL_RAISE_LAND:
        case TOOL_LOWER_LAND:
            draw_brush(tile, x, y);
            break;

        case TOOL_ROAD:
            draw_road(tile, x, y);
            break;
    }
}
