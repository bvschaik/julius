#ifndef BUILDING_PROPERTIES_H
#define BUILDING_PROPERTIES_H

#include "building/type.h"

typedef struct {
    int size;
    int fire_proof;
    int image_group;
    int image_offset;
    int rotation_offset;
    int sound_id;
    struct {
        const char *group;
        const char *id;
    } custom_asset;
    struct {
        const char *attr;
        int key;
        int cannot_count;
    } event_data;
} building_properties;

void building_properties_init(void);
const building_properties *building_properties_for_type(building_type type);

#endif // BUILDING_PROPERTIES_H
