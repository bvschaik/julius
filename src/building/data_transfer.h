#ifndef BUILDING_DATA_TRANSFER_H
#define BUILDING_DATA_TRANSFER_H

#include "building/building.h"
#include "building/storage.h"


typedef enum {
    DATA_TYPE_NOT_SUPPORTED,
    DATA_TYPE_WAREHOUSE,
    DATA_TYPE_GRANARY,
    DATA_TYPE_DOCK,
    DATA_TYPE_MARKET,
    DATA_TYPE_ROADBLOCK,
} building_data_type;

typedef struct {
    building_data_type data_type;
    int subtype;
    building_storage storage;
    int extended_data[16];
} building_data;


int building_data_transfer_copy(building *b);

int building_data_transfer_paste(building *b);

int building_data_transfer_possible(building *b);

building_data_type building_data_transfer_data_type_from_building_type(building_type type);

#endif // BUILDING_DATA_TRANSFER_H