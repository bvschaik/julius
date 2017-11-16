#ifndef DATA_SCENARIO_H
#define DATA_SCENARIO_H

typedef struct
{
    short startYear;
    short __unused1;
    short empireId;
    short __unused2[4];
    struct Data_Scenario_Requests
    {
        short year[20];
        short resourceId[20];
        short amount[20];
        short deadlineYears[20];
    } requests;
    struct Data_Scenario_Invasions
    {
        short year[20];
        short type[20];
        short amount[20];
        short from[20];
        short attackType[20];
    } invasions;
    short __unused3;
    int startFunds;
    short enemyId;
    short __unused4[3];
    int mapSizeX;
    int mapSizeY;
    int gridFirstElement;
    int gridBorderSize;
    char briefDescription[64];
    unsigned char briefing[522];
    unsigned char requests_canComplyDialogShown[20];
    short imageId;
    short isOpenPlay;
    short playerRank;
    struct
    {
        short x[4];
        short y[4];
    } herdPoints;
    struct
    {
        short year[20];
        unsigned char month[20];
        unsigned char resourceId[20];
        unsigned char routeId[20];
        unsigned char isRise[20];
    } demandChanges;
    struct
    {
        short year[20];
        unsigned char month[20];
        unsigned char resourceId[20];
        unsigned char amount[20];
        unsigned char isRise[20];
    } priceChanges;
    struct
    {
        int enabled;
        int year;
    } gladiatorRevolt;
    struct
    {
        int enabled;
        int year;
    } emperorChange;
    int seaTradeProblemEnabled;
    int landTradeProblemEnabled;
    int raiseWagesEnabled;
    int lowerWagesEnabled;
    int contaminatedWaterEnabled;
    int ironMineCollapseEnabled;
    int clayPitFloodEnabled;
    struct
    {
        short x[8];
        short y[8];
    } fishingPoints;
    unsigned char requests_favor[20];
    unsigned char invasions_month[20];
    unsigned char requests_month[20];
    unsigned char requests_state[20];
    unsigned char requests_isVisible[20];
    unsigned char requests_monthsToComply[20];
    int romeSuppliesWheat;
    struct
    {
        short __unused1;
        short farms;
        short rawMaterials;
        short workshops;
        short road;
        short wall;
        short aqueduct;
        short __unused2;
        short amphitheater;
        short theater;
        short hippodrome;
        short colosseum;
        short gladiatorSchool;
        short lionHouse;
        short actorColony;
        short chariotMaker;
        short gardens;
        short plaza;
        short statues;
        short doctor;
        short hospital;
        short bathhouse;
        short barber;
        short school;
        short academy;
        short library;
        short prefecture;
        short fort;
        short gatehouse;
        short tower;
        short smallTemples;
        short largeTemples;
        short market;
        short granary;
        short warehouse;
        short __unused3;
        short dock;
        short wharf;
        short governorHome;
        short engineersPost;
        short senate;
        short forum;
        short well;
        short oracle;
        short missionPost;
        short bridge;
        short barracks;
        short militaryAcademy;
        short distributionCenter;
        short __unused4;
        //short building[100];
    } allowedBuildings;
    struct
    {
        int culture;
        int prosperity;
        int peace;
        int favor;
        unsigned char cultureEnabled;
        unsigned char prosperityEnabled;
        unsigned char peaceEnabled;
        unsigned char favorEnabled;
        int timeLimitYearsEnabled;
        int timeLimitYears;
        int survivalYearsEnabled;
        int survivalYears;
    } winCriteria;
    int earthquakeSeverity;
    int earthquakeYear;
    int winCriteria_populationEnabled;
    int winCriteria_population;
    struct
    {
        short x;
        short y;
    } earthquakePoint;
    struct
    {
        short x;
        short y;
    } entryPoint;
    struct
    {
        short x;
        short y;
    } exitPoint;
    struct
    {
        short x[8];
        short y[8];
    } invasionPoints;
    struct
    {
        short x;
        short y;
    } riverEntryPoint;
    struct
    {
        short x;
        short y;
    } riverExitPoint;
    int rescueLoan;
    int milestone25;
    int milestone50;
    int milestone75;
    struct
    {
        int hut;
        int meetingCenter;
        int crops;
    } nativeGraphics;
    unsigned char climate;
    unsigned char flotsamEnabled;
    short __unused5;
    int empireHasExpanded;
    int empireExpansionYear;
    unsigned char distantBattleTravelMonthsRoman;
    unsigned char distantBattleTravelMonthsEnemy;
    unsigned char openPlayScenarioId;
    unsigned char _unused6;
} _Data_Scenario;

extern _Data_Scenario Data_Scenario;

#endif
