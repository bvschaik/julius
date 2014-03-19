#ifndef DATA_BUILDING_H
#define DATA_BUILDING_H

#define MAX_BUILDINGS 2000
#define MAX_STORAGES 200

enum {
	Building_Menu_Farms = 2,
	Building_Menu_RawMaterials = 3,
	Building_Menu_Workshops = 4,
	Building_Road = 5,
	Building_Wall = 6,
	Building_ReservoirSimon = 7,
	Building_Aqueduct = 8,
	Building_ClearLand = 9,
	Building_HouseVacantLot = 10,
	Building_HouseSmallTent = 10,
	Building_HouseLargeTent = 11,
	Building_HouseSmallShack = 12,
	Building_HouseLargeShack = 13,
	Building_HouseSmallHovel = 14,
	Building_HouseLargeHovel = 15,
	Building_HouseSmallCasa = 16,
	Building_HouseLargeCasa = 17,
	Building_HouseSmallInsula = 18,
	Building_HouseMediumInsula = 19,
	Building_HouseLargeInsula = 20,
	Building_HouseGrandInsula = 21,
	Building_HouseSmallVilla = 22,
	Building_HouseMediumVilla = 23,
	Building_HouseLargeVilla = 24,
	Building_HouseGrandVilla = 25,
	Building_HouseSmallPalace = 26,
	Building_HouseMediumPalace = 27,
	Building_HouseLargePalace = 28,
	Building_HouseLuxuryPalace = 29,
	Building_Amphitheater = 30,
	Building_Theater = 31,
	Building_Hippodrome = 32,
	Building_Colosseum = 33,
	Building_GladiatorSchool = 34,
	Building_LionHouse = 35,
	Building_ActorColony = 36,
	Building_ChariotMaker = 37,
	Building_Plaza = 38,
	Building_Gardens = 39,
	Building_FortLegionaries = 40,
	Building_SmallStatue = 41,
	Building_MediumStatue = 42,
	Building_LargeStatue = 43,
	Building_FortJavelin = 44,
	Building_FortMounted = 45,
	Building_Doctor = 46,
	Building_Hospital = 47,
	Building_Bathhouse = 48,
	Building_Barber = 49,
	Building_DistributionCenter_Unused = 50,
	Building_School = 51,
	Building_Academy = 52,
	Building_Library = 53,
	Building_FortGround = 54,
	Building_Prefecture = 55,
	Building_TriumphalArch = 56,
	Building_FortGround__ = 57,
	Building_Gatehouse = 58,
	Building_Tower = 59,
	Building_SmallTempleCeres = 60,
	Building_SmallTempleNeptune = 61,
	Building_SmallTempleMercury = 62,
	Building_SmallTempleMars = 63,
	Building_SmallTempleVenus = 64,
	Building_LargeTempleCeres = 65,
	Building_LargeTempleNeptune = 66,
	Building_LargeTempleMercury = 67,
	Building_LargeTempleMars = 68,
	Building_LargeTempleVenus = 69,
	Building_Market = 70,
	Building_Granary = 71,
	Building_Warehouse = 72,
	Building_WarehouseSpace = 73,
	Building_Shipyard = 74,
	Building_Dock = 75,
	Building_Wharf = 76,
	Building_GovernorsHouse = 77,
	Building_GovernorsVilla = 78,
	Building_GovernorsPalace = 79,
	Building_MissionPost = 80,
	Building_EngineersPost = 81,
	Building_LowBridge = 82,
	Building_ShipBridge = 83,
	Building_Senate = 84,
	Building_SenateUpgraded = 85,
	Building_Forum = 86,
	Building_ForumUpgraded = 87,
	Building_NativeHut = 88,
	Building_NativeMeeting = 89,
	Building_Reservoir = 90,
	Building_Fountain = 91,
	Building_Well = 92,
	Building_NativeCrops = 93,
	Building_MilitaryAcademy = 94,
	Building_Barracks = 95,
	Building_Menu_SmallTemples = 96,
	Building_Menu_LargeTemples = 97,
	Building_Oracle = 98,
	Building_BurningRuin = 99,
	Building_WheatFarm = 100,
	Building_VegetableFarm = 101,
	Building_FruitFarm = 102,
	Building_OliveFarm = 103,
	Building_VinesFarm = 104,
	Building_PigFarm = 105,
	Building_MarbleQuarry = 106,
	Building_IronMine = 107,
	Building_TimberYard = 108,
	Building_ClayPit = 109,
	Building_WineWorkshop = 110,
	Building_OilWorkshop = 111,
	Building_WeaponsWorkshop = 112,
	Building_FurnitureWorkshop = 113,
	Building_PotteryWorkshop = 114
};

extern struct Data_Building {
	char inUse;
	char __unknown_01;
	char __unknown_02;
	char size;
	char houseIsMerged;
	char houseSize;
	char x;
	char y;
	short gridOffset;
	short type;
	union {
		short houseLevel;
		short warehouseResourceId;
		short orientation;
		short fortWalkerType;
	} subtype;
	char __unknown_0e;
	char __unknown_0f;
	short placedSequence;
	short housesCovered;
	short percentageWorkers;
	short housePopulation;
	short housePopulationRoom;
	short __unknown_1a;
	short houseMaxPopulationSeen;
	char __unknown_1e;
	char __unknown_1f;
	char __unknown_20;
	char __unknown_21;
	short walkerId;
	char __unknown_24;
	char __unknown_25;
	short immigrantWalkerId;
	char __unknown_28;
	char __unknown_29;
	char __unknown_2a;
	char __unknown_2b;
	char __unknown_2c;
	char hasWaterAccess;
	char __unknown_2e;
	char __unknown_2f;
	short prevPartBuildingId;
	short nextPartBuildingId;
	short rawMaterialsStored;
	char __unused_36;
	char houseHasWellAccess;
	short numWorkers;
	char laborCategory;
	char outputResourceId;
	char __unknown_3c;
	char __unknown_3d;
	short damageRisk;
	short fireRisk;
	char __unknown_42;
	char __unknown_43;
	char __unknown_44;
	char __unknown_45;
	char houseTaxCoverage;
	char __unknown_47;
	short formationId;
	union {
		struct {
			char __unknown_4a;
			char __unknown_4b;
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
			char __unknown_58;
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
			char __unknown_65; // dock number of ships
			char __unknown_66;
			char __unknown_67;
			char __unknown_68;
			char __unknown_69;
			char __unknown_6a;
			char __unknown_6b; // farm cursed
			char __unknown_6c;
			char __unknown_6d;
			char __unknown_6e;
			char __unknown_6f;
			char __unknown_70;
			char __unknown_71;
			short boatWalkerId; // 72 + 73
			//char __unknown_73;
		} other;
		struct {
			short unknown;
			short food[4];
			short wine;
			short oil;
			short furniture;
			short pottery;
			short potteryDemand;
			short furnitureDemand;
			short oilDemand;
			short wineDemand;
		} market;
		struct {
			short unknown;
			short spaceLeft;
		} granary;
		struct {
			short progress; //4a+b
			char __padding[27];
			char blessingLeft; //67
			char __pad68;
			char hasFullResource; //69
			char __pad6a;
			char curseWeeksLeft; //6b
		} industry;
		struct {
			char __padding[26];
			char numShows; // 64
			char days1; // also: dock number of ships
			char days2;
			char show;
		} entertainment;
		struct {
			union {
				short all[8];
				struct {
					short wheat;
					short fruit; // TODO order?
					short vegetables;
					short meat;
					short wine;
					short oil;
					short furniture;
					short pottery;
				} one;
			} inventory;
			char theater; //5a
			char amphitheaterActor;
			char amphitheaterGladiator;
			char colosseumGladiator;
			char colosseumLion;
			char hippodrome;
			char school; //60
			char library;
			char academy;
			char barber;
			char clinic;
			char bathhouse;
			char hospital;
			char templeCeres;
			char templeNeptune;//68
			char templeMercury;
			char templeMars;
			char templeVenus;
			char noSpaceToExpand;
			char numFoods;
			char entertainment;
			char education;
			char health; //70
			char numGods;
			char devolveDelay;
			char evolveTextId;
		} house;
	} data;
	int taxIncomeOrStorage;
	char __unknown_78;
	char ruinHasPlague;
	char desirability;
	char isDeleted;
	char isAdjacentToWater;
	unsigned char storageId;
	union {
		char houseHappiness;
		char nativeRisk;
	} sentiment;
	char showOnProblemOverlay;
} Data_Buildings[MAX_BUILDINGS];

extern struct Data_Building_Storage {
	int startUnused;
	short buildingId;
	char inUse;
	char emptyAll;
	char resourceState[24];
} Data_Building_Storages[MAX_STORAGES];

extern struct Data_Buildings_Extra {
	int highestBuildingIdSeen;
	int placedSequence;
} Data_Buildings_Extra;

#endif
