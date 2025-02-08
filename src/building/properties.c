#include "properties.h"

#include "assets/assets.h"
#include "core/image_group.h"
#include "sound/city.h"
#include "translation/translation.h"
#include "type.h"

#include <stddef.h>

static building_properties properties[BUILDING_TYPE_MAX] = {
    [BUILDING_ANY] = {
        .event_data.attr = "any",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_ANY
    },
    [BUILDING_MENU_FARMS] = {
        .event_data.attr = "farms|all_farms",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_FARMS
    },
    [BUILDING_MENU_RAW_MATERIALS] = {
        .event_data.attr = "raw_materials|all_raw_materials",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_RAW_MATERIALS
    },
    [BUILDING_MENU_WORKSHOPS] = {
        .event_data.attr = "workshops|all_workshops",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_WORKSHOPS
    },
    [BUILDING_CLEAR_LAND] = {
        .event_data.attr = "clear_land",
        .event_data.cannot_count = 1
    },
    [BUILDING_ROAD] = {
        .size = 1,
        .image_group = 112,
        .event_data.attr = "road",
        .event_data.cannot_count = 1
    },
    [BUILDING_WALL] = {
        .size = 1,
        .image_group = 24,
        .image_offset = 26,
        .event_data.attr = "wall"
    },
    [BUILDING_DRAGGABLE_RESERVOIR] = {
        .size = 1,
        .event_data.attr = "reservoir"
    },
    [BUILDING_AQUEDUCT] = {
        .size = 1,
        .image_group = 19,
        .image_offset = 2,
        .event_data.attr = "aqueduct"
    },
    [BUILDING_HOUSE_SMALL_TENT] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_SLUM1,
        .event_data.attr = "vacant_lot|house_small_tent|housing"
    },
    [BUILDING_HOUSE_LARGE_TENT] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_SLUM2,
        .event_data.attr = "house_large_tent"
    },
    [BUILDING_HOUSE_SMALL_SHACK] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_SLUM3,
        .event_data.attr = "house_small_shack"
    },
    [BUILDING_HOUSE_LARGE_SHACK] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_SLUM4,
        .event_data.attr = "house_large_shack"
    },
    [BUILDING_HOUSE_SMALL_HOVEL] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_POOR1,
        .event_data.attr = "house_small_hovel"
    },
    [BUILDING_HOUSE_LARGE_HOVEL] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_POOR2,
        .event_data.attr = "house_large_hovel"
    },
    [BUILDING_HOUSE_SMALL_CASA] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_POOR3,
        .event_data.attr = "house_small_casa"
    },
    [BUILDING_HOUSE_LARGE_CASA] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_POOR4,
        .event_data.attr = "house_large_casa"
    },
    [BUILDING_HOUSE_SMALL_INSULA] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_MEDIUM1,
        .event_data.attr = "house_small_insula"
    },
    [BUILDING_HOUSE_MEDIUM_INSULA] = {
        .size = 1,
        .sound_id = SOUND_CITY_HOUSE_MEDIUM2,
        .event_data.attr = "house_medium_insula"
    },
    [BUILDING_HOUSE_LARGE_INSULA] = {
        .size = 2,
        .sound_id = SOUND_CITY_HOUSE_MEDIUM3,
        .event_data.attr = "house_large_insula"
    },
    [BUILDING_HOUSE_GRAND_INSULA] = {
        .size = 2,
        .sound_id = SOUND_CITY_HOUSE_MEDIUM4,
        .event_data.attr = "house_grand_insula"
    },
    [BUILDING_HOUSE_SMALL_VILLA] = {
        .size = 2,
        .sound_id = SOUND_CITY_HOUSE_GOOD1,
        .event_data.attr = "house_small_villa"
    },
    [BUILDING_HOUSE_MEDIUM_VILLA] = {
        .size = 2,
        .sound_id = SOUND_CITY_HOUSE_GOOD2,
        .event_data.attr = "house_medium_villa"
    },
    [BUILDING_HOUSE_LARGE_VILLA] = {
        .size = 3,
        .sound_id = SOUND_CITY_HOUSE_GOOD3,
        .event_data.attr = "house_large_villa"
    },
    [BUILDING_HOUSE_GRAND_VILLA] = {
        .size = 3,
        .sound_id = SOUND_CITY_HOUSE_GOOD4,
        .event_data.attr = "house_grand_villa"
    },
    [BUILDING_HOUSE_SMALL_PALACE] = {
        .size = 3,
        .sound_id = SOUND_CITY_HOUSE_POSH1,
        .event_data.attr = "house_small_palace"
    },
    [BUILDING_HOUSE_MEDIUM_PALACE] = {
        .size = 3,
        .sound_id = SOUND_CITY_HOUSE_POSH2,
        .event_data.attr = "house_medium_palace"
    },
    [BUILDING_HOUSE_LARGE_PALACE] = {
        .size = 4,
        .sound_id = SOUND_CITY_HOUSE_POSH3,
        .event_data.attr = "house_large_palace"
    },
    [BUILDING_HOUSE_LUXURY_PALACE] = {
        .size = 4,
        .sound_id = SOUND_CITY_HOUSE_POSH4,
        .event_data.attr = "house_luxury_palace"
    },
    [BUILDING_AMPHITHEATER] = {
        .size = 3,
        .image_group = 45,
        .sound_id = SOUND_CITY_AMPHITHEATER,
        .event_data.attr = "amphitheater"
    },
    [BUILDING_THEATER] = {
        .size = 2,
        .image_group = 46,
        .sound_id = SOUND_CITY_THEATER,
        .event_data.attr = "theater"
    },
    [BUILDING_HIPPODROME] = {
        .size = 5,
        .fire_proof = 1,
        .image_group = 213,
        .sound_id = SOUND_CITY_HIPPODROME,
        .event_data.attr = "hippodrome"
    },
    [BUILDING_COLOSSEUM] = {
        .size = 5,
        .fire_proof = 1,
        .image_group = 48,
        .sound_id = SOUND_CITY_COLOSSEUM,
        .event_data.attr = "colosseum"
    },
    [BUILDING_GLADIATOR_SCHOOL] = {
        .size = 3,
        .image_group = 49,
        .sound_id = SOUND_CITY_GLADIATOR_SCHOOL,
        .event_data.attr = "gladiator_school"
    },
    [BUILDING_LION_HOUSE] = {
        .size = 3,
        .image_group = 50,
        .sound_id = SOUND_CITY_LION_PIT,
        .event_data.attr = "lion_house"
    },
    [BUILDING_ACTOR_COLONY] = {
        .size = 3,
        .image_group = 51,
        .sound_id = SOUND_CITY_ACTOR_COLONY,
        .event_data.attr = "actor_colony"
    },
    [BUILDING_CHARIOT_MAKER] = {
        .size = 3,
        .image_group = 52,
        .sound_id = SOUND_CITY_CHARIOT_MAKER,
        .event_data.attr = "chariot_maker"
    },
    [BUILDING_PLAZA] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 58,
        .event_data.attr = "plaza",
        .event_data.cannot_count = 1
    },
    [BUILDING_GARDENS] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 59,
        .image_offset = 1,
        .sound_id = SOUND_CITY_GARDEN,
        .event_data.attr = "gardens",
        .event_data.cannot_count = 1
    },
    [BUILDING_FORT_LEGIONARIES] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 66,
        .event_data.attr = "fort_legionaries"
    },
    [BUILDING_SMALL_STATUE] = {
        .size = 1,
        .fire_proof = 1,
        .rotation_offset = -12,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "V Small Statue",
        .event_data.attr = "small_statue"
    },
    [BUILDING_MEDIUM_STATUE] = {
        .size = 2,
        .fire_proof = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Med_Statue_R",
        .event_data.attr = "medium_statue"
    },
    [BUILDING_LARGE_STATUE] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 61,
        .image_offset = 2,
        .event_data.attr = "large_statue"
    },
    [BUILDING_FORT_JAVELIN] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 66,
        .event_data.attr = "fort_javelin"
    },
    [BUILDING_FORT_MOUNTED] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 66,
        .event_data.attr = "fort_mounted"
    },
    [BUILDING_DOCTOR] = {
        .size = 1,
        .image_group = 68,
        .sound_id = SOUND_CITY_CLINIC,
        .event_data.attr = "doctor"
    },
    [BUILDING_HOSPITAL] = {
        .size = 3,
        .image_group = 70,
        .sound_id = SOUND_CITY_HOSPITAL,
        .event_data.attr = "hospital"
    },
    [BUILDING_BATHHOUSE] = {
        .size = 2,
        .image_group = 185,
        .sound_id = SOUND_CITY_BATHHOUSE,
        .event_data.attr = "bathhouse"
    },
    [BUILDING_BARBER] = {
        .size = 1,
        .image_group = 67,
        .sound_id = SOUND_CITY_BARBER,
        .event_data.attr = "barber"
    },
    [BUILDING_DISTRIBUTION_CENTER_UNUSED] = {
        .size = 3,
        .image_group = 66
    },
    [BUILDING_SCHOOL] = {
        .size = 2,
        .image_group = 41,
        .sound_id = SOUND_CITY_SCHOOL,
        .event_data.attr = "school"
    },
    [BUILDING_ACADEMY] = {
        .size = 3,
        .image_group = 43,
        .sound_id = SOUND_CITY_ACADEMY,
        .event_data.attr = "academy"
    },
    [BUILDING_LIBRARY] = {
        .size = 2,
        .sound_id = SOUND_CITY_LIBRARY,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Downgraded_Library",
        .event_data.attr = "library"
    },
    [BUILDING_FORT_GROUND] = {
        .size = 4,
        .fire_proof = 1,
        .image_group = 66,
        .image_offset = 1
    },
    [BUILDING_PREFECTURE] = {
        .size = 1,
        .image_group = 64,
        .sound_id = SOUND_CITY_PREFECTURE,
        .event_data.attr = "prefecture"
    },
    [BUILDING_TRIUMPHAL_ARCH] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 205,
        .event_data.attr = "triumphal_arch"
    },
    [BUILDING_FORT] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 66,
        .sound_id = SOUND_CITY_FORT,
        .event_data.attr = "fort"
    },
    [BUILDING_GATEHOUSE] = {
        .size = 2,
        .fire_proof = 1,
        .image_group = 17,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TOWER,
        .event_data.attr = "gatehouse"
    },
    [BUILDING_TOWER] = {
        .size = 2,
        .fire_proof = 1,
        .image_group = 17,
        .sound_id = SOUND_CITY_TOWER,
        .event_data.attr = "tower"
    },
    [BUILDING_SMALL_TEMPLE_CERES] = {
        .size = 2,
        .image_group = 71,
        .sound_id = SOUND_CITY_TEMPLE_CERES,
        .event_data.attr = "small_temple_ceres",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_SMALL_TEMPLE_CERES
    },
    [BUILDING_SMALL_TEMPLE_NEPTUNE] = {
        .size = 2,
        .image_group = 72,
        .sound_id = SOUND_CITY_TEMPLE_NEPTUNE,
        .event_data.attr = "small_temple_neptune",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_SMALL_TEMPLE_NEPTUNE
    },
    [BUILDING_SMALL_TEMPLE_MERCURY] = {
        .size = 2,
        .image_group = 73,
        .sound_id = SOUND_CITY_TEMPLE_MERCURY,
        .event_data.attr = "small_temple_mercury",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_SMALL_TEMPLE_MERCURY
    },
    [BUILDING_SMALL_TEMPLE_MARS] = {
        .size = 2,
        .image_group = 74,
        .sound_id = SOUND_CITY_TEMPLE_MARS,
        .event_data.attr = "small_temple_mars",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_SMALL_TEMPLE_MARS
    },
    [BUILDING_SMALL_TEMPLE_VENUS] = {
        .size = 2,
        .image_group = 75,
        .sound_id = SOUND_CITY_TEMPLE_VENUS,
        .event_data.attr = "small_temple_venus",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_SMALL_TEMPLE_VENUS
    },
    [BUILDING_LARGE_TEMPLE_CERES] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 71,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TEMPLE_CERES,
        .event_data.attr = "large_temple_ceres",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_LARGE_TEMPLE_CERES
    },
    [BUILDING_LARGE_TEMPLE_NEPTUNE] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 72,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TEMPLE_NEPTUNE,
        .event_data.attr = "large_temple_neptune",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_LARGE_TEMPLE_NEPTUNE
    },
    [BUILDING_LARGE_TEMPLE_MERCURY] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 73,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TEMPLE_MERCURY,
        .event_data.attr = "large_temple_mercury",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_LARGE_TEMPLE_MERCURY
    },
    [BUILDING_LARGE_TEMPLE_MARS] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 74,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TEMPLE_MARS,
        .event_data.attr = "large_temple_mars",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_LARGE_TEMPLE_MARS
    },
    [BUILDING_LARGE_TEMPLE_VENUS] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 75,
        .image_offset = 1,
        .sound_id = SOUND_CITY_TEMPLE_VENUS,
        .event_data.attr = "large_temple_venus",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_LARGE_TEMPLE_VENUS
    },
    [BUILDING_MARKET] = {
        .size = 2,
        .image_group = 22,
        .sound_id = SOUND_CITY_MARKET,
        .event_data.attr = "market"
    },
    [BUILDING_GRANARY] = {
        .size = 3,
        .image_group = 99,
        .sound_id = SOUND_CITY_GRANARY,
        .event_data.attr = "granary"
    },
    [BUILDING_WAREHOUSE] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 82,
        .sound_id = SOUND_CITY_WAREHOUSE,
        .event_data.attr = "warehouse"
    },
    [BUILDING_WAREHOUSE_SPACE] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 82
    },
    [BUILDING_SHIPYARD] = {
        .size = 2,
        .image_group = 77,
        .sound_id = SOUND_CITY_SHIPYARD,
        .event_data.attr = "shipyard"
    },
    [BUILDING_DOCK] = {
        .size = 3,
        .image_group = 78,
        .sound_id = SOUND_CITY_DOCK,
        .event_data.attr = "dock"
    },
    [BUILDING_WHARF] = {
        .size = 2,
        .image_group = 79,
        .sound_id = SOUND_CITY_WHARF,
        .event_data.attr = "wharf"
    },
    [BUILDING_GOVERNORS_HOUSE] = {
        .size = 3,
        .image_group = 85,
        .sound_id = SOUND_CITY_PALACE,
        .event_data.attr = "governors_house"
    },
    [BUILDING_GOVERNORS_VILLA] = {
        .size = 4,
        .image_group = 86,
        .sound_id = SOUND_CITY_PALACE,
        .event_data.attr = "governors_villa"
    },
    [BUILDING_GOVERNORS_PALACE] = {
        .size = 5,
        .image_group = 87,
        .sound_id = SOUND_CITY_PALACE,
        .event_data.attr = "governors_palace"
    },
    [BUILDING_MISSION_POST] = {
        .size = 2,
        .fire_proof = 1,
        .image_group = 184,
        .sound_id = SOUND_CITY_MISSION_POST,
        .event_data.attr = "mission_post"
    },
    [BUILDING_ENGINEERS_POST] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 81,
        .sound_id = SOUND_CITY_ENGINEERS_POST,
        .event_data.attr = "engineers_post"
    },
    [BUILDING_LOW_BRIDGE] = {
        .size = 1,
        .fire_proof = 1
    },
    [BUILDING_SHIP_BRIDGE] = {
        .size = 1,
        .fire_proof = 1
    },
    [BUILDING_SENATE] = {
        .size = 5,
        .image_group = 62,
        .sound_id = SOUND_CITY_SENATE,
        .event_data.attr = "senate"
    },
    [BUILDING_FORUM] = {
        .size = 2,
        .image_group = 63,
        .sound_id = SOUND_CITY_FORUM,
        .event_data.attr = "forum"
    },
    [BUILDING_NATIVE_HUT] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 183,
        .event_data.attr = "native_hut"
    },
    [BUILDING_NATIVE_MEETING] = {
        .size = 2,
        .fire_proof = 1,
        .image_group = 183,
        .image_offset = 2,
        .event_data.attr = "native_meeting"
    },
    [BUILDING_RESERVOIR] = {
        .size = 3,
        .fire_proof = 1,
        .image_group = 25,
        .sound_id = SOUND_CITY_RESERVOIR,
        .event_data.attr = "reservoir"
    },
    [BUILDING_FOUNTAIN] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 54,
       // .sound_id = SOUND_CITY_FOUNTAIN, // Disabled in original
        .event_data.attr = "fountain"
    },
    [BUILDING_WELL] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 23,
       // .sound_id = SOUND_CITY_WELL, // Disabled in original
        .event_data.attr = "well"
    },
    [BUILDING_NATIVE_CROPS] = {
        .size = 1,
        .fire_proof = 1,
        .image_group = 100,
        .event_data.attr = "native_crops",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_NATIVE_CROPS
    },
    [BUILDING_MILITARY_ACADEMY] = {
        .size = 3,
        .image_group = 201,
        .sound_id = SOUND_CITY_MILITARY_ACADEMY,
        .event_data.attr = "military_academy"
    },
    [BUILDING_BARRACKS] = {
        .size = 3,
        .image_group = 166,
        .sound_id = SOUND_CITY_BARRACKS,
        .event_data.attr = "barracks"
    },
    [BUILDING_MENU_SMALL_TEMPLES] = {
        .event_data.attr = "small_temples|all_small_temples",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_SMALL_TEMPLES
    },
    [BUILDING_MENU_LARGE_TEMPLES] = {
        .event_data.attr = "large_temples|all_large_temples",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_LARGE_TEMPLES
    },
    [BUILDING_ORACLE] = {
        .size = 2,
        .fire_proof = 1,
        .image_group = 76,
        .sound_id = SOUND_CITY_ORACLE,
        .event_data.attr = "amphitheater"
    },
    [BUILDING_BURNING_RUIN] = {
        .size = 1,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_BURNING_RUIN
    },
    [BUILDING_WHEAT_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_WHEAT_FARM,
        .event_data.attr = "wheat_farm"
    },
    [BUILDING_VEGETABLE_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_VEGETABLE_FARM,
        .event_data.attr = "vegetable_farm"
    },
    [BUILDING_FRUIT_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_FRUIT_FARM,
        .event_data.attr = "fruit_farm"
    },
    [BUILDING_OLIVE_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_OLIVE_FARM,
        .event_data.attr = "olive_farm"
    },
    [BUILDING_VINES_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_VINE_FARM,
        .event_data.attr = "vines_farm"
    },
    [BUILDING_PIG_FARM] = {
        .size = 3,
        .image_group = 37,
        .sound_id = SOUND_CITY_PIG_FARM,
        .event_data.attr = "pig_farm"
    },
    [BUILDING_MARBLE_QUARRY] = {
        .size = 2,
        .image_group = 38,
        .sound_id = SOUND_CITY_QUARRY,
        .event_data.attr = "marble_quarry"
    },
    [BUILDING_IRON_MINE] = {
        .size = 2,
        .image_group = 39,
        .sound_id = SOUND_CITY_IRON_MINE,
        .event_data.attr = "iron_mine"
    },
    [BUILDING_TIMBER_YARD] = {
        .size = 2,
        .image_group = 65,
        .sound_id = SOUND_CITY_TIMBER_YARD,
        .event_data.attr = "timber_yard"
    },
    [BUILDING_CLAY_PIT] = {
        .size = 2,
        .image_group = 40,
        .sound_id = SOUND_CITY_CLAY_PIT,
        .event_data.attr = "clay_pit"
    },
    [BUILDING_WINE_WORKSHOP] = {
        .size = 2,
        .image_group = 44,
        .sound_id = SOUND_CITY_WINE_WORKSHOP,
        .event_data.attr = "wine_workshop"
    },
    [BUILDING_OIL_WORKSHOP] = {
        .size = 2,
        .image_group = 122,
        .sound_id = SOUND_CITY_OIL_WORKSHOP,
        .event_data.attr = "oil_workshop"
    },
    [BUILDING_WEAPONS_WORKSHOP] = {
        .size = 2,
        .image_group = 123,
        .sound_id = SOUND_CITY_WEAPONS_WORKSHOP,
        .event_data.attr = "weapons_workshop"
    },
    [BUILDING_FURNITURE_WORKSHOP] = {
        .size = 2,
        .image_group = 124,
        .sound_id = SOUND_CITY_FURNITURE_WORKSHOP,
        .event_data.attr = "furniture_workshop"
    },
    [BUILDING_POTTERY_WORKSHOP] = {
        .size = 2,
        .image_group = 125,
        .sound_id = SOUND_CITY_POTTERY_WORKSHOP,
        .event_data.attr = "pottery_workshop"
    },
    [BUILDING_ROADBLOCK] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Admin_Logistics",
        .event_data.attr = "roadblock"
    },
    [BUILDING_WORKCAMP] = {
        .size = 3,
        .sound_id = SOUND_CITY_WORKCAMP,
        .custom_asset.group = "Admin_Logistics",
        .custom_asset.id = "Workcamp Central",
        .event_data.attr = "workcamp"
    },
    [BUILDING_GRAND_TEMPLE_CERES] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_TEMPLE_CERES,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Ceres Complex Off",
        .event_data.attr = "grand_temple_ceres"
    },
    [BUILDING_GRAND_TEMPLE_NEPTUNE] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_TEMPLE_NEPTUNE,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Neptune Complex Off",
        .event_data.attr = "grand_temple_neptune"
    },
    [BUILDING_GRAND_TEMPLE_MERCURY] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_TEMPLE_MERCURY,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Mercury Complex Off",
        .event_data.attr = "grand_temple_mercury"
    },
    [BUILDING_GRAND_TEMPLE_MARS] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_TEMPLE_MARS,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Mars Complex Off",
        .event_data.attr = "grand_temple_mars"
    },
    [BUILDING_GRAND_TEMPLE_VENUS] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_TEMPLE_VENUS,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Venus Complex Off",
        .event_data.attr = "grand_temple_venus"
    },
    [BUILDING_MENU_GRAND_TEMPLES] = {
        .event_data.attr = "grand_temples|all_grand_temples",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_GRAND_TEMPLES
    },
    [BUILDING_MENU_TREES] = {
        .event_data.attr = "trees|all_trees",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_TREES
    },
    [BUILDING_MENU_PATHS] = {
        .event_data.attr = "paths|all_paths",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_PATHS
    },
    [BUILDING_MENU_PARKS] = {
        .event_data.attr = "parks|all_parks",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_PARKS
    },
    [BUILDING_SMALL_POND] = {
        .size = 2,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "s pond south off",
        .event_data.attr = "small_pond"
    },
    [BUILDING_LARGE_POND] = {
        .size = 3,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "l pond south off",
        .event_data.attr = "large_pond"
    },
    [BUILDING_PINE_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental pine",
        .event_data.attr = "pine_tree"
    },
    [BUILDING_FIR_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental fir",
        .event_data.attr = "fir_tree"
    },
    [BUILDING_OAK_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental oak",
        .event_data.attr = "oak_tree"
    },
    [BUILDING_ELM_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental elm",
        .event_data.attr = "elm_tree"
    },
    [BUILDING_FIG_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental fig",
        .event_data.attr = "fig_tree"
    },
    [BUILDING_PLUM_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental plum",
        .event_data.attr = "plum_tree"
    },
    [BUILDING_PALM_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental palm",
        .event_data.attr = "palm_tree"
    },
    [BUILDING_DATE_TREE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "ornamental date",
        .event_data.attr = "date_tree"
    },
    [BUILDING_PINE_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn pine",
        .event_data.attr = "pine_tree"
    },
    [BUILDING_FIR_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn fir",
        .event_data.attr = "fir_path"
    },
    [BUILDING_OAK_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn oak",
        .event_data.attr = "oak_path"
    },
    [BUILDING_ELM_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn elm",
        .event_data.attr = "elm_path"
    },
    [BUILDING_FIG_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn fig",
        .event_data.attr = "fig_path"
    },
    [BUILDING_PLUM_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn plum",
        .event_data.attr = "plum_path"
    },
    [BUILDING_PALM_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn palm",
        .event_data.attr = "palm_path"
    },
    [BUILDING_DATE_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "path orn date",
        .event_data.attr = "date_path"
    },
    [BUILDING_PAVILION_BLUE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "pavilion blue",
        .event_data.attr = "pavilion_blue"
    },
    [BUILDING_PAVILION_RED] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "pavilion red",
        .event_data.attr = "pavilion_red"
    },
    [BUILDING_PAVILION_ORANGE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "pavilion orange",
        .event_data.attr = "pavilion_orange"
    },
    [BUILDING_PAVILION_YELLOW] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "pavilion yellow",
        .event_data.attr = "pavilion_yellow"
    },
    [BUILDING_PAVILION_GREEN] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "pavilion green",
        .event_data.attr = "pavilion_green"
    },
    [BUILDING_GODDESS_STATUE] = {
        .size = 1,
        .fire_proof = 1,
        .rotation_offset = 13,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "sml statue 2",
        .event_data.attr = "goddess_statue|small_statue_alt"
    },
    [BUILDING_SENATOR_STATUE] = {
        .size = 1,
        .fire_proof = 1,
        .rotation_offset = 13,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "sml statue 3",
        .event_data.attr = "senator_statue|small_statue_alt_b"
    },
    [BUILDING_OBELISK] = {
        .size = 2,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "obelisk",
        .event_data.attr = "obelisk"
    },
    [BUILDING_PANTHEON] = {
        .size = 7,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_ORACLE,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Pantheon Off",
        .event_data.attr = "pantheon"
    },
    [BUILDING_ARCHITECT_GUILD] = {
        .size = 2,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_ENGINEERS_POST,
        .custom_asset.group = "Admin_Logistics",
        .custom_asset.id = "Arch Guild OFF",
        .event_data.attr = "architect_guild"
    },
    [BUILDING_MESS_HALL] = {
        .size = 3,
        .sound_id = SOUND_CITY_MESS_HALL,
        .custom_asset.group = "Military",
        .custom_asset.id = "Mess OFF Central",
        .event_data.attr = "mess_hall"
    },
    [BUILDING_LIGHTHOUSE] = {
        .size = 3,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_LIGHTHOUSE,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Lighthouse OFF",
        .event_data.attr = "lighthouse"
    },
    [BUILDING_MENU_STATUES] = {
        .event_data.attr = "statues|all_statues",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_STATUES
    },
    [BUILDING_MENU_GOV_RES] = {
        .event_data.attr = "governor_home|all_governors_houses",
        .event_data.key = TR_PARAMETER_VALUE_BUILDING_MENU_GOV_RES
    },
    [BUILDING_TAVERN] = {
        .size = 2,
        .sound_id = SOUND_CITY_TAVERN,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Tavern OFF",
        .event_data.attr = "tavern"
    },
    [BUILDING_GRAND_GARDEN] = {
        .size = 2,
        .fire_proof = 1,
        .event_data.attr = "grand_garden"
    },
    [BUILDING_ARENA] = {
        .size = 3,
        .sound_id = SOUND_CITY_COLOSSEUM,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Arena OFF",
        .event_data.attr = "arena"
    },
    [BUILDING_HORSE_STATUE] = {
        .size = 3,
        .fire_proof = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Eque Statue",
        .event_data.attr = "horse_statue"
    },
    [BUILDING_DOLPHIN_FOUNTAIN] = {
        .size = 2,
        .fire_proof = 1,
        .event_data.attr = "dolphin_fountain"
    },
    [BUILDING_HEDGE_DARK] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "D Hedge 01",
        .event_data.attr = "hedge_dark"
    },
    [BUILDING_HEDGE_LIGHT] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "L Hedge 01",
        .event_data.attr = "hedge_light"
    },
    [BUILDING_LOOPED_GARDEN_WALL] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "C Garden Wall 01",
        .event_data.attr = "looped_garden_wall"
    },
    [BUILDING_LEGION_STATUE] = {
        .size = 2,
        .fire_proof = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "legio statue",
        .event_data.attr = "legion_statue"
    },
    [BUILDING_DECORATIVE_COLUMN] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "sml col B",
        .event_data.attr = "decorative_column"
    },
    [BUILDING_COLONNADE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "G Colonnade 01",
        .event_data.attr = "colonnade"
    },
    [BUILDING_GARDEN_PATH] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Garden Path 01",
        .event_data.attr = "garden_path"
    },
    [BUILDING_LARARIUM] = {
        .size = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Lararium 01",
        .event_data.attr = "lararium"
    },
    [BUILDING_NYMPHAEUM] = {
        .size = 3,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_ORACLE,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Nymphaeum OFF",
        .event_data.attr = "nymphaeum"
    },
    [BUILDING_SMALL_MAUSOLEUM] = {
        .size = 2,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_ORACLE,
        .rotation_offset = 1,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Mausoleum S",
        .event_data.attr = "small_mausoleum"
    },
    [BUILDING_LARGE_MAUSOLEUM] = {
        .size = 3,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_ORACLE,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Mausoleum L",
        .event_data.attr = "large_mausoleum"
    },
    [BUILDING_WATCHTOWER] = {
        .size = 2,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_WATCHTOWER,
        .custom_asset.group = "Military",
        .custom_asset.id = "Watchtower C OFF",
        .event_data.attr = "watchtower"
    },
    [BUILDING_CARAVANSERAI] = {
        .size = 4,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_CARAVANSERAI,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "Caravanserai_C_OFF",
        .event_data.attr = "caravanserai"
    },
    [BUILDING_ROOFED_GARDEN_WALL] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "R Garden Wall 01",
        .event_data.attr = "roofed_garden_wall"
    },
    [BUILDING_ROOFED_GARDEN_WALL_GATE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Garden_Gate_B",
        .event_data.attr = "garden_wall_gate"
    },
    [BUILDING_PALISADE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Military",
        .custom_asset.id = "Pal Wall C 01",
        .event_data.attr = "palisade"
    },
    [BUILDING_HEDGE_GATE_DARK] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "D Hedge Gate",
        .event_data.attr = "hedge_gate_dark"
    },
    [BUILDING_HEDGE_GATE_LIGHT] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "L Hedge Gate",
        .event_data.attr = "hedge_gate_light"
    },
    [BUILDING_PALISADE_GATE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Military",
        .custom_asset.id = "Palisade_Gate",
        .event_data.attr = "palisade_gate"
    },
    [BUILDING_GLADIATOR_STATUE] = {
        .size = 1,
        .fire_proof = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Aesthetics",
        .event_data.attr = "gladiator_statue"
    },
    [BUILDING_HIGHWAY] = {
        .size = 2,
        .fire_proof = 1,
        .custom_asset.group = "Admin_Logistics",
        .custom_asset.id = "Highway_Placement",
        .event_data.attr = "highway",
        .event_data.cannot_count = 1
    },
    [BUILDING_GOLD_MINE] = {
        .size = 2,
        .custom_asset.group = "Industry",
        .custom_asset.id = "gold_mine"
    },
    [BUILDING_STONE_QUARRY] = {
        .size = 2,
        .custom_asset.group = "Industry",
        .custom_asset.id = "Stone_Quarry_C_OFF",
        .event_data.attr = "stone_quarry"
    },
    [BUILDING_SAND_PIT] = {
        .size = 2,
        .custom_asset.group = "Industry",
        .custom_asset.id = "Sand_Pit_C_OFF",
        .event_data.attr = "sand_pit"
    },
    [BUILDING_BRICKWORKS] = {
        .size = 2,
        .sound_id = SOUND_CITY_BRICKWORKS,
        .custom_asset.group = "Industry",
        .custom_asset.id = "Brickworks_C_OFF",
        .event_data.attr = "brickworks"
    },
    [BUILDING_CONCRETE_MAKER] = {
        .size = 2,
        .sound_id = SOUND_CITY_CONCRETE_MAKER,
        .custom_asset.group = "Industry",
        .custom_asset.id = "Concrete_Maker_C_OFF",
        .event_data.attr = "concrete_maker"
    },
    [BUILDING_CITY_MINT] = {
        .size = 3,
        .fire_proof = 1,
        .sound_id = SOUND_CITY_CITY_MINT,
        .custom_asset.group = "Monuments",
        .custom_asset.id = "City_Mint_ON",
        .event_data.attr = "city_mint"
    },
    [BUILDING_DEPOT] = {
        .size = 2,
        .sound_id = SOUND_CITY_DEPOT,
        .custom_asset.group = "Admin_Logistics",
        .custom_asset.id = "Cart Depot N OFF",
        .event_data.attr = "cart_depot"
    },
    [BUILDING_LOOPED_GARDEN_GATE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Garden_Gate",
        .event_data.attr = "looped_garden_gate"
    },
    [BUILDING_PANELLED_GARDEN_GATE] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Garden_Gate_C",
        .event_data.attr = "panelled_garden_gate"
    },
    [BUILDING_PANELLED_GARDEN_WALL] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Garden_Wall_C",
        .event_data.attr = "panelled_garden_wall"
    },
    [BUILDING_SHRINE_CERES] = {
        .size = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Altar_Ceres",
        .event_data.attr = "shrine_ceres"
    },
    [BUILDING_SHRINE_MARS] = {
        .size = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Altar_Mars",
        .event_data.attr = "shrine_mars"
    },
    [BUILDING_SHRINE_MERCURY] = {
        .size = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Altar_Mercury",
        .event_data.attr = "shrine_mercury"
    },
    [BUILDING_SHRINE_NEPTUNE] = {
        .size = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Altar_Neptune",
        .event_data.attr = "shrine_neptune"
    },
    [BUILDING_SHRINE_VENUS] = {
        .size = 1,
        .rotation_offset = 1,
        .custom_asset.group = "Health_Culture",
        .custom_asset.id = "Altar_Venus",
        .event_data.attr = "shrine_venus"
    },
    [BUILDING_OVERGROWN_GARDENS] = {
        .size = 1,
        .fire_proof = 1,
        .custom_asset.group = "Aesthetics",
        .custom_asset.id = "Overgrown_Garden_01",
        .event_data.attr = "overgrown_gardens"
    },
    [BUILDING_FORT_AUXILIA_INFANTRY] = {
        .size = 3,
        .fire_proof = 1,
        .custom_asset.group = "Military",
        .event_data.attr = "fort_auxilia_infantry"
    },
    [BUILDING_FORT_ARCHERS] = {
        .size = 3,
        .fire_proof = 1,
        .custom_asset.group = "Military",
        .event_data.attr = "fort_archers"
    },
    [BUILDING_ARMOURY] = {
        .size = 2,
        .sound_id = SOUND_CITY_ARMOURY,
        .custom_asset.group = "Military",
        .custom_asset.id = "Armoury_OFF_C",
        .event_data.attr = "armoury"
    }
};

void building_properties_init(void)
{
    for (building_type type = BUILDING_NONE; type < BUILDING_TYPE_MAX; type++) {
        if (!properties[type].custom_asset.group) {
            continue;
        }
        if (properties[type].custom_asset.id) {
            properties[type].image_group = assets_get_image_id(properties[type].custom_asset.group,
                properties[type].custom_asset.id);
        } else {
            properties[type].image_group = assets_get_group_id(properties[type].custom_asset.group);
        }
    }
}

const building_properties *building_properties_for_type(building_type type)
{
    if (type <= BUILDING_NONE || type >= BUILDING_TYPE_MAX) {
        return &properties[0];
    }
    return &properties[type];
}
