#include "tool.h"

static struct {
    int active;
    tool_type type;
    int id;
    int brush_size;
} data = { 0, TOOL_GRASS, 0, 3 };

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
