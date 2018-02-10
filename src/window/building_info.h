#ifndef WINDOW_BUILDING_INFO_H
#define WINDOW_BUILDING_INFO_H

typedef enum {
    BUILDING_INFO_NONE = 0,
    BUILDING_INFO_TERRAIN = 1,
    BUILDING_INFO_BUILDING = 2,
    BUILDING_INFO_LEGION = 4
} building_info_type;

typedef enum {
    TERRAIN_INFO_NONE = 0,
    TERRAIN_INFO_TREE = 1,
    TERRAIN_INFO_ROCK = 2,
    TERRAIN_INFO_WATER = 3,
    TERRAIN_INFO_SCRUB = 4,
    TERRAIN_INFO_EARTHQUAKE = 5,
    TERRAIN_INFO_ROAD = 6,
    TERRAIN_INFO_AQUEDUCT = 7,
    TERRAIN_INFO_RUBBLE = 8,
    TERRAIN_INFO_WALL = 9,
    TERRAIN_INFO_EMPTY = 10,
    TERRAIN_INFO_BRIDGE = 11,
    TERRAIN_INFO_GARDEN = 12,
    TERRAIN_INFO_PLAZA = 13,
    TERRAIN_INFO_ENTRY_FLAG = 14,
    TERRAIN_INFO_EXIT_FLAG = 15
} terrain_info_type;

typedef struct {
    int xOffset;
    int yOffset;
    int widthBlocks;
    int heightBlocks;
    int helpId;
    int canPlaySound;
    int buildingId;
    int hasRoadAccess;
    int workerPercentage;
    int hasReservoirPipes;
    int aqueductHasWater;
    int formationId;
    int formationTypes;
    int barracksSoldiersRequested;
    int worstDesirabilityBuildingId;
    int warehouseSpaceText;
    building_info_type type;
    terrain_info_type terrainType;
    int advisor;
    int rubbleBuildingType;
    int storageShowSpecialOrders;
    struct {
        int soundId;
        int phraseId;
        int selectedIndex;
        int count;
        int drawn;
        int figureIds[7];
    } figure;
} BuildingInfoContext;

void window_building_info_show(int grid_offset);

int window_building_info_get_building_type();

#endif // WINDOW_BUILDING_INFO_H
