#include "building_state.h"

#include "game/resource.h"

static void write_type_data(buffer *buf, const building *b)
{
    if (building_is_house(b->type)) {
        for (int i = 0; i < INVENTORY_MAX; i++) {
            buffer_write_i16(buf, b->data.house.inventory[i]);
        }
        buffer_write_u8(buf, b->data.house.theater);
        buffer_write_u8(buf, b->data.house.amphitheaterActor);
        buffer_write_u8(buf, b->data.house.amphitheaterGladiator);
        buffer_write_u8(buf, b->data.house.colosseumGladiator);
        buffer_write_u8(buf, b->data.house.colosseumLion);
        buffer_write_u8(buf, b->data.house.hippodrome);
        buffer_write_u8(buf, b->data.house.school);
        buffer_write_u8(buf, b->data.house.library);
        buffer_write_u8(buf, b->data.house.academy);
        buffer_write_u8(buf, b->data.house.barber);
        buffer_write_u8(buf, b->data.house.clinic);
        buffer_write_u8(buf, b->data.house.bathhouse);
        buffer_write_u8(buf, b->data.house.hospital);
        buffer_write_u8(buf, b->data.house.templeCeres);
        buffer_write_u8(buf, b->data.house.templeNeptune);
        buffer_write_u8(buf, b->data.house.templeMercury);
        buffer_write_u8(buf, b->data.house.templeMars);
        buffer_write_u8(buf, b->data.house.templeVenus);
        buffer_write_u8(buf, b->data.house.noSpaceToExpand);
        buffer_write_u8(buf, b->data.house.numFoods);
        buffer_write_u8(buf, b->data.house.entertainment);
        buffer_write_u8(buf, b->data.house.education);
        buffer_write_u8(buf, b->data.house.health);
        buffer_write_u8(buf, b->data.house.numGods);
        buffer_write_u8(buf, b->data.house.devolveDelay);
        buffer_write_u8(buf, b->data.house.evolveTextId);
    } else if (b->type == BUILDING_MARKET) {
        buffer_write_i16(buf, 0);
        for (int i = 0; i < INVENTORY_MAX; i++) {
            buffer_write_i16(buf, b->data.market.inventory[i]);
        }
        buffer_write_i16(buf, b->data.market.pottery_demand);
        buffer_write_i16(buf, b->data.market.furniture_demand);
        buffer_write_i16(buf, b->data.market.oil_demand);
        buffer_write_i16(buf, b->data.market.wine_demand);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, 0);
        }
        buffer_write_u8(buf, b->data.market.fetch_inventory_id);
        for (int i = 0; i < 9; i++) {
            buffer_write_u8(buf, 0);
        }
    } else if (b->type == BUILDING_GRANARY) {
        buffer_write_i16(buf, 0);
        for (int i = 0; i < RESOURCE_MAX; i++) {
            buffer_write_i16(buf, b->data.granary.resource_stored[i]);
        }
        buffer_write_i32(buf, 0);
        buffer_write_i32(buf, 0);
    } else if (b->type == BUILDING_DOCK) {
        buffer_write_i16(buf, b->data.dock.queued_docker_id);
        for (int i = 0; i < 25; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.dock.num_ships);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        buffer_write_i8(buf, b->data.dock.orientation);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, 0);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, b->data.dock.docker_ids[i]);
        }
        buffer_write_i16(buf, b->data.dock.trade_ship_id);
    } else if (b->outputResourceId || b->type == BUILDING_NATIVE_CROPS || b->type == BUILDING_SHIPYARD) {
        buffer_write_i16(buf, b->data.industry.progress);
        for (int i = 0; i < 12; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.industry.has_fish);
        for (int i = 0; i < 14; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.industry.blessing_days_left);
        buffer_write_u8(buf, b->data.industry.orientation);
        buffer_write_u8(buf, b->data.industry.has_raw_materials);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, b->data.industry.curse_days_left);
        for (int i = 0; i < 6; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_i16(buf, b->data.industry.fishing_boat_id);
    } else if (b->type == BUILDING_THEATER || b->type == BUILDING_AMPHITHEATER
            || b->type == BUILDING_COLOSSEUM || b->type == BUILDING_HIPPODROME) {
        for (int i = 0; i < 26; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.entertainment.num_shows);
        buffer_write_u8(buf, b->data.entertainment.days1);
        buffer_write_u8(buf, b->data.entertainment.days2);
        buffer_write_u8(buf, b->data.entertainment.play);
        for (int i = 0; i < 12; i++) {
            buffer_write_u8(buf, 0);
        }
    } else {
        for (int i = 0; i < 42; i++) {
            buffer_write_u8(buf, 0);
        }
    }
}

void building_state_save_to_buffer(buffer *buf, const building *b)
{
    buffer_write_u8(buf, b->state);
    buffer_write_u8(buf, b->ciid);
    buffer_write_u8(buf, b->__unknown_02);
    buffer_write_u8(buf, b->size);
    buffer_write_u8(buf, b->houseIsMerged);
    buffer_write_u8(buf, b->houseSize);
    buffer_write_u8(buf, b->x);
    buffer_write_u8(buf, b->y);
    buffer_write_i16(buf, b->gridOffset);
    buffer_write_i16(buf, b->type);
    buffer_write_i16(buf, b->subtype.houseLevel); // which union field we use does not matter
    buffer_write_u8(buf, b->roadNetworkId);
    buffer_write_u8(buf, 0);
    buffer_write_u16(buf, b->createdSequence);
    buffer_write_i16(buf, b->housesCovered);
    buffer_write_i16(buf, b->percentageHousesCovered);
    buffer_write_i16(buf, b->housePopulation);
    buffer_write_i16(buf, b->housePopulationRoom);
    buffer_write_i16(buf, b->distanceFromEntry);
    buffer_write_i16(buf, b->houseMaxPopulationSeen);
    buffer_write_i16(buf, b->houseUnreachableTicks);
    buffer_write_u8(buf, b->roadAccessX); // 20
    buffer_write_u8(buf, b->roadAccessY); // 21
    buffer_write_i16(buf, b->figureId);
    buffer_write_i16(buf, b->figureId2); // labor seeker or market buyer
    buffer_write_i16(buf, b->immigrantFigureId);
    buffer_write_i16(buf, b->figureId4); // 28; tower ballista or burning ruin prefect
    buffer_write_u8(buf, b->figureSpawnDelay); // 2a
    buffer_write_u8(buf, 0);
    buffer_write_u8(buf, b->figureRoamDirection);
    buffer_write_u8(buf, b->hasWaterAccess);
    buffer_write_u8(buf, 0);
    buffer_write_u8(buf, 0);
    buffer_write_i16(buf, b->prevPartBuildingId);
    buffer_write_i16(buf, b->nextPartBuildingId);
    buffer_write_i16(buf, b->loadsStored); // 34
    buffer_write_u8(buf, 0);
    buffer_write_u8(buf, b->hasWellAccess);
    buffer_write_i16(buf, b->numWorkers);
    buffer_write_u8(buf, b->laborCategory);
    buffer_write_u8(buf, b->outputResourceId); //3b
    buffer_write_u8(buf, b->hasRoadAccess);
    buffer_write_u8(buf, b->houseCriminalActive);
    buffer_write_i16(buf, b->damageRisk);
    buffer_write_i16(buf, b->fireRisk);
    buffer_write_i16(buf, b->fireDuration); //42
    buffer_write_u8(buf, b->fireProof); //44 cannot catch fire or collapse
    buffer_write_u8(buf, b->houseGenerationDelay);
    buffer_write_u8(buf, b->houseTaxCoverage);
    buffer_write_u8(buf, 0);
    buffer_write_i16(buf, b->formationId);
    write_type_data(buf, b);
    buffer_write_i32(buf, b->taxIncomeOrStorage); // 74
    buffer_write_u8(buf, b->houseDaysWithoutFood); // 78
    buffer_write_u8(buf, b->ruinHasPlague);
    buffer_write_i8(buf, b->desirability);
    buffer_write_u8(buf, b->isDeleted); // 7b
    buffer_write_u8(buf, b->isAdjacentToWater);
    buffer_write_u8(buf, b->storage_id);
    buffer_write_i8(buf, b->sentiment.houseHappiness); // which union field we use does not matter
    buffer_write_u8(buf, b->showOnProblemOverlay);
}

static void read_type_data(buffer *buf, building *b)
{
    if (building_is_house(b->type)) {
        for (int i = 0; i < INVENTORY_MAX; i++) {
            b->data.house.inventory[i] = buffer_read_i16(buf);
        }
        b->data.house.theater = buffer_read_u8(buf);
        b->data.house.amphitheaterActor = buffer_read_u8(buf);
        b->data.house.amphitheaterGladiator = buffer_read_u8(buf);
        b->data.house.colosseumGladiator = buffer_read_u8(buf);
        b->data.house.colosseumLion = buffer_read_u8(buf);
        b->data.house.hippodrome = buffer_read_u8(buf);
        b->data.house.school = buffer_read_u8(buf);
        b->data.house.library = buffer_read_u8(buf);
        b->data.house.academy = buffer_read_u8(buf);
        b->data.house.barber = buffer_read_u8(buf);
        b->data.house.clinic = buffer_read_u8(buf);
        b->data.house.bathhouse = buffer_read_u8(buf);
        b->data.house.hospital = buffer_read_u8(buf);
        b->data.house.templeCeres = buffer_read_u8(buf);
        b->data.house.templeNeptune = buffer_read_u8(buf);
        b->data.house.templeMercury = buffer_read_u8(buf);
        b->data.house.templeMars = buffer_read_u8(buf);
        b->data.house.templeVenus = buffer_read_u8(buf);
        b->data.house.noSpaceToExpand = buffer_read_u8(buf);
        b->data.house.numFoods = buffer_read_u8(buf);
        b->data.house.entertainment = buffer_read_u8(buf);
        b->data.house.education = buffer_read_u8(buf);
        b->data.house.health = buffer_read_u8(buf);
        b->data.house.numGods = buffer_read_u8(buf);
        b->data.house.devolveDelay = buffer_read_u8(buf);
        b->data.house.evolveTextId = buffer_read_u8(buf);
    } else if (b->type == BUILDING_MARKET) {
        buffer_skip(buf, 2);
        for (int i = 0; i < INVENTORY_MAX; i++) {
            b->data.market.inventory[i] = buffer_read_i16(buf);
        }
        b->data.market.pottery_demand = buffer_read_i16(buf);
        b->data.market.furniture_demand = buffer_read_i16(buf);
        b->data.market.oil_demand = buffer_read_i16(buf);
        b->data.market.wine_demand = buffer_read_i16(buf);
        buffer_skip(buf, 6);
        b->data.market.fetch_inventory_id = buffer_read_u8(buf);
        buffer_skip(buf, 9);
    } else if (b->type == BUILDING_GRANARY) {
        buffer_skip(buf, 2);
        for (int i = 0; i < RESOURCE_MAX; i++) {
            b->data.granary.resource_stored[i] = buffer_read_i16(buf);
        }
        buffer_skip(buf, 8);
    } else if (b->type == BUILDING_DOCK) {
        b->data.dock.queued_docker_id = buffer_read_i16(buf);
        buffer_skip(buf, 25);
        b->data.dock.num_ships = buffer_read_u8(buf);
        buffer_skip(buf, 2);
        b->data.dock.orientation = buffer_read_i8(buf);
        buffer_skip(buf, 3);
        for (int i = 0; i < 3; i++) {
            b->data.dock.docker_ids[i] = buffer_read_i16(buf);
        }
        b->data.dock.trade_ship_id = buffer_read_i16(buf);
    } else if (b->outputResourceId || b->type == BUILDING_NATIVE_CROPS || b->type == BUILDING_SHIPYARD) {
        b->data.industry.progress = buffer_read_i16(buf);
        buffer_skip(buf, 12);
        b->data.industry.has_fish = buffer_read_u8(buf);
        buffer_skip(buf, 14);
        b->data.industry.blessing_days_left = buffer_read_u8(buf);
        b->data.industry.orientation = buffer_read_u8(buf);
        b->data.industry.has_raw_materials = buffer_read_u8(buf);
        buffer_skip(buf, 1);
        b->data.industry.curse_days_left = buffer_read_u8(buf);
        buffer_skip(buf, 6);
        b->data.industry.fishing_boat_id = buffer_read_i16(buf);
    } else if (b->type == BUILDING_THEATER || b->type == BUILDING_AMPHITHEATER
            || b->type == BUILDING_COLOSSEUM || b->type == BUILDING_HIPPODROME) {
        buffer_skip(buf, 26);
        b->data.entertainment.num_shows = buffer_read_u8(buf);
        b->data.entertainment.days1 = buffer_read_u8(buf);
        b->data.entertainment.days2 = buffer_read_u8(buf);
        b->data.entertainment.play = buffer_read_u8(buf);
        buffer_skip(buf, 12);
    } else {
        buffer_skip(buf, 42);
    }
}

void building_state_load_from_buffer(buffer *buf, building *b)
{
    b->state = buffer_read_u8(buf);
    b->ciid = buffer_read_u8(buf);
    b->__unknown_02 = buffer_read_u8(buf);
    b->size = buffer_read_u8(buf);
    b->houseIsMerged = buffer_read_u8(buf);
    b->houseSize = buffer_read_u8(buf);
    b->x = buffer_read_u8(buf);
    b->y = buffer_read_u8(buf);
    b->gridOffset = buffer_read_i16(buf);
    b->type = buffer_read_i16(buf);
    b->subtype.houseLevel = buffer_read_i16(buf); // which union field we use does not matter
    b->roadNetworkId = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    b->createdSequence = buffer_read_u16(buf);
    b->housesCovered = buffer_read_i16(buf);
    b->percentageHousesCovered = buffer_read_i16(buf);
    b->housePopulation = buffer_read_i16(buf);
    b->housePopulationRoom = buffer_read_i16(buf);
    b->distanceFromEntry = buffer_read_i16(buf);
    b->houseMaxPopulationSeen = buffer_read_i16(buf);
    b->houseUnreachableTicks = buffer_read_i16(buf);
    b->roadAccessX = buffer_read_u8(buf); // 20
    b->roadAccessY = buffer_read_u8(buf); // 21
    b->figureId = buffer_read_i16(buf);
    b->figureId2 = buffer_read_i16(buf); // labor seeker or market buyer
    b->immigrantFigureId = buffer_read_i16(buf);
    b->figureId4 = buffer_read_i16(buf); // 28; tower ballista or burning ruin prefect
    b->figureSpawnDelay = buffer_read_u8(buf); // 2a
    buffer_skip(buf, 1);
    b->figureRoamDirection = buffer_read_u8(buf);
    b->hasWaterAccess = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    buffer_skip(buf, 1);
    b->prevPartBuildingId = buffer_read_i16(buf);
    b->nextPartBuildingId = buffer_read_i16(buf);
    b->loadsStored = buffer_read_i16(buf); // 34
    buffer_skip(buf, 1);
    b->hasWellAccess = buffer_read_u8(buf);
    b->numWorkers = buffer_read_i16(buf);
    b->laborCategory = buffer_read_u8(buf);
    b->outputResourceId = buffer_read_u8(buf); //3b
    b->hasRoadAccess = buffer_read_u8(buf);
    b->houseCriminalActive = buffer_read_u8(buf);
    b->damageRisk = buffer_read_i16(buf);
    b->fireRisk = buffer_read_i16(buf);
    b->fireDuration = buffer_read_i16(buf); //42
    b->fireProof = buffer_read_u8(buf); //44 cannot catch fire or collapse
    b->houseGenerationDelay = buffer_read_u8(buf);
    b->houseTaxCoverage = buffer_read_u8(buf);
    buffer_skip(buf, 1);
    b->formationId = buffer_read_i16(buf);
    read_type_data(buf, b);
    b->taxIncomeOrStorage = buffer_read_i32(buf); // 74
    b->houseDaysWithoutFood = buffer_read_u8(buf); // 78
    b->ruinHasPlague = buffer_read_u8(buf);
    b->desirability = buffer_read_i8(buf);
    b->isDeleted = buffer_read_u8(buf); // 7b
    b->isAdjacentToWater = buffer_read_u8(buf);
    b->storage_id = buffer_read_u8(buf);
    b->sentiment.houseHappiness = buffer_read_i8(buf); // which union field we use does not matter
    b->showOnProblemOverlay = buffer_read_u8(buf);
}
