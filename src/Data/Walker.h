#ifndef DATA_WALKER_H
#define DATA_WALKER_H

#define MAX_WALKERS 1000
#define MAX_TRADERS 100

enum {
	Walker_None = 0,
	Walker_Immigrant = 1,
	Walker_Emigrant = 2,
	Walker_Homeless = 3,
	Walker_CartPusher = 4,
	Walker_LaborSeeker = 5,
	Walker_Explosion = 6,
	Walker_TaxCollector = 7,
	Walker_Engineer = 8,
	Walker_Warehouseman = 9,
	Walker_Prefect = 10,
	Walker_FortJavelin = 11,
	Walker_FortMounted = 12,
	Walker_FortLegionary = 13,
	Walker_FortStandard = 14,
	Walker_Actor = 15,
	Walker_Gladiator = 16,
	Walker_LionTamer = 17,
	Walker_Charioteer = 18,
	Walker_TradeCaravan = 19,
	Walker_TradeShip = 20,
	Walker_TradeCaravanDonkey = 21,
	Walker_Protester = 22,
	Walker_Criminal = 23,
	Walker_Rioter = 24,
	Walker_FishingBoat = 25,
	Walker_MarketTrader = 26,
	Walker_Priest = 27,
	Walker_SchoolChild = 28,
	Walker_Teacher = 29,
	Walker_Librarian = 30,
	Walker_Barber = 31,
	Walker_BathhouseWorker = 32,
	Walker_Doctor = 33,
	Walker_Surgeon = 34,
	Walker_35 = 35,
	Walker_MapFlag = 36,
	Walker_Flotsam = 37,
	Walker_Dockman = 38,
	Walker_MarketBuyer = 39,
	Walker_Patrician = 40,
	Walker_IndigenousNative = 41,
	Walker_TowerSentry = 42,
	Walker_Enemy43_Spear = 43,
	Walker_Enemy44_Sword = 44,
	Walker_Enemy45_Sword = 45,
	Walker_Enemy46_Camel = 46,
	Walker_Enemy47_Elephant = 47,
	Walker_Enemy48_Chariot = 48,
	Walker_Enemy49_FastSword = 49,
	Walker_Enemy50_Sword = 50,
	Walker_Enemy51_Spear = 51,
	Walker_Enemy52_MountedArcher = 52,
	Walker_Enemy53_Axe = 53,
	Walker_Enemy54_Gladiator = 54,
	Walker_Enemy55_Javelin = 55,
	Walker_Enemy56_Mounted = 56,
	Walker_EnemyCaesarLegionary = 57,
	Walker_NativeTrader = 58,
	Walker_Arrow = 59,
	Walker_Javelin = 60,
	Walker_Bolt = 61,
	Walker_Ballista = 62,
	Walker_Creature = 63,
	Walker_Missionary = 64,
	Walker_FishGulls = 65,
	Walker_DeliveryBoy = 66,
	Walker_Shipwreck = 67,
	Walker_Sheep = 68,
	Walker_Wolf = 69,
	Walker_Zebra = 70,
	Walker_Spear = 71,
	Walker_HippodromeMiniHorses = 72
};

#define WalkerIsEnemyOrNative(t) ((t) >= Walker_IndigenousNative && (t) <= Walker_NativeTrader)
#define WalkerIsEnemy(t) ((t) >= Walker_Enemy43_Spear && (t) <= Walker_EnemyCaesarLegionary)
#define WalkerIsLegion(t) ((t) >= Walker_FortJavelin && (t) <= Walker_FortLegionary)
#define WalkerIsHerd(t) ((t) >= Walker_Sheep && (t) <= Walker_Zebra)

#define WalkerIsDead(w) (Data_Walkers[w].state != WalkerState_Alive || Data_Walkers[w].actionState == WalkerActionState_149_Corpse)

enum {
	WalkerState_Alive = 1,
	WalkerState_Dead = 2
};

enum {
	WalkerActionState_1_ImmigrantCreated = 1,
	WalkerActionState_2_ImmigrantArriving = 2,
	WalkerActionState_3_ImmigrantEnteringHouse = 3,
	WalkerActionState_4_EmigrantCreated = 4,
	WalkerActionState_5_EmigrantExitingHouse = 5,
	WalkerActionState_6_EmigrantLeaving = 6,
	WalkerActionState_7_HomelessCreated = 7,
	WalkerActionState_8_HomelessGoingToHouse = 8,
	WalkerActionState_9_HomelessEnteringHouse = 9,
	WalkerActionState_10_HomelessLeaving = 10,
	WalkerActionState_20_CartpusherInitial = 20,
	WalkerActionState_21_CartpusherDeliveringToWarehouse = 21,
	WalkerActionState_22_CartpusherDeliveringToGranary = 22,
	WalkerActionState_23_CartpusherDeliveringToWorkshop = 23,
	WalkerActionState_24_CartpusherAtWarehouse = 24,
	WalkerActionState_25_CartpusherAtGranary = 25,
	WalkerActionState_26_CartpusherAtWorkshop = 26,
	WalkerActionState_27_CartpusherReturning = 27,
	WalkerActionState_40_TaxCollectorCreated = 40,
	WalkerActionState_41_TaxCollectorEnteringExiting = 41,
	WalkerActionState_42_TaxCollectorRoaming = 42,
	WalkerActionState_43_TaxCollectorReturning = 43,
	WalkerActionState_50_WarehousemanCreated = 50,
	WalkerActionState_51_WarehousemanDeliveringResource = 51,
	WalkerActionState_52_WarehousemanAtDeliveryBuilding = 52,
	WalkerActionState_53_WarehousemanReturningEmpty = 53,
	WalkerActionState_54_WarehousemanGettingFood = 54,
	WalkerActionState_55_WarehousemanAtGranaryGettingFood = 55,
	WalkerActionState_56_WarehousemanReturningWithFood = 56,
	WalkerActionState_57_WarehousemanGettingResource = 57,
	WalkerActionState_58_WarehousemanAtWarehouseGettingResource = 58,
	WalkerActionState_59_WarehousemanReturningWithResource = 59,
	WalkerActionState_60_EngineerCreated = 60,
	WalkerActionState_61_EngineerEnteringExiting = 61,
	WalkerActionState_62_EngineerRoaming = 62,
	WalkerActionState_63_EngineerReturning = 63,
	WalkerActionState_70_PrefectCreated = 70,
	WalkerActionState_71_PrefectEnteringExiting = 71,
	WalkerActionState_72_PrefectRoaming = 72,
	WalkerActionState_73_PrefectReturning = 73,
	WalkerActionState_74_PrefectGoingToFire = 74,
	WalkerActionState_75_PrefectAtFire = 75,
	WalkerActionState_76_PrefectGoingToEnemy = 76,
	WalkerActionState_77_PrefectAtEnemy = 77,
	WalkerActionState_80_SoldierAtRest = 80,
	WalkerActionState_81_SoldierGoingToFort = 81,
	WalkerActionState_82_SoldierReturningToBarracks = 82,
	WalkerActionState_83_SoldierGoingToStandard = 83,
	WalkerActionState_84_SoldierAtStandard = 84,
	WalkerActionState_85_SoldierGoingToMilitaryAcademy = 85,
	WalkerActionState_86_SoldierMoppingUp = 86,
	WalkerActionState_87_SoldierGoingToDistantBattle = 87,
	WalkerActionState_88_SoldierReturningFromDistantBattle = 88,
	WalkerActionState_89_SoldierAtDistantBattle = 89,
	WalkerActionState_90_EntertainerAtSchoolCreated = 90,
	WalkerActionState_91_EntertainerExitingSchool = 91,
	WalkerActionState_92_EntertainerGoingToVenue = 92,
	WalkerActionState_94_EntertainerRoaming = 94,
	WalkerActionState_95_EntertainerReturning = 95,
	WalkerActionState_100_TradeCaravanCreated = 100,
	WalkerActionState_101_TradeCaravanArriving = 101,
	WalkerActionState_102_TradeCaravanTrading = 102,
	WalkerActionState_103_TradeCaravanLeaving = 103,
	WalkerActionState_110_TradeShipCreated = 110,
	WalkerActionState_111_TradeShipGoingToDock = 111,
	WalkerActionState_112_TradeShipMoored = 112,
	WalkerActionState_113_TradeShipGoingToDockQueue = 113,
	WalkerActionState_114_TradeShipAnchored = 114,
	WalkerActionState_115_TradeShipLeaving = 115,
	WalkerActionState_120_RioterCreated = 120,
	WalkerActionState_121_RioterMoving = 121,
	WalkerActionState_125_Roaming = 125,
	WalkerActionState_126_RoamerReturning = 126,
	WalkerActionState_128_FlotsamCreated = 128,
	WalkerActionState_129_FlotsamFloating = 129,
	WalkerActionState_130_FlotsamLeftMap = 130,
	WalkerActionState_132_DockerIdling = 132,
	WalkerActionState_133_DockerImportQueue = 133,
	WalkerActionState_134_DockerExportQueue = 134,
	WalkerActionState_135_DockerImportGoingToWarehouse = 135,
	WalkerActionState_136_DockerExportGoingToWarehouse = 136,
	WalkerActionState_137_DockerExportReturning = 137,
	WalkerActionState_138_DockerImportReturning = 138,
	WalkerActionState_139_DockerImportAtWarehouse = 139,
	WalkerActionState_140_DockerExportAtWarehouse = 140,
	WalkerActionState_145_MarketBuyerGoingToStorage = 145,
	WalkerActionState_146_MarketBuyerReturning = 146,
	WalkerActionState_148_Fleeing = 148,
	WalkerActionState_149_Corpse = 149,
	WalkerActionState_150_Attack = 150,
	WalkerActionState_151_EnemyInitial = 151,
	WalkerActionState_152_EnemyWaiting = 152,
	WalkerActionState_153_EnemyMarching = 153,
	WalkerActionState_154_EnemyFighting = 154,
	WalkerActionState_156_NativeGoingToMeetingCenter = 156,
	WalkerActionState_157_NativeReturningFromMeetingCenter = 157,
	WalkerActionState_158_NativeCreated = 158,
	WalkerActionState_159_NativeAttacking = 159,
	WalkerActionState_160_NativeTraderGoingToWarehouse = 160,
	WalkerActionState_161_NativeTraderReturning = 161,
	WalkerActionState_162_NativeTraderCreated = 162,
	WalkerActionState_163_NativeTraderAtWarehouse = 163,
	WalkerActionState_170_TowerSentryAtRest = 170,
	WalkerActionState_171_TowerSentryPatrolling = 171,
	WalkerActionState_172_TowerSentryFiring = 172,
	WalkerActionState_173_TowerSentryReturning = 173,
	WalkerActionState_174_TowerSentryGoingToTower = 174,
	WalkerActionState_180_BallistaCreated = 180,
	WalkerActionState_181_BallistaFiring = 181,
	WalkerActionState_190_FishingBoatCreated = 190,
	WalkerActionState_191_FishingBoatGoingToFish = 191,
	WalkerActionState_192_FishingBoatFishing = 192,
	WalkerActionState_193_FishingBoatSailingToWharf = 193,
	WalkerActionState_194_FishingBoatAtWharf = 194,
	WalkerActionState_195_FishingBoatReturningWithFish = 195,
	WalkerActionState_196_HerdAnimalAtRest = 196,
	WalkerActionState_197_HerdAnimalMoving = 197,
	WalkerActionState_199_WolfAttacking = 199,
	WalkerActionState_200_HippodromeMiniHorseCreated = 200,
	WalkerActionState_201_HippodromeMiniHorseRacing = 201,
	WalkerActionState_202_HippodromeMiniHorseDone = 202,
};

enum {
	WalkerTerrainUsage_Roads = 1,
	WalkerTerrainUsage_Enemy = 2,
	WalkerTerrainUsage_AnyLand = 3,
	WalkerTerrainUsage_Walls = 4,
	WalkerTerrainUsage_Animal = 5,
};

extern struct Data_Walker_NameSequence {
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
} Data_Walker_NameSequence;

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
	unsigned char heightFromGround; // 65
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
} Data_Walkers[MAX_WALKERS];

extern struct Data_Walker_Trader {
	int totalBought;
	int totalSold;
	unsigned char boughtResources[16];
	unsigned char soldResources[16];
	int moneyBoughtResources;
	int moneySoldResources;
} Data_Walker_Traders[MAX_TRADERS];

extern struct Data_Walker_Extra {
	int highestWalkerIdEver;
	int createdSequence;
	int nextTraderId;
} Data_Walker_Extra;

#endif
