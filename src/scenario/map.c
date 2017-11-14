#include "map.h"

#include "core/calc.h"

#include "Data/Scenario.h"

#define MAX_HERDS 4
#define MAX_FISH 8

map_point scenario_map_entry()
{
    map_point point = {Data_Scenario.entryPoint.x, Data_Scenario.entryPoint.y};
    return point;
}

map_point scenario_map_exit()
{
    map_point point = {Data_Scenario.exitPoint.x, Data_Scenario.exitPoint.y};
    return point;
}

int scenario_map_has_river_entry()
{
    return Data_Scenario.riverEntryPoint.x != -1 && Data_Scenario.riverEntryPoint.y != -1;
}

map_point scenario_map_river_entry()
{
    map_point point = {Data_Scenario.riverEntryPoint.x, Data_Scenario.riverEntryPoint.y};
    return point;
}

int scenario_map_has_river_exit()
{
    return Data_Scenario.riverExitPoint.x != -1 && Data_Scenario.riverExitPoint.y != -1;
}

map_point scenario_map_river_exit()
{
    map_point point = {Data_Scenario.riverExitPoint.x, Data_Scenario.riverExitPoint.y};
    return point;
}

void scenario_map_foreach_herd_point(void (*callback)(int x, int y))
{
    for (int i = 0; i < MAX_HERDS; i++) {
        if (Data_Scenario.herdPoints.x[i] > 0) {
            callback(Data_Scenario.herdPoints.x[i], Data_Scenario.herdPoints.y[i]);
        }
    }
}

void scenario_map_foreach_fishing_point(void (*callback)(int x, int y))
{
    for (int i = 0; i < MAX_FISH; i++) {
        if (Data_Scenario.fishingPoints.x[i] > 0) {
            callback(Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i]);
        }
    }
}

int scenario_map_closest_fishing_point(int x, int y, int *fish_x, int *fish_y)
{
    int numFishingSpots = 0;
    for (int i = 0; i < MAX_FISH; i++) {
        if (Data_Scenario.fishingPoints.x[i] > 0) {
            numFishingSpots++;
        }
    }
    if (numFishingSpots <= 0) {
        return 0;
    }
    int minDist = 10000;
    int minFishId = 0;
    for (int i = 0; i < MAX_FISH; i++) {
        if (Data_Scenario.fishingPoints.x[i] > 0) {
            int dist = calc_maximum_distance(x, y,
                Data_Scenario.fishingPoints.x[i], Data_Scenario.fishingPoints.y[i]);
            if (dist < minDist) {
                minDist = dist;
                minFishId = i;
            }
        }
    }
    if (minDist < 10000) {
        *fish_x = Data_Scenario.fishingPoints.x[minFishId];
        *fish_y = Data_Scenario.fishingPoints.y[minFishId];
        return 1;
    }
    return 0;
}

int scenario_map_has_flotsam()
{
    return Data_Scenario.flotsamEnabled;
}
