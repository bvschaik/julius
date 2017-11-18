#include "map.h"

#include "data/scenario.hpp"

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
