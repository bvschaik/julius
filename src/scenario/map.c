#include "map.h"

#include "core/calc.h"
#include "map/grid.h"
#include "scenario/data.h"

void scenario_map_init(void)
{
    map_grid_init(scenario.map.width, scenario.map.height,
                  scenario.map.grid_start, scenario.map.grid_border_size);
}

int scenario_map_size(void)
{
    return scenario.map.width;
}

void scenario_map_init_entry_exit(void)
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

map_point scenario_map_entry(void)
{
    map_point point = {scenario.entry_point.x, scenario.entry_point.y};
    return point;
}

map_point scenario_map_exit(void)
{
    map_point point = {scenario.exit_point.x, scenario.exit_point.y};
    return point;
}

int scenario_map_has_river_entry(void)
{
    return scenario.river_entry_point.x != -1 && scenario.river_entry_point.y != -1;
}

map_point scenario_map_river_entry(void)
{
    map_point point = {scenario.river_entry_point.x, scenario.river_entry_point.y};
    return point;
}

int scenario_map_has_river_exit(void)
{
    return scenario.river_exit_point.x != -1 && scenario.river_exit_point.y != -1;
}

map_point scenario_map_river_exit(void)
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

int scenario_map_closest_fishing_point(int x, int y, map_point *fish)
{
    int num_fishing_spots = 0;
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        if (scenario.fishing_points[i].x > 0) {
            num_fishing_spots++;
        }
    }
    if (num_fishing_spots <= 0) {
        return 0;
    }
    int min_dist = 10000;
    int min_fish_id = 0;
    for (int i = 0; i < MAX_FISH_POINTS; i++) {
        if (scenario.fishing_points[i].x > 0) {
            int dist = calc_maximum_distance(x, y,
                scenario.fishing_points[i].x, scenario.fishing_points[i].y);
            if (dist < min_dist) {
                min_dist = dist;
                min_fish_id = i;
            }
        }
    }
    if (min_dist < 10000) {
        map_point_store_result(
            scenario.fishing_points[min_fish_id].x,
            scenario.fishing_points[min_fish_id].y,
            fish
        );
        return 1;
    }
    return 0;
}

int scenario_map_has_flotsam(void)
{
    return scenario.flotsam_enabled;
}
