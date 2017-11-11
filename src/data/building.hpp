#ifndef DATA_BUILDING_H
#define DATA_BUILDING_H

#include "building/type.h"

#define MAX_BUILDINGS 2000
#define MAX_STORAGES 200
#define MAX_HOUSE_LEVELS 20

enum
{
    BuildingState_Unused = 0,
    BuildingState_InUse = 1,
    BuildingState_Undo = 2,
    BuildingState_Created = 3,
    BuildingState_Rubble = 4,
    BuildingState_DeletedByGame = 5, // used for earthquakes, fires, house mergers
    BuildingState_DeletedByPlayer = 6
};

#define BuildingIsHouse(type) ((type) >= BUILDING_HOUSE_VACANT_LOT && (type) <= BUILDING_HOUSE_LUXURY_PALACE)
#define BuildingIsFarm(type) ((type) >= BUILDING_WHEAT_FARM && (type) <= BUILDING_PIG_FARM)
#define BuildingIsWorkshop(type) ((type) >= BUILDING_WINE_WORKSHOP && (type) <= BUILDING_POTTERY_WORKSHOP)
#define BuildingIsEntertainment(type) ((type) >= BUILDING_THEATER && (type) <= BUILDING_CHARIOT_MAKER)

#define BuildingIsInUse(buildingId) (Data_Buildings[buildingId].state == BuildingState_InUse)

extern struct Data_Building
{
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
    union
    {
        short houseLevel;
        short warehouseResourceId;
        short workshopResource;
        short orientation;
        short fortFigureType;
        short nativeMeetingCenterId;
    } subtype;
    unsigned char roadNetworkId;
    unsigned char __unknown_0f;
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
    unsigned char __unused_2b;
    unsigned char figureRoamDirection;
    unsigned char hasWaterAccess;
    unsigned char __unused_2e;
    unsigned char __unused_2f;
    short prevPartBuildingId;
    short nextPartBuildingId;
    short loadsStored; // 34
    unsigned char __unused_36;
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
    unsigned char __unused_47;
    short formationId;
    union
    {
        struct
        {
            short dockQueuedDockerId; // 4a
            char __unknown_4c;
            char __unknown_4d;
            char __unknown_4e;
            char __unknown_4f;
            char __unknown_50;
            char __unknown_51;
            char __unknown_52;
            char __unknown_53;
            char __unknown_54;
            char __unknown_55;
            char __unknown_56;
            char __unknown_57;
            char fishingBoatHasFish; // 58
            char __unknown_59;
            char __unknown_5a;
            char __unknown_5b;
            char __unknown_5c;
            char __unknown_5d;
            char __unknown_5e;
            char __unknown_5f;
            char __unknown_60;
            char __unknown_61;
            char __unknown_62;
            char __unknown_63;
            char __unknown_64;
            unsigned char dockNumShips; // 65 dock number of ships
            char __unknown_66;
            char __unknown_67;
            char dockOrientation; // 68
            char __unknown_69;
            char __unknown_6a;
            char __unknown_6b; // farm cursed
            short dockFigureIds[3]; // 6c, 6e, 70
            short boatFigureId; // 72
        } other;
        struct
        {
            short unknown; //4a
            short inventory[8];
            short potteryDemand; // 5c
            short furnitureDemand; // 5e
            short oilDemand; // 60
            short wineDemand; // 62
            char __padding[6]; // 64 - 69
            unsigned char fetchInventoryId; // 6a
        } market;
        struct
        {
            short unknown; //4a
            short resourceStored[16]; //4c and further
        } storage;
        struct
        {
            short progress; //4a+b
            char __padding[27];
            unsigned char blessingDaysLeft; //67
            char __pad68;
            unsigned char hasFullResource; //69
            char __pad6a;
            unsigned char curseDaysLeft; //6b
        } industry;
        struct
        {
            char __padding[26];
            unsigned char numShows; // 64
            unsigned char days1;
            unsigned char days2;
            unsigned char play;
        } entertainment;
        struct
        {
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
    union
    {
        char houseHappiness;
        char nativeAnger;
    } sentiment;
    unsigned char showOnProblemOverlay;
} Data_Buildings[MAX_BUILDINGS];

extern struct _Data_Buildings_Extra
{
    int highestBuildingIdInUse;
    int highestBuildingIdEver;
    int createdSequence;
    int barracksTowerSentryRequested;
} Data_Buildings_Extra;

extern struct _Data_BuildingList
{
    struct
    {
        int size;
        short items[500];
        int index;
        int totalBurning;
    } burning;
} Data_BuildingList;

#endif
