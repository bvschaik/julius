#ifndef DATA_GRID_H
#define DATA_GRID_H
#include "Data.h"

#define Int16_Grid(x) unsigned short x[GRID_SIZE * GRID_SIZE]
#define Int8_Grid(x) unsigned char x[GRID_SIZE * GRID_SIZE]

enum {
	GRID_SIZE = 162,
};

EXTERN Int16_Grid(Data_Grid_graphicIds);
EXTERN Int16_Grid(Data_Grid_buildingIds);
EXTERN Int16_Grid(Data_Grid_walkerIds);
EXTERN Int16_Grid(Data_Grid_terrain);

EXTERN Int8_Grid(Data_Grid_edge);
EXTERN Int8_Grid(Data_Grid_bitfields);
EXTERN Int8_Grid(Data_Grid_random);
EXTERN Int8_Grid(Data_Grid_elevation);
EXTERN Int8_Grid(Data_Grid_aqueducts);
EXTERN Int8_Grid(Data_Grid_animation);
EXTERN Int8_Grid(Data_Grid_desirability);
EXTERN Int8_Grid(Data_Grid_buildingDamage);

#endif
