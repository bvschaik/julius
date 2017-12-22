#include "resource.h"

#include "building/type.h"
#include "scenario/building.h"

int resource_image_offset(resource_type resource, resource_image_type type)
{
    if (resource == RESOURCE_MEAT && scenario_building_allowed(BUILDING_WHARF)) {
        switch (type) {
            case RESOURCE_IMAGE_STORAGE: return 40;
            case RESOURCE_IMAGE_CART: return 648;
            case RESOURCE_IMAGE_FOOD_CART: return 8;
            case RESOURCE_IMAGE_ICON: return 11;
            default: return 0;
        }
    } else {
        return 0;
    }
}

int resource_is_food(resource_type resource)
{
    return resource == RESOURCE_WHEAT || resource == RESOURCE_VEGETABLES ||
        resource == RESOURCE_FRUIT || resource == RESOURCE_MEAT;
}
