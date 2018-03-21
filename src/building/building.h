#ifndef BUILDING_BUILDING_H
#define BUILDING_BUILDING_H

#include "building/type.h"
#include "core/buffer.h"

#define MAX_BUILDINGS 2000

typedef struct {
    unsigned char state;
    unsigned char ciid;
    unsigned char __unknown_02;
    unsigned char size;
    unsigned char houseIsMerged;
    unsigned char houseSize;
    unsigned char x;
    unsigned char y;
    short gridOffset;
    short type;
    union {
        short houseLevel;
        short warehouseResourceId;
        short workshopType;
        short orientation;
        short fortFigureType;
        short nativeMeetingCenterId;
    } subtype;
    unsigned char roadNetworkId;
    unsigned short createdSequence;
    short housesCovered;
    short percentageHousesCovered;
    short housePopulation;
    short housePopulationRoom;
    short distanceFromEntry;
    short houseMaxPopulationSeen;
    short houseUnreachableTicks;
    unsigned char roadAccessX; // 20
    unsigned char roadAccessY; // 21
    short figureId;
    short figureId2; // labor seeker or market buyer
    short immigrantFigureId;
    short figureId4; // 28; tower ballista or burning ruin prefect
    unsigned char figureSpawnDelay; // 2a
    unsigned char figureRoamDirection;
    unsigned char hasWaterAccess;
    short prevPartBuildingId;
    short nextPartBuildingId;
    short loadsStored; // 34
    unsigned char hasWellAccess;
    short numWorkers;
    unsigned char laborCategory;
    unsigned char outputResourceId; //3b
    unsigned char hasRoadAccess;
    unsigned char houseCriminalActive;
    short damageRisk;
    short fireRisk;
    short fireDuration; //42
    unsigned char fireProof; //44 cannot catch fire or collapse
    unsigned char houseGenerationDelay;
    unsigned char houseTaxCoverage;
    short formationId;
    union {
        struct {
            short queued_docker_id; // 4a
            unsigned char num_ships; // 65 dock number of ships
            char orientation; // 68
            short docker_ids[3]; // 6c, 6e, 70
            short trade_ship_id; // 72
        } dock;
        struct {
            short inventory[8];
            short potteryDemand; // 5c
            short furnitureDemand; // 5e
            short oilDemand; // 60
            short wineDemand; // 62
            unsigned char fetchInventoryId; // 6a
        } market;
        struct {
            short resource_stored[16]; //4c and further
        } granary;
        struct {
            short progress; //4a+b
            unsigned char blessingDaysLeft; //67
            unsigned char hasFullResource; //69
            unsigned char curseDaysLeft; //6b
            char has_fish; // 58
            char orientation; // 68
            short fishing_boat_id; // 72
        } industry;
        struct {
            unsigned char numShows; // 64
            unsigned char days1;
            unsigned char days2;
            unsigned char play;
        } entertainment;
        struct {
            short inventory[8]; //4a
            unsigned char theater; //5a
            unsigned char amphitheaterActor;
            unsigned char amphitheaterGladiator;
            unsigned char colosseumGladiator;
            unsigned char colosseumLion;
            unsigned char hippodrome;
            unsigned char school; //60
            unsigned char library;
            unsigned char academy;
            unsigned char barber;
            unsigned char clinic;
            unsigned char bathhouse;
            unsigned char hospital;
            unsigned char templeCeres;
            unsigned char templeNeptune;//68
            unsigned char templeMercury;
            unsigned char templeMars;
            unsigned char templeVenus;
            unsigned char noSpaceToExpand;
            unsigned char numFoods;
            unsigned char entertainment;
            unsigned char education;
            unsigned char health; //70
            unsigned char numGods;
            unsigned char devolveDelay;
            unsigned char evolveTextId;
        } house;
    } data;
    int taxIncomeOrStorage; // 74
    unsigned char houseDaysWithoutFood; // 78
    unsigned char ruinHasPlague;
    signed char desirability;
    unsigned char isDeleted; // 7b
    unsigned char isAdjacentToWater;
    unsigned char storage_id;
    union {
        char houseHappiness;
        char nativeAnger;
    } sentiment;
    unsigned char showOnProblemOverlay;

    int id;
} building;

building *building_get(int id);

building *building_main(building *b);

building *building_next(building *b);

building *building_create(building_type type, int x, int y);

void building_clear_related_data(building *b);

void building_update_state();

void building_update_desirability();

int building_is_house(building_type type);

int building_get_highest_id();

void building_update_highest_id();

void building_totals_add_corrupted_house(int unfixable);

void building_clear_all();

void building_save_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses);

void building_load_state(buffer *buf, buffer *highest_id, buffer *highest_id_ever,
                         buffer *sequence, buffer *corrupt_houses);

#endif // BUILDING_BUILDING_H
