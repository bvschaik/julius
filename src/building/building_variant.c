#include "building_variant.h"

#include "building/properties.h"
#include "building/rotation.h"
#include "core/image.h"

#define MAX_VARIANTS_PER_BUILDING 10
#define BUILDINGS_WITH_VARIANTS 1

struct building_variant {
	const unsigned char number_of_variants;
	building_type type;
	int variants_offsets[MAX_VARIANTS_PER_BUILDING];
};

static struct building_variant variants[BUILDINGS_WITH_VARIANTS] = {
	{5, BUILDING_PAVILION_BLUE, {0,1,2,3,4}},

};

int building_variant_has_variants(building_type type)
{
	switch (type) {
	case BUILDING_PAVILION_BLUE:
		return 1;
	default:
		return 0;
		break;
	}
}

int building_variant_get_image_id(building_type type)
{
	if (!building_variant_has_variants(type)) {
		return 0;
	}
	struct building_variant *variant = 0;
	for (int i = 0; i < BUILDINGS_WITH_VARIANTS; i++) {
		if (variants[i].type == type) {
			variant = &variants[i];
			break;
		}
	}
	if (!variant) {
		return 0;
	}
	int rotation = building_rotation_get_rotation_with_limit(variant->number_of_variants);
	int group_id = building_properties_for_type(type)->image_group;
	int image_offset = building_properties_for_type(type)->image_offset + variant->variants_offsets[rotation];
	int image_id = group_id+image_offset;

	return image_id;
}

int building_variant_get_number_of_variants(building_type type)
{
	if (!building_variant_has_variants(type)) {
		return 0;
	}
	struct building_variant* variant = 0;
	for (int i = 0; i < BUILDINGS_WITH_VARIANTS; i++) {
		if (variants[i].type == type) {
			variant = &variants[i];
			break;
		}
	}
	if (!variant) {
		return 0;
	}
	return variant->number_of_variants;
}