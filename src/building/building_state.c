#include "building_state.h"

#include "game/resource.h"

void building_state_load_from_buffer(buffer *buf, building *b)
{
    // TODO
}

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
        buffer_write_i16(buf, b->data.market.potteryDemand);
        buffer_write_i16(buf, b->data.market.furnitureDemand);
        buffer_write_i16(buf, b->data.market.oilDemand);
        buffer_write_i16(buf, b->data.market.wineDemand);
        for (int i = 0; i < 3; i++) {
            buffer_write_i16(buf, 0);
        }
        buffer_write_u8(buf, b->data.market.fetchInventoryId);
        for (int i = 0; i < 9; i++) {
            buffer_write_u8(buf, 0);
        }
    } else if (b->type == BUILDING_GRANARY) {
        buffer_write_i16(buf, 0);
        for (int i = 0; i < RESOURCE_MAX; i++) {
            buffer_write_i16(buf, b->data.storage.resourceStored[i]);
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
    } else if (b->type == BUILDING_SHIPYARD || b->type == BUILDING_WHARF) {
        buffer_write_i16(buf, b->data.fishing.build_progress);
        for (int i = 0; i < 12; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_i8(buf, b->data.fishing.has_fish);
        for (int i = 0; i < 15; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_i8(buf, b->data.fishing.orientation);
        for (int i = 0; i < 9; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_i16(buf, b->data.fishing.boat_id);
    } else if (b->outputResourceId || b->type == BUILDING_NATIVE_CROPS) {
        // Industry. NB: wharf also has an outputResourceId, keep above this "if"!
        buffer_write_i16(buf, b->data.industry.progress);
        for (int i = 0; i < 27; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.industry.blessingDaysLeft);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, b->data.industry.hasFullResource);
        buffer_write_u8(buf, 0);
        buffer_write_u8(buf, b->data.industry.curseDaysLeft);
        buffer_write_i32(buf, 0);
        buffer_write_i32(buf, 0);
    } else if (b->type == BUILDING_THEATER || b->type == BUILDING_AMPHITHEATER
            || b->type == BUILDING_COLOSSEUM || b->type == BUILDING_HIPPODROME) {
        for (int i = 0; i < 26; i++) {
            buffer_write_u8(buf, 0);
        }
        buffer_write_u8(buf, b->data.entertainment.numShows);
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
    buffer_write_u8(buf, b->__unknown_0f);
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
    buffer_write_u8(buf, b->__unused_2b);
    buffer_write_u8(buf, b->figureRoamDirection);
    buffer_write_u8(buf, b->hasWaterAccess);
    buffer_write_u8(buf, b->__unused_2e);
    buffer_write_u8(buf, b->__unused_2f);
    buffer_write_i16(buf, b->prevPartBuildingId);
    buffer_write_i16(buf, b->nextPartBuildingId);
    buffer_write_i16(buf, b->loadsStored); // 34
    buffer_write_u8(buf, b->__unused_36);
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
    buffer_write_u8(buf, b->__unused_47);
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
