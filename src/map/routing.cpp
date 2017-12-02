#include "routing.h"

#include <data>
#include <core>

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

void map_routing_save_state(buffer *buf)
{
    buffer_write_i32(buf, 0); // unused counter
    buffer_write_i32(buf, Data_Routes.enemyRoutesCalculated);
    buffer_write_i32(buf, Data_Routes.totalRoutesCalculated);
    buffer_write_i32(buf, 0); // unused counter
}

void map_routing_load_state(buffer *buf)
{
    buffer_skip(buf, 4); // unused counter
    Data_Routes.enemyRoutesCalculated = buffer_read_i32(buf);
    Data_Routes.totalRoutesCalculated = buffer_read_i32(buf);
    buffer_skip(buf, 4); // unused counter
}
