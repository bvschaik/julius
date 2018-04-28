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

workshop_type resource_to_workshop_type(resource_type resource)
{
    switch (resource) {
        case RESOURCE_OLIVES:
            return WORKSHOP_OLIVES_TO_OIL;
        case RESOURCE_VINES:
            return WORKSHOP_VINES_TO_WINE;
        case RESOURCE_IRON:
            return WORKSHOP_IRON_TO_WEAPONS;
        case RESOURCE_TIMBER:
            return WORKSHOP_TIMBER_TO_FURNITURE;
        case RESOURCE_CLAY:
            return WORKSHOP_CLAY_TO_POTTERY;
        default:
            return WORKSHOP_NONE;
    }
}
