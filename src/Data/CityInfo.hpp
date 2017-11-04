#ifndef DATA_CITYINFO_H
#define DATA_CITYINFO_H

#define MIN_TREASURY -5000

struct _Data_CityInfo
{
    char __otherPlayer[18068];
    char __unknown_00a0;
    char __unknown_00a1;
    char __unknown_00a2;
    char __unknown_00a3;
    char __unknown_00a4;
    char __unknown_00a5;
    char __unknown_00a6;
    char __unknown_00a7;
    int taxPercentage;
    int treasury;
    int citySentiment;
    int healthRateTarget;
    int healthRate;
    int numHospitalWorkers;
    int __unknown_00c0;
    int population;
    int populationLastYear;
    int populationSchoolAge;
    int populationAcademyAge;
    int populationMaxSupported;
    int populationRoomInHouses;
    int monthlyPopulation[2400];
    int monthlyPopulationNextIndex;
    int monthsSinceStart;
    short populationPerAge[100];//2664
    int populationPerLevel[20];//272c
    int populationYearlyBirths;
    int populationYearlyDeaths;
    int populationLostInRemoval;
    int populationImmigrationAmountPerBatch;
    int populationEmigrationAmountPerBatch;
    int populationEmigrationQueueSize;
    int populationImmigrationQueueSize;
    int populationLostHomeless;
    int populationLastChange;
    int populationAveragePerYear;
    int populationTotalAllYears;
    int populationPeopleInTentsShacks;
    int populationPeopleInVillasPalaces;
    int populationTotalYears;
    int populationYearlyUpdatedNeeded;
    int populationLastTargetHouseAdd;
    int populationLastTargetHouseRemove;
    int populationImmigratedToday;
    int populationEmigratedToday;
    int populationRefusedImmigrantsNoRoom;
    int populationMigrationPercentage;
    int __unused_27d0;
    int populationImmigrationDuration;
    int populationEmigrationDuration;
    int populationNewcomersThisMonth;
    char __unknown_27e0;
    char __unknown_27e1;
    char __unknown_27e2;
    char __unknown_27e3;
    char __unknown_27e4;
    char __unknown_27e5;
    char __unknown_27e6;
    char __unknown_27e7;
    char __unknown_27e8;
    char __unknown_27e9;
    char __unknown_27ea;
    char __unknown_27eb;
    char __unknown_27ec;
    char __unknown_27ed;
    char __unknown_27ee;
    char __unknown_27ef;
    char __unknown_27f0;
    char __unknown_27f1;
    short resourceLastTargetWarehouse;
    char __unknown_27f4;
    char __unknown_27f5;
    char __unknown_27f6;
    char __unknown_27f7;
    char __unknown_27f8;
    char __unknown_27f9;
    char __unknown_27fa;
    char __unknown_27fb;
    char __unknown_27fc;
    char __unknown_27fd;
    char __unknown_27fe;
    char __unknown_27ff;
    char __unknown_2800;
    char __unknown_2801;
    char __unknown_2802;
    char __unknown_2803;
    char __unknown_2804;
    char __unknown_2805;
    char __unknown_2806;
    char __unknown_2807;
    char __unknown_2808;
    char __unknown_2809;
    char __unknown_280a;
    char __unknown_280b;
    char __unknown_280c;
    char __unknown_280d;
    char __unknown_280e;
    char __unknown_280f;
    char __unknown_2810;
    char __unknown_2811;
    char __unknown_2812;
    char __unknown_2813;
    char __unknown_2814;
    char __unknown_2815;
    char __unknown_2816;
    char __unknown_2817;
    unsigned char entryPointX;
    unsigned char entryPointY;
    short entryPointGridOffset;
    unsigned char exitPointX;
    unsigned char exitPointY;
    short exitPointGridOffset;
    unsigned char buildingSenateX;
    unsigned char buildingSenateY;
    short buildingSenateGridOffset;
    int buildingSenateBuildingId;
    char __unknown_2828;
    char __unknown_2829;
    short resourceSpaceInWarehouses[16];
    short resourceStored[16];
    short resourceTradeStatus[16];
    short resourceTradeExportOver[16];
    short resourceIndustryMothballed[16];
    char __unused_28ca;
    char __unused_28cb;
    int resourceGranaryFoodStored[7];
    int resourceWorkshopRawMaterialStored[6];
    int resourceWorkshopRawMaterialSpace[6];//2900
    int foodInfoFoodStoredInGranaries;
    int foodInfoFoodTypesAvailable;
    int foodInfoFoodTypesEaten;
    char __unknown_2924[272];
    int resourceStockpiled[16];
    int foodInfoFoodSupplyMonths;
    int foodInfoGranariesOperating;
    int populationPercentagePlebs;
    int populationWorkingAge;
    int workersAvailable;
    struct
    {
        int workersNeeded;
        int workersAllocated;
        int totalHousesCovered;
        int buildings;
        int priority;
    } laborCategory[10];
    int workersEmployed;
    int workersUnemployed;
    int unemploymentPercentage;
    int unemploymentPercentageForSenate;
    int workersNeeded;
    int wages;
    int wagesRome;
    char __unknown_2b6c;
    char __unknown_2b6d;
    char __unknown_2b6e;
    char __unknown_2b6f;
    int financeWagesPaidThisYear;
    int financeWagesThisYear;
    int financeWagesLastYear;
    int monthlyTaxedPlebs;
    int monthlyTaxedPatricians;
    int monthlyUntaxedPlebs;
    int monthlyUntaxedPatricians;
    int percentageTaxedPlebs;
    int percentageTaxedPatricians;
    int percentageTaxedPeople;
    int yearlyCollectedTaxFromPlebs;
    int yearlyCollectedTaxFromPatricians;
    int yearlyUncollectedTaxFromPlebs;
    int yearlyUncollectedTaxFromPatricians;
    int financeTaxesThisYear;
    int financeTaxesLastYear;
    int monthlyCollectedTaxFromPlebs;
    int monthlyUncollectedTaxFromPlebs;
    int monthlyCollectedTaxFromPatricians;
    int monthlyUncollectedTaxFromPatricians;
    int financeExportsThisYear;
    int financeExportsLastYear;
    int financeImportsThisYear;
    int financeImportsLastYear;
    int financeInterestPaidThisYear;
    int financeInterestLastYear;
    int financeInterestThisYear;
    int financeSundriesLastYear;
    int financeSundriesThisYear;
    int financeConstructionLastYear;
    int financeConstructionThisYear;
    int financeSalaryLastYear;
    int financeSalaryThisYear;
    int salaryAmount;
    int salaryRank;
    int financeSalaryPaidThisYear;
    int financeTotalIncomeLastYear;
    int financeTotalIncomeThisYear;
    int financeTotalExpensesLastYear;
    int financeTotalExpensesThisYear;
    int financeNetInOutLastYear;
    int financeNetInOutThisYear;
    int financeBalanceLastYear;
    int financeBalanceThisYear;
    int __unknown_2c20[1400];
    int __housesRequiringUnknownToEvolve[8];
    int tradeNextImportResourceCaravan;
    int tradeNextImportResourceCaravanBackup;
    int ratingCulture;
    int ratingProsperity;
    int ratingPeace;
    int ratingFavor;
    char __unknown_4238;
    char __unknown_4239;
    char __unknown_423a;
    char __unknown_423b;
    char __unknown_423c;
    char __unknown_423d;
    char __unknown_423e;
    char __unknown_423f;
    char __unknown_4240;
    char __unknown_4241;
    char __unknown_4242;
    char __unknown_4243;
    char __unknown_4244;
    char __unknown_4245;
    char __unknown_4246;
    char __unknown_4247;
    int ratingProsperityTreasuryLastYear;
    int ratingCulturePointsTheater;
    int ratingCulturePointsReligion;
    int ratingCulturePointsSchool;
    int ratingCulturePointsLibrary;
    int ratingCulturePointsAcademy;
    int ratingPeaceNumCriminalsThisYear;
    int ratingPeaceNumRiotersThisYear;
    int housesRequiringFountainToEvolve;//4268
    int housesRequiringWellToEvolve;//426c
    int housesRequiringMoreEntertainmentToEvolve;//4270
    int housesRequiringMoreEducationToEvolve;//4274
    int housesRequiringEducationToEvolve;//4278
    int housesRequiringSchool;//427c
    int housesRequiringLibrary;//4280
    char __unknown_4284;
    char __unknown_4285;
    char __unknown_4286;
    char __unknown_4287;
    int housesRequiringBarberToEvolve;
    int housesRequiringBathhouseToEvolve;
    int housesRequiringFoodToEvolve;
    char __unknown_4294;
    char __unknown_4295;
    char __unknown_4296;
    char __unknown_4297;
    char __unknown_4298;
    char __unknown_4299;
    char __unknown_429a;
    char __unknown_429b;
    int buildingHippodromePlaced;
    int housesRequiringClinicToEvolve;
    int housesRequiringHospitalToEvolve;
    int housesRequiringBarber;
    int housesRequiringBathhouse;
    int housesRequiringClinic;
    int housesRequiringReligionToEvolve;
    int housesRequiringMoreReligionToEvolve;
    int housesRequiringEvenMoreReligionToEvolve;
    int housesRequiringReligion;
    int entertainmentTheaterShows;
    int entertainmentTheaterNoShowsWeighted;
    int entertainmentAmphitheaterShows;
    int entertainmentAmphitheaterNoShowsWeighted;
    int entertainmentColosseumShows;
    int entertainmentColosseumNoShowsWeighted;
    int entertainmentHippodromeShows;
    int entertainmentHippodromeNoShowsWeighted;
    int entertainmentNeedingShowsMost;
    int citywideAverageEntertainment;
    int housesRequiringEntertainmentToEvolve;
    int monthsSinceFestival;
    char godTargetHappiness[5];
    char godHappiness[5];
    char godWrathBolts[5];
    char godBlessingDone[5];
    char godSmallCurseDone[5];
    char godUnused1[5];
    char godUnused2[5];
    char godUnused3[5];
    int godMonthsSinceFestival[5];
    int godLeastHappy;
    char __unknown_4334;
    char __unknown_4335;
    char __unknown_4336;
    char __unknown_4337;
    int populationEmigrationCauseTextId;
    int numProtestersThisMonth;
    int numCriminalsThisMonth; // muggers+rioters
    int healthDemand;
    int religionDemand;
    int educationDemand;
    int entertainmentDemand;
    int numRiotersInCity;
    int ratingAdvisorSelection;
    int ratingAdvisorExplanationCulture;
    int ratingAdvisorExplanationProsperity;
    int ratingAdvisorExplanationPeace;
    int ratingAdvisorExplanationFavor;
    int playerRank;
    int personalSavings;
    char __unknown_4374;
    char __unknown_4375;
    char __unknown_4376;
    char __unknown_4377;
    char __unknown_4378;
    char __unknown_4379;
    char __unknown_437a;
    char __unknown_437b;
    int financeDonatedLastYear;
    int financeDonatedThisYear;
    int donateAmount;
    short workingDockBuildingIds[10];
    char __unknown_439c;
    char __unknown_439d;
    char __unknown_439e;
    char __unknown_439f;
    char __unknown_43a0;
    char __unknown_43a1;
    short numAnimalsInCity;
    short tradeNumOpenSeaRoutes;
    short tradeNumOpenLandRoutes;
    short tradeSeaProblemDuration;
    short tradeLandProblemDuration;
    short numWorkingDocks;
    short buildingSenatePlaced;
    short numWorkingWharfs;
    char __padding_43b2[2];
    short financeStolenThisYear;
    short financeStolenLastYear;
    int tradeNextImportResourceDocker;
    int tradeNextExportResourceDocker;
    int debtState;
    int monthsInDebt;
    int cheatedMoney;
    char buildingBarracksX;
    char buildingBarracksY;
    short buildingBarracksGridOffset;
    int buildingBarracksBuildingId;
    int buildingBarracksPlaced;
    char __unknown_43d8;
    char __unknown_43d9;
    char __unknown_43da;
    char __unknown_43db;
    char __unknown_43dc;
    char __unknown_43dd;
    char __unknown_43de;
    char __unknown_43df;
    char __unknown_43e0;
    char __unknown_43e1;
    char __unknown_43e2;
    char __unknown_43e3;
    char __unknown_43e4;
    char __unknown_43e5;
    char __unknown_43e6;
    char __unknown_43e7;
    char __unknown_43e8;
    char __unknown_43e9;
    char __unknown_43ea;
    char __unknown_43eb;
    int populationLostTroopRequest;
    char __unknown_43f0;
    char __unknown_43f1;
    char __unknown_43f2;
    char __unknown_43f3;
    int victoryHasWonScenario;
    int victoryContinueMonths;
    int victoryContinueMonthsChosen;
    int wageRatePaidThisYear;
    int financeTributeThisYear;
    int financeTributeLastYear;
    int tributeNotPaidLastYear;
    int tributeNotPaidTotalYears;
    int festivalGod;
    int festivalSize;
    int plannedFestivalSize;
    int plannedFestivalMonthsToGo;
    int plannedFestivalGod;
    int festivalCostSmall;
    int festivalCostLarge;
    int festivalCostGrand;
    int festivalWineGrand;
    int festivalNotEnoughWine;
    int citywideAverageReligion;
    int citywideAverageEducation;
    int citywideAverageHealth;
    int cultureCoverageReligion;
    int festivalEffectMonthsDelayFirst;
    int festivalEffectMonthsDelaySecond;
    int __unused_4454;
    int populationSentimentUnemployment;
    int citySentimentLastTime;
    int citySentimentChangeMessageDelay;
    int populationEmigrationCause; // 4464
    int riotersOrAttackingNativesInCity;
    int __unknown_446c;
    int __unknown_4470;
    int __unknown_4474;
    int __unknown_4478;
    int giftSizeSelected;
    int giftMonthsSinceLast;
    int giftOverdosePenalty;
    int __unused_4488;
    int giftId_modest;
    int giftId_generous;
    int giftId_lavish;
    int giftCost_modest;
    int giftCost_generous;
    int giftCost_lavish;
    int ratingFavorSalaryPenalty;
    int ratingFavorMilestonePenalty;
    int ratingFavorIgnoredRequestPenalty;
    int ratingFavorLastYear;
    int ratingFavorChange; // 0 = dropping, 1 = stalling, 2 = rising
    int nativeAttackDuration;
    int __unused_nativeForceAttack;
    int nativeMissionPostOperational;
    int nativeMainMeetingCenterX;
    int nativeMainMeetingCenterY;
    int wageRatePaidLastYear;
    int foodInfoFoodNeededPerMonth; //44d0
    int foodInfoGranariesUnderstaffed;
    int foodInfoGranariesNotOperating;
    int foodInfoGranariesNotOperatingWithFood;
    int __unused_44e0;
    int __unused_44e4;
    int godCurseVenusActive;
    int __unused_44ec;
    int godBlessingNeptuneDoubleTrade;
    int godBlessingMarsEnemiesToKill;
    int __unused_44f8;
    int godAngryMessageDelay;
    int foodInfoFoodConsumedLastMonth;
    int foodInfoFoodStoredLastMonth;
    int foodInfoFoodStoredSoFarThisMonth;
    int riotCause;
    int estimatedTaxIncome;
    int tutorial1SenateBuilt;
    char buildingDistributionCenterX;
    char buildingDistributionCenterY;
    short buildingDistributionCenterGridOffset;
    int buildingDistributionCenterBuildingId;
    int buildingDistributionCenterPlaced;
    int __unused_4524[11];
    int shipyardBoatsRequested;
    int numEnemiesInCity;
    int populationSentimentWages;
    int populationPeopleInTents;
    int populationPeopleInLargeInsulaAndAbove;
    int numImperialSoldiersInCity;
    int caesarInvasionDurationDayCountdown;
    int caesarInvasionWarningsGiven;
    int caesarInvasionDaysUntilInvasion;
    int caesarInvasionRetreatMessageShown;
    int ratingPeaceNumDestroyedBuildingsThisYear;
    int ratingPeaceYearsOfPeace;
    unsigned char distantBattleCityId;
    unsigned char distantBattleEnemyStrength;
    unsigned char distantBattleRomanStrength;
    char distantBattleMonthsToBattle;
    char distantBattleRomanMonthsToReturn;
    char distantBattleRomanMonthsToTravel;
    char distantBattleCityMonthsUntilRoman;
    char triumphalArchesAvailable;
    char distantBattleTotalCount;
    char distantBattleWonCount;
    char distantBattleEnemyMonthsTraveled;
    char distantBattleRomanMonthsTraveled;
    char militaryTotalLegions;
    char militaryTotalLegionsEmpireService;
    char __unknown_458e;
    char militaryTotalSoldiers;
    char triumphalArchesPlaced;
    unsigned char dieSoundCitizen;
    unsigned char dieSoundSoldier;
    signed char soundShootArrow;
    int buildingTradeCenterBuildingId;
    int numSoldiersInCity;
    signed char soundHitSoldier;
    signed char soundHitSpear;
    signed char soundHitClub;
    unsigned char soundMarchEnemy;
    unsigned char soundMarchHorse;
    signed char soundHitElephant;
    signed char soundHitAxe;
    signed char soundHitWolf;
    unsigned char soundMarchWolf;
    char __unused_45a5[6];
    char populationSentimentIncludeTents;
    int caesarInvasionCount;
    int caesarInvasionSize;
    int caesarInvasionSoldiersDied;
    int militaryLegionaryLegions;
    int populationHighestEver;
    int estimatedYearlyWages;
    int resourceWineTypesAvailable;
    int ratingProsperityMax;
    struct
    {
        int id;
        int size;
    } largestRoadNetworks[10];
    int housesRequiringSecondWineToEvolve;
    int godCurseNeptuneSankShips;
    int entertainmentHippodromeHasShow;
    int messageShownHippodrome;
    int messageShownColosseum;
    int messageShownEmigration;
    int messageShownFired;
    int messageShownVictory;
    int missionSavedGameWritten;
    int tutorial1FireMessageShown;
    int tutorial3DiseaseMessageShown;
    int numAttackingNativesInCity;
    char __unknown_464c;
    char __unknown_464d;
    char __unknown_464e;
    char __unknown_464f;
    char __unknown_4650;
    char __unknown_4651;
    char __unknown_4652;
    char __unknown_4653;
    char __unknown_4654;
    char __unknown_4655;
    char __unknown_4656;
    char __unknown_4657;
    char __unknown_4658;
    char __unknown_4659;
    char __unknown_465a;
    char __unknown_465b;
    char __unknown_465c;
    char __unknown_465d;
    char __unknown_465e;
    char __unknown_465f;
    char __unknown_4660;
    char __unknown_4661;
    char __unknown_4662;
    char __unknown_4663;
    char __unknown_4664;
    char __unknown_4665;
    char __unknown_4666;
    char __unknown_4667;
    char __unknown_4668;
    char __unknown_4669;
    char __unknown_466a;
    char __unknown_466b;
    char __unknown_466c;
    char __unknown_466d;
    char __unknown_466e;
    char __unknown_466f;
    char __unknown_4670;
    char __unknown_4671;
    char __unknown_4672;
    char __unknown_4673;
    char __unknown_4674;
    char __unknown_4675;
    char __unknown_4676;
    char __unknown_4677;
    char __unknown_4678;
    char __unknown_4679;
    char __unknown_467a;
    char __unknown_467b;
    char __unknown_467c;
    char __unknown_467d;
    char __unknown_467e;
    char __unknown_467f;
    char __unknown_4680;
    char __unknown_4681;
    char __unknown_4682;
    char __unknown_4683;
    char __unknown_4684;
    char __unknown_4685;
    char __unknown_4686;
    char __unknown_4687;
    char __unknown_4688;
    char __unknown_4689;
    char __unknown_468a;
    char __unknown_468b;
    char __unknown_468c;
    char __unknown_468d;
    char __unknown_468e;
    char __unknown_468f;
    char __unknown_4690;
    char __unknown_4691;
    char __unknown_4692;
    char __unknown_4693;
    char __unknown_4694;
    char __unknown_4695;
    char __unknown_4696;
    char __unknown_4697;
    char __unknown_4698;
    char __unknown_4699;
    char __unknown_469a;
    char __unknown_469b;
    char __unknown_469c;
    char __unknown_469d;
    char __unknown_469e;
    char __unknown_469f;
    char __unknown_46a0;
    char __unknown_46a1;
    char __unknown_46a2;
    char __unknown_46a3;
    char __unknown_46a4;
    char __unknown_46a5;
    char __unknown_46a6;
    char __unknown_46a7;
    char __unknown_46a8;
    char __unknown_46a9;
    char __unknown_46aa;
    char __unknown_46ab;
    char __unknown_46ac;
    char __unknown_46ad;
    char __unknown_46ae;
    char __unknown_46af;
    char __unknown_46b0;
    char __unknown_46b1;
    char __unknown_46b2;
    char __unknown_46b3;
    char __unknown_46b4;
    char __unknown_46b5;
    char __unknown_46b6;
    char __unknown_46b7;
    char __unknown_46b8;
    char __unknown_46b9;
    char __unknown_46ba;
    char __unknown_46bb;
    char __unknown_46bc;
    char __unknown_46bd;
    char __unknown_46be;
    char __unknown_46bf;
    char __unknown_46c0;
    char __unknown_46c1;
    char __unknown_46c2;
    char __unknown_46c3;
    char __unknown_46c4;
    char __unknown_46c5;
    char __unknown_46c6;
    char __unknown_46c7;
    char __unknown_46c8;
    char __unknown_46c9;
    char __unknown_46ca;
    char __unknown_46cb;
    char __unknown_46cc;
    char __unknown_46cd;
    char __unknown_46ce;
    char __unknown_46cf;
    char __unknown_46d0;
    char __unknown_46d1;
    char __unknown_46d2;
    char __unknown_46d3;
    char __unknown_46d4;
    char __unknown_46d5;
    char __unknown_46d6;
    char __unknown_46d7;
    char __unknown_46d8;
    char __unknown_46d9;
    char __unknown_46da;
    char __unknown_46db;
    char __unknown_46dc;
    char __unknown_46dd;
    char __unknown_46de;
    char __unknown_46df;
    char __unknown_46e0;
    char __unknown_46e1;
    char __unknown_46e2;
    char __unknown_46e3;
    char __unknown_46e4;
    char __unknown_46e5;
    char __unknown_46e6;
    char __unknown_46e7;
    char __unknown_46e8;
    char __unknown_46e9;
    char __unknown_46ea;
    char __unknown_46eb;
    char __unknown_46ec;
    char __unknown_46ed;
    char __unknown_46ee;
    char __unknown_46ef;
    char __unknown_46f0;
    char __unknown_46f1;
    char __unknown_46f2;
    char __unknown_46f3;
    char __unknown_46f4;
    char __unknown_46f5;
    char __unknown_46f6;
    char __unknown_46f7;
    char __unknown_46f8;
    char __unknown_46f9;
    char __unknown_46fa;
    char __unknown_46fb;
    char __unknown_46fc;
    char __unknown_46fd;
    char __unknown_46fe;
    char __unknown_46ff;
    char __unknown_4700;
    char __unknown_4701;
    char __unknown_4702;
    char __unknown_4703;
    char __unknown_4704;
    char __unknown_4705;
    char __unknown_4706;
    char __unknown_4707;
    char __unknown_4708;
    char __unknown_4709;
    char __unknown_470a;
    char __unknown_470b;
    char __unknown_470c;
    char __unknown_470d;
    char __unknown_470e;
    char __unknown_470f;
    char __unknown_4710;
    char __unknown_4711;
    char __unknown_4712;
    char __unknown_4713;
    char __unknown_4714;
    char __unknown_4715;
    char __unknown_4716;
    char __unknown_4717;
    char __unknown_4718;
    char __unknown_4719;
    char __unknown_471a;
    char __unknown_471b;
    char __unknown_471c;
    char __unknown_471d;
    char __unknown_471e;
    char __unknown_471f;
    char __unknown_4720;
    char __unknown_4721;
    char __unknown_4722;
    char __unknown_4723;
    char __unknown_4724;
    char __unknown_4725;
    char __unknown_4726;
    char __unknown_4727;
    char __unknown_4728;
    char __unknown_4729;
    char __unknown_472a;
    char __unknown_472b;
    char __unknown_472c;
    char __unknown_472d;
    char __unknown_472e;
    char __unknown_472f;
    char __unknown_4730;
    char __unknown_4731;
    char __unknown_4732;
    char __unknown_4733;
};

extern _Data_CityInfo Data_CityInfo;

struct Data_CityInfo_Building
{
    int total;
    int working;
};


extern struct _Data_CityInfo_CultureCoverage
{
    int theater;
    int amphitheater;
    int colosseum;
    int hippodrome;
    int hospital;
    int school;
    int academy;
    int library;
    int religionCeres;
    int religionNeptune;
    int religionMercury;
    int religionMars;
    int religionVenus;
    int oracle;
} Data_CityInfo_CultureCoverage;

extern struct _Data_CityInfo_Extra
{
    int populationGraphOrder;
    int startingFavor;
    int personalSavingsLastMission;
    struct
    {
        int x;
        int y;
    } bookmarks[4];
    struct
    {
        int x;
        int y;
        int gridOffset;
    } entryPointFlag;
    struct
    {
        int x;
        int y;
        int gridOffset;
    } exitPointFlag;
    int ciid;
    // not referenced, only used in saved game
    int unknownOrder;
    unsigned char unknownBytes[2];
} Data_CityInfo_Extra;

extern struct _Data_CityInfo_Resource
{
    int numAvailableResources;
    int availableResources[16];
    int numAvailableFoods;
    int availableFoods[16];
} Data_CityInfo_Resource;

#endif
