#ifndef BUILDING_PROPERTIES_H
#define BUILDING_PROPERTIES_H

#include "building/type.h"

typedef struct {
    int size;
    int fire_proof;
    int image_group;
    int image_offset;
    int rotation_offset;
} building_properties;

typedef struct {
    int type;
    building_properties properties;
    const char* asset_name;
    const char* asset_image_id;
} augustus_building_properties_mapping;

const building_properties *building_properties_for_type(building_type type);

void init_augustus_building_properties(void);

#endif // BUILDING_PROPERTIES_H
