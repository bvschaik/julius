#ifndef BUILDING_BUILDING_VARIANT_H
#define BUILDING_BUILDING_VARIANT_H

#include "building/building.h"

int building_variant_has_variants(building_type type);

int building_variant_get_image_id(building_type type);

int building_variant_get_image_id_with_rotation(building_type type, int rotation);

int building_variant_get_offset_with_rotation(building_type type, int rotation);

int building_variant_get_number_of_variants(building_type type);
#endif // BUILDING_VARIANT_STATE_H
