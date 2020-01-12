#include "editor_map.h"

#include "figuretype/water.h"
#include "map/routing_terrain.h"
#include "scenario/data.h"

void scenario_editor_set_entry_point(int x, int y)
{
    scenario.entry_point.x = x;
    scenario.entry_point.y = y;
    scenario.is_saved = 0;
}

void scenario_editor_set_exit_point(int x, int y)
{
    scenario.exit_point.x = x;
    scenario.exit_point.y = y;
    scenario.is_saved = 0;
}

static void update_river(void)
{
    figure_create_flotsam();
    map_routing_update_water();
}

void scenario_editor_set_river_entry_point(int x, int y)
{
    scenario.river_entry_point.x = x;
    scenario.river_entry_point.y = y;
    scenario.is_saved = 0;
    update_river();
}

void scenario_editor_set_river_exit_point(int x, int y)
{
    scenario.river_exit_point.x = x;
    scenario.river_exit_point.y = y;
    scenario.is_saved = 0;
    update_river();
}

void scenario_editor_clear_herd_points(void)
{
    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        scenario.herd_points[i].x = -1;
        scenario.herd_points[i].y = -1;
    }
    scenario.is_saved = 0;
}

map_point scenario_editor_herd_point(int id)
{
    return scenario.herd_points[id];
}

void scenario_editor_set_herd_point(int id, int x, int y)
{
    scenario.herd_points[id].x = x;
    scenario.herd_points[id].y = y;
    scenario.is_saved = 0;
}

void scenario_editor_clear_fishing_points(void)
{
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        scenario.fishing_points[i].x = -1;
        scenario.fishing_points[i].y = -1;
    }
    scenario.is_saved = 0;
}

map_point scenario_editor_fishing_point(int id)
{
    return scenario.fishing_points[id];
}

void scenario_editor_set_fishing_point(int id, int x, int y)
{
    scenario.fishing_points[id].x = x;
    scenario.fishing_points[id].y = y;
    scenario.is_saved = 0;
}

int scenario_editor_count_invasion_points(void)
{
    int points = 0;
    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        if (scenario.invasion_points[i].x != -1) {
            points++;
        }
    }
    return points;
}

void scenario_editor_clear_invasion_points(void)
{
    for (int i = 0; i < MAX_INVASION_POINTS; i++) {
        scenario.invasion_points[i].x = -1;
        scenario.invasion_points[i].y = -1;
    }
    scenario.is_saved = 0;
}

map_point scenario_editor_invasion_point(int id)
{
    return scenario.invasion_points[id];
}

void scenario_editor_set_invasion_point(int id, int x, int y)
{
    scenario.invasion_points[id].x = x;
    scenario.invasion_points[id].y = y;
    scenario.is_saved = 0;
}

map_point scenario_editor_earthquake_point(void)
{
    return scenario.earthquake_point;
}

void scenario_editor_set_earthquake_point(int x, int y)
{
    scenario.earthquake_point.x = x;
    scenario.earthquake_point.y = y;
    scenario.is_saved = 0;
}

void scenario_editor_updated_terrain(void)
{
    scenario.is_saved = 0;
}
