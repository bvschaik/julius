#ifndef BUILDING_PROPERTIES_H
#define BUILDING_PROPERTIES_H

#include "building/type.h"

typedef struct {
    int size;
    int fire_proof;
    int image_group;
    int image_offset;
} building_properties;

typedef struct {
    int type;
    building_properties properties;
    const char* mod_author;
    const char* mod_name;
    const char* mod_image_id;
} mod_building_properties_mapping;

const building_properties *building_properties_for_type(building_type type);

void init_mod_building_properties();

#endif // BUILDING_PROPERTIES_H
