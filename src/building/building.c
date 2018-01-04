#include "building.h"

#include "building/properties.h"
#include "building/storage.h"
#include "city/warning.h"
#include "figure/formation_legion.h"
#include "game/resource.h"
#include "game/undo.h"
#include "map/grid.h"
#include "map/random.h"
#include "map/terrain.h"

#include "Data/CityInfo.h"

#include <string.h>

static building Data_Buildings[MAX_BUILDINGS];

static struct {
    int highest_id_in_use;
    int highest_id_ever;
    int created_sequence;
    int incorrect_houses;
    int unfixable_houses;
} extra = {0, 0, 0, 0};

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
        if (Data_Buildings[i].state == BUILDING_STATE_UNUSED && !game_undo_contains_building(i)) {
            b = &Data_Buildings[i];
            break;
        }
    }
    if (!b) {
        city_warning_show(WARNING_DATA_LIMIT_REACHED);
        return &Data_Buildings[0];
    }
    
    const building_properties *props = building_properties_for_type(type);
    
    b->state = BUILDING_STATE_CREATED;
    b->ciid = 1;
    b->__unknown_02 = Data_CityInfo.__unknown_00a5; // TODO ??
    b->type = type;
    b->size = props->size;
    b->createdSequence = extra.created_sequence++;
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
    if (building_is_house(type)) {
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
    b->isAdjacentToWater = map_terrain_is_adjacent_to_water(x, y, b->size);

    return b;
}

void building_delete(building *b)
{
    building_clear_related_data(b);
    int id = b->id;
    memset(b, 0, sizeof(building));
    b->id = id;
}

void building_clear_related_data(building *b)
{
    if (b->storage_id) {
        building_storage_delete(b->storage_id);
    }
    if (b->type == BUILDING_SENATE_UPGRADED && b->gridOffset == Data_CityInfo.buildingSenateGridOffset) {
        Data_CityInfo.buildingSenateGridOffset = 0;
        Data_CityInfo.buildingSenateX = 0;
        Data_CityInfo.buildingSenateY = 0;
        Data_CityInfo.buildingSenatePlaced = 0;
    }
    if (b->type == BUILDING_DOCK) {
        --Data_CityInfo.numWorkingDocks;
    }
    if (b->type == BUILDING_BARRACKS && b->gridOffset == Data_CityInfo.buildingBarracksGridOffset) {
        Data_CityInfo.buildingBarracksGridOffset = 0;
        Data_CityInfo.buildingBarracksX = 0;
        Data_CityInfo.buildingBarracksY = 0;
        Data_CityInfo.buildingBarracksPlaced = 0;
    }
    if (b->type == BUILDING_DISTRIBUTION_CENTER_UNUSED && b->gridOffset == Data_CityInfo.buildingDistributionCenterGridOffset) {
        Data_CityInfo.buildingDistributionCenterGridOffset = 0;
        Data_CityInfo.buildingDistributionCenterX = 0;
        Data_CityInfo.buildingDistributionCenterY = 0;
        Data_CityInfo.buildingDistributionCenterPlaced = 0;
    }
    if (b->type == BUILDING_FORT) {
        formation_legion_delete_for_fort(b);
    }
    if (b->type == BUILDING_HIPPODROME) {
        Data_CityInfo.buildingHippodromePlaced = 0;
    }
}

void building_clear_all()
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        memset(&Data_Buildings[i], 0, sizeof(building));
        Data_Buildings[i].id = i;
    }
    extra.highest_id_in_use = 0;
    extra.highest_id_ever = 0;
    extra.created_sequence = 0;
    extra.incorrect_houses = 0;
    extra.unfixable_houses = 0;
}

int building_is_house(building_type type)
{
    return type >= BUILDING_HOUSE_VACANT_LOT && type <= BUILDING_HOUSE_LUXURY_PALACE;
}

int building_get_highest_id()
{
    return extra.highest_id_in_use;
}

void building_update_highest_id()
{
    extra.highest_id_in_use = 0;
    for (int i = 1; i < MAX_BUILDINGS; i++) {
        if (Data_Buildings[i].state != BUILDING_STATE_UNUSED) {
            extra.highest_id_in_use = i;
        }
    }
    if (extra.highest_id_in_use > extra.highest_id_ever) {
        extra.highest_id_ever = extra.highest_id_in_use;
    }
}

void building_totals_add_corrupted_house(int unfixable)
{
    extra.incorrect_houses++;
    if (unfixable) {
        extra.unfixable_houses++;
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

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_save(&Data_Buildings[i], buf);
    }
    buffer_write_i32(highest_id, extra.highest_id_in_use);
    buffer_write_i32(highest_id_ever, extra.highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    buffer_write_i32(sequence, extra.created_sequence);
    
    buffer_write_i32(corrupt_houses, extra.incorrect_houses);
    buffer_write_i32(corrupt_houses, extra.unfixable_houses);
}

void building_load_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        building_load(&Data_Buildings[i], buf);
        Data_Buildings[i].id = i;
    }
    extra.highest_id_in_use = buffer_read_i32(highest_id);
    extra.highest_id_ever = buffer_read_i32(highest_id_ever);
    buffer_skip(highest_id_ever, 4);
    extra.created_sequence = buffer_read_i32(sequence);

    extra.incorrect_houses = buffer_read_i32(corrupt_houses);
    extra.unfixable_houses = buffer_read_i32(corrupt_houses);
}
