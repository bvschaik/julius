#ifndef DATA_FIGURE_H
#define DATA_FIGURE_H

#define MAX_FIGURES 1000
#define MAX_TRADERS 100

enum {
	Figure_None = 0,
	Figure_Immigrant = 1,
	Figure_Emigrant = 2,
	Figure_Homeless = 3,
	Figure_CartPusher = 4,
	Figure_LaborSeeker = 5,
	Figure_Explosion = 6,
	Figure_TaxCollector = 7,
	Figure_Engineer = 8,
	Figure_Warehouseman = 9,
	Figure_Prefect = 10,
	Figure_FortJavelin = 11,
	Figure_FortMounted = 12,
	Figure_FortLegionary = 13,
	Figure_FortStandard = 14,
	Figure_Actor = 15,
	Figure_Gladiator = 16,
	Figure_LionTamer = 17,
	Figure_Charioteer = 18,
	Figure_TradeCaravan = 19,
	Figure_TradeShip = 20,
	Figure_TradeCaravanDonkey = 21,
	Figure_Protester = 22,
	Figure_Criminal = 23,
	Figure_Rioter = 24,
	Figure_FishingBoat = 25,
	Figure_MarketTrader = 26,
	Figure_Priest = 27,
	Figure_SchoolChild = 28,
	Figure_Teacher = 29,
	Figure_Librarian = 30,
	Figure_Barber = 31,
	Figure_BathhouseWorker = 32,
	Figure_Doctor = 33,
	Figure_Surgeon = 34,
	Figure_35 = 35,
	Figure_MapFlag = 36,
	Figure_Flotsam = 37,
	Figure_Dockman = 38,
	Figure_MarketBuyer = 39,
	Figure_Patrician = 40,
	Figure_IndigenousNative = 41,
	Figure_TowerSentry = 42,
	Figure_Enemy43_Spear = 43,
	Figure_Enemy44_Sword = 44,
	Figure_Enemy45_Sword = 45,
	Figure_Enemy46_Camel = 46,
	Figure_Enemy47_Elephant = 47,
	Figure_Enemy48_Chariot = 48,
	Figure_Enemy49_FastSword = 49,
	Figure_Enemy50_Sword = 50,
	Figure_Enemy51_Spear = 51,
	Figure_Enemy52_MountedArcher = 52,
	Figure_Enemy53_Axe = 53,
	Figure_Enemy54_Gladiator = 54,
	Figure_Enemy55_Javelin = 55,
	Figure_Enemy56_Mounted = 56,
	Figure_EnemyCaesarLegionary = 57,
	Figure_NativeTrader = 58,
	Figure_Arrow = 59,
	Figure_Javelin = 60,
	Figure_Bolt = 61,
	Figure_Ballista = 62,
	Figure_Creature = 63,
	Figure_Missionary = 64,
	Figure_FishGulls = 65,
	Figure_DeliveryBoy = 66,
	Figure_Shipwreck = 67,
	Figure_Sheep = 68,
	Figure_Wolf = 69,
	Figure_Zebra = 70,
	Figure_Spear = 71,
	Figure_HippodromeMiniHorses = 72
};

#define WalkerIsEnemyOrNative(t) ((t) >= Figure_IndigenousNative && (t) <= Figure_NativeTrader)
#define WalkerIsEnemy(t) ((t) >= Figure_Enemy43_Spear && (t) <= Figure_EnemyCaesarLegionary)
#define WalkerIsLegion(t) ((t) >= Figure_FortJavelin && (t) <= Figure_FortLegionary)
#define WalkerIsHerd(t) ((t) >= Figure_Sheep && (t) <= Figure_Zebra)

#define FigureIsDead(f) (Data_Walkers[f].state != FigureState_Alive || Data_Walkers[f].actionState == FigureActionState_149_Corpse)

enum {
	FigureState_Alive = 1,
	FigureState_Dead = 2
};

enum {
	FigureActionState_1_ImmigrantCreated = 1,
	FigureActionState_2_ImmigrantArriving = 2,
	FigureActionState_3_ImmigrantEnteringHouse = 3,
	FigureActionState_4_EmigrantCreated = 4,
	FigureActionState_5_EmigrantExitingHouse = 5,
	FigureActionState_6_EmigrantLeaving = 6,
	FigureActionState_7_HomelessCreated = 7,
	FigureActionState_8_HomelessGoingToHouse = 8,
	FigureActionState_9_HomelessEnteringHouse = 9,
	FigureActionState_10_HomelessLeaving = 10,
	FigureActionState_20_CartpusherInitial = 20,
	FigureActionState_21_CartpusherDeliveringToWarehouse = 21,
	FigureActionState_22_CartpusherDeliveringToGranary = 22,
	FigureActionState_23_CartpusherDeliveringToWorkshop = 23,
	FigureActionState_24_CartpusherAtWarehouse = 24,
	FigureActionState_25_CartpusherAtGranary = 25,
	FigureActionState_26_CartpusherAtWorkshop = 26,
	FigureActionState_27_CartpusherReturning = 27,
	FigureActionState_40_TaxCollectorCreated = 40,
	FigureActionState_41_TaxCollectorEnteringExiting = 41,
	FigureActionState_42_TaxCollectorRoaming = 42,
	FigureActionState_43_TaxCollectorReturning = 43,
	FigureActionState_50_WarehousemanCreated = 50,
	FigureActionState_51_WarehousemanDeliveringResource = 51,
	FigureActionState_52_WarehousemanAtDeliveryBuilding = 52,
	FigureActionState_53_WarehousemanReturningEmpty = 53,
	FigureActionState_54_WarehousemanGettingFood = 54,
	FigureActionState_55_WarehousemanAtGranaryGettingFood = 55,
	FigureActionState_56_WarehousemanReturningWithFood = 56,
	FigureActionState_57_WarehousemanGettingResource = 57,
	FigureActionState_58_WarehousemanAtWarehouseGettingResource = 58,
	FigureActionState_59_WarehousemanReturningWithResource = 59,
	FigureActionState_60_EngineerCreated = 60,
	FigureActionState_61_EngineerEnteringExiting = 61,
	FigureActionState_62_EngineerRoaming = 62,
	FigureActionState_63_EngineerReturning = 63,
	FigureActionState_70_PrefectCreated = 70,
	FigureActionState_71_PrefectEnteringExiting = 71,
	FigureActionState_72_PrefectRoaming = 72,
	FigureActionState_73_PrefectReturning = 73,
	FigureActionState_74_PrefectGoingToFire = 74,
	FigureActionState_75_PrefectAtFire = 75,
	FigureActionState_76_PrefectGoingToEnemy = 76,
	FigureActionState_77_PrefectAtEnemy = 77,
	FigureActionState_80_SoldierAtRest = 80,
	FigureActionState_81_SoldierGoingToFort = 81,
	FigureActionState_82_SoldierReturningToBarracks = 82,
	FigureActionState_83_SoldierGoingToStandard = 83,
	FigureActionState_84_SoldierAtStandard = 84,
	FigureActionState_85_SoldierGoingToMilitaryAcademy = 85,
	FigureActionState_86_SoldierMoppingUp = 86,
	FigureActionState_87_SoldierGoingToDistantBattle = 87,
	FigureActionState_88_SoldierReturningFromDistantBattle = 88,
	FigureActionState_89_SoldierAtDistantBattle = 89,
	FigureActionState_90_EntertainerAtSchoolCreated = 90,
	FigureActionState_91_EntertainerExitingSchool = 91,
	FigureActionState_92_EntertainerGoingToVenue = 92,
	FigureActionState_94_EntertainerRoaming = 94,
	FigureActionState_95_EntertainerReturning = 95,
	FigureActionState_100_TradeCaravanCreated = 100,
	FigureActionState_101_TradeCaravanArriving = 101,
	FigureActionState_102_TradeCaravanTrading = 102,
	FigureActionState_103_TradeCaravanLeaving = 103,
	FigureActionState_110_TradeShipCreated = 110,
	FigureActionState_111_TradeShipGoingToDock = 111,
	FigureActionState_112_TradeShipMoored = 112,
	FigureActionState_113_TradeShipGoingToDockQueue = 113,
	FigureActionState_114_TradeShipAnchored = 114,
	FigureActionState_115_TradeShipLeaving = 115,
	FigureActionState_120_RioterCreated = 120,
	FigureActionState_121_RioterMoving = 121,
	FigureActionState_125_Roaming = 125,
	FigureActionState_126_RoamerReturning = 126,
	FigureActionState_128_FlotsamCreated = 128,
	FigureActionState_129_FlotsamFloating = 129,
	FigureActionState_130_FlotsamLeftMap = 130,
	FigureActionState_132_DockerIdling = 132,
	FigureActionState_133_DockerImportQueue = 133,
	FigureActionState_134_DockerExportQueue = 134,
	FigureActionState_135_DockerImportGoingToWarehouse = 135,
	FigureActionState_136_DockerExportGoingToWarehouse = 136,
	FigureActionState_137_DockerExportReturning = 137,
	FigureActionState_138_DockerImportReturning = 138,
	FigureActionState_139_DockerImportAtWarehouse = 139,
	FigureActionState_140_DockerExportAtWarehouse = 140,
	FigureActionState_145_MarketBuyerGoingToStorage = 145,
	FigureActionState_146_MarketBuyerReturning = 146,
	FigureActionState_148_Fleeing = 148,
	FigureActionState_149_Corpse = 149,
	FigureActionState_150_Attack = 150,
	FigureActionState_151_EnemyInitial = 151,
	FigureActionState_152_EnemyWaiting = 152,
	FigureActionState_153_EnemyMarching = 153,
	FigureActionState_154_EnemyFighting = 154,
	FigureActionState_156_NativeGoingToMeetingCenter = 156,
	FigureActionState_157_NativeReturningFromMeetingCenter = 157,
	FigureActionState_158_NativeCreated = 158,
	FigureActionState_159_NativeAttacking = 159,
	FigureActionState_160_NativeTraderGoingToWarehouse = 160,
	FigureActionState_161_NativeTraderReturning = 161,
	FigureActionState_162_NativeTraderCreated = 162,
	FigureActionState_163_NativeTraderAtWarehouse = 163,
	FigureActionState_170_TowerSentryAtRest = 170,
	FigureActionState_171_TowerSentryPatrolling = 171,
	FigureActionState_172_TowerSentryFiring = 172,
	FigureActionState_173_TowerSentryReturning = 173,
	FigureActionState_174_TowerSentryGoingToTower = 174,
	FigureActionState_180_BallistaCreated = 180,
	FigureActionState_181_BallistaFiring = 181,
	FigureActionState_190_FishingBoatCreated = 190,
	FigureActionState_191_FishingBoatGoingToFish = 191,
	FigureActionState_192_FishingBoatFishing = 192,
	FigureActionState_193_FishingBoatSailingToWharf = 193,
	FigureActionState_194_FishingBoatAtWharf = 194,
	FigureActionState_195_FishingBoatReturningWithFish = 195,
	FigureActionState_196_HerdAnimalAtRest = 196,
	FigureActionState_197_HerdAnimalMoving = 197,
	FigureActionState_199_WolfAttacking = 199,
	FigureActionState_200_HippodromeMiniHorseCreated = 200,
	FigureActionState_201_HippodromeMiniHorseRacing = 201,
	FigureActionState_202_HippodromeMiniHorseDone = 202,
};

enum {
	FigureTerrainUsage_Any = 0,
	FigureTerrainUsage_Roads = 1,
	FigureTerrainUsage_Enemy = 2,
	FigureTerrainUsage_PreferRoads = 3,
	FigureTerrainUsage_Walls = 4,
	FigureTerrainUsage_Animal = 5,
};

extern struct _Data_Figure_NameSequence {
	int citizenMale;
	int patrician;
	int citizenFemale;
	int taxCollector;
	int engineer;
	int prefect;
	int javelinThrower;
	int cavalry;
	int legionary;
	int actor;
	int gladiator;
	int lionTamer;
	int charioteer;
	int barbarian;
	int enemyGreek;
	int enemyEgyptian;
	int enemyArabian;
	int trader;
	int tradeShip;
	int warShip;
	int enemyShip;
} Data_Figure_NameSequence;

extern struct Data_Walker {
	unsigned char alternativeLocationIndex;
	unsigned char graphicOffset;
	unsigned char isEnemyGraphic;
	unsigned char flotsamVisible;
	short graphicId; // 04
	short cartGraphicId;
	short nextWalkerIdOnSameTile; // 08
	unsigned char type;
	unsigned char resourceId; //0b
	unsigned char useCrossCountry;
	unsigned char isFriendly;
	unsigned char state;
	unsigned char ciid; // 0f
	unsigned char actionStateBeforeAttack; // 10
	signed char direction;
	signed char previousTileDirection; // 12
	signed char attackDirection; // 13
	unsigned char x;
	unsigned char y;
	unsigned char previousTileX;
	unsigned char previousTileY;
	unsigned char missileDamage;
	unsigned char damage; //19
	short gridOffset; // 1a
	unsigned char destinationX; // 1c
	unsigned char destinationY;
	short destinationGridOffsetSoldier;
	unsigned char sourceX; // 20
	unsigned char sourceY;
	signed char formationPositionX;
	signed char formationPositionY;
	short __unused_24;
	short waitTicks;
	unsigned char actionState;
	unsigned char progressOnTile; // 29
	short routingPathId;
	short routingPathCurrentTile;
	short routingPathLength;
	unsigned char inBuildingWaitTicks; // 30
	unsigned char isOnRoad;
	short maxRoamLength;
	short roamLength;
	unsigned char roamChooseDestination;
	unsigned char roamRandomCounter;
	signed char roamTurnDirection;
	signed char roamTicksUntilNextTurn;
	short crossCountryX; // 3a - position = 15 * x + offset on tile
	short crossCountryY; // 3c - position = 15 * y + offset on tile
	short ccDestinationX; // 3e
	short ccDestinationY; // 40
	short ccDeltaX; // 42
	short ccDeltaY; // 44
	short ccDeltaXY; // 46
	unsigned char ccDirection; // 48: 1 = x, 2 = y
	unsigned char speedMultiplier;
	short buildingId;
	short immigrantBuildingId;
	short destinationBuildingId;
	short formationId; //50
	unsigned char indexInFormation; //52
	unsigned char formationAtRest; //53
	unsigned char migrantNumPeople;
	unsigned char isGhost; // 55
	unsigned char minMaxSeen;
	unsigned char __unused_57;
	short inFrontWalkerId;
	unsigned char attackGraphicOffset;
	unsigned char waitTicksMissile;
	signed char xOffsetCart; // 5c
	signed char yOffsetCart; // 5d
	unsigned char empireCityId; // 5e
	unsigned char traderAmountBought;
	short name; // 60
	unsigned char terrainUsage;
	unsigned char loadsSoldOrCarrying;
	unsigned char isBoat; // 64
	unsigned char heightAdjustedTicks; // 65
	unsigned char currentHeight;
	unsigned char targetHeight;
	unsigned char collectingItemId; // NOT a resource ID for cartpushers! IS a resource ID for warehousemen
	unsigned char tradeShipFailedDockAttempts;
	unsigned char phraseSequenceExact;
	signed char phraseId;
	unsigned char phraseSequenceCity;
	unsigned char traderId;
	unsigned char waitTicksNextTarget;
	unsigned char __unused_6f;
	short targetWalkerId; // 70
	short targetedByWalkerId; // 72
	unsigned short createdSequence;
	unsigned short targetWalkerCreatedSequence;
	unsigned char numPreviousWalkersOnSameTile;
	unsigned char numAttackers;
	short attackerId1;
	short attackerId2;
	short opponentId; // 7e
} Data_Walkers[MAX_FIGURES];

extern struct Data_Figure_Trader {
	int totalBought;
	int totalSold;
	unsigned char boughtResources[16];
	unsigned char soldResources[16];
	int moneyBoughtResources;
	int moneySoldResources;
} Data_Figure_Traders[MAX_TRADERS];

extern struct _Data_Figure_Extra {
	int highestFigureIdEver;
	int createdSequence;
	int nextTraderId;
} Data_Figure_Extra;

#endif
