#include "routing.h"

#include "Data/Grid.h"
#include "../Routing.h"

int map_routing_distance(int grid_offset)
{
    return Data_Grid_routingDistance[grid_offset];
}

int map_routing_is_wall_passable(int grid_offset)
{
    return Data_Grid_routingWalls[grid_offset] == 0;
}

int map_routing_citizen_is_passable(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road ||
        Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_citizen_is_road(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_0_Road;
}

int map_routing_citizen_is_passable_terrain(int grid_offset)
{
    return Data_Grid_routingLandCitizen[grid_offset] == Routing_Citizen_2_PassableTerrain;
}

int map_routing_noncitizen_terrain(int grid_offset)
{
    return Data_Grid_routingLandNonCitizen[grid_offset];
}

