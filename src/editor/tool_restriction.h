#ifndef EDITOR_TOOL_RESTRICTION_H
#define EDITOR_TOOL_RESTRICTION_H

#include "editor/tool.h"
#include "map/point.h"

int editor_tool_can_place_flag(tool_type type, const map_tile *tile);

int editor_tool_can_place_access_ramp(const map_tile *tile, int *orientation_index);

int editor_tool_can_place_building(const map_tile *tile, int num_tiles, int *blocked_tiles);

#endif // EDITOR_TOOL_RESTRICTION_H
