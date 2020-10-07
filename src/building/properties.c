#include "properties.h"
#include "core/image_group.h" 
#include "type.h"
#include "core/mods.h"

static building_properties properties[170] = {
// SZ FIRE GRP OFF
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {1, 0, 112, 0},
    {1, 0,  24, 26},
    {1, 0,   0, 0},
    {1, 0,  19, 2},
    {0, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {1, 0,   0, 0},
    {2, 0,   0, 0},
    {2, 0,   0, 0},
    {2, 0,   0, 0},
    {2, 0,   0, 0},
    {3, 0,   0, 0},
    {3, 0,   0, 0},
    {3, 0,   0, 0},
    {3, 0,   0, 0},
    {4, 0,   0, 0},
    {4, 0,   0, 0},
    {3, 0,  45, 0},
    {2, 0,  46, 0},
    {5, 0, 213, 0},
    {5, 0,  48, 0},
    {3, 0,  49, 0},
    {3, 0,  50, 0},
    {3, 0,  51, 0},
    {3, 0,  52, 0},
    {1, 1,  58, 0},
    {1, 1,  59, 1},
    {3, 1,  66, 0},
    {1, 1,  61, 0},
    {2, 1,  61, 1},
    {3, 1,  61, 2},
    {3, 1,  66, 0},
    {3, 1,  66, 0},
    {1, 0,  68, 0},
    {3, 0,  70, 0},
    {2, 0, 185, 0},
    {1, 0,  67, 0},
    {3, 0,  66, 0},
    {2, 0,  41, 0},
    {3, 0,  43, 0},
    {2, 0,  42, 0},
    {4, 1,  66, 1},
    {1, 0,  64, 0},
    {3, 1, 205, 0},
    {3, 1,  66, 0},
    {2, 1,  17, 1},
    {2, 1,  17, 0},
    {2, 0,  71, 0},
    {2, 0,  72, 0},
    {2, 0,  73, 0},
    {2, 0,  74, 0},
    {2, 0,  75, 0},
    {3, 0,  71, 1},
    {3, 0,  72, 1},
    {3, 0,  73, 1},
    {3, 0,  74, 1},
    {3, 0,  75, 1},
    {2, 0,  22, 0},
    {3, 0,  99, 0},
    {1, 1,  82, 0},
    {1, 1,  82, 0},
    {2, 0,  77, 0},
    {3, 0,  78, 0},
    {2, 0,  79, 0},
    {3, 0,  85, 0},
    {4, 0,  86, 0},
    {5, 0,  87, 0},
    {2, 1, 184, 0},
    {1, 1,  81, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {0, 0,   0, 0},
    {5, 0,  62, 0},
    {2, 0,  63, 0},
    {0, 0,   0, 0},
    {1, 1, 183, 0},
    {2, 1, 183, 2},
    {3, 1,  25, 0},
    {1, 1,  54, 0},
    {1, 1,  23, 0},
    {1, 1, 100, 0},
    {3, 0, 201, 0},
    {3, 0, 166, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {2, 0,  76, 0},
    {1, 1,   0, 0},
    {3, 0,  37, 0},
    {3, 0,  37, 0},
    {3, 0,  37, 0},
    {3, 0,  37, 0},
    {3, 0,  37, 0},
    {3, 0,  37, 0},
    {2, 0,  38, 0},
    {2, 0,  39, 0},
    {2, 0,  65, 0},
    {2, 0,  40, 0},
    {2, 0,  44, 0},
    {2, 0, 122, 0},
    {2, 0, 123, 0},
    {2, 0, 124, 0},
    {2, 0, 125, 0},
    {0, 0,   0, 0},
    {3, 0,   GROUP_BUILDING_WAREHOUSE, 0},
    {7, 0,   GROUP_BUILDING_TEMPLE_CERES, 1},
    {7, 0,   GROUP_BUILDING_TEMPLE_NEPTUNE, 1},
    {7, 0,   GROUP_BUILDING_TEMPLE_MERCURY, 1},
    {7, 0,   GROUP_BUILDING_TEMPLE_MARS, 1},
    {7, 0,   GROUP_BUILDING_TEMPLE_VENUS, 1},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {2, 1, 216, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {1, 1,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
    {0, 0,   0, 0},
};

// these are for modded assets and the group/image offset values are not static and should not be used
building_properties SMALL_CUSTOM_DECOR_PROPERTIES = { 1, 1, 0, 0 };
building_properties MEDIUM_CUSTOM_DECOR_PROPERTIES = { 2, 1, 0, 0 };
building_properties LARGE_CUSTOM_DECOR_PROPERTIES = {3, 1, 0, 0 };
building_properties GRAND_TEMPLE_PROPERTIES = { 7, 0, 0, 1 };
building_properties WORKCAMP_PROPERTIES = { 3, 0, 0, 0 };
building_properties ENGINEER_GUILD_PROPERTIES = { 2, 0, 0, 0 };

const building_properties *building_properties_for_type(building_type type)
{
    // Roadblock properties, as plaza
    if (type == BUILDING_ROADBLOCK){
        return &properties[38];
    }

    if (type == BUILDING_WORKCAMP) {
        return &WORKCAMP_PROPERTIES;
    }

    if (type == BUILDING_ENGINEER_GUILD) {
        return &ENGINEER_GUILD_PROPERTIES;
    }

    if (type >= BUILDING_GRAND_TEMPLE_CERES && type <= BUILDING_GRAND_TEMPLE_VENUS) {
        return &GRAND_TEMPLE_PROPERTIES;
    }

    if (type == BUILDING_SMALL_POND || type == BUILDING_OBELISK){
        return &MEDIUM_CUSTOM_DECOR_PROPERTIES;
    }

    if (type == BUILDING_LARGE_POND){
        return &LARGE_CUSTOM_DECOR_PROPERTIES;
    }

    if (type >= BUILDING_PINE_TREE && type <= BUILDING_SMALL_STATUE_ALT_B) {
        return &SMALL_CUSTOM_DECOR_PROPERTIES;
    }


    return &properties[type];
}

