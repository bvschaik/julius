#include "properties.h"

#include "assets/assets.h"
#include "core/image_group.h" 
#include "type.h"

#define AUGUSTUS_BUILDINGS 57

augustus_building_properties_mapping augustus_building_properties[AUGUSTUS_BUILDINGS] = {
    {BUILDING_ROADBLOCK, { 1, 1, 0, 0, 0 },  "Areldir", "Roadblocks", 0},
    {BUILDING_WORKCAMP, { 3, 0, 0, 0, 0 },  "Areldir", "Workcamps", "Workcamp Central"},
    {BUILDING_GRAND_TEMPLE_CERES, { 7, 1, 0, 0, 0 },  "Areldir", "Ceres_Temple", "Ceres Complex Const 01"},
    {BUILDING_GRAND_TEMPLE_NEPTUNE, { 7, 1, 0, 0, 0 },  "Areldir", "Neptune_Temple", "Neptune Complex Const 01"},
    {BUILDING_GRAND_TEMPLE_MERCURY, { 7, 1, 0, 0, 0 },  "Areldir", "Mercury_Temple", "Mercury Complex Const 01"},
    {BUILDING_GRAND_TEMPLE_MARS, { 7, 1, 0, 0, 0 },  "Areldir", "Mars_Temple", "Mars Complex Const 01"},
    {BUILDING_GRAND_TEMPLE_VENUS, { 7, 1, 0, 0, 0 },  "Areldir", "Venus_Temple", "Venus Complex Const 01"},
    {BUILDING_SMALL_POND, { 2, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "s pond south off"},
    {BUILDING_LARGE_POND, { 3, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "l pond south off"},
    {BUILDING_PINE_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental pine"},
    {BUILDING_FIR_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental fir"},
    {BUILDING_OAK_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental oak"},
    {BUILDING_ELM_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental elm"},
    {BUILDING_FIG_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental fig"},
    {BUILDING_PLUM_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental plum"},
    {BUILDING_PALM_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental palm"},
    {BUILDING_DATE_TREE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "ornamental date"},
    {BUILDING_PINE_PATH,{ 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn pine",},
    {BUILDING_FIR_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn fir",},
    {BUILDING_OAK_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn oak",},
    {BUILDING_ELM_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn elm"},
    {BUILDING_FIG_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn fig"},
    {BUILDING_PLUM_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn plum"},
    {BUILDING_PALM_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn palm"},
    {BUILDING_DATE_PATH, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "path orn date"},
    {BUILDING_PAVILION_BLUE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "pavilion blue"},
    {BUILDING_PAVILION_RED, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "pavilion red"},
    {BUILDING_PAVILION_ORANGE, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "pavilion orange"},
    {BUILDING_PAVILION_YELLOW, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "pavilion yellow"},
    {BUILDING_PAVILION_GREEN, { 1, 1, 0, 0, 0 },  "Areldir", "Aesthetics", "pavilion green"},
    {BUILDING_SMALL_STATUE_ALT, { 1, 1, 0, 0, 14 },  "Areldir", "Aesthetics", "sml statue 2"},
    {BUILDING_SMALL_STATUE_ALT_B, { 1, 1, 0, 0, 14 },  "Areldir", "Aesthetics", "sml statue 3"},
    {BUILDING_OBELISK, { 2, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "obelisk"},
    {BUILDING_PANTHEON, { 7, 1, 0, 0, 0 }, "Areldir", "Pantheon", "Pantheon Const 01"},
    {BUILDING_ARCHITECT_GUILD, { 2, 1, 0, 0, 0 }, "Areldir", "Architect", "Arch Guild OFF"},
    {BUILDING_MESS_HALL, { 3, 0, 0, 0, 0 }, "Areldir", "Mess_Hall", "Mess OFF Central"},
    {BUILDING_LIGHTHOUSE, { 3, 1, 0, 0, 0 }, "Areldir", "Lighthouses", "Lighthouse Const 01"},
    {BUILDING_TAVERN, { 2, 0, 0, 0, 0 }, "Areldir", "Entertainment", "Tavern OFF"},
    {BUILDING_GRAND_GARDEN, { 2, 1, 0, 0, 0 }, "Areldir", "Engineer", "Eng Guild OFF"},
    {BUILDING_ARENA, { 3, 0, 0, 0, 0 }, "Areldir", "Entertainment", "Arena OFF" },
    {BUILDING_HORSE_STATUE, { 3, 1, 0, 0, 1 }, "Areldir", "Aesthetics", "Eque Statue"},
    {BUILDING_DOLPHIN_FOUNTAIN, { 2, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "Eng Guild OFF"},
    {BUILDING_HEDGE_DARK, { 1, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "D Hedge 01"},
    {BUILDING_HEDGE_LIGHT, { 1, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "L Hedge 01"},
    {BUILDING_GARDEN_WALL, { 1, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "Eng Guild OFF"},
    {BUILDING_LEGION_STATUE, { 2, 1, 0, 0, 1 }, "Areldir", "Aesthetics", "legio statue"},
    {BUILDING_DECORATIVE_COLUMN, { 1, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "sml col B"},
    {BUILDING_COLONNADE, { 1, 1, 0, 0, 0 }, "Lizzaran", "Aesthetics_L", "G Colonnade 01"},
    {BUILDING_GARDEN_PATH, { 1, 1, 0, 0, 0 }, "Areldir", "Aesthetics", "Garden Path 01"},
    {BUILDING_LARARIUM, {1,0,0,0,0}, "Areldir", "Minor_Monuments", "Lararium 01"},
    {BUILDING_NYMPHAEUM, {3,0,0,0,0}, "Areldir", "Minor_Monuments", "Nymphaeum OFF"},
    {BUILDING_SMALL_MAUSOLEUM, {2,0,0,0,1}, "Areldir", "Minor_Monuments", "Mausoleum S"},
    {BUILDING_LARGE_MAUSOLEUM, {3,0,0,0,0}, "Areldir", "Minor_Monuments", "Mausoleum L"},
    {BUILDING_WATCHTOWER, {2,1,0,0,0}, "Areldir", "Watchtowers", "Watchtower C OFF"},
    {BUILDING_LIBRARY, {2,0,0,0,0}, "Tomasz", "Building_Upgrades", "Downgraded_Library"},
    {BUILDING_CARAVANSERAI, { 4, 0, 0, 0, 0 },  "Areldir", "Econ_Logistics", "Caravanserai N OFF"},
    {BUILDING_SMALL_STATUE, {1,1,0,0,-12}, "Lizzaran", "Aesthetics_L", "V Small Statue" }
    //    {BUILDING_PALLISADE, {1,0,0,0,0}, "Areldir", "Palisade", "Palisade"},
};

void init_augustus_building_properties()
{
    for (int i = 0; i < AUGUSTUS_BUILDINGS; ++i) {
        int group = assets_get_group_id(augustus_building_properties[i].asset_author, augustus_building_properties[i].asset_name);
        if (augustus_building_properties[i].asset_image_id) {
            augustus_building_properties[i].properties.image_group = assets_get_image_id(group, augustus_building_properties[i].asset_image_id);
        } else {
            augustus_building_properties[i].properties.image_group = group;
        }
    }
}

static building_properties properties[170] = {
    // SZ FIRE GRP OFF
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {1, 0, 112, 0, 0 },
        {1, 0,  24, 26, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,  19, 2, 0 },
        {0, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {1, 0,   0, 0, 0 },
        {2, 0,   0, 0, 0 },
        {2, 0,   0, 0, 0 },
        {2, 0,   0, 0, 0 },
        {2, 0,   0, 0, 0 },
        {3, 0,   0, 0, 0 },
        {3, 0,   0, 0, 0 },
        {3, 0,   0, 0, 0 },
        {3, 0,   0, 0, 0 },
        {4, 0,   0, 0, 0 },
        {4, 0,   0, 0, 0 },
        {3, 0,  45, 0, 0 },
        {2, 0,  46, 0, 0 },
        {5, 1, 213, 0, 0 },
        {5, 1,  48, 0, 0 },
        {3, 0,  49, 0, 0 },
        {3, 0,  50, 0, 0 },
        {3, 0,  51, 0, 0 },
        {3, 0,  52, 0, 0 },
        {1, 1,  58, 0, 0 },
        {1, 1,  59, 1, 0 },
        {3, 1,  66, 0, 0 },
        {1, 1,  61, 0, 0 },
        {2, 1,  61, 1, 0 },
        {3, 1,  61, 2, 0 },
        {3, 1,  66, 0, 0 },
        {3, 1,  66, 0, 0 },
        {1, 0,  68, 0, 0 },
        {3, 0,  70, 0, 0 },
        {2, 0, 185, 0, 0 },
        {1, 0,  67, 0, 0 },
        {3, 0,  66, 0, 0 },
        {2, 0,  41, 0, 0 },
        {3, 0,  43, 0, 0 },
        {2, 0,  42, 0, 0 },
        {4, 1,  66, 1, 0 },
        {1, 0,  64, 0, 0 },
        {3, 1, 205, 0, 0 },
        {3, 1,  66, 0, 0 },
        {2, 1,  17, 1, 0 },
        {2, 1,  17, 0, 0 },
        {2, 0,  71, 0, 0 },
        {2, 0,  72, 0, 0 },
        {2, 0,  73, 0, 0 },
        {2, 0,  74, 0, 0 },
        {2, 0,  75, 0, 0 },
        {3, 0,  71, 1, 0 },
        {3, 0,  72, 1, 0 },
        {3, 0,  73, 1, 0 },
        {3, 0,  74, 1, 0 },
        {3, 0,  75, 1, 0 },
        {2, 0,  22, 0, 0 },
        {3, 0,  99, 0, 0 },
        {1, 1,  82, 0, 0 },
        {1, 1,  82, 0, 0 },
        {2, 0,  77, 0, 0 },
        {3, 0,  78, 0, 0 },
        {2, 0,  79, 0, 0 },
        {3, 0,  85, 0, 0 },
        {4, 0,  86, 0, 0 },
        {5, 0,  87, 0, 0 },
        {2, 1, 184, 0, 0 },
        {1, 1,  81, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {5, 0,  62, 0, 0 },
        {2, 0,  63, 0, 0 },
        {0, 0,   0, 0, 0 },
        {1, 1, 183, 0, 0 },
        {2, 1, 183, 2, 0 },
        {3, 1,  25, 0, 0 },
        {1, 1,  54, 0, 0 },
        {1, 1,  23, 0, 0 },
        {1, 1, 100, 0, 0 },
        {3, 0, 201, 0, 0 },
        {3, 0, 166, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {2, 0,  76, 0, 0 },
        {1, 1,   0, 0, 0 },
        {3, 0,  37, 0, 0 },
        {3, 0,  37, 0, 0 },
        {3, 0,  37, 0, 0 },
        {3, 0,  37, 0, 0 },
        {3, 0,  37, 0, 0 },
        {3, 0,  37, 0, 0 },
        {2, 0,  38, 0, 0 },
        {2, 0,  39, 0, 0 },
        {2, 0,  65, 0, 0 },
        {2, 0,  40, 0, 0 },
        {2, 0,  44, 0, 0 },
        {2, 0, 122, 0, 0 },
        {2, 0, 123, 0, 0 },
        {2, 0, 124, 0, 0 },
        {2, 0, 125, 0, 0 },
        {0, 0,   0, 0, 0 },
        {3, 0,   GROUP_BUILDING_WAREHOUSE, 0, 0 },
        {7, 0,   GROUP_BUILDING_TEMPLE_CERES, 1, 0 },
        {7, 0,   GROUP_BUILDING_TEMPLE_NEPTUNE, 1, 0 },
        {7, 0,   GROUP_BUILDING_TEMPLE_MERCURY, 1, 0 },
        {7, 0,   GROUP_BUILDING_TEMPLE_MARS, 1, 0 },
        {7, 0,   GROUP_BUILDING_TEMPLE_VENUS, 1, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {2, 1, 216, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {1, 1,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
        {0, 0,   0, 0, 0 },
};

static int is_vanilla_building_with_changed_properties(building_type type)
{
    switch (type) {
    case BUILDING_LIBRARY:
    case BUILDING_SMALL_STATUE:
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

const building_properties *building_properties_for_type(building_type type)
{
    if (type < 0 || type > BUILDING_TYPE_MAX) {
        return &properties[0];
    }
    if (type >= BUILDING_ROADBLOCK || is_vanilla_building_with_changed_properties(type)) {
        for (int i = 0; i < AUGUSTUS_BUILDINGS; ++i) {
            if (augustus_building_properties[i].type == type) {
                return &augustus_building_properties[i].properties;
            }
        }
    }

    return &properties[type];
}
