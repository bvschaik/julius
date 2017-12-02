#ifndef MAP_ROUTING_H
#define MAP_ROUTING_H

struct buffer;

enum
{
    Routing_Citizen_0_Road = 0,
    Routing_Citizen_2_PassableTerrain = 2,
    Routing_Citizen_4_ClearTerrain = 4,
    Routing_Citizen_m1_Blocked = -1,
    Routing_Citizen_m3_Aqueduct = -3,
    Routing_Citizen_m4_ReservoirConnector = -4,

    Routing_NonCitizen_0_Passable = 0,
    Routing_NonCitizen_1_Building = 1,
    Routing_NonCitizen_2_Clearable = 2,
    Routing_NonCitizen_3_Wall = 3,
    Routing_NonCitizen_4_Gatehouse = 4,
    Routing_NonCitizen_5_Fort = 5,
    Routing_NonCitizen_m1_Blocked = -1,

    Routing_Water_0_Passable = 0,
    Routing_Water_m1_Blocked = -1,
    Routing_Water_m2_MapEdge = -2,
    Routing_Water_m3_LowBridge = -3,

    Routing_Wall_0_Passable = 0,
    Routing_Wall_m1_Blocked = -1,
};

int map_routing_distance(int grid_offset);

int map_routing_is_wall_passable(int grid_offset);

int map_routing_citizen_is_passable(int grid_offset);
int map_routing_citizen_is_road(int grid_offset);
int map_routing_citizen_is_passable_terrain(int grid_offset);

int map_routing_noncitizen_terrain(int grid_offset);

void map_routing_save_state(buffer *buf);

void map_routing_load_state(buffer *buf);

#endif // MAP_ROUTING_H
