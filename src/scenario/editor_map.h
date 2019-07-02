#ifndef SCENARIO_EDITOR_MAP_H
#define SCENARIO_EDITOR_MAP_H

#include "map/point.h"

void scenario_editor_clear_herd_points(void);
void scenario_editor_set_herd_point(int id, int x, int y);

void scenario_editor_clear_fishing_points(void);
void scenario_editor_set_fishing_point(int id, int x, int y);

int scenario_editor_count_invasion_points(void);
void scenario_editor_clear_invasion_points(void);
void scenario_editor_set_invasion_point(int id, int x, int y);

map_point scenario_editor_earthquake_point(void);

#endif // SCENARIO_EDITOR_MAP_H
