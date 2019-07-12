#include "tool.h"

static struct {
    int tool_type;
    int id;
    int brush_size;
} data;

int editor_tool_type(void)
{
    return data.tool_type;
}

void editor_tool_set_type(int tool)
{
    data.tool_type = tool;
    data.id = 0;
}

void editor_tool_set_with_id(int tool, int id)
{
    data.tool_type = tool;
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
