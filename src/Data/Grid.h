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
	Terrain_Building = 8,
	Terrain_Road = 0x40,
	// flagged items
	Terrain_NotClear = 0xd77f,
	Terrain_1237 = 0x1237,
	Terrain_127f = 0x127f,
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
