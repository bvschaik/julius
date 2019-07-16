#include "tool.h"

#include "core/image.h"
#include "core/image_group_editor.h"
#include "core/random.h"
#include "editor/tool_restriction.h"
#include "map/building_tiles.h"
#include "map/grid.h"
#include "map/tiles.h"
#include "map/terrain.h"
#include "scenario/editor_events.h"
#include "scenario/editor_map.h"
#include "city/warning.h"

static struct {
    int active;
    tool_type type;
    int id;
    int brush_size;
    int build_in_progress;
} data = { 0, TOOL_GRASS, 0, 3, 0 };

tool_type editor_tool_type(void)
{
    return data.type;
}

int editor_tool_is_active(void)
{
    return data.active;
}

void editor_tool_deactivate(void)
{
    data.active = 0;
}

void editor_tool_set_type(tool_type type)
{
    editor_tool_set_with_id(type, 0);
}

void editor_tool_set_with_id(tool_type type, int id)
{
    data.active = 1;
    data.type = type;
    data.id = id;
}

int editor_tool_brush_size(void)
{
    return data.brush_size;
}

void editor_tool_set_brush_size(int size)
{
    data.brush_size = size;
}

void editor_tool_foreach_brush_tile(void (*callback)(const void *data, int dx, int dy), const void *user_data)
{
    for (int dy = -data.brush_size + 1; dy < data.brush_size; dy++) {
        for (int dx = -data.brush_size + 1; dx < data.brush_size; dx++) {
            int steps = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
            if (steps < data.brush_size) {
                callback(user_data, dx, dy);
            }
        }
    }
}

int editor_tool_is_in_use(void)
{
    return data.build_in_progress;
}

void editor_tool_start_use(const map_tile *tile)
{
    data.build_in_progress = 1;
}

static int is_brush(tool_type type)
{
    switch (type) {
        case TOOL_GRASS:
        case TOOL_TREES:
        case TOOL_WATER:
        case TOOL_SCRUB:
        case TOOL_ROCKS:
        case TOOL_MEADOW:
        //case TOOL_RAISE_LAND:
        //case TOOL_LOWER_LAND:
            return 1;
        default:
            return 0;
    }
}

static void add_terrain(const void *tile_data, int dx, int dy)
{
    const map_tile *tile = (const map_tile *) tile_data;
    int x = tile->x + dx;
    int y = tile->y + dy;
    if (!map_grid_is_inside(x, y, 1)) {
        return;
    }
    int grid_offset = tile->grid_offset + map_grid_delta(dx, dy);
    int terrain = map_terrain_get(grid_offset);
    if (terrain & TERRAIN_BUILDING) {
        map_building_tiles_remove(0, x, y);
        terrain = map_terrain_get(grid_offset);
    }
    switch (data.type) {
        case TOOL_TREES:
            if (!(terrain & TERRAIN_TREE)) {
                terrain |= TERRAIN_TREE;
                terrain &= ~(TERRAIN_ROCK | TERRAIN_WATER | TERRAIN_BUILDING | TERRAIN_SCRUB | TERRAIN_GARDEN | TERRAIN_ROAD | TERRAIN_MEADOW);
            }
            break;
    }
    map_terrain_set(grid_offset, terrain);
}

void editor_tool_update_use(const map_tile *tile)
{
    if (!data.build_in_progress) {
        return;
    }
    if (data.type == TOOL_ROAD) {
        // TODO
        return;
    }
    if (!is_brush(data.type)) {
        return;
    }

    editor_tool_foreach_brush_tile(add_terrain, tile);

    int x_min = tile->x - data.brush_size;
    int x_max = tile->x + data.brush_size;
    int y_min = tile->y - data.brush_size;
    int y_max = tile->y + data.brush_size;
    switch (data.type) {
        case TOOL_TREES:
            // terrain_context_clear(water);
            map_tiles_update_region_water(x_min, y_min, x_max, y_max);
            map_tiles_update_all_rocks();
            map_tiles_update_region_trees(x_min, y_min, x_max, y_max);
            break;
    }
}

static void place_earthquake_flag(const map_tile *tile)
{
    int warning = 0;
    if (editor_tool_can_place_flag(data.type, tile, &warning)) {
        if (scenario_editor_earthquake_severity()) {
            scenario_editor_set_earthquake_point(tile->x, tile->y);
        } else {
            city_warning_show(WARNING_EDITOR_NO_EARTHQUAKE_SCHEDULED);
        }
    } else {
        city_warning_show(warning);
    }
}

static void place_flag(const map_tile *tile, void (*update)(int x, int y))
{
    int warning = 0;
    if (editor_tool_can_place_flag(data.type, tile, &warning)) {
        update(tile->x, tile->y);
    } else {
        city_warning_show(warning);
    }
}

static void place_flag_with_id(const map_tile *tile, void (*update)(int id, int x, int y))
{
    int warning = 0;
    if (editor_tool_can_place_flag(data.type, tile, &warning)) {
        update(data.id, tile->x, tile->y);
    } else {
        city_warning_show(warning);
    }
}

static void place_building(const map_tile *tile)
{
    int image_id;
    int size;
    switch (data.type) {
        case TOOL_NATIVE_HUT:
            image_id = image_group(GROUP_EDITOR_BUILDING_NATIVE) + (random_byte() & 1);
            size = 1;
            break;
        case TOOL_NATIVE_CENTER:
            image_id = image_group(GROUP_EDITOR_BUILDING_NATIVE) + 2;
            size = 2;
            break;
        case TOOL_NATIVE_FIELD:
            image_id = image_group(GROUP_EDITOR_BUILDING_CROPS);
            size = 1;
            break;
        default:
            return;
    }

    if (editor_tool_can_place_building(tile, size * size, 0)) {
        map_building_tiles_add(0, tile->x, tile->y, size, image_id, TERRAIN_BUILDING);
    } else {
        city_warning_show(WARNING_EDITOR_CANNOT_PLACE);
    }
}

void editor_tool_end_use(const map_tile *tile)
{
    if (!data.build_in_progress) {
        return;
    }
    data.build_in_progress = 0;
    switch (data.type) {
        case TOOL_EARTHQUAKE_POINT:
            place_earthquake_flag(tile);
            break;
        case TOOL_ENTRY_POINT:
            place_flag(tile, scenario_editor_set_entry_point);
            break;
        case TOOL_EXIT_POINT:
            place_flag(tile, scenario_editor_set_exit_point);
            break;
        case TOOL_RIVER_ENTRY_POINT:
            place_flag(tile, scenario_editor_set_river_entry_point);
            break;
        case TOOL_RIVER_EXIT_POINT:
            place_flag(tile, scenario_editor_set_river_exit_point);
            break;
        case TOOL_INVASION_POINT:
            place_flag_with_id(tile, scenario_editor_set_invasion_point);
            break;
        case TOOL_FISHING_POINT:
            place_flag_with_id(tile, scenario_editor_set_fishing_point);
            break;
        case TOOL_HERD_POINT:
            place_flag_with_id(tile, scenario_editor_set_herd_point);
            break;
        case TOOL_NATIVE_CENTER:
        case TOOL_NATIVE_FIELD:
        case TOOL_NATIVE_HUT:
            place_building(tile);
            break;
        // TODO TOOL_ACCESS_RAMP, TOOL_ROAD
        default:
            break;
    }
}
