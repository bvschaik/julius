#ifndef DATA_GRID_H
#define DATA_GRID_H
#include "Data.h"

#include <stdint.h>

#define UInt8_Grid(x) uint8_t x[GRID_SIZE * GRID_SIZE]

enum {
	GRID_SIZE = 162,
};

EXTERN UInt8_Grid(Data_Grid_aqueducts);
EXTERN UInt8_Grid(Data_Grid_spriteOffsets);

// undo
EXTERN UInt8_Grid(Data_Grid_Undo_aqueducts);
EXTERN UInt8_Grid(Data_Grid_Undo_spriteOffsets);

#endif
