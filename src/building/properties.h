#ifndef BUILDING_PROPERTIES_H
#define BUILDING_PROPERTIES_H

#include "building/type.h"

typedef struct {
    int size;
    int fire_proof;
    int image_group;
    int image_offset;
} building_properties;

const building_properties *building_properties_for_type(building_type type);

#endif // BUILDING_PROPERTIES_H
