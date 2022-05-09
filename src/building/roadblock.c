#include "building/roadblock.h"

#include "building/building.h"
#include "building/type.h"





void building_roadblock_set_permission(roadblock_permission p, building* b) {
    if (building_type_is_roadblock(b->type)) {
        int permission_bit = 1 << p;
        b->data.roadblock.exceptions ^= permission_bit;
    }
}

int building_roadblock_get_permission(roadblock_permission p, building* b) {
    if (!building_type_is_roadblock(b->type)) {
        return 0;
    }
    int permission_bit = 1 << p;
    return (b->data.roadblock.exceptions & permission_bit);
}

int building_type_is_roadblock(building_type type)
{
    switch (type) {
        case BUILDING_ROADBLOCK:
        case BUILDING_GARDEN_WALL_GATE:
        case BUILDING_HEDGE_GATE_DARK:
        case BUILDING_HEDGE_GATE_LIGHT:
        case BUILDING_PALISADE_GATE:
            return 1;
        default:
            return 0;
    }
}
