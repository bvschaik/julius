#include "building.h"

#include "building/properties.h"
#include "city/warning.h"
#include "game/resource.h"
#include "map/grid.h"
#include "map/random.h"

#include "Data/CityInfo.h"
#include "Terrain.h"
#include "Undo.h"

#include <string.h>

static building Data_Buildings[MAX_BUILDINGS];

building *building_get(int id)
{
    return &Data_Buildings[id];
}

building *building_main(building *b)
{
    for (int guard = 0; guard < 9; guard++) {
        if (b->prevPartBuildingId <= 0) {
            return b;
        }
        b = &Data_Buildings[b->prevPartBuildingId];
    }
    return &Data_Buildings[0];
}

building *building_next(building *b)
{
    return &Data_Buildings[b->nextPartBuildingId];
}

building *building_create(building_type type, int x, int y)
{
    building *b = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (Data_Buildings[i].state == BuildingState_Unused && !Undo_isBuildingInList(i)) {
            b = &Data_Buildings[i];
            break;
        }
    }
    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED);
        return &Data_Buildings[0];
    }
    
    const building_properties *props = building_properties_for_type(type);
    
    b->state = BuildingState_Created;
    b->ciid = 1;
    b->__unknown_02 = Data_CityInfo.__unknown_00a5; // TODO ??
    b->type = type;
    b->size = props->size;
    b->createdSequence = Data_Buildings_Extra.createdSequence++;
    b->sentiment.houseHappiness = 50;
    b->distanceFromEntry = 0;
    
    // house size
    b->houseSize = 0;
    if (type >= BUILDING_HOUSE_SMALL_TENT && type <= BUILDING_HOUSE_MEDIUM_INSULA) {
        b->houseSize = 1;
    } else if (type >= BUILDING_HOUSE_LARGE_INSULA && type <= BUILDING_HOUSE_MEDIUM_VILLA) {
        b->houseSize = 2;
    } else if (type >= BUILDING_HOUSE_LARGE_VILLA && type <= BUILDING_HOUSE_MEDIUM_PALACE) {
        b->houseSize = 3;
    } else if (type >= BUILDING_HOUSE_LARGE_PALACE && type <= BUILDING_HOUSE_LUXURY_PALACE) {
        b->houseSize = 4;
    }
    
    // subtype
    if (BuildingIsHouse(type)) {
        b->subtype.houseLevel = type - 10;
    } else {
        b->subtype.houseLevel = 0;
    }
    
    // input/output resources
    switch (type) {
        case BUILDING_WHEAT_FARM:
            b->outputResourceId = RESOURCE_WHEAT;
            break;
        case BUILDING_VEGETABLE_FARM:
            b->outputResourceId = RESOURCE_VEGETABLES;
            break;
        case BUILDING_FRUIT_FARM:
            b->outputResourceId = RESOURCE_FRUIT;
            break;
        case BUILDING_OLIVE_FARM:
            b->outputResourceId = RESOURCE_OLIVES;
            break;
        case BUILDING_VINES_FARM:
            b->outputResourceId = RESOURCE_VINES;
            break;
        case BUILDING_PIG_FARM:
            b->outputResourceId = RESOURCE_MEAT;
            break;
        case BUILDING_MARBLE_QUARRY:
            b->outputResourceId = RESOURCE_MARBLE;
            break;
        case BUILDING_IRON_MINE:
            b->outputResourceId = RESOURCE_IRON;
            break;
        case BUILDING_TIMBER_YARD:
            b->outputResourceId = RESOURCE_TIMBER;
            break;
        case BUILDING_CLAY_PIT:
            b->outputResourceId = RESOURCE_CLAY;
            break;
        case BUILDING_WINE_WORKSHOP:
            b->outputResourceId = RESOURCE_WINE;
            b->subtype.workshopType = WORKSHOP_VINES_TO_WINE;
            break;
        case BUILDING_OIL_WORKSHOP:
            b->outputResourceId = RESOURCE_OIL;
            b->subtype.workshopType = WORKSHOP_OLIVES_TO_OIL;
            break;
        case BUILDING_WEAPONS_WORKSHOP:
            b->outputResourceId = RESOURCE_WEAPONS;
            b->subtype.workshopType = WORKSHOP_IRON_TO_WEAPONS;
            break;
        case BUILDING_FURNITURE_WORKSHOP:
            b->outputResourceId = RESOURCE_FURNITURE;
            b->subtype.workshopType = WORKSHOP_TIMBER_TO_FURNITURE;
            break;
        case BUILDING_POTTERY_WORKSHOP:
            b->outputResourceId = RESOURCE_POTTERY;
            b->subtype.workshopType = WORKSHOP_CLAY_TO_POTTERY;
            break;
        default:
            b->outputResourceId = RESOURCE_NONE;
            break;
    }
    
    if (type == BUILDING_GRANARY) {
        b->data.storage.resourceStored[RESOURCE_NONE] = 2400;
    }
    
    b->x = x;
    b->y = y;
    b->gridOffset = map_grid_offset(x, y);
    b->houseGenerationDelay = map_random_get(b->gridOffset) & 0x7f;
    b->figureRoamDirection = b->houseGenerationDelay & 6;
    b->fireProof = props->fire_proof;
    b->isAdjacentToWater = Terrain_isAdjacentToWater(x, y, b->size);

    return b;
}

void building_delete(building *b)
{
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;
}

void building_clear_all()
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&Data_Buildings[i], 0, sizeof(building));
        Data_Buildings[i].id = i;
    }
}

static void building_save(building *b, buffer *buf)
{
    buffer_write_raw(buf, b, 128);
}

static void building_load(building *b, buffer *buf)
{
    buffer_read_raw(buf, b, 128);
}

void building_save_state(buffer *buf)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_save(&Data_Buildings[i], buf);
    }
}

void building_load_state(buffer *buf)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_load(&Data_Buildings[i], buf);
        Data_Buildings[i].id = i;
    }
}
