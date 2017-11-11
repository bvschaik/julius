#ifndef DATA_GRID_H
#define DATA_GRID_H
#include "data.hpp"

#define Int8_Grid(x) char x[GRID_SIZE * GRID_SIZE]
#define UInt8_Grid(x) unsigned char x[GRID_SIZE * GRID_SIZE]
#define UInt16_Grid(x) unsigned short x[GRID_SIZE * GRID_SIZE]

enum
{
    GRID_SIZE = 162,
};

enum
{
    Terrain_Tree = 1,
    Terrain_Rock = 2,
    Terrain_Water = 4,
    Terrain_Building = 8,
    Terrain_Scrub = 0x10,
    Terrain_Garden = 0x20,
    Terrain_Road = 0x40,
    Terrain_ReservoirRange = 0x80,
    Terrain_Aqueduct = 0x100,
    Terrain_Elevation = 0x200,
    Terrain_AccessRamp = 0x400,
    Terrain_Meadow = 0x800,
    Terrain_Rubble = 0x1000,
    Terrain_FountainRange = 0x2000,
    Terrain_Wall = 0x4000,
    Terrain_Gatehouse = 0x8000,
    // flagged items
    Terrain_OutsideMap = 5,
    Terrain_WallOrGatehouse = 0xc0000,
    Terrain_NotClear = 0xd77f,
    Terrain_1237 = 0x1237,
    Terrain_127f = 0x127f,
    Terrain_GardenAccessRampRubble = 0x1420,
    Terrain_c75f = 0xc75f,
    Terrain_d73f = 0xd73f,
    Terrain_NaturalElements = 0x1677,
    Terrain_2e80 = 0x2e80,
    Terrain_All = 0xffff
};

EXTERN UInt16_Grid(Data_Grid_graphicIds);
EXTERN UInt16_Grid(Data_Grid_buildingIds);
EXTERN UInt16_Grid(Data_Grid_figureIds);
EXTERN UInt16_Grid(Data_Grid_terrain);

EXTERN UInt8_Grid(Data_Grid_edge);
EXTERN UInt8_Grid(Data_Grid_bitfields);
EXTERN UInt8_Grid(Data_Grid_random);
EXTERN UInt8_Grid(Data_Grid_elevation);
EXTERN UInt8_Grid(Data_Grid_aqueducts);
EXTERN UInt8_Grid(Data_Grid_spriteOffsets);
EXTERN Int8_Grid(Data_Grid_desirability);
EXTERN UInt8_Grid(Data_Grid_buildingDamage);
EXTERN UInt8_Grid(Data_Grid_rubbleBuildingType);

EXTERN Int8_Grid(Data_Grid_routingLandCitizen);
EXTERN Int8_Grid(Data_Grid_routingLandNonCitizen);
EXTERN Int8_Grid(Data_Grid_routingWater);
EXTERN Int8_Grid(Data_Grid_routingWalls);
EXTERN UInt16_Grid(Data_Grid_routingDistance);

EXTERN UInt8_Grid(Data_Grid_romanSoldierConcentration);
EXTERN UInt8_Grid(Data_Grid_roadNetworks);

// undo
EXTERN UInt16_Grid(Data_Grid_Undo_graphicIds);
EXTERN UInt16_Grid(Data_Grid_Undo_terrain);
EXTERN UInt8_Grid(Data_Grid_Undo_aqueducts);
EXTERN UInt8_Grid(Data_Grid_Undo_bitfields);
EXTERN UInt8_Grid(Data_Grid_Undo_edge);
EXTERN UInt8_Grid(Data_Grid_Undo_spriteOffsets);

enum
{
    Bitfield_Size1 = 0x00,
    Bitfield_Size2 = 0x01,
    Bitfield_Size3 = 0x02,
    Bitfield_Size4 = 0x04,
    Bitfield_Size5 = 0x08,
    Bitfield_Sizes = 0x0f,
    Bitfield_NoSizes = 0xf0,
    Bitfield_Overlay = 0x10,
    Bitfield_NoOverlay = 0xef,
    Bitfield_AlternateTerrain = 0x20,
    Bitfield_Deleted = 0x40,
    Bitfield_NoDeleted = 0xbf,
    Bitfield_PlazaOrEarthquake = 0x80,
    Bitfield_NoPlaza = 0x7f,
    Bitfield_NoOverlayAndDeleted = 0xaf,
    Edge_MaskX = 0x7,
    Edge_MaskY = 0x38,
    Edge_MaskXY = 0x3f,
    Edge_X0Y0 = 0,
    Edge_X1Y0 = 1,
    Edge_X2Y0 = 2,
    Edge_X0Y1 = 8,
    Edge_X1Y1 = 9,
    Edge_X2Y1 = 10,
    Edge_X0Y2 = 16,
    Edge_X1Y2 = 17,
    Edge_X2Y2 = 18,
    Edge_LeftmostTile = 0x40,
    Edge_NoLeftmostTile = 0xbf,
    Edge_NativeLand = 0x80,
    Edge_NoNativeLand = 0x7f,
};

#define EdgeXY(x,y) (8 * (y) + (x))

#endif
