#include "tool.h"

#include "editor/tool_restriction.h"
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

void editor_tool_start_use(const map_tile *tile)
{
    data.build_in_progress = 1;
}

void editor_tool_update_use(const map_tile *tile)
{
}

void editor_tool_end_use(const map_tile *tile)
{
    if (!data.build_in_progress) {
        return;
    }
    data.build_in_progress = 0;
    switch (data.type) {
        case TOOL_ENTRY_POINT:
            if (editor_tool_can_place_flag(data.type, tile)) {
                scenario_editor_set_entry_point(tile->x, tile->y);
            } else {
                city_warning_show(WARNING_EDITOR_NEED_MAP_EDGE);
            }
            break;
        case TOOL_EXIT_POINT:
            if (editor_tool_can_place_flag(data.type, tile)) {
                scenario_editor_set_exit_point(tile->x, tile->y);
            } else {
                city_warning_show(WARNING_EDITOR_NEED_MAP_EDGE);
            }
            break;
    }
}
