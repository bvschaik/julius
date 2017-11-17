#include "map.h"

#include "core/calc.h"
#include "scenario/data.h"

#include "Data/Settings.h"

void scenario_map_init()
{
    Data_Settings_Map.width = scenario.map.width;
    Data_Settings_Map.height = scenario.map.height;
    Data_Settings_Map.gridStartOffset = scenario.map.grid_start;
    Data_Settings_Map.gridBorderSize = scenario.map.grid_border_size;
}

int scenario_map_size()
{
    return scenario.map.width;
}

void scenario_map_init_entry_exit()
{
    if (scenario.entry_point.x == -1 || scenario.entry_point.y == -1) {
        scenario.entry_point.x = scenario.map.width - 1;
        scenario.entry_point.y = scenario.map.height / 2;
    }
    if (scenario.exit_point.x == -1 || scenario.exit_point.y == -1) {
        scenario.exit_point.x = scenario.entry_point.x;
        scenario.exit_point.y = scenario.entry_point.y;
    }
}

map_point scenario_map_entry()
{
    map_point point = {scenario.entry_point.x, scenario.entry_point.y};
    return point;
}

map_point scenario_map_exit()
{
    map_point point = {scenario.exit_point.x, scenario.exit_point.y};
    return point;
}

int scenario_map_has_river_entry()
{
    return scenario.river_entry_point.x != -1 && scenario.river_entry_point.y != -1;
}

map_point scenario_map_river_entry()
{
    map_point point = {scenario.river_entry_point.x, scenario.river_entry_point.y};
    return point;
}

int scenario_map_has_river_exit()
{
    return scenario.river_exit_point.x != -1 && scenario.river_exit_point.y != -1;
}

map_point scenario_map_river_exit()
{
    map_point point = {scenario.river_exit_point.x, scenario.river_exit_point.y};
    return point;
}

void scenario_map_foreach_herd_point(void (*callback)(int x, int y))
{
    for (int i = 0; i < MAX_HERD_POINTS; i++) {
        if (scenario.herd_points[i].x > 0) {
            callback(scenario.herd_points[i].x, scenario.herd_points[i].y);
        }
    }
}

void scenario_map_foreach_fishing_point(void (*callback)(int x, int y))
{
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        if (scenario.fishing_points[i].x > 0) {
            callback(scenario.fishing_points[i].x, scenario.fishing_points[i].y);
        }
    }
}

int scenario_map_closest_fishing_point(int x, int y, int *fish_x, int *fish_y)
{
    int numFishingSpots = 0;
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        if (scenario.fishing_points[i].x > 0) {
            numFishingSpots++;
        }
    }
    if (numFishingSpots <= 0) {
        return 0;
    }
    int minDist = 10000;
    int minFishId = 0;
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        if (scenario.fishing_points[i].x > 0) {
            int dist = calc_maximum_distance(x, y,
                scenario.fishing_points[i].x, scenario.fishing_points[i].y);
            if (dist < minDist) {
                minDist = dist;
                minFishId = i;
            }
        }
    }
    if (minDist < 10000) {
        *fish_x = scenario.fishing_points[minFishId].x;
        *fish_y = scenario.fishing_points[minFishId].y;
        return 1;
    }
    return 0;
}

int scenario_map_has_flotsam()
{
    return scenario.flotsam_enabled;
}
