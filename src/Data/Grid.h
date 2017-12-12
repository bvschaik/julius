#ifndef DATA_GRID_H
#define DATA_GRID_H
#include "Data.h"

#include <stdint.h>

#define Int8_Grid(x) int8_t x[GRID_SIZE * GRID_SIZE]
#define UInt8_Grid(x) uint8_t x[GRID_SIZE * GRID_SIZE]
#define UInt16_Grid(x) uint16_t x[GRID_SIZE * GRID_SIZE]

enum {
	GRID_SIZE = 162,
};

enum {
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

EXTERN UInt16_Grid(Data_Grid_terrain);

EXTERN UInt8_Grid(Data_Grid_elevation);
EXTERN UInt8_Grid(Data_Grid_aqueducts);
EXTERN UInt8_Grid(Data_Grid_spriteOffsets);

// undo
EXTERN UInt8_Grid(Data_Grid_Undo_aqueducts);
EXTERN UInt8_Grid(Data_Grid_Undo_spriteOffsets);

#endif
